from datetime import datetime
from ivybells import db
try:
    class Operator(db.Model):
        __table_args__ = {'extend_existing': True}
        __tablename__ = 'operator'
        name = db.Column(db.String(), primary_key=True)
        password = db.Column(db.String(), nullable=False)
        privs = db.Column(db.String(), default="Operator")
        def toString(self):
            return f"{self.name} - {self.privs}"
except Exception as e:
    pass


class Agent(db.Model):
   # __table_args__ = {'extend_existing': True}
    __tablename__ = 'agent'
    time = db.Column(db.String(), default=str(datetime.utcnow()))
    agentUUID = db.Column(db.String(), nullable=False, primary_key=True)
    hostname = db.Column(db.String(), nullable=False)
    password = db.Column(db.String(), nullable=False)
    modifier = db.Column(db.String(), nullable=False) # feed into a switch statement to modify the password
    modules = db.Column(db.String(), nullable=True, default="")
    def __repr__(self):
        return f"[AGENT REGISTERED]\t[{self.agentUUID}] registered on {self.time}. Password->{self.password}\tModifier={self.modifier}\n"
    def toString(self):
        return f"[AGENT REGISTERED]\t[{self.agentUUID}] registered on {self.time}. Password->{self.password}\tModifier={self.modifier}\n"

class Module(db.Model):
    __tablename__ = 'modules'
    name = db.Column(db.String(), nullable=False, primary_key=True)
    filename = db.Column(db.String(), nullable=False)
    uuid = db.Column(db.String(), nullable=False)



class Task(db.Model):
    #__table_args__ = {'extend_existing': True}
    __tablename__ = 'tasks'
    time = db.Column(db.String(), default=str(datetime.utcnow()))
    agentUUID = db.Column(db.String(), nullable=False)
    result = db.Column(db.String(), default="PENDING")
    data = db.Column(db.String(), nullable=True, default="")
    args = db.Column(db.String(), nullable=True)
    id = db.Column(db.String(), nullable=False, primary_key=True)
    code = db.Column(db.Integer, nullable=False)
    command = db.Column(db.String(), nullable=False)
    author = db.Column(db.String()) # add foreign key to a users model when login functionality is added
    def __repr__(self):
        if self.result == "PENDING":
            return f"{self.author} tasked [{self.agent}] with {self.command}/--/{self.commandName}/--/{self.time}"
        elif self.result == "FAILURE":
            return f"Tasking by {self.author} to [{self.agent}] FAILED` --> {self.command}/--/{self.commandName}/--/{self.time}"
        else:
            return f"Tasking by {self.author} to [{self.agent}] SUCCESS --> {self.command}/--/{self.commandName}/--/{self.time}"

