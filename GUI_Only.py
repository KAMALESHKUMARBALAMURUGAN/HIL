import tkinter as tk
from tkinter import ttk

# Create the main window
root = tk.Tk()
root.title("Control Panel")
root.geometry("800x400")

# Create frames
left_frame = ttk.Frame(root)
left_frame.pack(side=tk.LEFT, padx=20, pady=20)

right_frame = ttk.Frame(root)
right_frame.pack(side=tk.RIGHT, padx=20, pady=20)

# Function to update the scale label
def update_label(name, var, label):
    value = int(var.get())  # Cast float value to int
    label.config(text=f"{name}: {value}")
    var.set(value)  # Update the variable to hold an integer value

# Define scales and labels for real-time value display
scales_info = [
    ("Throttle", 0, 100),
    ("SOC", 0, 100),
    ("Battery temp", 0, 150),
    ("Motor temp", 0, 250),
    ("Controller temp", 0, 200),
    ("PCB temp", 0, 200)
]

scales = {}
for name, min_val, max_val in scales_info:
    scale_var = tk.IntVar()  # Use IntVar instead of DoubleVar
    label = ttk.Label(left_frame, text=f"{name}: {min_val}")
    label.pack()
    scale = ttk.Scale(left_frame, from_=min_val, to=max_val, orient=tk.HORIZONTAL, variable=scale_var, command=lambda event, name=name, var=scale_var, label=label: update_label(name, var, label))
    scale.pack(fill=tk.X)
    scales[name] = (scale, scale_var)

# Define radio buttons for each mode with options 0 and 1
modes_info = [
    "Mode L",
    "Mode R",
    "Reverse",
    "Break",
    "Ignition"
]

modes_vars = {}
for mode in modes_info:
    frame = ttk.LabelFrame(right_frame, text=mode)
    frame.pack(padx=10, pady=10, fill=tk.X)
    var = tk.IntVar(value=0)  # default to 0
    modes_vars[mode] = var
    ttk.Radiobutton(frame, text="0", variable=var, value=0).pack(side=tk.LEFT)
    ttk.Radiobutton(frame, text="1", variable=var, value=1).pack(side=tk.LEFT)

# Start the event loop
root.mainloop()
