#pragma once
#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <algorithm>

/*utility*/
std::wstring to_lowercase(const std::wstring& input) {
    std::wstring output = input;
    std::transform(output.begin(), output.end(), output.begin(), ::towlower);
    return output;
}

std::wstring to_uppercase(const std::wstring& input) {
    std::wstring output = input;
    std::transform(output.begin(), output.end(), output.begin(), ::towupper);
    return output;
}
/*utility*/

DWORD get_procress_id(const std::wstring& processName) {
    DWORD processIds[1024], bytesReturned;
    if (!EnumProcesses(processIds, sizeof(processIds), &bytesReturned)) {
        std::cerr << "Failed to enumerate processes.\n";
        return 0;
    }
    DWORD count = bytesReturned / sizeof(DWORD);
    for (DWORD i = 0; i < count; ++i) {
        HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processIds[i]);
        if (processHandle != NULL) {
            TCHAR buffer[MAX_PATH];
            if (GetModuleBaseName(processHandle, NULL, buffer, MAX_PATH) > 0) {
                std::wstring moduleName = to_lowercase(buffer);
                if (moduleName == to_lowercase(processName)) {
                    CloseHandle(processHandle);
                    return processIds[i];
                }
            }
            CloseHandle(processHandle);
        }
    }
    return 0;
}

bool set_working_size(DWORD processId) {
    HANDLE processHandle = OpenProcess(PROCESS_SET_QUOTA, FALSE, processId);
    if (processHandle != NULL) {
        if (SetProcessWorkingSetSize(processHandle, (SIZE_T)-1, (SIZE_T)-1)) {
            CloseHandle(processHandle);
            return true;
        }
        std::cerr << "Failed to set working set size for process ID: " << processId << "\n";
        CloseHandle(processHandle);
    }
    else std::cerr << "Failed to open process for setting quota: " << processId << "\n";    
    return false;
}

void monitor_and_limit_mem() {
    constexpr size_t MEMORY_LIMIT = 500 * 1024 * 1024;
    constexpr auto SLEEP_DURATION = std::chrono::seconds(1);

    while (true) {
        DWORD processId = get_procress_id(L"Discord.exe");
        if (processId != 0) {
            PROCESS_MEMORY_COUNTERS pmc;
            HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
            if (processHandle != NULL) {
                if (GetProcessMemoryInfo(processHandle, &pmc, sizeof(pmc))) {
                    if (pmc.WorkingSetSize > MEMORY_LIMIT) {
                        std::cout << "Memory usage exceeded limit ("
                            << pmc.WorkingSetSize / (1024 * 1024)
                            << " MB). Limiting memory usage for process Discord.\n";
                        set_working_size(processId);
                    }
                }
                else std::cerr << "Failed to get process memory info for process ID: " << processId << "\n";
                CloseHandle(processHandle);
            }
            else std::cerr << "Failed to open process for memory monitoring: " << processId << "\n";
        }
        else std::cout << "Discord.exe not found.\n";
        std::this_thread::sleep_for(SLEEP_DURATION);
    }
}