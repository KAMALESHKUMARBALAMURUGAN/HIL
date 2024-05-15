import tkinter as tk
from tkinter import ttk
import serial

# Setup Serial Connection
ser = serial.Serial('COM5', 115200)  # Update 'COM_PORT' to your specific port

def send_uart(id, value):
    """Send parameter id and value over UART."""
    ser.write(f"{id},{value}\n".encode())
    print(id, value)

# Create the main window
root = tk.Tk()
root.title("HIL- Hardware In Loop")
root.geometry("500x500")

# Create frames
left_frame = ttk.Frame(root)
left_frame.pack(side=tk.LEFT, padx=20, pady=20)

right_frame = ttk.Frame(root)
right_frame.pack(side=tk.RIGHT, padx=20, pady=20)

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
    value = int(var.get())  # Cast float value to int
    entry.delete(0, tk.END)
    entry.insert(0, str(value))
    var.set(value)  # Update the variable to hold an integer value
    send_uart(id, value)  # Send update through UART

# Function to activate the button
def press_button(button, var, id):
    var.set(1)
    button.config(text="On")
    send_uart(id, 1)  # Send update through UART

# Function to deactivate the button
def release_button(button, var, id):
    var.set(0)
    button.config(text="Off")
    send_uart(id, 0)  # Send update through UART

# Function to simulate pressing both Ignition and Brake
def press_both(ignition_button, Brake_button, ignition_var, Brake_var):
    press_button(ignition_button, ignition_var)
    press_button(Brake_button, Brake_var)

# Function to simulate releasing both Ignition and Brake
def release_both(ignition_button, Brake_button, ignition_var, Brake_var):
    release_button(ignition_button, ignition_var)
    release_button(Brake_button, Brake_var)

# Define ids for each mode and sensor
ids = {
    "Mode L": 1,
    "Mode R": 2,
    "Reverse": 3,
    "Ignition": 4,
    "Brake": 5,
    "Throttle": 6,
    "SOC": 7,
    "Battery temp": 8,
    "Motor temp": 9,
    "Controller temp": 10,
    "PCB temp": 11
}
# Find the maximum label width needed
scales_info = [
    ("Throttle", 0, 100),
    ("SOC", 0, 100),
    ("Battery temp", 0, 150),
    ("Motor temp", 0, 250),
    ("Controller temp", 0, 200),
    ("PCB temp", 0, 200)
]

label_width = max(len(info[0]) for info in scales_info)  # Find the max length of label text

scales = {}
for i, (name, min_val, max_val) in enumerate(scales_info):
    control_frame = ttk.Frame(left_frame)  # Frame to hold both scale and entry
    control_frame.grid(row=i, column=0, sticky='ew', padx=5, pady=5)
    left_frame.grid_columnconfigure(0, weight=1)  # Ensure the frame uses all available horizontal space
    
    label = ttk.Label(control_frame, text=name, width=label_width, anchor='w')  # Uniform width based on longest label
    label.grid(row=0, column=0)
    
    scale_var = tk.IntVar()
    scale = ttk.Scale(control_frame, from_=min_val, to=max_val, orient=tk.HORIZONTAL, variable=scale_var)
    scale.grid(row=0, column=1, sticky='ew')
    control_frame.grid_columnconfigure(1, weight=1)  # Make scale expand
    
    entry = ttk.Entry(control_frame, width=4)  # Adjusted width to fit three digits
    entry.insert(0, str(min_val))
    entry.grid(row=0, column=2, padx=(5, 0))
    entry.bind('<Return>', lambda event, entry=entry, scale=scale, id=ids[name]: update_scale_from_entry(entry, scale, id))
    scale.config(command=lambda event, name=name, var=scale_var, entry=entry, id=ids[name]: update_entry_from_scale(name, var, entry, id))
    
    scales[name] = (scale, scale_var, entry)

# Function to simulate pressing both Reverse and Brake
def press_reverse_Brake(reverse_button, Brake_button, reverse_var, Brake_var):
    press_button(reverse_button, reverse_var)
    press_button(Brake_button, Brake_var)

# Function to simulate releasing both Reverse and Brake
def release_reverse_Brake(reverse_button, Brake_button, reverse_var, Brake_var):
    release_button(reverse_button, reverse_var)
    release_button(Brake_button, Brake_var)

# Define push buttons for each mode
modes_info = [
    "Mode L",
    "Mode R",
    "Reverse",
    "Ignition",
    "Brake"
]

mode_buttons = {}
mode_vars = {}
for mode in modes_info:
    frame = ttk.LabelFrame(right_frame, text=mode)
    frame.pack(padx=10, pady=10)

    var = tk.IntVar(value=0)  # Default to 0 (Off)
    mode_vars[mode] = var
    button = tk.Button(frame, text="Off")
    button.pack(side=tk.LEFT, padx=10)
    button.bind("<ButtonPress-1>", lambda event, b=button, v=var, id=ids[mode]: press_button(b, v, id))
    button.bind("<ButtonRelease-1>", lambda event, b=button, v=var,id=ids[mode]: release_button(b, v, id))
    mode_buttons[mode] = button

# Additional button for Ignition & Brake, now with updated label directly on the button and no frame label
ign_Brake_frame = ttk.Frame(right_frame)
ign_Brake_frame.pack(padx=10, pady=10)

ign_Brake_button = tk.Button(ign_Brake_frame, text="Ignition & Brake")
ign_Brake_button.pack(side=tk.LEFT, padx=10)

# Additional button for Reverse & Brake
reverse_Brake_frame = ttk.Frame(right_frame)
reverse_Brake_frame.pack(padx=10, pady=10)

reverse_Brake_button = tk.Button(reverse_Brake_frame, text="Reverse & Brake")
reverse_Brake_button.pack(side=tk.LEFT, padx=10)

# Bind mouse press and release to both buttons
ign_Brake_button.bind("<ButtonPress-1>", lambda event: press_both(mode_buttons["Ignition"], mode_buttons["Brake"], mode_vars["Ignition"], mode_vars["Brake"]))
ign_Brake_button.bind("<ButtonRelease-1>", lambda event: release_both(mode_buttons["Ignition"], mode_buttons["Brake"], mode_vars["Ignition"], mode_vars["Brake"]))

# Bind mouse press and release to both buttons
reverse_Brake_button.bind("<ButtonPress-1>", lambda event: press_reverse_Brake(mode_buttons["Reverse"], mode_buttons["Brake"], mode_vars["Reverse"], mode_vars["Brake"]))
reverse_Brake_button.bind("<ButtonRelease-1>", lambda event: release_reverse_Brake(mode_buttons["Reverse"], mode_buttons["Brake"], mode_vars["Reverse"], mode_vars["Brake"]))

# Start the event loop
root.mainloop()