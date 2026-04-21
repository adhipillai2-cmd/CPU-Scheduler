import sys
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import pandas as pd
from collections import deque

# --- CONFIGURATION ---
WINDOW_SIZE = 50  # Number of time-steps to show on the live chart
# Buffers to store data for visualization
times = deque(maxlen=WINDOW_SIZE)
utilization = deque(maxlen=WINDOW_SIZE)
carbon_intensity = deque(maxlen=WINDOW_SIZE)
energy_ledger = 0.0

def update_dashboard(frame):
    """Parses incoming C++ logs and updates the live plots."""
    global energy_ledger
    
    # Read the next line from the pipe
    line = sys.stdin.readline()
    if not line:
        return
    
    parts = line.strip().split(",")
    
    # LOGIC: Parse the Telemetry Format we defined in C++
    # Example: EXEC,Time,PID,Slice,Power,Carbon
    if parts[0] == "EXEC":
        time_stamp = int(parts[1])
        power_draw = float(parts[4])
        intensity = float(parts[5])
        
        # Calculate CO2 grams for this specific slice
        # Energy (Wh) = Power (W) * Time (hours)
        # Note: 'Slice' is in ms, so divide by 3,600,000 for hours
        slice_hours = int(parts[3]) / 3600000
        carbon_grams = (power_draw * slice_hours) * intensity
        energy_ledger += carbon_grams
        
        # Update plot buffers
        times.append(time_stamp)
        utilization.append(power_draw) # Use Power as a proxy for utilization
        carbon_intensity.append(intensity)

    # --- PLOTTING LOGIC ---
    plt.cla()
    plt.subplot(2, 1, 1)
    plt.plot(list(times), list(utilization), label="Power Draw (W)", color='blue')
    plt.title(f"Live Resource Utilization | Total CO2: {energy_ledger:.4f}g")
    plt.legend()

    plt.subplot(2, 1, 2)
    plt.plot(list(times), list(carbon_intensity), label="Grid Carbon Intensity", color='green')
    plt.legend()

# Setup Animation
fig = plt.figure(figsize=(10, 6))
ani = animation.FuncAnimation(fig, update_dashboard, interval=100)
plt.tight_layout()
plt.show()