import tkinter as tk
from tkinter import ttk
import serial
import serial.tools.list_ports
from time import sleep

def find_esp32_port():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        # Adjust the keyword to match the description of the ESP32 on your system
        if 'Silicon Labs CP210x USB to UART Bridge' in port.description:
            return port.device
    return None

port = find_esp32_port()
if port:
    ser = serial.Serial(port, 115200)
    print(f"Connected to {port}")
else:
    print("No ESP32 device found.")

def send_uart(id, value):
    """Send parameter id and value over UART."""
    cmd=f'{id}{value}'
    ser.write(cmd.encode())
    print(cmd.encode())

# Function to update the scale and entry from each other
def update_scale_from_entry(entry, scale, id):
    value = entry.get()
    if value.isdigit():
        value = int(value)
        min_val = scale.cget('from')  # Correct method to get the minimum value
        max_val = scale.cget('to')    # Correct method to get the maximum value
        if min_val <= value <= max_val:
            scale.set(value)
            entry.delete(0, tk.END)
            entry.insert(0, str(value))
            send_uart(id, value)  # Send update through UART
        else:
            entry.delete(0, tk.END)
            entry.insert(0, str(scale.get()))

def update_entry_from_scale(name, var, entry, id):
    current_value = int(var.get())  # Cast float value to int                           //current_value contains the value of the slider (not the slider number)
    if current_value != int(entry.get()):  # Only send UART if value has changed
        entry.delete(0, tk.END)
        entry.insert(0, str(current_value))
        var.set(current_value)  # Update the variable to hold an integer value
        send_uart(id, current_value)  # Send update through UART

# Function to activate the button
def press_button(button, var, id):
    sleep(1)
    var.set(1)
    button.config(text="On")
    send_uart(id, 1)  # Send update through UART

# Function to deactivate the button
def release_button(button, var, id):
    sleep(1)
    var.set(0)
    button.config(text="Off")
    send_uart(id, 0)  # Send update through UART

# Function to simulate pressing both Reverse and Brake
def press_reverse_Brake(reverse_button, Brake_button, reverse_var, Brake_var, reverse_id, Brake_id):
    press_button(Brake_button, Brake_var, Brake_id)
    sleep(0.1)
    press_button(reverse_button, reverse_var, reverse_id)
    sleep(0.2)

# Function to simulate releasing both Reverse and Brake
def release_reverse_Brake(reverse_button, Brake_button, reverse_var, Brake_var, reverse_id, Brake_id):
    sleep(0.2)
    release_button(reverse_button, reverse_var, reverse_id)
    sleep(0.1)
    release_button(Brake_button, Brake_var, Brake_id)

# Create the main window
root = tk.Tk()
root.title("HIL- Hardware In Loop")
root.geometry("1000x700")

# Create frames
left_frame = ttk.Frame(root)
left_frame.pack(side=tk.LEFT, padx=20, pady=20, fill=tk.BOTH, expand=True)

# Define ids for each mode and sensor
ids = {
    "Brake": 1,
    "Reverse": 2,
    "Mode R": 3,
    "Mode L": 4,
    # "Sidestand": 5, (Push button not added in the GUI)
    "Ignition": 6,
    "SOC": 7,
    "Throttle(boost)": 8,  #Throttle(boost) is added to see the boost bar on the VCU screen(not for the speed change)
    "Battery temp": 9,
    "Motor temp": 'a',
    "Controller temp": 'b',
    "PCB temp": 'c',
    "RPM(SPEED)": 'd',
    "Motor Over Temperature Warning": 'e',
    "throttle error":'f',
    "Controller(MCU) Over Temperature Warning":'v',
    "Controller Over Voltage Warning": 'g',
    "Controller Under Voltage Warning": 'h',
    "Overcurrent Fault": 'i',
    "Motor Hall Input Abnormal": 'j',
    "Motor Stalling": 'k',
    "Motor Phase Loss": 'l',
    "Brake forever": 1,
    "BattLowSocWarn": 'm',
    "CellUnderVolWarn": 'n',
    "CellOverVolWarn": 'o',
    "PackUnderVolWarn": 'p',
    "PackOverVolWarn": 'q',
    "ChgUnderTempWarn": 'r',
    "ChgOverTempWarn": 's',
    "DchgUnderTempWarn": 't',
    "DchgOverTempWarn": 'u'
}

scales_info = [
    ("Throttle(boost)", 0, 100),
    ("SOC", 0, 100),
    ("Battery temp", 0, 150),
    ("Motor temp", 0, 250),
    ("Controller temp", 0, 200),
    ("PCB temp", 0, 200),
    ("RPM(SPEED)", 0, 4500)
]

scales = {}
scale_length = 100  # Define a fixed length for all scales
label_width = max(len(info[0]) for info in scales_info)  # Find the max length of label text
for i, (name, min_val, max_val) in enumerate(scales_info):
    control_frame = ttk.Frame(left_frame)  # Frame to hold both scale and entry
    control_frame.grid(row=i, column=0, sticky='ew', padx=5, pady=5)
    left_frame.grid_columnconfigure(0, weight=0)  # Set weight to 0 to prevent expansion
    
    label = ttk.Label(control_frame, text=name, width=label_width, anchor='w')  # Uniform width based on longest label
    label.grid(row=0, column=0, sticky='w')  # Stick to the left (west)
    
    scale_var = tk.IntVar()
    scale = ttk.Scale(control_frame, from_=min_val, to=max_val, orient=tk.HORIZONTAL, variable=scale_var, length=scale_length)
    scale.grid(row=0, column=1)
    
    entry = ttk.Entry(control_frame, width=4)  # Adjusted width to fit three digits
    entry.insert(0, str(min_val))
    entry.grid(row=0, column=2, padx=(5, 0))
    entry.bind('<Return>', lambda event, entry=entry, scale=scale, id=ids[name]: update_scale_from_entry(entry, scale, id))
    scale.config(command=lambda event, name=name, var=scale_var, entry=entry, id=ids[name]: update_entry_from_scale(name, var, entry, id))
    
    scales[name] = (scale, scale_var, entry)

# Grid configuration for left_frame to handle two columns
left_frame.grid_columnconfigure(0, weight=0)
left_frame.grid_columnconfigure(1, weight=0)  # Add this line to manage the second column for faults

# Define push buttons for each mode
modes_info = [
    ("Mode L", 4),
    ("Mode R", 3),
    ("Reverse", 2),
    ("Ignition", 6),
    ("Brake", 1),
]

mode_vars = {}
# Create frames for each mode's control
for i, (mode, id) in enumerate(modes_info):
    mode_frame = ttk.LabelFrame(left_frame, text=mode)
    mode_frame.grid(row=i, column=1, padx=10, pady=10, sticky='ew')  # Position fault controls in the second column

    # Create a control variable for each mode
    var = tk.StringVar(value="OFF")  # Default to 'OFF'
    mode_vars[mode] = var

    # Create RadioButtons for ON and OFF states
    ttk.Radiobutton(mode_frame, text="OFF", variable=var, value="OFF", command=lambda id=id: send_uart(id, 0)).pack(side="left")
    ttk.Radiobutton(mode_frame, text="ON", variable=var, value="ON", command=lambda id=id: send_uart(id, 1)).pack(side="left")

# Radio buttons for different faults
faults_info = [
    ("Motor Over Temperature Warning", "e"),
    ("throttle error","f"),
    ("Controller(MCU) Over Temperature Warning","v"),
    ("Controller Over Voltage Warning", "g"),
    ("Controller Under Voltage Warning", "h"),
    ("Overcurrent Fault", "i"),
    ("Motor Hall Input Abnormal", "j"),
    ("Motor Stalling", "k"),
    ("Motor Phase Loss", "l"),
    ("Brake forever", "1") 
]

fault_vars = {}
for i, (fault, fault_id) in enumerate(faults_info):
    fault_frame = ttk.LabelFrame(left_frame, text=fault)
    fault_frame.grid(row=i, column=2, padx=10, pady=10, sticky='ew')  # Position fault controls in the second column

    fault_var = tk.StringVar(value="OFF")
    fault_vars[fault] = fault_var
    ttk.Radiobutton(fault_frame, text="OFF", variable=fault_var, value="OFF", command=lambda id=fault_id: send_uart(id, 0)).pack(side=tk.LEFT, padx=10)
    ttk.Radiobutton(fault_frame, text="ON", variable=fault_var, value="ON", command=lambda id=fault_id: send_uart(id, 1)).pack(side=tk.LEFT, padx=10)

warnings_info = [
    ("BattLowSocWarn", "m"),
    ("CellUnderVolWarn", "n"),
    ("CellOverVolWarn", "o"),
    ("PackUnderVolWarn", "p"),
    ("PackOverVolWarn", "q"),
    ("ChgUnderTempWarn", "r"),
    ("ChgOverTempWarn", "s"),
    ("DchgUnderTempWarn", "t"),
    ("DchgOverTempWarn", "u")
]
warning_vars = {}
for i, (warning, warning_id) in enumerate(warnings_info):
    warning_frame = ttk.LabelFrame(left_frame, text=warning)
    warning_frame.grid(row=i, column=3, padx=10, pady=10, sticky='ew')  # Position warnings in the third column

    warning_var = tk.StringVar(value="OFF")
    warning_vars[warning] = warning_var
    ttk.Radiobutton(warning_frame, text="OFF", variable=warning_var, value="OFF", command=lambda id=warning_id: send_uart(id, 0)).pack(side=tk.LEFT, padx=10)
    ttk.Radiobutton(warning_frame, text="ON", variable=warning_var, value="ON", command=lambda id=warning_id: send_uart(id, 1)).pack(side=tk.LEFT, padx=10)

# Start the event loop
root.mainloop()