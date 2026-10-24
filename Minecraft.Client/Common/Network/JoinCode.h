#pragma once
#include <cstdint>
#include <string>
namespace JoinCode {
inline unsigned crc(const unsigned char*bytes,int count){unsigned value=0xffff;for(int i=0;i<count;++i){value^=unsigned(bytes[i])<<8;for(int b=0;b<8;++b)value=(value&0x8000)?((value<<1)^0x1021):(value<<1);value&=0xffff;}return value;}
inline bool address(const std::wstring&host,int port,unsigned char out[6]){
 if(port<1||port>65535)return false;size_t p=0;
 for(int i=0;i<4;++i){int n=0,digits=0;while(p<host.size()&&host[p]>=L'0'&&host[p]<=L'9'){n=n*10+host[p++]-L'0';if(++digits>3||n>255)return false;}if(!digits)return false;out[i]=static_cast<unsigned char>(n);if(i<3&&(p==host.size()||host[p++]!=L'.'))return false;}
 if(p!=host.size()||out[0]==0||out[0]>=224)return false;out[4]=static_cast<unsigned char>(port>>8);out[5]=static_cast<unsigned char>(port);return true;
}
inline const wchar_t*alphabet(){return L"0123456789ABCDEFGHJKMNPQRSTVWXYZ";}
inline std::wstring encode(const std::wstring&host,int port){
 unsigned char bytes[6];if(!address(host,port,bytes))return L"";std::uint64_t value=0;for(auto b:bytes)value=(value<<8)|b;value=(value<<16)|crc(bytes,6);
 std::wstring raw(13,L'0');for(int i=12;i>=0;--i){raw[i]=alphabet()[value&31];value>>=5;}return L"LCE1-"+raw.substr(0,4)+L"-"+raw.substr(4,4)+L"-"+raw.substr(8);
}
inline bool looksLikeCode(const std::wstring&s){return s.size()>=5&&(s[0]==L'L'||s[0]==L'l')&&(s[1]==L'C'||s[1]==L'c')&&(s[2]==L'E'||s[2]==L'e')&&s[3]==L'1'&&s[4]==L'-';}
inline bool decode(const std::wstring&code,std::wstring&host,int&port){
 if(!looksLikeCode(code))return false;std::wstring raw;for(size_t i=5;i<code.size();++i){wchar_t c=code[i];if(c==L'-'||c==L' ')continue;if(c>=L'a'&&c<=L'z')c-=32;if(c==L'O')c=L'0';if(c==L'I'||c==L'L')c=L'1';raw+=c;}
 if(raw.size()!=13)return false;std::uint64_t value=0;for(size_t i=0;i<raw.size();++i){auto p=std::wstring(alphabet()).find(raw[i]);if(p==std::wstring::npos||(i==0&&p>15))return false;value=(value<<5)|p;}
 unsigned sum=unsigned(value&65535);value>>=16;unsigned char bytes[6];for(int i=5;i>=0;--i){bytes[i]=static_cast<unsigned char>(value&255);value>>=8;}if(sum!=crc(bytes,6))return false;
 int decodedPort=(int(bytes[4])<<8)|bytes[5];std::wstring decodedHost=std::to_wstring(bytes[0])+L"."+std::to_wstring(bytes[1])+L"."+std::to_wstring(bytes[2])+L"."+std::to_wstring(bytes[3]);unsigned char checked[6];if(!address(decodedHost,decodedPort,checked))return false;host=decodedHost;port=decodedPort;return true;
}
}
