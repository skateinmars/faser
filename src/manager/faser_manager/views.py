from faser_manager import app
from faser_manager import serial
from faser_manager.auth import auth

from flask import request

@app.route('/', methods=['GET'])
@auth.login_required
def index():
    return ''

@app.route('/sensors/sensitivities', methods=['GET'])
@auth.login_required
def read_sensitivities():
    return serial.read_sensitivities()

@app.route('/sensors/<int:index>/sensitivity', methods=['POST'])
@auth.login_required
def update_sensitivity(index):
    value = request.form['value']
    return serial.update_sensitivity(index, value)

# 1 = enabled
@app.route('/sensors/debounce', methods=['POST'])
@auth.login_required
def update_debounce():
    value = request.form['value']
    return serial.update_debounce(value)
