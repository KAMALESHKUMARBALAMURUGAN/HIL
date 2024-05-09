import tkinter as tk
from tkinter import messagebox, Scale
import serial
import time

# Configure the serial connection
ser = serial.Serial(
    port='COM5',  # Replace 'COM5' with your ESP32 COM port
    baudrate=115200,
    timeout=1
)

def send_value(value):
    try:
        ser.write(str(value).encode())  # Send the value to the ESP32
        time.sleep(0.1)
    except serial.SerialException as e:
        messagebox.showerror("Serial Error", str(e))

# Set up the GUI
root = tk.Tk()
root.title("LED Controller")

scale = Scale(root, from_=0, to=100, orient='horizontal', label='Set Value', command=send_value)
scale.pack(pady=20)

root.mainloop()
