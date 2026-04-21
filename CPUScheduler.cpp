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
    float powerDraw; //amount of power the process draws while running
    int memoryUsage; //the amount of memory this process will take from the datacenter while running




public:
//constructor for process class
    Process(int id, int arrival, int burst) {
        processID = id; 
        arrivalTime = arrival;
        burstTime = burst;

        remainingTime = burst; //initially remaining time is equal to burst time
        state = READY; //default state is READY
        waitingTime = 0; //has not waited yet so waiting time is 0
    }


    //pulls private variables into public scope and doesnt change variable with const
    int getProcessID() const {
        return processID;
    }

    int getArrivalTime() const {
        return arrivalTime;
    }

    int getBurstTime() const {
        return burstTime;
    }

    int getRemainingTime() const {
        return remainingTime;
    }

    int getWaitingTime() const {
        return waitingTime;
    }

    float getPowerDraw() const {
        return powerDraw;
    }

    //checks if process is finished
    bool isFinished()  const {
        return remainingTime <= 0;
    }

    //updates waiting time of process
    void updateWaitingTime(int time) {
        waitingTime += time;
    }

    //executes process for a given time slice
    void execute(int timeSlice){
        //checks if process is not terminated
        //then checks if remaining time is greater than time slice, if so subtracts time slice from remaining time
        //otherwise sets remaining time to 0 and changes state to TERMINATED
        if (state != TERMINATED) {
            state = RUNNING;
            if (remainingTime > timeSlice) {
                remainingTime -= timeSlice;
            } 
            else {
                remainingTime = 0;
                state = TERMINATED;
            }
        }
    }
};

    //scheduler class to manage processes
    class Scheduler {
    //private variables/vectors for allProcesses and readyQueue
    private:
        
        std::vector<std::shared_ptr<Process>> allProcesses; //list of processes to schedule

        std::deque<std::shared_ptr<Process>> readyQueue; //queue of processes ready to run

        //variables
        int globalTime; //current time in the scheduling simulation
        int timeQuantum; //time slice for round-robin scheduling

    //public
    public:
    //constructor for scheduler class that sets time for processes based on user input
        Scheduler(int quantum){
            timeQuantum = quantum;
            globalTime = 0;

        }
        //loads processes from a file, in this its processes.
        void checkForNewTasks(){
            while (std::cin && std::cin.peek() != EOF) {
                std::string line;
                if (std::getline(std::cin, line)) {
                    if (line.empty()) continue;

                    std::stringstream ss(line);
                    int id, arrival, burst, deadline, power, memory;
                    
                    if (ss >> id >> arrival >> burst >> deadline >> power >> memory) {
                        // Create a shared_ptr managed process
                        auto newProcess = std::make_shared<Process>(
                            id, arrival, burst, deadline, power, memory
                        );
                        
                        allProcesses.push_back(newProcess);
                        std::cout << "[Time " << globalTime << "] Task " << id 
                                << " ingested from live stream." << std::endl;
                    }
                }
                
                // After reading one line, peek again to see if more are waiting.
                // If the pipe is empty, we break to let the simulation clock tick.
                if (std::cin.peek() == EOF || std::cin.rdbuf()->in_avail() == 0) {
                    break;
                }
            }
        }
        /*
        
        
        void loadProcessesFromFile(string filename) { 
            ifstream file(filename);
            //checks if file opens correctly
            if (!file.is_open()) {
                cerr << "Error opening file " << filename << endl;
                return;
            }
            
            int id, arrival, burst;
            //checks for the variables in the file and loads them into process class
            while (file >> id >> arrival >> burst) {
                Process temp (id, arrival, burst);
                allProcesses.push_back(temp);
            }
            file.close();

            //informs that the file was loaded correctly, and displays number of processes loaded
            cout <<"Loaded " << allProcesses.size() << " processes from " << filename << endl;
        }
            */



        //runs the scheduling simulation

        void runSimulation() {
            int completedProcesses = 0;
            bool streamActive = true;

            // Condition: Continue if the pipe is open OR there is work left to do
            while (streamActive || !readyQueue.empty()) {
                
                // 1. NON-BLOCKING INGESTION
                // Instead of a static vector, we poll the live feed every iteration
                if (streamActive) { checkForNewTasks(); }

                // 2. DISPATCH LOGIC
                if (readyQueue.empty()) {
                    if (streamActive) {
                        globalTime++; // Clock keeps ticking while waiting for input
                        std::this_thread::sleep_for(std::chrono::milliseconds(1)); 
                        continue;
                    } else {
                        break; // Stream closed and queue empty
                    }
                }

                // 3. SMART POINTER DISPATCH (O(1))
                // Replace raw pointer erasures with deque pop_front
                std::shared_ptr<Process> current = readyQueue.front();
                readyQueue.pop_front();

                // 4. POWER-AWARE EXECUTION
                int slice = std::min(timeQuantum, current->getRemainingTime());
                current->execute(slice);
                
                // Energy tracking: You'll need to store this in your ledger vector
                double currentEnergy = current->getPowerDraw() * slice;
                
                globalTime += slice;

                // 5. UPDATE WAIT TIMES & REQUEUE
                // Your current for-loop for waitingTime remains valid logic, 
                // but must use smart pointers.
                for (auto& p : readyQueue) {
                    p->updateWaitingTime(slice);
                }

                if (current->isFinished()) {
                    completedProcesses++;
                    std::cout << "Time " << globalTime << ": Task " << current->getProcessID() << " finished." << std::endl;
                } else {
                    readyQueue.push_back(current);
                }
            }
        }

        /*
        
        
        
        //prints the final statistics of the scheduling simulation
        void printStatistics() {
            cout << "\nFINAL STATISTICS:" << endl;
            cout << "PID\tArrival\tBurst\tFinish\tTurnaround\tWaiting" << endl;
            
            double totalWait = 0;
            double totalTurnaround = 0;

            for (int i = 0; i < allProcesses.size(); i++) {
                Process p = allProcesses[i];
                
                //calculates the turnaround by adding burst time and waiting time
                //calculates finish time by adding arrival time and turnaround time
                int turnaround = p.getBurstTime() + p.getWaitingTime();
                int finishTime = p.getArrivalTime() + turnaround;
                
                //adds to totals
                totalWait += p.getWaitingTime();
                totalTurnaround += turnaround;

                //prints the row
                cout << p.getProcessID() << "\t" << p.getArrivalTime() << "\t" << p.getBurstTime() << "\t" << finishTime << "\t" << turnaround << "\t" << p.getWaitingTime() << endl;
            }

            //averages
            cout << "Average Waiting Time: " << (totalWait / allProcesses.size()) << " ms" << endl;
            cout << "Average Turnaround Time: " << (totalTurnaround / allProcesses.size()) << " ms" << endl;
        }

        */

        /*
        
        

        void fileUpdater(){
            //asks user if they want to update the processes texgt file
            char choice;
            cout << "would you like to update the process file? (y/n): ";
            cin >> choice;

            //given option to say no
            if (choice == 'y' || choice == 'Y') {
                //opens the file and will APPEND only to avoid overwriting existing tasks
                ofstream file("processes.txt", ios::app);

                //checks if file opened correctly for saftey precautions 
                if (!file.is_open()) {
                    cout << "Error opening file for writing." << endl;
                    return;
                }

                //asks for for data on the new tasks taht the user wants to input
                int id, arrival, burst;
                cout << "Enter Process Details (Enter ID -1 to stop adding)." << endl;
            
                //as long as the user doesnt break this loop will keep running
            while (true) {
                cout << "\nNew ID: ";
                cin >> id;
                
                //the break condition
                if (id == -1) break;

                //asks for arrival and burst times
                cout << "Arrival Time: ";
                cin >> arrival;
                cout << "Burst Time: ";
                cin >> burst;

                //adds a newline first to ensure it doesn't merge with the previous line
                file << endl << id << " " << arrival << " " << burst;
                
                //informs user that the process was saved
                cout << "Saved Process " << id << " to file." << endl;
            }

            file.close();
            cout << "File update complete.\n" << endl;
        }
            
    }
    */
};

int main(int argc, char* argv[]) {
    // Default quantum or take from command line for automation
    int quantum = 100; 
    if (argc > 1) {
        quantum = std::stoi(argv[1]);
    }

    Scheduler cpu(quantum);
    
    // The simulation now starts immediately without asking for file updates
    cpu.runSimulation();

    return 0;
}