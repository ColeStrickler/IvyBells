#pragma once
#include <vector>
#include <Windows.h>
#include <string>



std::vector<BYTE> obfuscateString(char* string);
char* resolveString(std::vector<BYTE> oldString);
