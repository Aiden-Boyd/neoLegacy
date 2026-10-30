/*
MIT License

Copyright (c) 2026 Patoke

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "STO_SaveGame.h"
#include "STO_Main.h"
#include <cstdio>
#include <cstring>

namespace
{
bool EnsureSaveDirectory(const char* path)
{
    if (CreateDirectoryA(path, nullptr)) return true;
    const DWORD error = GetLastError();
    const DWORD attributes = GetFileAttributesA(path);
    if (error == ERROR_ALREADY_EXISTS && attributes != INVALID_FILE_ATTRIBUTES &&
        (attributes & FILE_ATTRIBUTE_DIRECTORY)) return true;
    fprintf(stderr, "[SAVE] Cannot create directory '%s': error=%lu\n", path, error);
    return false;
}

bool IsWorldFile(const WIN32_FIND_DATAA& file)
{
    if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) return false;
    const char* extension = strrchr(file.cFileName, '.');
    return extension && _stricmp(extension, ".ms") == 0;
}

bool WriteSaveFile(const char* path, const void* data, DWORD size)
{
    char temporary[640];
    if (_snprintf_s(temporary, sizeof(temporary), _TRUNCATE, "%s.tmp", path) < 0) return false;
    HANDLE file = CreateFileA(temporary, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "[SAVE] Cannot open temporary '%s': error=%lu\n", temporary, GetLastError());
        return false;
    }
    DWORD written = 0;
    bool success = WriteFile(file, data, size, &written, nullptr) != FALSE && written == size;
    DWORD error = success ? ERROR_SUCCESS : GetLastError();
    if (success && !FlushFileBuffers(file)) { success = false; error = GetLastError(); }
    if (!CloseHandle(file)) { success = false; error = GetLastError(); }
    if (success && !MoveFileExA(temporary, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    { success = false; error = GetLastError(); }
    if (!success)
    {
        fprintf(stderr, "[SAVE] Commit FAILED '%s': error=%lu bytes=%lu/%lu; previous file retained\n", path, error, written, size);
        DeleteFileA(temporary);
    }
    else fprintf(stderr, "[SAVE] Committed '%s': bytes=%lu\n", path, size);
    fflush(stderr);
    return success;
}
}


// @Patoke add
char CSaveGame::szPNGHeader[] = "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A";

CSaveGame::CSaveGame()
{
    m_pSaveData = nullptr;
    m_uiSaveSize = 0;
    m_bIsSafeDisabled = false;

    ZeroMemory(m_szSaveUniqueName, sizeof(m_szSaveUniqueName));
    ZeroMemory(m_szSaveTitle, sizeof(m_szSaveTitle));

    m_pSaveDetails = nullptr;
    m_bHasSaveDetails = false;

    // @Patoke add
    this->m_pbThumbnailData = nullptr;
    this->m_uiThumbnailSize = 0;
    this->m_pbImageData = nullptr;
    this->m_uiImageSize = 0;
}

void CSaveGame::SetSaveDisabled(bool bDisable)
{
    m_bIsSafeDisabled = bDisable;
}

bool CSaveGame::GetSaveDisabled(void)
{
    return m_bIsSafeDisabled;
}

void CSaveGame::ResetSaveData()
{
    free(m_pSaveData);
    m_pSaveData = nullptr;
    m_uiSaveSize = 0;
}

C4JStorage::ESaveGameState CSaveGame::GetSavesInfo(int iPad, int (*Func)(LPVOID lpParam, SAVE_DETAILS *pSaveDetails, const bool), LPVOID lpParam,
                                                   char *pszSavePackName)
{
    WIN32_FIND_DATAA findFileData;
    WIN32_FILE_ATTRIBUTE_DATA fileInfoBuffer = {};

    if (!m_pSaveDetails)
    {
        m_pSaveDetails = new SAVE_DETAILS();
        memset(m_pSaveDetails, 0, sizeof(SAVE_DETAILS));
    }

    delete[] m_pSaveDetails->SaveInfoA;
    m_pSaveDetails->SaveInfoA = nullptr;
    m_pSaveDetails->iSaveC = 0;

    char dirName[256];
    char curDir[256];
    strcpy_s(dirName, ".");
    sprintf(curDir, "%s\\Windows64\\GameHDD\\*", dirName);

    int resultCount = 0;
    HANDLE h = FindFirstFileExA(curDir, FindExInfoStandard, &findFileData, FindExSearchLimitToDirectories, 0, 0);
    if (h == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();
        printf("Error finding save dirs: 0x%08lx\n", error);
    }
    else
    {
        do
        {
            if ((findFileData.dwFileAttributes & 0x10) != 0)
            {
                resultCount++;
            }
        } while (FindNextFileA(h, &findFileData));
        FindClose(h);
    }

    if (resultCount > 0)
    {
        m_pSaveDetails->SaveInfoA = new SAVE_INFO[resultCount];
        memset(m_pSaveDetails->SaveInfoA, 0, sizeof(SAVE_INFO) * resultCount);

        m_pSaveDetails->iSaveC = 0;
        int i = 0;
        HANDLE fi = FindFirstFileExA(curDir, FindExInfoStandard, &findFileData, FindExSearchLimitToDirectories, 0, 0);
        if (fi != INVALID_HANDLE_VALUE)
        {
            do
            {
                if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 && strcmp(findFileData.cFileName, ".") &&
                    strcmp(findFileData.cFileName, ".."))
                {
                    strcpy_s(m_pSaveDetails->SaveInfoA[i].UTF8SaveFilename, findFileData.cFileName);

                    // @Patoke add: we want to preserve the title name, so we save this in the actual save file name
                    char searchPath[280];
                    sprintf(searchPath, "%s\\Windows64\\GameHDD\\%s\\*", dirName, findFileData.cFileName);

                    WIN32_FIND_DATAA saveFileData = {};
                    HANDLE hSaveFile = FindFirstFileA(searchPath, &saveFileData);

                    char szTitleName[256] = {0};
                    bool foundWorld = false;

                    if (hSaveFile != INVALID_HANDLE_VALUE)
                    {
                        do
                        {
                            // @Patoke todo: we assume the first actual file here to be the save file, ideally we would want to check the extension
                            // too but this is good enough for now
                            if (IsWorldFile(saveFileData))
                            {
                                // remove the file extension
                                std::string decoratedSaveTitle = saveFileData.cFileName;
                                uint64_t extensionDot = decoratedSaveTitle.rfind('.');

                                // populate the save title
                                if (extensionDot != std::string::npos)
                                {
                                    strcpy_s(this->m_szSaveTitle, decoratedSaveTitle.substr(0, extensionDot).c_str());
                                }
                                else
                                {
                                    strcpy_s(this->m_szSaveTitle, decoratedSaveTitle.c_str());
                                }

                                strcpy_s(szTitleName, sizeof(szTitleName), this->m_szSaveTitle);
                                foundWorld = true;
                                break;
                            }
                        } while (FindNextFileA(hSaveFile, &saveFileData));

                        FindClose(hSaveFile);
                    }

                    if (!foundWorld) continue;

                    // set the actual save file name as the title now, before we would use the folder name which is the unique save name
                    strcpy_s(m_pSaveDetails->SaveInfoA[i].UTF8SaveTitle, szTitleName);

                    char fileName[280];
                    sprintf(fileName, "%s\\Windows64\\GameHDD\\%s\\%s", dirName, findFileData.cFileName, saveFileData.cFileName);

                    GetFileAttributesExA(fileName, GetFileExInfoStandard, &fileInfoBuffer);
                    m_pSaveDetails->SaveInfoA[i].metaData.dataSize = fileInfoBuffer.nFileSizeLow;

                    // @Patoke todo: a save can have multiple thumbnails, implement this behaviour
                    char thumbName[280];
                    sprintf(thumbName, "%s\\Windows64\\GameHDD\\%s\\thumbnails\\thumbData.png", dirName, findFileData.cFileName);

                    // don't try loading a thumbnail if the file doesn't exist
                    BOOL res = GetFileAttributesExA(thumbName, GetFileExInfoStandard, &fileInfoBuffer);
                    if (res == 0)
                    {
                        if (m_pSaveDetails->SaveInfoA[i].thumbnailData)
                        {
                            free(m_pSaveDetails->SaveInfoA[i].thumbnailData);
                            m_pSaveDetails->SaveInfoA[i].thumbnailData = nullptr;
                        }
                        m_pSaveDetails->SaveInfoA[i].metaData.thumbnailSize = 0;
                    }
                    else
                    {
                        m_pSaveDetails->SaveInfoA[i].metaData.thumbnailSize = fileInfoBuffer.nFileSizeLow;
                    }

                    m_pSaveDetails->iSaveC++;
                    i++;
                }
            } while (FindNextFileA(fi, &findFileData));
            FindClose(fi);
        }
    }

    m_bHasSaveDetails = true;
    if (Func)
    {
        Func(lpParam, m_pSaveDetails, true);
    }

    return C4JStorage::ESaveGame_Idle;
}

PSAVE_DETAILS CSaveGame::ReturnSavesInfo()
{
    if (m_bHasSaveDetails)
        return m_pSaveDetails;
    else
        return nullptr;
}

void CSaveGame::ClearSavesInfo()
{
    m_bHasSaveDetails = false;
    if (m_pSaveDetails)
    {
        if (m_pSaveDetails->SaveInfoA)
        {
            delete[] m_pSaveDetails->SaveInfoA;
            m_pSaveDetails->SaveInfoA = nullptr;
            m_pSaveDetails->iSaveC = 0;
        }
        delete m_pSaveDetails;
        m_pSaveDetails = 0;
    }
}

// @Patoke add
C4JStorage::ESaveGameState CSaveGame::LoadSaveDataThumbnail(PSAVE_INFO pSaveInfo,
                                                            int (*Func)(LPVOID lpParam, PBYTE pbThumbnail, DWORD dwThumbnailBytes), LPVOID lpParam)
{
    if (pSaveInfo == nullptr)
    {
        return C4JStorage::ESaveGame_Idle;
    }

    DWORD thumbSize = pSaveInfo->metaData.thumbnailSize;
    if (thumbSize > 0 && pSaveInfo->thumbnailData == nullptr)
    {
        char curDir[256];
        GetCurrentDirectoryA(sizeof(curDir), curDir);

        const char *saveName = (const char *)pSaveInfo;

        char thumbPath[512];
        sprintf(thumbPath, "%s/Windows64/GameHDD/%s/thumbnails/thumbData.png", curDir, saveName);

        HANDLE hThumb = CreateFileA(thumbPath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

        if (hThumb != INVALID_HANDLE_VALUE)
        {
            pSaveInfo->thumbnailData = new BYTE[thumbSize];

            DWORD bytesRead = 0;
            BOOL res = ReadFile(hThumb, pSaveInfo->thumbnailData, thumbSize, &bytesRead, 0);

            // If the read fails or is incomplete, clean up to prevent corrupted image data
            if (!res || bytesRead != thumbSize)
            {
                delete[] pSaveInfo->thumbnailData;
                pSaveInfo->thumbnailData = nullptr;
            }

            CloseHandle(hThumb);
        }
    }

    Func(lpParam, pSaveInfo->thumbnailData, pSaveInfo->metaData.thumbnailSize);
    return C4JStorage::ESaveGame_GetSaveThumbnail;
}

C4JStorage::ESaveGameState CSaveGame::LoadSaveData(PSAVE_INFO pSaveInfo, int (*Func)(LPVOID lpParam, const bool, const bool), LPVOID lpParam)
{
    ResetSaveData();
    bool success = false;
    if (pSaveInfo)
    {
        SetSaveUniqueFilename(pSaveInfo->UTF8SaveFilename);
        strncpy_s(m_szSaveTitle, sizeof(m_szSaveTitle), pSaveInfo->UTF8SaveTitle, _TRUNCATE);
        char path[512];
        _snprintf_s(path, sizeof(path), _TRUNCATE, "Windows64\\GameHDD\\%s\\%s.ms", m_szSaveUniqueName, m_szSaveTitle);
        HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        DWORD error = ERROR_SUCCESS;
        if (file != INVALID_HANDLE_VALUE)
        {
            LARGE_INTEGER size = {};
            if (GetFileSizeEx(file, &size) && size.QuadPart > 0 && size.QuadPart <= MAXDWORD)
            {
                void* data = malloc(static_cast<size_t>(size.QuadPart));
                if (data)
                {
                    DWORD read = 0;
                    success = ReadFile(file, data, static_cast<DWORD>(size.QuadPart), &read, nullptr) != FALSE &&
                        read == static_cast<DWORD>(size.QuadPart);
                    error = success ? ERROR_SUCCESS : GetLastError();
                    if (success) { m_pSaveData = data; m_uiSaveSize = read; }
                    else free(data);
                }
                else error = ERROR_NOT_ENOUGH_MEMORY;
            }
            else error = ERROR_INVALID_DATA;
            CloseHandle(file);
        }
        else error = GetLastError();
        fprintf(stderr, "[SAVE] Load success=%d path='%s' size=%u error=%lu\n", success ? 1 : 0, path, m_uiSaveSize, error);
    }
    if (Func) Func(lpParam, false, success);
    fflush(stderr);
    return C4JStorage::ESaveGame_Idle;
}

unsigned int CSaveGame::GetSaveSize()
{
    return m_uiSaveSize;
}

void CSaveGame::GetSaveData(void *pvData, unsigned int *puiBytes)
{
    if (pvData)
    {
        memmove(pvData, m_pSaveData, m_uiSaveSize);
        *puiBytes = m_uiSaveSize;
    }
    else
    {
        *puiBytes = 0;
    }
}

// @Patoke add
bool CSaveGame::GetSaveUniqueNumber(INT *piVal)
{
    if (m_szSaveUniqueName[0] == '\0')
    {
        return 0;
    }

    int year, month, day, hour, minute;
    sscanf(&m_szSaveUniqueName[4], "%02d%02d%02d%02d%02d", &year, &month, &day, &hour, &minute);

    *piVal = 2678400 * year + 86400 * month + 3600 * day + 60 * hour + minute;

    return true;
}

// @Patoke add
bool CSaveGame::GetSaveUniqueFilename(char *pszName)
{
    if (m_szSaveUniqueName[0] == '\0')
    {
        return false;
    }

    memset(pszName, 0, 14);
    for (int i = 0; i < 12; i++)
    {
        pszName[i] = m_szSaveUniqueName[i + 2];
    }

    return true;
}

void CSaveGame::SetSaveTitle(LPCWSTR pwchDefaultSaveName)
{
    CreateSaveUniqueName();
    _snprintf_s(m_szSaveTitle, sizeof(m_szSaveTitle), _TRUNCATE, "%ls", pwchDefaultSaveName ? pwchDefaultSaveName : L"World");
}

PVOID CSaveGame::AllocateSaveData(unsigned int uiBytes)
{
    free(m_pSaveData);

    m_pSaveData = malloc(uiBytes);
    if (m_pSaveData)
    {
        m_uiSaveSize = uiBytes;
    }

    return m_pSaveData;
}

// @Patoke add
void CSaveGame::SetSaveImages(PBYTE pbThumbnail, DWORD dwThumbnailBytes, PBYTE pbImage, DWORD dwImageBytes, PBYTE pbTextData, DWORD dwTextDataBytes)
{
    if (this->m_pbThumbnailData)
    {
        free(this->m_pbThumbnailData);
    }

    this->m_pbImageData = pbImage;
    this->m_uiImageSize = dwImageBytes;

    DWORD dwNewThumbnailBytes = dwThumbnailBytes;
    if (dwTextDataBytes > 0)
    {
        // add extra bytes to the allocation for the text chunk (4 bytes for size, 4 bytes for type, 4 bytes for checksum)
        dwNewThumbnailBytes += dwTextDataBytes + 12;
    }

    // allocate the thumbnail
    this->m_pbThumbnailData = static_cast<PBYTE>(malloc(dwNewThumbnailBytes));
    this->m_uiThumbnailSize = dwNewThumbnailBytes;
    memset(this->m_pbThumbnailData, 0, dwNewThumbnailBytes);

    // copy original thumbnail data to new buffer
    memcpy(this->m_pbThumbnailData, pbThumbnail, dwThumbnailBytes);

    // inject text metadata into the thumbnail if it exists
    if (dwTextDataBytes > 0)
    {
        this->AddTextFieldToPNG(this->m_pbThumbnailData, dwThumbnailBytes, pbTextData, dwTextDataBytes, dwNewThumbnailBytes);
    }
}

// @Patoke add
void CSaveGame::AddTextFieldToPNG(PBYTE pbImageData, DWORD dwImageBytes, PBYTE pbTextData, DWORD dwTextBytes, DWORD dwTotalSizeAllocated)
{
    if (dwImageBytes == 0)
    {
        return;
    }

    for (int j = 0; j < 8; ++j)
    {
        if (CSaveGame::szPNGHeader[j] != reinterpret_cast<char*>(pbImageData)[j])
        {
            return;
        }
    }

    unsigned int offset = 8;
    while (offset < dwImageBytes)
    {
        unsigned int chunkStart = offset;

        unsigned int chunkLength = this->ReverseBytes(*reinterpret_cast<unsigned int *>(&pbImageData[offset]));
        offset += 4;

        unsigned int chunkType = this->ReverseBytes(*reinterpret_cast<unsigned int *>(&pbImageData[offset]));
        offset += 4;

        if (chunkType == 'IEND')
        {
            offset = chunkStart;

            // write the tEXt chunk before the IEND chunk
            *reinterpret_cast<unsigned int *>(&pbImageData[offset]) = this->ReverseBytes(static_cast<unsigned int>(dwTextBytes));
            offset += 4;

            unsigned __int8 *textTypeStart = &pbImageData[offset];
            *reinterpret_cast<unsigned int *>(&pbImageData[offset]) = this->ReverseBytes('tEXt');
            offset += 4;

            memcpy(&pbImageData[offset], pbTextData, dwTextBytes);
            offset += dwTextBytes;

            unsigned int textCrc = InternalStorageManager.CRC(textTypeStart, dwTextBytes + 4);
            *reinterpret_cast<unsigned int *>(&pbImageData[offset]) = this->ReverseBytes(textCrc);
            offset += 4;

            // add a new IEND chunk
            *reinterpret_cast<unsigned int *>(&pbImageData[offset]) = 0;
            offset += 4;

            unsigned __int8 *iendTypeStart = &pbImageData[offset];
            *reinterpret_cast<unsigned int *>(&pbImageData[offset]) = this->ReverseBytes('IEND');
            offset += 4;

            unsigned int iendCrc = InternalStorageManager.CRC(iendTypeStart, 4);
            *reinterpret_cast<unsigned int *>(&pbImageData[offset]) = this->ReverseBytes(iendCrc);
            offset += 4;

            assert("uiCount <= dwTotalSizeAllocated");

            break;
        }
        else
        {
            // not 'IEND' chunk, continue to next chunk
            offset += chunkLength + 4;
        }
    }
}

// @Patoke add
unsigned int CSaveGame::ReverseBytes(unsigned int uiValue)
{
    unsigned int uiReturn = 0;

    uiReturn = (uiValue << 24) | ((uiValue << 8) & 0x00FF0000) | ((uiValue >> 8) & 0x0000FF00) | (uiValue >> 24);

    return uiReturn;
}

C4JStorage::ESaveGameState CSaveGame::SaveSaveData(int (*Func)(LPVOID, const bool), LPVOID lpParam)
{
    bool success = false;
    char directory[128] = {};
    char path[512] = {};
    char title[256] = {};
    // A title is display text, not a filesystem path.
    strncpy_s(title, sizeof(title), m_szSaveTitle, _TRUNCATE);
    for (char* p = title; *p; ++p)
        if (static_cast<unsigned char>(*p) < 32 || strchr("\\/:*?\"<>|", *p)) *p = '_';
    size_t length = strlen(title);
    while (length && (title[length - 1] == ' ' || title[length - 1] == '.')) title[--length] = '\0';
    if (!length) strcpy_s(title, "World");
    const bool validName = m_szSaveUniqueName[0] &&
        strnlen_s(m_szSaveUniqueName, sizeof(m_szSaveUniqueName)) < sizeof(m_szSaveUniqueName) &&
        !strpbrk(m_szSaveUniqueName, "\\/:*?\"<>|") &&
        strcmp(m_szSaveUniqueName, ".") && strcmp(m_szSaveUniqueName, "..");
    if (!m_bIsSafeDisabled && m_pSaveData && m_uiSaveSize && validName &&
        EnsureSaveDirectory("Windows64") && EnsureSaveDirectory("Windows64\\GameHDD"))
    {
        _snprintf_s(directory, sizeof(directory), _TRUNCATE, "Windows64\\GameHDD\\%s", m_szSaveUniqueName);
        if (EnsureSaveDirectory(directory))
        {
            _snprintf_s(path, sizeof(path), _TRUNCATE, "%s\\%s.ms", directory, title);
            success = WriteSaveFile(path, m_pSaveData, m_uiSaveSize);
            if (success && m_pbThumbnailData && m_uiThumbnailSize)
            {
                char thumbnails[160];
                char thumbnail[192];
                _snprintf_s(thumbnails, sizeof(thumbnails), _TRUNCATE, "%s\\thumbnails", directory);
                _snprintf_s(thumbnail, sizeof(thumbnail), _TRUNCATE, "%s\\thumbData.png", thumbnails);
                if (!EnsureSaveDirectory(thumbnails) || !WriteSaveFile(thumbnail, m_pbThumbnailData, m_uiThumbnailSize))
                    fprintf(stderr, "[SAVE] WARNING world saved, thumbnail failed\n");
            }
        }
    }
    else fprintf(stderr, "[SAVE] Rejected save: disabled=%d data=%p size=%u validName=%d\n",
        m_bIsSafeDisabled ? 1 : 0, m_pSaveData, m_uiSaveSize, validName ? 1 : 0);
    fprintf(stderr, "[SAVE] Completion success=%d relativePath='%s'\n", success ? 1 : 0, path);
    fflush(stderr);
    if (Func) Func(lpParam, success);
    return C4JStorage::ESaveGame_Idle;
}

// @Patoke add
C4JStorage::ESaveGameState CSaveGame::DeleteSaveData(PSAVE_INFO pSaveInfo, int (*Func)(LPVOID lpParam, const bool), LPVOID lpParam)
{
    char dirName[256];
    char curDir[256];
    char fileName[280];
    char thumbDir[280];
    char thumbName[280];

    GetCurrentDirectoryA(sizeof(curDir), curDir);

    sprintf(dirName, "%s/Windows64/GameHDD/%s", curDir, pSaveInfo->UTF8SaveFilename);
    sprintf(fileName, "%s/%s.ms", dirName, pSaveInfo->UTF8SaveTitle);
    sprintf(thumbDir, "%s/thumbnails", dirName);
    sprintf(thumbName, "%s/thumbData.png", thumbDir);

    // delete all files under the unique save directory
    char searchPath[280];
    sprintf(searchPath, "%s\\*", dirName);

    WIN32_FIND_DATAA saveFileData;
    HANDLE hSaveFile = FindFirstFileA(searchPath, &saveFileData);

    char szTitleName[256] = {0};

    if (hSaveFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (IsWorldFile(saveFileData))
            {
                sprintf(fileName, "%s/%s", dirName, saveFileData.cFileName);
                DeleteFileA(fileName);
                break;
            }
            // @Patoke todo: add fail case?
        } while (FindNextFileA(hSaveFile, &saveFileData));

        FindClose(hSaveFile);
    }

    // delete thumbnails and directories
    DeleteFileA(thumbName);
    RemoveDirectoryA(thumbDir);
    RemoveDirectoryA(dirName);

    PSAVE_INFO m_pDeleteInfo = pSaveInfo; // only here for consistency with the xbox one assert
    assert((m_pDeleteInfo >= &m_pSaveDetails->SaveInfoA[0]) && (m_pDeleteInfo < &m_pSaveDetails->SaveInfoA[m_pSaveDetails->iSaveC]));

    uint64_t index = pSaveInfo - this->m_pSaveDetails->SaveInfoA;

    // shift all save data by 1 to fill the gap
    for (int j = index; j < this->m_pSaveDetails->iSaveC - 1; ++j)
    {
        this->m_pSaveDetails->SaveInfoA[j] = this->m_pSaveDetails->SaveInfoA[j + 1];
    }

    --this->m_pSaveDetails->iSaveC;

    // not calling this function is what caused the softlock in the original binaries
    if (Func)
    {
        Func(lpParam, true);
    }

    return C4JStorage::ESaveGame_Idle;
}

void CSaveGame::SetSaveUniqueFilename(char *szFilename)
{
    strcpy_s(m_szSaveUniqueName, szFilename);
}

void CSaveGame::CreateSaveUniqueName(void)
{
    _SYSTEMTIME UTCSysTime;
    GetSystemTime(&UTCSysTime);

    sprintf_s(m_szSaveUniqueName, sizeof(m_szSaveUniqueName), "%4d%02d%02d%02d%02d%02d", UTCSysTime.wYear, UTCSysTime.wMonth, UTCSysTime.wDay,
              UTCSysTime.wHour, UTCSysTime.wMinute, UTCSysTime.wSecond);
}
