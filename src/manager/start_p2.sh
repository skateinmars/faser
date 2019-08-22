#!/bin/sh

TTY=/dev/ttyACM1 FLASK_APP=faser_manager FLASK_ENV=development flask run --host 0.0.0.0 --port 8899
