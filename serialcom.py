import tkinter as tk
from tkinter import ttk
import serial
import serial.tools.list_ports
from time import sleep

# Function to find the ESP32 port
def find_esp32_port():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        print(port)
        # Adjust the keyword to match the description of the ESP32 on your system
        if 'Silicon Labs CP210x USB to UART Bridge' in port.description:
            return port.device
        if 'USB-Enhanced-SERIAL CH9102' in port.description:
            return port.device
        
        
        
    return None

# Function to send UART commands
def send_uart(id, value):
    """Send parameter id and value over UART."""
    cmd = f'{id}{value}'
    ser.write(cmd.encode())
    print(cmd.encode())

# Function to update the scale and entry from each other
def update_scale_from_entry(entry, scale, id, is_float=False):
    value = entry.get()
    try:
        if is_float:
            value = float(value)
        else:
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
    except ValueError:
        entry.delete(0, tk.END)
        entry.insert(0, str(scale.get()))

# Function to update the entry from the scale
def update_entry_from_scale(name, var, entry, id, is_float=False):
    current_value = var.get()
    if is_float:
        current_value = float(current_value)
    else:
        current_value = int(current_value)
    
    if is_float:
        entry_value = float(entry.get())
    else:
        entry_value = int(entry.get())

    if current_value != entry_value:  # Only send UART if value has changed
        entry.delete(0, tk.END)
        entry.insert(0, str(current_value))
        var.set(current_value)  # Update the variable to hold an integer value
        send_uart(id, current_value)  # Send update through UART

# Function to press a button
def press_button(button, var, id):
    sleep(1)
    var.set(1)
    button.config(text="On")
    send_uart(id, 1)  # Send update through UART

# Function to release a button
def release_button(button, var, id):
    sleep(1)
    var.set(0)
    button.config(text="Off")
    send_uart(id, 0)  # Send update through UART

# Function to press both Ignition and Brake buttons
def press_both(ignition_button, Brake_button, ignition_var, Brake_var, ignition_id, Brake_id):
    press_button(Brake_button, Brake_var, Brake_id)
    sleep(0.1)
    press_button(ignition_button, ignition_var, ignition_id)
    sleep(0.2)

# Function to release both Ignition and Brake buttons
def release_both(ignition_button, Brake_button, ignition_var, Brake_var, ignition_id, Brake_id):
    sleep(0.2)
    release_button(ignition_button, ignition_var, ignition_id)
    sleep(0.1)
    release_button(Brake_button, Brake_var, Brake_id)

# Function to press both Reverse and Brake buttons
def press_reverse_Brake(reverse_button, Brake_button, reverse_var, Brake_var, reverse_id, Brake_id):
    press_button(Brake_button, Brake_var, Brake_id)
    sleep(0.1)
    press_button(reverse_button, reverse_var, reverse_id)
    sleep(0.2)

# Function to release both Reverse and Brake buttons
def release_reverse_Brake(reverse_button, Brake_button, reverse_var, Brake_var, reverse_id, Brake_id):
    sleep(0.2)
    release_button(reverse_button, reverse_var, reverse_id)
    sleep(0.1)
    release_button(Brake_button, Brake_var, Brake_id)

# Function to read data from the serial port
def read_serial():
    try:
        line = ser.readline().decode('utf-8').strip()
        if line.startswith("Motor_status:"):
            Motor_status = line.split(":")[1]
            if Motor_status == '1':
                Motor_label.config(text="Motor: ON")
            else:
                Motor_label.config(text="Motor: OFF")

        if line.startswith("DC_current_limit:"):
            DC_current_limit = line.split(":")[1]
            DC_current_limit_label.config(text=f"DC_current_limit: {DC_current_limit}")

        if line.startswith("Motor_RPM:"):
            Motor_RPM = line.split(":")[1]
            Motor_RPM_label.config(text=f"Motor_RPM: {Motor_RPM}")

        if line.startswith("SOC_RX:"):
            SOC_RX = line.split(":")[1]
            SOC_label.config(text=f"SOC  : {SOC_RX}")

        if line.startswith("Pack_curr_Rx:"):
            Pack_curr_Rx = line.split(":")[1]
            Pack_curr_Rx_label.config(text=f"Pack_curr_Rx  : {Pack_curr_Rx}")

    except Exception as e:
        print(f"Error reading serial: {e}")
    root.after(200, read_serial)  # Schedule the function to be called again after 100 ms

# Find the ESP32 port and initialize the serial connection
port = find_esp32_port()
if port:
    ser = serial.Serial(port, 115200)
    print(f"Connected to {port}")
else:
    print("No ESP32 device found.")
    ser = None

# Create the main window
root = tk.Tk()
root.title("HIL- Hardware In Loop")
root.geometry("900x700")

# Create frames
left_frame = ttk.Frame(root)
left_frame.pack(side=tk.LEFT, padx=20, pady=20, fill=tk.BOTH, expand=True)

# Define ids for each mode and sensor
ids = {
    "Brake": 1,
    "Reverse": 2,
    "Mode R": 3,
    "Mode L": 4,
    "Ignition": 6,
    "SOC": 7,
    "Throttle(boost)": 8,
    "Battery temp": 9,
    "Motor temp": 'a',
    "Controller temp": 'b',
    "PCB temp": 'c',
    "RPM(SPEED)": 'd',
    "Motor Over Temperature Warning": 'e',
    "throttle error": 'f',
    "Controller(MCU) Over Temperature Warning": 'v',
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
    "DchgOverTempWarn": 'u',
    "Pack Voltage": 'w',
    "Pack Current": 'x',
    "TempSensorFault": 'y',
    "sidestand_pulse":'z',
}

# Create scales for different parameters
scales_info = [
    ("Throttle(boost)", 0, 100),
    ("SOC", 0, 100),
    ("Battery temp", 0, 150),
    ("Motor temp", 0, 250),
    ("Controller temp", 0, 200),
    ("PCB temp", 0, 200),
    ("RPM(SPEED)", 0, 4500),
    ("Pack Voltage", 0.0, 66.0),
    ("Pack Current", -500.0, 500.0)
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
    
    if name in ["Pack Voltage", "Pack Current"]:
        scale_var = tk.DoubleVar()
        scale = ttk.Scale(control_frame, from_=min_val, to=max_val, orient=tk.HORIZONTAL, variable=scale_var, length=scale_length)
        scale.grid(row=0, column=1)
        
        entry = ttk.Entry(control_frame, width=6)  # Adjusted width to fit decimal values
        entry.insert(0, str(min_val))
        entry.grid(row=0, column=2, padx=(5, 0))
        entry.bind('<Return>', lambda event, entry=entry, scale=scale, id=ids[name], is_float=True: update_scale_from_entry(entry, scale, id, is_float))
        scale_var.trace("w", lambda *args, name=name, var=scale_var, entry=entry, id=ids[name], is_float=True: update_entry_from_scale(name, var, entry, id, is_float))
    else:
        scale_var = tk.IntVar()
        scale = ttk.Scale(control_frame, from_=min_val, to=max_val, orient=tk.HORIZONTAL, variable=scale_var, length=scale_length)
        scale.grid(row=0, column=1)
        
        entry = ttk.Entry(control_frame, width=4)
        entry.insert(0, str(min_val))
        entry.grid(row=0, column=2, padx=(5, 0))
        entry.bind('<Return>', lambda event, entry=entry, scale=scale, id=ids[name]: update_scale_from_entry(entry, scale, id))
        scale_var.trace("w", lambda *args, name=name, var=scale_var, entry=entry, id=ids[name]: update_entry_from_scale(name, var, entry, id))

    scales[name] = (scale, entry)

# Create buttons for different modes
modes_info = ["Brake", "Reverse", "Mode R", "Mode L", "Ignition"]
mode_buttons = {}
mode_vars = {}
for i, mode in enumerate(modes_info):
    frame = ttk.LabelFrame(left_frame, text=mode)
    frame.grid(row=i, column=1, padx=10, pady=10, sticky='ew')  # Position mode controls in the first column

    var = tk.IntVar(value=0)  # Default to 0 (Off)
    mode_vars[mode] = var  # Add this line to store the variable in mode_vars
    button = tk.Button(frame, text="Off")
    button.pack(side=tk.LEFT, padx=10)
    button.bind("<ButtonPress-1>", lambda event, b=button, v=var, id=ids[mode]: press_button(b, v, id))
    button.bind("<ButtonRelease-1>", lambda event, b=button, v=var, id=ids[mode]: release_button(b, v, id))
    mode_buttons[mode] = button

# Additional button for Ignition & Brake, moved to column 4
ign_Brake_frame = ttk.Frame(left_frame)
ign_Brake_frame.grid(row=len(modes_info), column=4, padx=10, pady=10, sticky='ew')

ign_Brake_button = tk.Button(ign_Brake_frame, text="Ignition & Brake")
ign_Brake_button.pack(side=tk.LEFT, padx=10)

# Additional button for Reverse & Brake, moved to column 4
reverse_Brake_frame = ttk.Frame(left_frame)
reverse_Brake_frame.grid(row=len(modes_info) + 1, column=4, padx=10, pady=10, sticky='ew')

reverse_Brake_button = tk.Button(reverse_Brake_frame, text="Reverse & Brake")
reverse_Brake_button.pack(side=tk.LEFT, padx=10)

# Radio buttons for different faults
faults_info = [
    ("Motor Over Temperature Warning", "e"),
    ("throttle error", "f"),
    ("Controller(MCU) Over Temperature Warning", "v"),
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

# Radio buttons for different warnings
warnings_info = [
    ("BattLowSocWarn", "m"),
    ("CellUnderVolWarn", "n"),
    ("CellOverVolWarn", "o"),
    ("PackUnderVolWarn", "p"),
    ("PackOverVolWarn", "q"),
    ("ChgUnderTempWarn", "r"),
    ("ChgOverTempWarn", "s"),
    ("DchgUnderTempWarn", "t"),
    ("DchgOverTempWarn", "u"),
    ("TempSensorFault","y"),
    ("sidestand_pulse",'z'),
    
]
warning_vars = {}
for i, (warning, warning_id) in enumerate(warnings_info):
    warning_frame = ttk.LabelFrame(left_frame, text=warning)
    warning_frame.grid(row=i, column=3, padx=10, pady=10, sticky='ew')  # Position warnings in the third column

    warning_var = tk.StringVar(value="OFF")
    warning_vars[warning] = warning_var
    ttk.Radiobutton(warning_frame, text="OFF", variable=warning_var, value="OFF", command=lambda id=warning_id: send_uart(id, 0)).pack(side=tk.LEFT, padx=10)
    ttk.Radiobutton(warning_frame, text="ON", variable=warning_var, value="ON", command=lambda id=warning_id: send_uart(id, 1)).pack(side=tk.LEFT, padx=10)

# Bind mouse press and release to both buttons
ign_Brake_button.bind("<ButtonPress-1>", lambda event: press_both(mode_buttons["Ignition"], mode_buttons["Brake"], mode_vars["Ignition"], mode_vars["Brake"], ids["Ignition"], ids["Brake"]))
ign_Brake_button.bind("<ButtonRelease-1>", lambda event: release_both(mode_buttons["Ignition"], mode_buttons["Brake"], mode_vars["Ignition"], mode_vars["Brake"], ids["Ignition"], ids["Brake"]))

# Bind mouse press and release to both buttons
reverse_Brake_button.bind("<ButtonPress-1>", lambda event: press_reverse_Brake(mode_buttons["Reverse"], mode_buttons["Brake"], mode_vars["Reverse"], mode_vars["Brake"], ids["Reverse"], ids["Brake"]))
reverse_Brake_button.bind("<ButtonRelease-1>", lambda event: release_reverse_Brake(mode_buttons["Reverse"], mode_buttons["Brake"], mode_vars["Reverse"], mode_vars["Brake"], ids["Reverse"], ids["Brake"]))

# Create a label for Motor status
Motor_label = tk.Label(root, text="Motor: ", font=("Helvetica", 16))
Motor_label.pack(pady=0)

# Create a label for DC CURRENT LIMIT
DC_current_limit_label = tk.Label(root, text="DC_Current_Limit: ", font=("Helvetica", 16))
DC_current_limit_label.pack(pady=0)

Motor_RPM_label = tk.Label(root, text="Motor_RPM: ", font=("Helvetica", 16))
Motor_RPM_label.pack(pady=0)

SOC_label = tk.Label(root, text="SOC: ", font=("Helvetica", 16))
SOC_label.pack(pady=0)

Pack_curr_Rx_label = tk.Label(root, text="Pack_curr_Rx: ", font=("Helvetica", 16))
Pack_curr_Rx_label.pack(pady=0)

# Start reading the serial data after 100 ms
if ser:
    root.after(200, read_serial)

# Start the event loop
root.mainloop()
