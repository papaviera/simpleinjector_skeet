#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <filesystem>

static HANDLE GetProcessByName(const std::wstring& name)
{
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W process;
    ZeroMemory(&process, sizeof(process));
    process.dwSize = sizeof(process);

    if (Process32FirstW(snapshot, &process))
    {
        do
        {
            if (std::wstring(process.szExeFile) == name)
            {
                pid = process.th32ProcessID;
                break;
            }
        } while (Process32NextW(snapshot, &process));
    }

    CloseHandle(snapshot);

    if (pid != 0)
    {
        return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    }
    return INVALID_HANDLE_VALUE;
}

int main()
{
    std::cout << "waiting for cs2...\n";

    HANDLE hProc = INVALID_HANDLE_VALUE;
    auto path = std::filesystem::current_path();
    auto pathStr = path.wstring() + L"\\skeet_damage.dll";

    while (hProc == INVALID_HANDLE_VALUE) {
        hProc = GetProcessByName(L"cs2.exe");
        Sleep(100);
    }

    auto arg = VirtualAllocEx(hProc, nullptr, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    WriteProcessMemory(hProc, arg, pathStr.c_str(), (pathStr.length() + 1) * sizeof(wchar_t), nullptr);

    auto hThread = CreateRemoteThread(
        hProc, 
        nullptr, 
        0,
        (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW"),
        arg, 
        0, 
        nullptr
    );


    WaitForSingleObject(hThread, INFINITE);

    CloseHandle(hThread);
    CloseHandle(hProc);

    return 0;
}