import os
from ivybells import db, app, create_defaultUser

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
try:
    db.create_all()
except Exception:
    pass


if __name__ == '__main__':
    port = int(os.environ.get('PORT', 80))
    app.run(debug=False, host='0.0.0.0', port=port)