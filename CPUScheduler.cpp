#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <queue>
#include <memory>
#include <deque>
#include <sstream>
#include <thread>

using namespace std;

//enum for state of proccess for tasks
enum processState {
    READY, 
    RUNNING, 
    TERMINATED,
    HOLD,
    THROTTLED
};

//main class for process
class Process {


//variables for tasks, private to avoid accidental changes to variables
private:
    int processID; //number ID for process
    int arrivalTime; //time process arrives
    int burstTime; //total time process needs to run
    int remainingTime; //time left for process to complete
    processState state; //current state of the process
    int waitingTime; //total time process has waited
    int deadline; //deadline for when task needs to finish
    double powerDraw; //amount of power the process draws while running
    int memoryUsage; //the amount of memory this process will take from the datacenter while running




public:
//constructor for process class
    Process(int id, int arrival, int burst, int deadline, double power, int memory): 
    processID(id), arrivalTime(arrival), burstTime(burst), remainingTime(burst), deadline(deadline), 
    powerDraw(power), memoryUsage(memory) , state(READY), waitingTime(0) {}

    //pulls private variables into public scope and doesnt change variable with const
    int getProcessID() const {
        return processID;
    }

    int getArrivalTime() const {
        return arrivalTime;
    }

    int getRemainingTime() const {
        return remainingTime;
    }

    double getPowerDraw() const {
        return powerDraw;
    }

    bool isFinished() const {
        return remainingTime <= 0;
    }

    void updateWaitingTime(int time) {
        waitingTime += time;
    }

    void execute(int timeSlice) {
        state = RUNNING;
        remainingTime -= timeSlice;
        if (remainingTime <= 0) {
            remainingTime = 0;
            state = TERMINATED;
        }
    }
};

    //scheduler class to manage processes
class Scheduler {
private:
    deque<shared_ptr<Process>> readyQueue;
    int globalTime = 0;
    int timeQuantum;
    double currentCarbonIntensity = 200.0; // Default baseline

    // Non-blocking check for new tasks from the Python pipe
    void checkForNewTasks(bool& streamOpen) {
        while (cin && cin.peek() != EOF) {
            string line;
            if (getline(cin, line)) {
                if (line.empty()) continue;
                stringstream ss(line);
                int id, arr, burst, dl, mem;
                double pwr;
                if (ss >> id >> arr >> burst >> dl >> pwr >> mem) {
                    auto p = make_shared<Process>(id, arr, burst, dl, pwr, mem);
                    readyQueue.push_back(p);
                    // TELEMETRY: Notify Python of arrival
                    cout << "ARRIVAL," << globalTime << "," << id << "," << burst << "," << dl << "," << pwr << endl;
                }
            }
            if (cin.peek() == EOF) { streamOpen = false; break; }
            break; // Process one task at a time to keep clock moving
        }
    }

public:
    Scheduler(int quantum) : timeQuantum(quantum) {}

    void runSimulation() {
        bool streamOpen = true;

        while (streamOpen || !readyQueue.empty()) {
            checkForNewTasks(streamOpen);

            if (readyQueue.empty()) {
                if (streamOpen) {
                    globalTime++;
                    this_thread::sleep_for(chrono::milliseconds(1));
                    continue;
                } else break;
            }

            shared_ptr<Process> current = readyQueue.front();
            readyQueue.pop_front();

            int slice = min(timeQuantum, current->getRemainingTime());
            
            // TELEMETRY: Log execution slice for Energy Ledger
            cout << "EXEC," << globalTime << "," << current->getProcessID() << "," << slice << "," << current->getPowerDraw() << "," << currentCarbonIntensity << endl;

            current->execute(slice);
            globalTime += slice;

            for (auto& p : readyQueue) p->updateWaitingTime(slice);

            if (current->isFinished()) {
                cout << "FINISH," << globalTime << "," << current->getProcessID() << endl;
            } else {
                readyQueue.push_back(current);
            }
        }
    }
};

int main(int argc, char* argv[]) {
    int quantum = 20; // Default
    if (argc > 1) {
        quantum = stoi(argv[1]);
    }

    // Initialize Scheduler and start the simulation
    Scheduler cpu(quantum);
    cpu.runSimulation();

    return 0;
}