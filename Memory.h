#pragma once
#include <windows.h>
#include <vector>
#include <iostream>
#include <TlHelp32.h>

class Memory {
private:
    DWORD processId = 0;
    HANDLE processHandle = NULL;

public:
    Memory(const char* processName) {
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        while (Process32Next(snapshot, &entry)) {
            if (!strcmp(processName, entry.szExeFile)) {
                processId = entry.th32ProcessID;
                processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
                break;
            }
        }

        if (snapshot) CloseHandle(snapshot);
    }

    ~Memory() {
        if (processHandle) CloseHandle(processHandle);
    }

    DWORD GetProcessId() { return processId; }
    HANDLE GetProcessHandle() { return processHandle; }

    uintptr_t GetModuleAddress(const char* moduleName) {
        MODULEENTRY32 entry;
        entry.dwSize = sizeof(MODULEENTRY32);

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);

        uintptr_t moduleAddr = 0;
        while (Module32Next(snapshot, &entry)) {
            if (!strcmp(moduleName, entry.szModule)) {
                moduleAddr = (uintptr_t)entry.modBaseAddr;
                break;
            }
        }

        if (snapshot) CloseHandle(snapshot);
        return moduleAddr;
    }

    template <typename T>
    T Read(uintptr_t address) {
        T value;
        ReadProcessMemory(processHandle, (LPCVOID)address, &value, sizeof(T), NULL);
        return value;
    }

    template <typename T>
    void Write(uintptr_t address, T value) {
        WriteProcessMemory(processHandle, (LPVOID)address, &value, sizeof(T), NULL);
    }
};
