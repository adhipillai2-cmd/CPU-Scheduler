#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <queue>

using namespace std;

//enum for state of proccess for tasks
enum processState {
    READY, 
    RUNNING, 
    TERMINATED
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
        vector<Process> allProcesses; //list of processes to schedule

        vector<Process*> readyQueue; //queue of processes ready to run

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
        //loads processes from a file, in this its processes.txt
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

        //runs the scheduling simulation
        void runSimulation(){
            int completedProcesses = 0;

            //add processes that have arrived to the ready queue
            vector<bool> isProcessInQueue(allProcesses.size(), false);

            //main loop to run scheduling simulation until all tasks are completed
            while (completedProcesses < allProcesses.size()) {

                //loops through all processes
                for (int i = 0; i < allProcesses.size(); ++i) {
                    //checks if process has arrived and is not already in the ready queue
                    //then adds it to the ready queue through address pointers
                    if (allProcesses[i].getArrivalTime() <= globalTime && isProcessInQueue[i] == false) {
                        readyQueue.push_back(&allProcesses[i]);
                        //marks process as being in the ready queue
                        isProcessInQueue[i] = true;
                        cout << "Time " << globalTime << ": Process " << allProcesses[i].getProcessID() << " arrived." << endl;
                    }
                }

                //if ready queue is empty, increment global time and continue
                if (readyQueue.empty()){
                    globalTime++;
                    continue;
                }

                //gets the next process from the ready queue
                Process* currentProcess = readyQueue.front();
                readyQueue.erase(readyQueue.begin());
                
                //calculate how long to run
                int timeSlice = timeQuantum;
                if (currentProcess->getRemainingTime() < timeQuantum) {
                    timeSlice = currentProcess->getRemainingTime();
                }

                //execute the current process for the calculated time slice
                currentProcess->execute(timeSlice);
                globalTime += timeSlice;


                // Loop through everyone currently in the line
                for (int i = 0; i < readyQueue.size(); i++) {
                    // readyQueue[i] is a pointer so ->
                    readyQueue[i]->updateWaitingTime(timeSlice);
                }

                //check if current process is finished
                if (currentProcess->isFinished()) {
                    completedProcesses++;
                    cout << "Time " << globalTime << ": Process " << currentProcess->getProcessID() << " finished." << endl;
                } 
                else 
                {
                    readyQueue.push_back(currentProcess);
                }
            }
        };
        
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
};


int main() {
    int quantum;
    
    
    //gets the Time Quantum from the user
    cout << "Enter Time Quantum: ";
    cin >> quantum;

    //creates the Scheduler Object
    Scheduler cpu(quantum);
    
    //asks user if they want to update the process file
    cpu.fileUpdater();

    cpu.loadProcessesFromFile("processes.txt");

    //runs the Simulation
    cpu.runSimulation();

    //prints the final statistics
    cpu.printStatistics();

    return 0;
}