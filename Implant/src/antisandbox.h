#pragma once
#include "dynamicresolution.h"


bool checkProcessors(PAPI api);
__declspec(noinline) int doSomeSleeps(int numSecondstimes2Point3);
bool checkMemory(PAPI api, int numGigs);
bool checkScreenResolution(PAPI api);
bool checkUpTime(PAPI api);

bool antiSandbox(PAPI api);