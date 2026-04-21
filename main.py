import subprocess
import threading
import sys
import queue
import time

def stream_to_queue(process_stdout, data_queue, prefix=""):
    """Reads lines from a subprocess and puts them into a queue."""
    for line in iter(process_stdout.readline, ""):
        if line:
            data_queue.put(prefix + line.strip())
    process_stdout.close()

def main():
    # 1. Start the 'Sensors' (Generator and Telemetry)
    gen_proc = subprocess.Popen(['python', 'generator.py'], stdout=subprocess.PIPE, text=True)
    tele_proc = subprocess.Popen(['python', 'telemetry.py'], stdout=subprocess.PIPE, text=True)

    # 2. Start the 'Brain' (C++ Scheduler)
    # Ensure your executable name matches (e.g., GreenCompute.exe on Windows)
    cpp_proc = subprocess.Popen(['./GreenCompute.exe'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)

    # 3. Start the 'Sink' (Visualizer)
    vis_proc = subprocess.Popen(['python', 'visualizer.py'], stdin=subprocess.PIPE, text=True)

    # Use a queue to merge Generator and Telemetry streams for C++
    input_queue = queue.Queue()

    # Threads to read from Python scripts without blocking each other
    threading.Thread(target=stream_to_queue, args=(gen_proc.stdout, input_queue, ""), daemon=True).start()
    threading.Thread(target=stream_to_queue, args=(tele_proc.stdout, input_queue, ""), daemon=True).start()

    output_queue = queue.Queue()

    threading.Thread(target=stream_to_queue, args=(cpp_proc.stdout, output_queue, ""), daemon=True).start()

    try:
        while True:
            # 1. Feed the Brain (C++ Input)
            try:
                data_in = input_queue.get_nowait()
                cpp_proc.stdin.write(data_in + "\n")
                cpp_proc.stdin.flush()
            except queue.Empty:
                pass

            # 2. Feed the Sink (Visualizer Input)
            try:
                # Get data from the C++ output queue
                cpp_data = output_queue.get_nowait()
                vis_proc.stdin.write(cpp_data + "\n")
                vis_proc.stdin.flush()
            except queue.Empty:
                pass

            # 3. Prevent 100% CPU usage
            time.sleep(0.01)

    except KeyboardInterrupt:
        print("\nShutting down GreenCompute system...")
    finally:
        gen_proc.terminate()
        tele_proc.terminate()
        cpp_proc.terminate()
        vis_proc.terminate()

if __name__ == "__main__":
    main()  