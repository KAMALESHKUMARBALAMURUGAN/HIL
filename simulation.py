import tkinter as tk
from tkinter import ttk
import pandas as pd
import time
import serial
from time import sleep

# Setup serial connection
ser = serial.Serial('COM3', 115200)  # Adjust COM port and baud rate as needed

# Load the simulation data
simulation_data = pd.read_csv(r"C:\Lectrix_company\work\Data\Feature_data\Daily_Analysis\MAIN_FOLDER\MAR_21\log_file_chronological order.csv")
 
 
# Clean and prepare the data
simulation_data = simulation_data.dropna(subset=['SOC_8', 'MotorSpeed_340920578'])
simulation_data['SOC_8'] = pd.to_numeric(simulation_data['SOC_8'], errors='coerce')
simulation_data['MotorSpeed_340920578'] = pd.to_numeric(simulation_data['MotorSpeed_340920578'], errors='coerce')
simulation_data = simulation_data.dropna(subset=['SOC_8', 'MotorSpeed_340920578'])
simulation_data.reset_index(drop=True, inplace=True)  # Reset indices after modifications
 
# Function to send data over UART
def send_uart(id, value):
    """Send parameter id and value over UART."""
    cmd = f'{id}{value}'  # Include newline or other terminator if needed
    ser.write(cmd.encode())
    print(f'Sending over UART: {cmd.encode()}')

# Function to run the simulation
def run_simulation():
    run_button.config(state=tk.DISABLED)
    timestamps = simulation_data['timestamp']
    start_time = timestamps.min()
    relative_times = (timestamps - start_time) / 1000
    send_uart('7', f'{100}') # Battery should not be empty initially
    sleep(5)
    send_uart('1', f'{1}')
    sleep(1)
    send_uart('6', f'{1}')
    sleep(1)
    send_uart('6', f'{0}')
    sleep(1)
    send_uart('1', f'{0}')
    sleep(1)
    previous_SOC_8 = 100  # Initialize previous SOC_8 value
    for i in range(len(simulation_data) - 1):
        SOC_8 = simulation_data.at[i, 'SOC_8']
        motor_speed = simulation_data.at[i, 'MotorSpeed_340920578']
        if pd.notna(SOC_8) and pd.notna(motor_speed):
            soc_scale.set(float(SOC_8))
            motor_speed_scale.set(float(motor_speed))
            root.update()
            print("Prev_SOC", previous_SOC_8)
            print("SOC_8", SOC_8)
            if SOC_8 != previous_SOC_8: # Check if SOC_8 has changed from the previous value
                send_uart('7', f'{int(SOC_8)}')  # Send SOC_8 value as an integer over UART only if it changed
                sleep(5)
                previous_SOC_8 = SOC_8  # Update previous SOC_8 value
            send_uart('d', f'{motor_speed}')  # Send motor speed value over UART
            time_to_next = relative_times.iloc[i + 1] - relative_times.iloc[i]
            time.sleep(time_to_next)
   
    run_button.config(state=tk.NORMAL)
 
# Create the main window
root = tk.Tk()
root.title("SOC_8 and Motor Speed Simulation GUI")
 
# Create labels and sliders for SOC_8 and Motor Speed
label_soc = ttk.Label(root, text="SOC_8 (State of Charge)")
label_soc.pack(pady=10)
soc_scale = ttk.Scale(root, from_=0, to=100, orient='horizontal')
soc_scale.pack(fill='x', padx=20)
 
label_motor_speed = ttk.Label(root, text="Motor Speed (rpm)")
label_motor_speed.pack(pady=10)
motor_speed_scale = ttk.Scale(root, from_=0, to=5000, orient='horizontal')
motor_speed_scale.pack(fill='x', padx=20)
 
# Create a Run button
run_button = ttk.Button(root, text="Run Simulation", command=run_simulation)
run_button.pack(pady=20)
 
# Start the GUI
root.mainloop()