import serial
import tkinter as tk

def read_serial():
    try:
        line = ser.readline().decode('utf-8').strip()
        if line.startswith("RPM:"):
            rpm_value = line.split(":")[1]
            rpm_label.config(text=f"RPM: {rpm_value}")
    except Exception as e:
        print(f"Error reading serial: {e}")
    root.after(100, read_serial)  # Schedule the function to be called again after 100 ms

ser = serial.Serial('COM4', 115200, timeout=1)  # Update 'COM4' to your serial port

root = tk.Tk()
root.title("ESP32 UART Data")

rpm_label = tk.Label(root, text="RPM: ", font=("Helvetica", 16))
rpm_label.pack(pady=20)

root.after(100, read_serial)  # Start reading the serial data after 100 ms
root.mainloop()
