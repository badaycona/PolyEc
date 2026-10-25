#define NOMINMAX
#include <windows.h>
#include <filesystem>
#include <string>
int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR,int){wchar_t buffer[32768];DWORD n=GetModuleFileNameW(nullptr,buffer,32768);if(!n||n>=32768)return 1;auto root=std::filesystem::path(buffer).parent_path();auto engine=root/L"runtime"/L"Godot.exe";auto game=root/L"game";std::wstring command=L"\""+engine.wstring()+L"\" --path \""+game.wstring()+L"\"";STARTUPINFOW si{};si.cb=sizeof(si);PROCESS_INFORMATION pi{};if(!CreateProcessW(engine.c_str(),command.data(),nullptr,nullptr,FALSE,0,nullptr,root.c_str(),&si,&pi)){MessageBoxW(nullptr,L"Khong mo duoc Godot runtime. Hay giai nen toan bo thu muc PolyEc.",L"PolyEc",MB_OK|MB_ICONERROR);return 1;}CloseHandle(pi.hThread);CloseHandle(pi.hProcess);return 0;}
