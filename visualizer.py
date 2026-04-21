import sys
import threading
import queue
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque

# --- CONFIGURATION & BUFFERS ---
WINDOW_SIZE = 100 
times = deque(maxlen=WINDOW_SIZE)
utilization = deque(maxlen=WINDOW_SIZE)
carbon_intensity = deque(maxlen=WINDOW_SIZE)
energy_ledger = 0.0

# Thread-safe queue for incoming data
data_queue = queue.Queue()

def read_stdin():
    """Background thread to read lines without blocking the GUI."""
    for line in sys.stdin:
        data_queue.put(line)

# Start the background reader thread
reader_thread = threading.Thread(target=read_stdin, daemon=True)
reader_thread.start()

# --- DASHBOARD SETUP ---
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 8), gridspec_kw={'hspace': 0.3})
fig.canvas.manager.set_window_title("GreenCompute Telemetry")

line_power, = ax1.plot([], [], label="Power Draw (W)", color='#1f77b4', linewidth=2)
line_carbon, = ax2.plot([], [], label="Grid Carbon Intensity", color='#2ca02c', linewidth=2)

ax1.set_title("Live Resource Utilization", fontsize=14, pad=10)
ax1.set_ylabel("Watts (W)", fontsize=10)
ax1.grid(True, linestyle='--', alpha=0.6)
ax1.legend(loc="upper right")

ax2.set_title("Environmental State", fontsize=14, pad=10)
ax2.set_ylabel("Intensity (gCO2/kWh)", fontsize=10)
ax2.set_xlabel("Simulation Time (Ticks)", fontsize=10)
ax2.grid(True, linestyle='--', alpha=0.6)
ax2.legend(loc="upper right")

def update_dashboard(frame):
    global energy_ledger
    
    data_processed = False
    
    # Empty the queue of all data that arrived since the last frame
    while not data_queue.empty():
        try:
            line = data_queue.get_nowait()
            parts = line.strip().split(",")
            
            if parts[0] == "EXEC":
                time_stamp = int(parts[1])
                power_draw = float(parts[4])
                intensity = float(parts[5])
                
                slice_hours = int(parts[3]) / 3600000.0
                carbon_grams = (power_draw * slice_hours) * intensity
                energy_ledger += carbon_grams
                
                times.append(time_stamp)
                utilization.append(power_draw)
                carbon_intensity.append(intensity)
                data_processed = True
        except Exception:
            pass # Ignore malformed lines during startup
            
    # Only redraw the graph if new data actually arrived
    if data_processed and times:
        line_power.set_data(times, utilization)
        line_carbon.set_data(times, carbon_intensity)

        ax1.set_xlim(max(0, times[-1] - WINDOW_SIZE), times[-1] + 10)
        ax2.set_xlim(max(0, times[-1] - WINDOW_SIZE), times[-1] + 10)
        
        ax1.set_ylim(0, max(utilization) * 1.2 if max(utilization) > 0 else 100)
        ax2.set_ylim(0, max(carbon_intensity) * 1.2 if max(carbon_intensity) > 0 else 500)

        fig.suptitle(f"Total Carbon Footprint: {energy_ledger:.4f} gCO2", fontsize=16, fontweight='bold')

    return line_power, line_carbon

ani = animation.FuncAnimation(fig, update_dashboard, interval=50, blit=False, cache_frame_data=False)

# Removed tight_layout() to resolve the UserWarning conflict
plt.subplots_adjust(top=0.88) 
plt.show()