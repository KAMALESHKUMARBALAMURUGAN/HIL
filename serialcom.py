import tkinter as tk
from tkinter import messagebox
import serial
import time
 
# Configure the serial connection
ser = serial.Serial(
    port='COM4',  # Replace 'COM3' with your ESP32 COM port
    baudrate=115200,
    timeout=1
)
 
def send_command(cmd):
    try:
        ser.write(cmd.encode())  # Send the command to the ESP32
        print("cmd----------------->%d",cmd)
        time.sleep(1)
    except serial.SerialException as e:
        messagebox.showerror("Serial Error", str(e))
 
def on_button():
    send_command('1')
 
def off_button():
    send_command('0')
 
# Set up the GUI
root = tk.Tk()
root.title("LED Controller")
 
on_button = tk.Button(root, text="Turn ON LED", command=on_button)
on_button.pack(pady=20)
 
off_button = tk.Button(root, text="Turn OFF LED", command=off_button)
off_button.pack(pady=20)
 
root.mainloop()