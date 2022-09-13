from flask import render_template, request
from werkzeug.utils import secure_filename
import os
from ivybells import app



#########################HOME PAGE#########################
@app.route("/")
def home():
    return render_template('base.html', title="")
#########################HOME PAGE#########################
ALLOWED_EXTENSIONS = {'txt', 'pdf', 'png', 'jpg', 'jpeg', 'gif', 'exe', 'doc', 'docx', 'xlsm', 'xls', 'h'}
def allowed_file(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS
@app.route('/upload', methods=['POST'])
def upload_file():
    if request.method == 'POST':
        print(app.config['UPLOAD_FOLDER'])
        print(request.files['multipart/form-data'].stream)
        # check if the post request has the file part
        file = request.files['multipart/form-data']
        # If the user does not select a file, the browser submits an
        # empty file without a filename.
        if file.filename == '':
            return "error", 500
        if file and allowed_file(file.filename):
            filename = secure_filename(file.filename)
            file.save(os.path.join(app.config['UPLOAD_FOLDER'], filename))
            return "success", 200
    return ''





#########################REGISTRATION#########################



