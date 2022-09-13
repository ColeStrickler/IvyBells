from flask import Flask, render_template, url_for, flash, redirect
from flask_sqlalchemy import SQLAlchemy
from flask_restful import Api
from hashlib import md5
import os
import sys
import sqlite3
import sys


UPLOAD_FOLDER = ".\\route\\network\\binaryconfigmetadata"


app = Flask(__name__)
app.config['SECRET_KEY'] = '696969'
#engine:[//[user[:password]@][host]/[dbname]]
app.config['SQLALCHEMY_DATABASE_URI'] = "postgresql://postgres:6969@localhost/IB"
app.config["SQLALCHEMY_TRACK_MODIFICATIONS"] = False
app.secret_key = 'secret key'
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

api = Api(app)
db = SQLAlchemy(app)

from ivybells import routes
#from ivybells.resources import Tasking, Registration
from ivybells.resources import RegisterAgent, RetrievePayload, LoginCheck, OperatorEndpoint, RetrieveTasks, \
    Init, create_defaultUser

#api.add_resource(Tasking, '/forward&uuid=user5534e78-10&server=42315-eastb-38z&auth=TRUE')
#/sync?newUUID=user5534e78-10
api.add_resource(RegisterAgent, '/sync&newUUID=user5534e78-10')
api.add_resource(RetrievePayload, '/download')
api.add_resource(LoginCheck, '/joint-ops')
api.add_resource(RetrieveTasks, '/tasks')
api.add_resource(Init, '/yolo')
try:
    api.add_resource(OperatorEndpoint, sys.argv[3])
except Exception as e:
    print("USING DEFAULT OPERATOR ENDPOINT-->/op")
    api.add_resource(OperatorEndpoint, "/op")

try:
    create_defaultUser()
except Exception as e:
    pass

