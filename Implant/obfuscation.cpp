#include "obfuscation.h"



std::vector<BYTE> obfuscateString(char* string) {
	std::vector<BYTE> newString;
	while (*string != 0) {
		BYTE newSymbol = ((BYTE)*string + 0x06) ^ 0x2E;
		newString.push_back(newSymbol);
		string++;
	}
	return newString;
}


char* resolveString(std::vector<BYTE> oldString) { // allocates heap memory, remember to free
	const int size = oldString.size() + 1;
	char* resolveString = new char[size];
	int index = 0;
	for (BYTE i : oldString) {
		char add = (char)((i ^ 0x2E) - 0x06);
		resolveString[index] = add;
		index++;
	}
	resolveString[index] = (char)0x00;
	return resolveString;
}



