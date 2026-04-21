import random
import time
import sys

def generate_controlled_workload(num_tasks):
    start_time = 0
    
    # Use a for loop to limit the data stream
    for task_id in range(1, num_tasks + 1):
        # 1. Random inter-arrival delay
        arrival_delay = random.uniform(0.5, 2.0)
        time.sleep(arrival_delay)
        start_time += arrival_delay
        
        # 2. Random Task Specs
        burst = random.randint(10, 500)
        # Ensuring the deadline is always possible (SLA logic)
        deadline = int(start_time + burst + random.randint(50, 200))
        power_draw = random.randint(10, 150)
        memory_usage = random.randint(256, 8192)
        
        # 3. Stream to Standard Output
        task_string = f"{task_id} {int(start_time)} {burst} {deadline} {power_draw} {memory_usage}"
        print(task_string, flush=True)

if __name__ == "__main__":
    # You can take the limit as a command line argument or hardcode it
    limit = 50 
    generate_controlled_workload(limit)