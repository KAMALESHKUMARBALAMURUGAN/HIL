import serial
import time
 
# Configure the serial connection
ser = serial.Serial(
    port='COM5', # Replace 'COMX' with your ESP32 COM port
    baudrate=115200,
    timeout=1
)
 
while True:
    cmd = input("Enter 1 to turn ON the LED, 0 to turn OFF the LED: ")
    if cmd in ['0', '1']:
        ser.write(cmd.encode())  # Send the command to the ESP32
        time.sleep(1)
    else:
        print("Invalid input. Please enter 1 or 0.")