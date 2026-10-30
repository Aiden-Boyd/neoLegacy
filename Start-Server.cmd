@echo off
cd /d "%~dp0"
Minecraft.Server.exe -port 25565 -bind 0.0.0.0 -name DedicatedServer
set "serverExit=%ERRORLEVEL%"
echo Server exited with code %serverExit%.
pause
exit /b %serverExit%
