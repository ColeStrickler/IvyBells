#include "enumeration.h"


#define TH32CS_SNAPPROCESS  0x00000002
int checkSusProcs(PAPI api) {
	using snap = HANDLE(*__stdcall)(DWORD, DWORD);
	snap snapProc = reinterpret_cast<snap>(api->CreateToolhelp32Snapshot);
	using firstProc = BOOL(*__stdcall)(HANDLE, LPPROCESSENTRY32W);
	firstProc procFirst = reinterpret_cast<firstProc>(api->Process32FirstW);
	using nextProc = BOOL(*__stdcall)(HANDLE, LPPROCESSENTRY32W);
	nextProc procNext = reinterpret_cast<nextProc>(api->Process32NextW);
	susProcNames sus;

	HANDLE hSnap = snapProc(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE) {
		return -1;
	}
	else {
		PROCESSENTRY32W procEntry;
		procEntry.dwSize = sizeof(PROCESSENTRY32W);

		if (procFirst(hSnap, &procEntry)) {
			do {
				std::vector<BYTE> temp;
				for (BYTE i : sus.names) {
					if (i != 0xf5) {
						temp.push_back(i);
					}
					else {
						//temp.push_back(0x00);
						char* tempName = resolveString(temp);
						if (!_wcsicmp(procEntry.szExeFile, charToWchar(tempName))) {
							return true;
						}
						temp.clear();
						free(tempName);
					}
				}
			} while (procNext(hSnap, &procEntry));
		}

	}
	return false;
}

