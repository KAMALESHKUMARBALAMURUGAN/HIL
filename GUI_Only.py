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

# Function to update the scale and entry from each other
def update_scale_from_entry(entry, scale):
    value = entry.get()
    if value.isdigit():
        value = int(value)
        min_val = scale.cget('from')  # Correct method to get the minimum value
        max_val = scale.cget('to')    # Correct method to get the maximum value
        if min_val <= value <= max_val:
            scale.set(value)
            entry.delete(0, tk.END)
            entry.insert(0, str(value))
        else:
            entry.delete(0, tk.END)
            entry.insert(0, str(scale.get()))

def update_entry_from_scale(name, var, entry):
    value = int(var.get())  # Cast float value to int
    entry.delete(0, tk.END)
    entry.insert(0, str(value))
    var.set(value)  # Update the variable to hold an integer value

# Define scales, entries, and labels for real-time value display
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
    control_frame = ttk.Frame(left_frame)  # Frame to hold both scale and entry
    control_frame.pack(fill=tk.X, pady=5)
    
    label = ttk.Label(control_frame, text=name)  # Label for the name
    label.pack(side=tk.LEFT)
    
    scale_var = tk.IntVar()
    scale = ttk.Scale(control_frame, from_=min_val, to=max_val, orient=tk.HORIZONTAL, variable=scale_var)
    scale.pack(side=tk.LEFT, fill=tk.X, expand=True)
    
    entry = ttk.Entry(control_frame, width=4)  # Adjusted width to fit three digits
    entry.insert(0, str(min_val))
    entry.pack(side=tk.LEFT, padx=(5, 0))
    entry.bind('<Return>', lambda event, entry=entry, scale=scale: update_scale_from_entry(entry, scale))
    scale.config(command=lambda event, name=name, var=scale_var, entry=entry: update_entry_from_scale(name, var, entry))
    
    scales[name] = (scale, scale_var, entry)

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
