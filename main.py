import subprocess
import threading
import sys
import queue

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

    try:
        # Loop to pipe merged data into C++ and C++ output into Visualizer
        while True:
            # Check for data to send to C++
            try:
                data_in = input_queue.get_nowait()
                cpp_proc.stdin.write(data_in + "\n")
                cpp_proc.stdin.flush()
            except queue.Empty:
                pass

            # Check for data coming out of C++ to send to Visualizer
            cpp_output = cpp_proc.stdout.readline()
            if cpp_output:
                vis_proc.stdin.write(cpp_output)
                vis_proc.stdin.flush()

            # Exit if the C++ process terminates
            if cpp_proc.poll() is not None:
                break

    except KeyboardInterrupt:
        print("\nShutting down GreenCompute system...")
    finally:
        gen_proc.terminate()
        tele_proc.terminate()
        cpp_proc.terminate()
        vis_proc.terminate()

if __name__ == "__main__":
    main()