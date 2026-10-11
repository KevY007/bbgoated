// See bbgoated.h.
#include "host/bbgoated.h"

#include "log.h"

#if defined(_WIN32)
#include <windows.h>
#include <tlhelp32.h>
#endif

void bbgoated_warn_other_instances() {
#if defined(_WIN32)
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return;
    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);
    const DWORD self = GetCurrentProcessId();
    for (BOOL ok = Process32FirstW(snap, &pe); ok; ok = Process32NextW(snap, &pe)) {
        if (pe.th32ProcessID != self && _wcsicmp(pe.szExeFile, L"bbhost.exe") == 0)
            host_log("bbgoated: bbhost.exe is running (pid %lu) - close it before playing online with the same account",
                     static_cast<unsigned long>(pe.th32ProcessID));
    }
    CloseHandle(snap);
#endif
}
