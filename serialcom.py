import tkinter as tk
from tkinter import messagebox
import serial

# Configure the serial connection
ser = serial.Serial(
    port='COM4',  # Replace 'COM3' with your ESP32 COM port
    baudrate=115200,
    timeout=1
)

def send_command(switch_number, state):
    cmd = f'{switch_number}{state}'
    try:
        bytes_written = ser.write(cmd.encode())
        print("data_written------------->",cmd.encode())
        # print(f"Bytes written: {bytes_written}")
        # time.sleep(1)
    except serial.SerialException as e:
        messagebox.showerror("Serial Error", str(e))

def button_pressed(switch_number, state):
    send_command(switch_number, state)
    

def slider_moved(slider_number, value):
    send_command(slider_number, str(value))
    

# Set up the GUI
root = tk.Tk()
root.title("Switch Control Panel")

# Create a frame for each switch
switches = {
    "Brake": 1,
    "Reverse": 2,
    "Mode R": 3,
    "Mode L": 4,
    "Sidestand": 5,
    "Ignition": 6,
    "Slider 7": 7,
    "Slider 8": 8,
    "Slider 9": 9
}

for switch_name, switch_number in switches.items():
    switch_frame = tk.LabelFrame(root, text=switch_name)
    switch_frame.pack(pady=5)
    if switch_number in [1, 2, 3, 4, 5, 6]:
        on_button = tk.Button(switch_frame, text="ON", command=lambda num=switch_number: button_pressed(num, '1'))
        on_button.pack(side=tk.LEFT, padx=5)
        off_button = tk.Button(switch_frame, text="OFF", command=lambda num=switch_number: button_pressed(num, '0'))
        off_button.pack(side=tk.LEFT, padx=5)
    else:
        slider = tk.Scale(switch_frame, from_=0, to=100, orient=tk.HORIZONTAL,
                          command=lambda val, num=switch_number: slider_moved(num, val))
        slider.pack()

root.mainloop()
