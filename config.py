import time
import usb_cdc
import os

DEBUG = True
version = 'V0.1'

serial = usb_cdc.data
in_data = bytearray()

def command(msg):
    print(msg.split())
    cmd = msg.split()[0]
    try:
        args = msg.split()
        args.pop(0)
    except:
        pass
    print('cmd: ' + str(cmd))
    print('args: ' + str(args))
    
    
    if cmd == 'help':
        serial.write(b'''
PICOPASS HELP MENU
ping
    returns pong
version
    returns versions of files
list-account
    returns list of accounts stored
create-account
    creates account
delete-account
    deletes account
debug
    changes debug constant\n
''')
    elif cmd == 'ping':
        serial.write(b'pong\n')
        #time.sleep(1)
        #serial.write(b'Executed command!\n')
    elif cmd == 'version':
        serial.write(b'PicopassOS version: {}\n'.format('TBC'))
        serial.write(b'configurator version: {}\n'.format(version))
        serial.write(b'loader version: {}\n'.format('TBC'))
        serial.write(b'circuitpython version: {}\n'.format(os.uname()[3]))
    elif cmd == 'list-account':
        import json
        with open('config.json', 'r')as file:
            data = json.load(file)
            file.close()
        serial.write(b'====================================\n')
        print(len(data['passwords']))
        for x in range(0, len(data['passwords'])):
            serial.write(b'Name: {}\n'.format(data['passwords'][x]['Name']))
            serial.write(b'Username: {}\n'.format(data['passwords'][x]['Username']))
            serial.write(b'Password: {}\n'.format(data['passwords'][x]['password']))
            serial.write(b'====================================\n')
        serial.write(b'finished account lists\n')
    elif cmd == 'delete-account':
        import json
        with open('config.json', 'r')as file:
                data = json.load(file)
                file.close()
        for i in range(0, len(data['passwords'])):
            if data['passwords'][i]['Name'] == args[0]:
                data['passwords'].pop(i)
        with open('config.json', 'w')as file:
            json.dump(data, file)
            #file.write(data)
            file.close()      
    elif cmd == 'create-account':
        import json
        if len(args) == 3:
            with open('config.json', 'r')as file:
                data = json.load(file)
                file.close()
            #data.append()
            print(data)
            data['passwords'].append({'Name': args[0], 'Username': args[1], 'password': args[2]})
            print(data)
            with open('config.json', 'w')as file:
                json.dump(data, file)
                #file.write(data)
                file.close()
            
            serial.write(b'Account created successfully!\n')
                
        else:
            serial.write(b'error correct amount of arguments not given!\n')
    elif cmd == 'debug':
        global DEBUG
        if args[0] == 'enable':
            if DEBUG:
                serial.write(b'debug already enabled\n')
            else:
                DEBUG = True
                serial.write(b'debug enabled\n')
        elif args[0] == 'disable':
            if not DEBUG:
                serial.write(b'debug already disabled\n')
            else:
                DEBUG = False
                serial.write(b'debug disabled\n')
        else:
            serial.write(b'argument unknown\n')
    
    else:
        serial.write(b'command unknown!\n')
    serial.write(b'Finished!\n')
    print('responded')



print("booted into configuration mode!")

while True:
    if serial.in_waiting > 0:
        byte = serial.read(1)
        if byte == b'\r':
            #print(in_data.decode("utf-8"))
            command(in_data.decode("utf-8"))
            out_data = in_data
            out_data += b'  '
            in_data = bytearray()
            out_index = 0
        else:
            in_data += byte
            if len(in_data) == 129:
                in_data = in_data[128] + in_data[1:127]
