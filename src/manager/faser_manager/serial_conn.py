import serial

class SerialConn:
    device = "/dev/ttyACM0"
    timeout = 1

    def __init__(self):
        self.conn = serial.Serial()
        self.conn.baudrate = 9600
        self.conn.port = self.device
        self.conn.timeout = self.timeout
        self.conn.setDTR(1)

    def __send_command(self, command):
        self.conn.open()
        encoded_command = (command + "\n").encode()
        self.conn.write(encoded_command)
        response = self.conn.read(256).decode()
        self.conn.close()

        return response

    def read_sensitivities(self):
        return self.__send_command("R")

    def update_sensitivity(self, index, value):
        return self.__send_command(str(index)+str(value))

    def update_debounce(self, value):
        return self.__send_command("D"+str(value))
