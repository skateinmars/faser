#!/bin/sh

TTY=/dev/ttyACM0 FLASK_APP=faser_manager FLASK_ENV=development flask run --host 0.0.0.0 --port 8888
