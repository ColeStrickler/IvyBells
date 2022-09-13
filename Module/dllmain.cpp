// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <Windows.h>
#include <string>




int STRCMP(const char* p1, const char* p2)
{
    const unsigned char* s1 = (const unsigned char*)p1;
    const unsigned char* s2 = (const unsigned char*)p2;
    unsigned char c1, c2;
    do
    {
        c1 = (unsigned char)*s1++;
        c2 = (unsigned char)*s2++;
        if (c1 == '\0')
            return c1 - c2;
    } while (c1 == c2);
    return c1 - c2;
}


void writeFeedback(void* buffer, const char* command) {
    int i = 0;
    char* b = (char*)buffer;
    while (true) {
        b[i] = command[i];

        if (command[i] == 0x00) {
            break;
        }
        i++;
    }
}


char* readCommand(void* buffer) {
    return (char*)buffer;
}


void clearCommand(void* buffer) {
    int i = 0;
    char* b = (char*)buffer;
    while (b[i] != 0x00) {
        b[i] = 0x00;
    }
}


void* initModuleComms(void** commandBuffer, void** argBuffer) {
    
    bool wait = true;
    void* buffer = NULL;
    std::wstring mapName = L"IPC_";
    std::wstring argBufName = L"ARG_";
    while (wait) {    
        for (int i = 0; i < 10; i++) { // read through file mappings until the correct one is found, then return it
            wchar_t w = *(std::to_wstring(i).c_str());
            mapName.push_back(w);

            HANDLE sharedMem = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, 1 << 12, mapName.c_str());
            buffer = MapViewOfFile(sharedMem, FILE_MAP_WRITE, 0, 0, 0);

            char* ret = readCommand(buffer);
           // std::wcout << L"mapName: " << mapName << std::endl;
          //  std::cout << "ret: " << ret << std::endl;
            if (!STRCMP(ret, "load")) {
                wait = false;
                argBufName.push_back(w);
                HANDLE argMem = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, 1 << 12, argBufName.c_str());
                *argBuffer = MapViewOfFile(argMem, FILE_MAP_WRITE, 0, 0, 0);
                break;
            }
            mapName.pop_back();
            CloseHandle(sharedMem);
            Sleep(1200);
        }
    }
    clearCommand(buffer);
    writeFeedback(buffer, "initialized");
    std::wcout << L"INITIALIZED WITH MAPNAME: " << mapName << std::endl;
    std::wcout << L"INITIALIZED WITH ARGBUFNAME: " << argBufName << std::endl;


    *commandBuffer = buffer;

    printf("0x%p\n", *commandBuffer);
    printf("0x%p\n", *argBuffer);
}










int main() {
    void* commandBuffer{0};
    void* argBuffer{0};
    initModuleComms(&commandBuffer, &argBuffer);
    printf("[STAGE-1] Successfully initialized.\n");
    printf("hmm\n");
    printf("0x%p\n", commandBuffer);
    printf("0x%p\n", argBuffer);

    while (true) {
        char* IssuedCommand = readCommand(commandBuffer);
          // 
        if (IssuedCommand != NULL) {
            printf("[STAGE-1]: COMMAND --> %s\n", IssuedCommand);
            if (!STRCMP(IssuedCommand, "1")) { // The way the module config files are parsed, the commands will be passed as numbers
                clearCommand(commandBuffer);
                std::string feedback = readCommand(argBuffer);
                clearCommand(argBuffer);
                writeFeedback(commandBuffer, feedback.c_str());
                writeFeedback(argBuffer, "FINISHED");
            }
            else if (!STRCMP(IssuedCommand, "2")) {
                clearCommand(commandBuffer);
                writeFeedback(commandBuffer, "FEEDBACK 2");
                clearCommand(argBuffer);
                writeFeedback(argBuffer, "FINISHED");
            }
        }
        Sleep(500);
    }
       Sleep(2500);

   
    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)main, 0, 0, 0));
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

