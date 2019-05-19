from flask import Flask
from faser_manager.serial_conn import SerialConn

from werkzeug import exceptions as wexceptions
from serial.serialutil import SerialException

app = Flask(__name__)

serial = SerialConn()

import faser_manager.views

@app.errorhandler(wexceptions.BadRequestKeyError)
def handle_missing_param(e):
    return 'Missing param', 400
@app.errorhandler(SerialException)
def handle_unavailable_serial(e):
    return 'Unavailable Serial port', 500
