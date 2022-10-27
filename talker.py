import serial
import time
import serial.tools.list_ports

ports = serial.tools.list_ports.comports()

for port, desc, hwid in sorted(ports):
        print("{}: {} [{}]".format(port, desc, hwid))


port = None
port = serial.Serial(port="/dev/cu.usbmodem142303", baudrate=115200)
msg = 'Hello from host\r'

def serial_write(serial_port, msg):
    try:
        if serial_port:
            serial_port.write(msg.encode('utf-8'))
        else:
            print("[ERROR] Missing port")
    except:
        print("[ERROR] Port write fail")

while True:
    msg = str(input('> ')+'\r')
    serial_write(port, msg)
    print("sent message!")
    #time.sleep(5)
    finished = False
    if msg == 'Exit':
        exit(1)
    while not finished:
        response = (port.readline()).decode('utf-8')
        print(response)
        #print("received: {}".format(response))
        if response == 'Finished!\n':
            finished = True
        
    #time.sleep(5)