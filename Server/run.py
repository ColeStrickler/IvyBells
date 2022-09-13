from ivybells import db, app

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
    print(f"run operator fail\n{e}")
    pass



if __name__ == '__main__':
    db.create_all()
    app.run(debug=True, port=80)

