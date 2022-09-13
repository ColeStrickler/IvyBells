#include "loader.h"
typedef struct relocBlock {
	WORD pageRVA;
	WORD type;
}RELOC_RECORD, * PRELOC_RECORD;



SIZE_T WideStringLength(LPWSTR str)
{
	SIZE_T len = 0;
	SIZE_T i = 0;

	while (str[i++])
		++len;

	return len;
}

BOOL WideStringCompare(LPWSTR lpwStr1, LPWSTR lpwStr2, SIZE_T cbMaxCount)
{
	BOOL match = TRUE;

	for (SIZE_T i = 0; i < cbMaxCount; i++)
	{
		WCHAR a, b;
		a = lpwStr1[i];
		b = lpwStr2[i];
		if (a >= 'A' && a <= 'Z')
			a += 32;
		if (b >= 'A' && b <= 'Z')
			b += 32;
		if (a != b)
		{
			match = FALSE;
			break;
		}
	}

	return match;
}

PLDR_DATA_TABLE_ENTRY2 t_FindLdrTableEntry(LPWSTR BaseName)
{
	PPEB2 pPeb;
	PLDR_DATA_TABLE_ENTRY2 pCurEntry;
	PLIST_ENTRY pListHead, pListEntry;

	pPeb = (PPEB2)READ_MEMLOC(PEB_OFFSET);

	if (pPeb == NULL)
	{
		return NULL;
	}

	pListHead = &pPeb->Ldr->InLoadOrderModuleList;
	pListEntry = pListHead->Flink;

	do
	{
		pCurEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY2, InLoadOrderLinks);
		pListEntry = pListEntry->Flink;

		//BOOL BaseName1 = WideStringCompare(BaseName, pCurEntry->BaseDllName.Buffer, (pCurEntry->BaseDllName.Length / sizeof(wchar_t)) - 4);
		BOOL BaseName2 = WideStringCompare(BaseName, pCurEntry->BaseDllName.Buffer, WideStringLength(BaseName));

		if (BaseName2 == TRUE)
		{
			return pCurEntry;
		}

	} while (pListEntry != pListHead);

	return NULL;

}



bool spoofLoad(PAPI api, LPVOID base, size_t size, LPWSTR spoofDll, LPWSTR spoofPath) {
	UNICODE_STRING fullPath = {0};
	UNICODE_STRING fName = {0};


	using getFunc = FARPROC(*__stdcall)(HMODULE, LPCSTR); // GetProcAddress
	getFunc getFunction = reinterpret_cast<getFunc>(api->GetProcAddress);


	using getMod = HMODULE(*__stdcall)(LPCSTR); //GetModuleHandleA
	getMod getModule = reinterpret_cast<getMod>(api->GetModuleHandleA);





	HMODULE hMod = getModule("ntdll.dll");
	if (!hMod) {
		printf("FAILED!\n\n");
	}

	using hashUnicode = NTSTATUS(*NTAPI)(PCUNICODE_STRING, BOOLEAN, ULONG, PULONG);
	hashUnicode rtlHashUnicode = reinterpret_cast<hashUnicode>(getFunction(hMod, "RtlHashUnicodeString"));
	using initUnicode = void(*__stdcall)(PUNICODE_STRING, PCWSTR);
	initUnicode unicodeInit = reinterpret_cast<initUnicode>(getFunction(hMod, "RtlInitUnicodeString"));




	LPVOID timeAddy = getFunction(hMod, "NtQuerySystemTime");
	using getTime = NTSTATUS(*__stdcall)(PLARGE_INTEGER);
	getTime getSysTime = reinterpret_cast<getTime>(timeAddy);
	//CloseHandle(hMod);

	unicodeInit(&fullPath, spoofPath);
	unicodeInit(&fName, spoofDll);
	printf("path-->%ws\n", fullPath.Buffer);
	printf("file-->%ws\n", fName.Buffer);
	printf("0x%p\n", rtlHashUnicode);
	LARGE_INTEGER sysTime = { 0 };
	getSysTime(&sysTime);

	


	//using _LlA = HMODULE(*WINAPI)(LPCWSTR moduleName);
	//_LlA loadlib = reinterpret_cast<_LlA>(getFunction(hMod, "LoadLibraryW"));
	HMODULE hmod = LoadLibraryW((LPCWSTR)spoofPath);
	if (!hmod) {
		printf("BAD HMOD!\n");
		return 0;
	}
	//printf("0x%p\n", t_FindLdrTableEntry);
	PLDR_DATA_TABLE_ENTRY2 entry = t_FindLdrTableEntry((wchar_t*)spoofDll);
	if (entry == NULL) {
		printf("COULD NOT GET LDR ENTRY\n");
		return 0;
	}


	entry->LoadTime = sysTime;
	entry->LoadReason = LoadReasonDynamicLoad;
	//ULONG hashVal = 0xFFFFFFFFFFF;
	//printf("\n0x%p\n", &hashVal);
	ULONG hashVal;
	rtlHashUnicode(&fName, TRUE, 0, &hashVal);
	entry->BaseNameHashValue = hashVal;
	entry->ImageDll = TRUE;
	entry->LoadNotificationsSent = TRUE;
	entry->EntryProcessed = TRUE;
	entry->InLegacyLists = TRUE;
	entry->InIndexes = TRUE;
	entry->ProcessAttachCalled = TRUE;
	entry->InExceptionTable = FALSE;
	entry->OriginalBase = (ULONG_PTR)base;
	entry->SizeOfImage = size;
	entry->TimeDateStamp = 0;
	entry->DllBase = base;
	printf("\nOld DLL name-->%ws\n", entry->BaseDllName.Buffer);
	entry->BaseDllName = fName;
	entry->FullDllName = fullPath;
	entry->Flags = LDRP_IMAGE_DLL | LDRP_ENTRY_INSERTED | LDRP_ENTRY_PROCESSED | LDRP_PROCESS_ATTACH_CALLED;


	entry->DdagNode = (PLDR_DDAG_NODE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LDR_DDAG_NODE));
	if (!entry->DdagNode)
	{
		return 0;
	}

	entry->NodeModuleLink.Flink = &entry->DdagNode->Modules;
	entry->NodeModuleLink.Blink = &entry->DdagNode->Modules;
	entry->DdagNode->Modules.Flink = &entry->NodeModuleLink;
	entry->DdagNode->Modules.Blink = &entry->NodeModuleLink;
	entry->DdagNode->State = LdrModulesReadyToRun;
	entry->DdagNode->LoadCount = 1;

	return 1;
}




bool loadDLL(BYTE* dllBase, PAPI api) {

	BYTE* base = dllBase;
	


	PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)base;
	PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(base + dos->e_lfanew);
	//printf("[*] Performing signature check.....\n");
	BYTE sigCheck = *((BYTE*)&nt->Signature + 4);

	if (sigCheck == 0x64) {
		printf("[*] PE\\0\\0d found  --> 64 bit identified.\n");
	}
	else if (sigCheck == 0x4c) {
		printf("[*] PE\\0\\0L found  --> 32 bit identified.\n");
	}
	else {
		printf("[!] Unable to find PE signature in NT Headers. Found --> %x Exiting..\n", sigCheck);
		exit(-1);
	}



	DWORD nBytes = nt->OptionalHeader.SizeOfImage;
	DWORD freeSize = nt->OptionalHeader.SizeOfImage;
	uintptr_t baseAddress = (uintptr_t)VirtualAlloc(NULL, nBytes, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	spoofLoad(api, (LPVOID)baseAddress, freeSize, (LPWSTR)L"WINMM.dll", (LPWSTR)L"C:\\Windows\\System32\\WINMM.dll"); // SUB IN WHO TO SPOOF HERE
	
	// copy over headers
	nBytes = nt->OptionalHeader.SizeOfHeaders;
	memcpy((void*)baseAddress, (void*)base, nBytes);

	// copy over sections
	uintptr_t sectionAddress = (uintptr_t)&nt->OptionalHeader + (uintptr_t)nt->FileHeader.SizeOfOptionalHeader; // sections are immediately after the optional header
	DWORD nSections = nt->FileHeader.NumberOfSections;
	for (int i = 0; i < nSections; i++) {
		PIMAGE_SECTION_HEADER sectionHeader;
		uintptr_t addrInBuff;
		uintptr_t addrInMem;


		sectionHeader = (PIMAGE_SECTION_HEADER)sectionAddress;
		addrInBuff = (uintptr_t)base + sectionHeader->PointerToRawData;
		addrInMem = baseAddress + sectionHeader->VirtualAddress; // this is just adding the offset to the base of the newly allocated heap
		nBytes = sectionHeader->SizeOfRawData;

		//printf("\t> Copying over (%s) section..\n", sectionHeader->Name);
		memcpy((void*)addrInMem, (void*)addrInBuff, nBytes);
		sectionAddress = sectionAddress + sizeof(IMAGE_SECTION_HEADER);

}

	// populate the IAT
	PIMAGE_DATA_DIRECTORY dataDirectory = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	//printf("[*] dataDirectory->Size: %d\n", dataDirectory->Size);
	uintptr_t descriptorAddress = baseAddress + dataDirectory->VirtualAddress;
	PIMAGE_IMPORT_DESCRIPTOR descriptor = (PIMAGE_IMPORT_DESCRIPTOR)descriptorAddress;
	while (descriptor->Characteristics != 0) {
		uintptr_t nameAddr;
		HMODULE importedDLLBaseAddr;

		nameAddr = baseAddress + descriptor->Name; // this is the name of the DLL
	//	printf("[*] Processing imports for %s...\n", (char*)nameAddr);
		importedDLLBaseAddr = LoadLibraryA((LPCSTR)nameAddr);



		DWORD nFunctions = { 0 };
		DWORD nOrdinals = { 0 };

		uintptr_t firstThunkAddress = baseAddress + descriptor->FirstThunk;
		uintptr_t originalFirstThunkAddress = baseAddress + descriptor->OriginalFirstThunk;


		// both the IAT and ILT are arrays of IMAGE_THUNK_DATA structures
#ifdef _WIN64
		PIMAGE_THUNK_DATA64 IAT = (PIMAGE_THUNK_DATA64)firstThunkAddress;
		PIMAGE_THUNK_DATA64 ILT = (PIMAGE_THUNK_DATA64)originalFirstThunkAddress;
		uintptr_t flag = IMAGE_ORDINAL_FLAG64;
#else
		PIMAGE_THUNK_DATA32 IAT = (PIMAGE_THUNK_DATA32)firstThunkAddress;
		PIMAGE_THUNK_DATA32 ILT = (PIMAGE_THUNK_DATA32)originalFirstThunkAddress;
		uintptr_t flag = IMAGE_ORDINAL_FLAG32;
#endif

		while (IAT->u1.Function != 0) {
			// do not do anything for ordinal imports
			if (ILT->u1.Ordinal & flag) {
		//		printf("\t> Import by Ordinal\n");
				nOrdinals++;
			}
			else {
				PIMAGE_IMPORT_BY_NAME nameArray;
				uintptr_t funcNameAddress;

				nameArray = (PIMAGE_IMPORT_BY_NAME)(ILT->u1.AddressOfData);
				funcNameAddress = baseAddress + (uintptr_t)(nameArray->Name);
			//	printf("\t> Populating IAT with --> %s", (char*)funcNameAddress);
				//if (strlen((char*)funcNameAddress) < 6) {
			//		printf("\t\t\t\t\t");
			//	}
			//	else if (strlen((char*)funcNameAddress) < 14) {
			//		printf("\t\t\t\t");
			//	}
			//	else if (strlen((char*)funcNameAddress) > 29) {
			//		printf("\t");
			//	}
			//	else if (strlen((char*)funcNameAddress) > 21) {
			//		printf("\t\t");
			//	}
			//	else {
			//		printf("\t\t\t");
			//	}
			//	printf("RVA --> 0x%p\n", IAT->u1.Function);


				IAT->u1.Function = (uintptr_t)GetProcAddress(importedDLLBaseAddr, (LPCSTR)funcNameAddress); /// update each IAT entry to point to actual routine address
				nFunctions++;
			}


			IAT++;
			ILT++;
		}



		descriptor += 1;
	}


	uintptr_t delta = baseAddress - nt->OptionalHeader.ImageBase;
	PIMAGE_DATA_DIRECTORY relocDir = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

	uintptr_t tableEntryAddress = baseAddress + dataDirectory->VirtualAddress;
	PIMAGE_BASE_RELOCATION tableEntry = (PIMAGE_BASE_RELOCATION)tableEntryAddress;

	//printf("[*] Fixing up relocation addresses.\n");
	while (tableEntry->SizeOfBlock > 0) {

		uintptr_t pageAddress;
		DWORD nRelocs;
		uintptr_t relocRecordAddress;
		PRELOC_RECORD relocRecord;
		DWORD i = 0;

		// calculate address of 4kb page
		pageAddress = (baseAddress + tableEntry->VirtualAddress);

		// Determine # of IMAGE_RELOC elements in the relocation block
		nRelocs = tableEntry->SizeOfBlock;
		nRelocs = nRelocs - sizeof(IMAGE_BASE_RELOCATION);
		nRelocs = nRelocs / sizeof(RELOC_RECORD);



		// address of 1st IMAGE_RELOC following IMAGE_BASE_RELOCATION
		relocRecordAddress = tableEntryAddress + sizeof(IMAGE_BASE_RELOCATION);
		relocRecord = (PRELOC_RECORD)relocRecordAddress;

		for (i = 0; i < nRelocs; i++) {
			uintptr_t fixupAddress;
			DWORD fixupType;


			// find fixup address within 4kb
			fixupAddress = pageAddress + relocRecord[i].pageRVA;
			fixupType = relocRecord[i].type;

			if (fixupType == IMAGE_REL_BASED_HIGH) {
				*(WORD*)fixupAddress += HIWORD(delta);
			}
			else if (fixupType == IMAGE_REL_BASED_LOW) {
				*(WORD*)fixupAddress += LOWORD(delta);
			}
			else if (fixupType == IMAGE_REL_BASED_HIGHLOW) {
				*(DWORD*)fixupAddress += delta;
			}

		}

		tableEntryAddress = tableEntryAddress + tableEntry->SizeOfBlock;
		tableEntry = (PIMAGE_BASE_RELOCATION)tableEntryAddress;

	}


	if (nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size) {
		auto* pTLS = reinterpret_cast<IMAGE_TLS_DIRECTORY*>(baseAddress + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
		auto* pCallback = reinterpret_cast<PIMAGE_TLS_CALLBACK*>(pTLS->AddressOfCallBacks);
		for (; pCallback && *pCallback; ++pCallback)
			(*pCallback)((PVOID)baseAddress, DLL_PROCESS_ATTACH, nullptr);
	}


	f_RtlAddFunctionTable addfuncTable = reinterpret_cast<f_RtlAddFunctionTable>(api->RtlAddFunctionTable);
	auto excep = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
	if (excep.Size) {
		if (addfuncTable(
			reinterpret_cast<IMAGE_RUNTIME_FUNCTION_ENTRY*>(baseAddress + excep.VirtualAddress),
			excep.Size / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY), (DWORD64)baseAddress)) {
			//printf("Succsessfully Added\n");
		}
	}

	printf("[*] Loading finished successfully!\n");
	typedef struct args {
		HINSTANCE inst;
		DWORD dw;
		void* v;
	};

	args a;
	a.inst = (HINSTANCE)baseAddress;
	a.dw = DLL_PROCESS_ATTACH;
	a.v = 0;

	using dllMain = BOOL(*WINAPI)(HINSTANCE, DWORD, LPVOID);
	auto _DllMain = reinterpret_cast<dllMain>((uintptr_t)baseAddress + nt->OptionalHeader.AddressOfEntryPoint);
	printf("DllBase-->0x%p\n", baseAddress);
	//printf("Entry-->0x%p\n", (uintptr_t)baseAddress + nt->OptionalHeader.AddressOfEntryPoint);
	//printf("EntryVirtual-->0x%p\n", nt->OptionalHeader.AddressOfEntryPoint);
	//printf("dllMain-->0x%p\n", _DllMain);
	(*_DllMain)(a.inst, a.dw, a.v);
	return 1;
} 