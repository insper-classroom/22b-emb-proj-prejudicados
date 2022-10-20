import pydirectinput
import serial
import argparse
import time
import logging

class MyControllerMap:
    def __init__(self):
        self.button = {'joyright': 'right', 'joyleft' : 'left', 'start': 'space', 'out':'esc'} # Fast forward (10 seg) pro Youtube

class SerialControllerInterface:
    # Protocolo
    # byte 1 -> Botão 1 (estado - Apertado 1 ou não 0)
    # byte 2 -> EOP - End of Packet -> valor reservado 'X'

    def __init__(self, port, baudrate):
        self.ser = serial.Serial(port, baudrate=baudrate)
        self.mapping = MyControllerMap()
        self.incoming = '0'
        pydirectinput.PAUSE = 0  ## remove delay   
    
    def update(self):
        ## Sync protocol
        while self.incoming != b'X':
            self.incoming = self.ser.read()
            logging.debug("Received INCOMING: {}".format(self.incoming))

        data = self.ser.read()

        print(f"data: {data}")

        """
        1 = start but
        2 = joystick right
        3 = joystick left
        4 = esc but
        """

        logging.debug("Received DATA: {}".format(data))

        if data == b'0':
            logging.info("KEYUP B")
            pydirectinput.keyUp(self.mapping.button['joyleft'])
            logging.info("KEYUP A")
            pydirectinput.keyUp(self.mapping.button['joyright'])

        if data == b'1':
            logging.info("KEYDOWN C")
            pydirectinput.keyDown(self.mapping.button['start'])
            logging.info("KEYUP B")
            pydirectinput.keyUp(self.mapping.button['joyleft'])
            logging.info("KEYUP A")
            pydirectinput.keyUp(self.mapping.button['joyright'])
            logging.info("KEYUP D")
            pydirectinput.keyUp(self.mapping.button['out'])

        if data == b'2':
            logging.info("KEYDOWN A")
            pydirectinput.keyDown(self.mapping.button['joyright']) 
            logging.info("KEYUP B")
            pydirectinput.keyUp(self.mapping.button['joyleft'])
            logging.info("KEYUP C")
            pydirectinput.keyUp(self.mapping.button['start'])
            logging.info("KEYUP D")
            pydirectinput.keyUp(self.mapping.button['out'])
        
        if data == b'3':
            logging.info("KEYDOWN B")
            pydirectinput.keyDown(self.mapping.button['joyleft'])
            logging.info("KEYUP C")
            pydirectinput.keyUp(self.mapping.button['start'])
            logging.info("KEYUP A")
            pydirectinput.keyUp(self.mapping.button['joyright'])
            logging.info("KEYUP D")  
            pydirectinput.keyUp(self.mapping.button['out'])   

        if data == b'4':
            logging.info("KEYDOWN D")
            pydirectinput.keyDown(self.mapping.button['out'])
            logging.info("KEYUP C")
            pydirectinput.keyUp(self.mapping.button['start'])
            logging.info("KEYUP A")
            pydirectinput.keyUp(self.mapping.button['joyright'])
            logging.info("KEYUP B")
            pydirectinput.keyUp(self.mapping.button['joyleft']) 
        

        self.incoming = self.ser.read()




class DummyControllerInterface:
    def __init__(self):
        self.mapping = MyControllerMap()

    def update(self):
        pydirectinput.keyDown(self.mapping.button['A'])
        time.sleep(0.1)
        pydirectinput.keyUp(self.mapping.button['A'])
        logging.info("[Dummy] Pressed A button")
        time.sleep(1)


if __name__ == '__main__':
    interfaces = ['dummy', 'serial']
    argparse = argparse.ArgumentParser()
    argparse.add_argument('serial_port', type=str)
    argparse.add_argument('-b', '--baudrate', type=int, default=9600)
    argparse.add_argument('-c', '--controller_interface', type=str, default='serial', choices=interfaces)
    argparse.add_argument('-d', '--debug', default=False, action='store_true')
    args = argparse.parse_args()
    if args.debug:
        logging.basicConfig(level=logging.DEBUG)

    print("Connection to {} using {} interface ({})".format(args.serial_port, args.controller_interface, args.baudrate))
    if args.controller_interface == 'dummy':
        controller = DummyControllerInterface()
    else:
        controller = SerialControllerInterface(port=args.serial_port, baudrate=args.baudrate)

    while True:
        controller.update()
