#include "dynamicresolution.h"
#include <stdio.h>



PEB* GetPEB() {
#ifdef _WIN64
    PEB* peb = (PEB*)__readgsqword(0x60);
    return peb;
#else 
    PEB* peb = (PEB*)__readfsdword(0x30);
    return peb;
#endif
}



LPVOID GetModule(wchar_t* modName) { // this function walks the PEB LDR_DATA module list to return the base address of a loaded module
    PEB* peb = GetPEB();
    if (peb == NULL) {
        return NULL;
    }
  //  printf("Searching for %ws...\n", modName);
    PPEB_LDR_DATA ldrData = peb->Ldr;
    PLIST_ENTRY head = &ldrData->InMemoryOrderModuleList;
    PLIST_ENTRY entry = head;

    LDR_DATA_TABLE_ENTRY completeTable = *(LDR_DATA_TABLE_ENTRY*)((BYTE*)entry - sizeof(LIST_ENTRY));
    if (!_wcsicmp(completeTable.FullDllName.Buffer, modName)) {
      //  printf("Found %ws", completeTable.FullDllName.Buffer);
        return completeTable.DllBase;
    }
    else {
        entry = entry->Flink;
        while (entry != head) {
            completeTable = *(LDR_DATA_TABLE_ENTRY*)((BYTE*)entry - sizeof(LIST_ENTRY));
            if (!_wcsicmp(completeTable.FullDllName.Buffer, modName)) {
           //     printf("Found %ws", completeTable.FullDllName.Buffer);
                return completeTable.DllBase;
            }
            entry = entry->Flink;
        }
    }
    return NULL;
}

void* GetSymbolAddress(uintptr_t baseAddress, LPCSTR lpProcName) { // given a dll base address, this function will resolve an exported function address
    if (baseAddress == NULL) {
        return 0;
    }

    PLONG symbolAddress = 0;
    PLONG exportAddressTable = 0;
    PLONG nameTable = 0;
    PLONG ordinalTable = 0;

    auto dos = (PIMAGE_DOS_HEADER)baseAddress;
    auto nt = (PIMAGE_NT_HEADERS)(baseAddress + dos->e_lfanew);
    PIMAGE_DATA_DIRECTORY dataDirectory = (PIMAGE_DATA_DIRECTORY)&nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    PIMAGE_EXPORT_DIRECTORY exportDirectory = (PIMAGE_EXPORT_DIRECTORY)(dataDirectory->VirtualAddress + baseAddress);

    exportAddressTable = (PLONG)(baseAddress + exportDirectory->AddressOfFunctions);
    nameTable = (PLONG)(baseAddress + exportDirectory->AddressOfNames);
    ordinalTable = (PLONG)(baseAddress + exportDirectory->AddressOfNameOrdinals);
    DWORD numEntry = exportDirectory->NumberOfNames;
    for (int i = 0; i < numEntry; i++) {
        LPCSTR name = (LPCSTR)(baseAddress + nameTable[i]);
        if (!STRCMP(name, lpProcName)) {
           // printf("FOUND %s\n", name);
            symbolAddress = (PLONG)(baseAddress + exportAddressTable[i]);
        }
    }
    return (void*)symbolAddress;
}


// Initializes ApiString struct with dynamic function addresses
void initAPIStruct(PAPISTRING apistring, PAPI api, LPVOID modbase) {
    std::vector<BYTE> temp = {};
    LONG64* entry = (LONG64*)api;
    for (BYTE i : apistring->names) {
        if (i != 0x99) {
            temp.push_back(i);
        }
        else {
            char* name = resolveString(temp);
            uintptr_t base = (uintptr_t)modbase;
            LPVOID routineAddr = GetSymbolAddress(base, name);
            if (routineAddr != NULL) {
                *(LONG64*)entry = (LONG64)routineAddr;
            }
            entry++;
            temp.clear();
            free(name);
        }
        
        
       // printf("ENTRY ADDRESS-->0x%p\n", entry);
    }
}

// Dynamically locates base address of DLLs
void initModList(PMODULESTRING modstring, PMODULELIST modlist, PAPISTRING apistring, PAPI api) {
    std::vector<BYTE> temp = {};
    LPVOID entry = (LPVOID)modlist;

    for (BYTE i : modstring->names) {
        if (i != 0xff) {
            temp.push_back(i);
        }
        else {
            char* name = resolveString(temp);
            LPVOID modBase = GetModule((wchar_t*)charToWchar(name));
            if (modBase == NULL) {
                using _LlA = HMODULE(*WINAPI)(LPCSTR moduleName);
                _LlA loadlib = reinterpret_cast<_LlA>(api->LoadLibraryA);
                loadlib(name);
                modBase = GetModule((wchar_t*)charToWchar(name));
            }
            initAPIStruct(apistring, api, modBase);
            entry = modBase;
            entry = (LPVOID)((PLONG)entry + sizeof(LPVOID));
            temp.clear();
            free(name);
        }
    }
    return;
}




