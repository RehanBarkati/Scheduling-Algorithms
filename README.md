# Scheduling-Algorithms

 In this assignment,  I build own discrete-event process scheduling simulator for a general OS. It have the following properties 
 
 1. Processes are generated randomly in time. The inter-arrival time between two processes is 
chosen from an exponential distribution, whose meantime can be set as a parameter of the 
simulator.
2. Each process have the following attributes

    a. PID b. Time generated c. Expected time to completion
  
4. Implemented the following scheduling strategies to organize process execution

   a. First come first serve
   
   b. Round robin (with parametrized time-slice)
   
   c. Shortest Job first
   
   d. Shortest remaining time first
   
   e. Multi-level feedback queue (with parametrized time-slices, 3 queues) 

6. Maintained a Gantt chart for process execution. Used appropriate visualization tools to demonstrate the execution of processes, their turnaround times and average turnaround time, average response 
time. Experimented with choosing different values of parameters. Give detailed insights to explain your observations

Input format 

The program accepts input from a text file, the path to which will be specified as a command line argument. The input consists for one workload of processes. Each line of the input file represents a process. The structure of the line is as follows:

   PID ArrivalTime JobTime

Program will accept this workload, and then run all the 5 scheduling algorithms on it. It produces an output which, for every context switch, switch specifies which the PID of the job running, the start time of that context, and the time of the switch. This can be understood as the blueprint of a gantt chart.

An example will be as following:

PID ArrivalTime JobTime

P1 0 18
P2 2 7
P3 2 10

Then for FCFS statergy, the blueprint to be given as output looks like this:

P1 0 18 P2 18 25 P3 25 35

It starts with the PID of the job executed first, the time at which it starts, and then the time at which the first context switch occurs. Then for the next context, it again outlines which process is running, the start time of the context and the end time for that context, and so on.

Hence, the output will look like :

FCFS blue print

AvgTurnAroundTime AvgResponseTime for FCFS

Round robin blue print

AvgTurnAroundTime AvgResponseTime for Round Robin

Shortest Job first blue print

AvgTurnAroundTime AvgResponseTime for Shortest Job first

Shortest remaining time first blue print

AvgTurnAroundTime AvgResponseTime for Shortest remaining time first

Multi-level feedback queue blue print

AvgTurnAroundTime AvgResponseTime for Multi-level feedback queue

The command line arguments to your code will consist of the following:

1. The input file path
  
2. The output file path
   
3. The time slice for round robin (TsRR)
   
4. The time slice for highest priority MLFQ (TsMLFQ1)

5. The time slice for the 2nd queue for MLFQ (TsMLFQ2)

6. The time slice for the 3rd queue for MLFQ (TsMLFQ3)

7. The boost parameter for the MLFQ (BMLFQ).
   
Hence the calls will look like this:

$ gcc main.c.cpp -o main

$ ./main input.txt output.txt TsRR TsMLFQ1 TsMLFQ2 TsMLFQ3 BMLFQ


