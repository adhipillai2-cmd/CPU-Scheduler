import random
import time
import sys

def generate_processes(num_processes):
    task_id = 0
    start_time = 0

    try:
        for i in range(num_processes):
            # 1. Random inter-arrival delay (Simulates live traffic)
            arrival_delay = random.uniform(0.5, 2.0)
            time.sleep(arrival_delay)
            start_time += arrival_delay
            
            # 2. Random Task Specs
            burst = random.randint(10, 500)      # Time needed to run
            # Deadline logic: Current Time + Burst + Slack
            deadline = int(start_time + burst + random.randint(50, 200))
            power_draw = random.randint(10, 150) # Watts
            memory_usage = random.randint(256, 8192) # MB
            
            # 3. Stream to Standard Output
            # We use flush=True so C++ receives it instantly
            task_string = f"{task_id} {int(start_time)} {burst} {deadline} {power_draw} {memory_usage}"
            print(task_string, flush=True)
            
            task_id += 1
            
    except KeyboardInterrupt:
        sys.exit(0)

if __name__ == "__main__":
    generate_workload()