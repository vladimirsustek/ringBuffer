import serial
import time
import random

ser = serial.Serial('COM9')  # open serial port
ser.baudrate = 115200
ser.timeout=1

names = ["Jana", "Vladimir", "Lenka", "Lu", "Alexei"]

iter = 0
idx = 0

while iter < 2400:

    time.sleep(0.250)
    num = random.randint(0, 99999)
    
    if iter % 10 == 0:
        send = "Hojsahej" + '\r\n'
    elif iter % 4 == 0:
        send = "Name=" + names[idx] + '\r\n'
        idx = (idx + 1) % 5
    else:
        send = 'Value=' + str(num) + '\r\n'

    send = send.encode('utf-8')

    ser.write(send)
    receive = ser.readline()

    if len(receive) == 0:
        result = "Timeout\r\n"
    elif receive.decode('utf-8') == 'Match\r\n':
        result = "OK: " + receive.decode('utf-8').rstrip()
    elif receive == send:
        result = "OK: " + receive.decode('utf-8').rstrip()
    elif receive.decode('utf-8')[0:10] + '\r\n' == send.decode('utf-8')[0:10] + '\r\n':
        result = "OK: " + receive.decode('utf-8').rstrip()
    else:
        result = send.decode('utf-8').rstrip() + "!=" + receive.decode('utf-8').rstrip()

    print(result)

    iter = iter + 1

ser.close()
