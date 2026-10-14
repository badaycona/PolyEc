#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <vector>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR arguments, int) {
    std::vector<wchar_t> path(32768);
    DWORD count = GetModuleFileNameW(nullptr, path.data(), DWORD(path.size()));
    if (!count || count >= path.size()) return 1;
    std::wstring directory(path.data(), count);
    directory.resize(directory.find_last_of(L"\\/"));
    std::wstring runtime = directory + L"\\PolyEc-runtime.exe";
    std::wstring pack = directory + L"\\PolyEc-runtime.pck";
    std::wstring command = L"\"" + runtime + L"\" --main-pack \"" + pack + L"\" ";
    command += arguments;
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION child{};
    if (!CreateProcessW(runtime.c_str(), command.data(), nullptr, nullptr, FALSE, 0,
            nullptr, directory.c_str(), &startup, &child)) {
        MessageBoxW(nullptr, L"Không mở được PolyEc. Hãy giải nén toàn bộ thư mục trước khi chạy.",
                    L"PolyEc", MB_OK | MB_ICONERROR);
        return 1;
    }
    CloseHandle(child.hThread);
    WaitForSingleObject(child.hProcess, INFINITE);
    DWORD exit_code = 1;
    GetExitCodeProcess(child.hProcess, &exit_code);
    CloseHandle(child.hProcess);
    return int(exit_code);
}
