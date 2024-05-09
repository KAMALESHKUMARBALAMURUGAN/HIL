import tkinter as tk
from tkinter import messagebox
import serial
import time
 
# Configure the serial connection
ser = serial.Serial(
    port='COM5',  # Change this to match your ESP32's connected COM port
    baudrate=115200,
    timeout=1
)
 
def send_command(led_number, state):
    cmd = f'{led_number}{state}'
    print(f"Command to send: {cmd}")
    try:
        bytes_written = ser.write(cmd.encode())
        print(f"Bytes written: {bytes_written}")
        time.sleep(1)
    except serial.SerialException as e:
        messagebox.showerror("Serial Error", str(e))
 
# GUI setup
root = tk.Tk()
root.title("LED Controller")
 
for led_number in range(1, 4):
    tk.Button(root, text=f"Turn ON LED {led_number}", command=lambda num=led_number: send_command(num, '1')).pack(pady=(20,0))
    tk.Button(root, text=f"Turn OFF LED {led_number}", command=lambda num=led_number: send_command(num, '0')).pack(pady=(0,20))
 
root.mainloop()
