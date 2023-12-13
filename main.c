#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

FILE *outputFile;
float tolerance=0.05;

struct Process {
    char PID[50];
    float arrival_time;
    float burst_time;
	float response_time;
	float turnaround_time;
    float remaining_time;
    bool inqueue;
};
struct context{
	char PID[50];
	float start_time;
	float end_time;
};

// Queue data structure
struct Queue {
    int *array;
    int front, rear;
    unsigned capacity;
};

// Function to create a new queue
struct Queue *createQueue(unsigned capacity) {
    struct Queue *queue = (struct Queue *)malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->rear = -1;
    queue->array = (int *)malloc(capacity * sizeof(int));
    return queue;
}

// Function to check if the queue is empty
int isEmpty(struct Queue *queue) {
    return queue->front == -1;
}

// Function to check if the queue is full
int isFull(struct Queue *queue) {
    return (queue->rear + 1) % queue->capacity == queue->front;
}

// Function to enqueue a process
void enqueue(struct Queue *queue, int contextSwitches) {
    if (isFull(queue))
        return;
    if (isEmpty(queue))
        queue->front = 0;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = contextSwitches;
}

// Function to dequeue a process
int dequeue(struct Queue *queue) {
    int contextSwitches=-1;
    if (isEmpty(queue))
        return contextSwitches;
    contextSwitches = queue->array[queue->front];
    if (queue->front == queue->rear)
        queue->front = queue->rear = -1;
    else
        queue->front = (queue->front + 1) % queue->capacity;
    return contextSwitches;
}
struct MLFQ {
    struct Queue *queues[3]; 
    int quantum[3];        
};

struct MLFQ *createMLFQ(int TsRR1, int TsRR2, int TsRR3,int n) {
    struct MLFQ *mlfq = (struct MLFQ *)malloc(sizeof(struct MLFQ));
    for (int i = 0; i < 3; i++) {
        mlfq->queues[i] = createQueue(n);
        mlfq->quantum[i] = (i == 0) ? TsRR1 : ((i == 1) ? TsRR2 : TsRR3);
    }
    return mlfq;
}

void destroyMLFQ(struct MLFQ *mlfq) {
    for (int i = 0; i < 3; i++) {
        free(mlfq->queues[i]->array);
        free(mlfq->queues[i]);
    }
    free(mlfq);
}
int min(int x,int y){
     if(x<y){
        return x;
     }
     return y;
}
int sortByArrivalTime(const void *a, const void *b) {
    const struct Process *process1 = (const struct Process *)a;
    const struct Process *process2 = (const struct Process *)b;
    
    if (process1->arrival_time < process2->arrival_time) 
        return -1;
    else if (process1->arrival_time > process2->arrival_time)
        return 1;
    else 
        return strcmp(process1->PID, process2->PID);
}
void readInput(char *filename, struct Process **process, int *count) {
    FILE *inputFile = fopen(filename, "r");
    if (inputFile == NULL) {
        perror("Error opening file");
        return;
    }
   
    *count = 0;
    int capacity = 1;

    char line[50]; 
    while (fgets(line, sizeof(line), inputFile) != NULL) {
        char pid[15];
        float arrivalTime, jobTime;
        if (sscanf(line, "%s %f %f", pid, &arrivalTime, &jobTime) != 3) {
            fprintf(stderr, "Error parsing line: %s", line);
            continue;
        }

        struct Process newProcess;
        strcpy(newProcess.PID, pid); 
        newProcess.arrival_time = arrivalTime;
        newProcess.burst_time =  jobTime;
        newProcess.remaining_time = jobTime;
        newProcess.response_time = -1;
        newProcess.inqueue = false;

        if (*count == capacity-1) {
            capacity *= 2; // Double the capacity
            *process = realloc(*process, capacity * sizeof(struct Process));
            if (*process == NULL) {
                perror("Error reallocating memory");
                fclose(inputFile);
                return;
            }
        }

        (*process)[(*count)++] = newProcess;
    }

    fclose(inputFile);
}
void writeOutput(struct Process processes[],struct context Contexts[],int n,int contextSwitches){

    int i=0;
    while(i<contextSwitches){
        int j=i+1;
        while(j<contextSwitches && strcmp(Contexts[i].PID,Contexts[j].PID)==0){
            j++;
        }
		fprintf(outputFile,"%s %.1f %.1f ", Contexts[i].PID, Contexts[i].start_time, Contexts[j-1].end_time);
        i=j;
    }
	fprintf(outputFile,"\n");
	float avg_turnaround_time = 0.0;
	float avg_response_time = 0.0;
	
	
	for(int i=0;i<n;i++){
		avg_turnaround_time = avg_turnaround_time + processes[i].turnaround_time;
		avg_response_time = avg_response_time + processes[i].response_time;
	}
	
	avg_turnaround_time = avg_turnaround_time/n;
	avg_response_time = avg_response_time/n;
	
    fprintf(outputFile,"%.3f %.3f \n",avg_turnaround_time,avg_response_time);
}

void FCFS(struct Process processes[], int n){
	
	qsort(processes, n, sizeof(struct Process), sortByArrivalTime);
	int contextSwitches = n;
	struct context Contexts[n];
	float curr_time=0;

	
	for(int i=0;i<n;i++){
        if(curr_time<processes[i].arrival_time){
            curr_time=processes[i].arrival_time;
        }
		struct context c;
		strcpy(c.PID, processes[i].PID);
		c.start_time = curr_time;
        curr_time+=processes[i].burst_time;
		c.end_time = curr_time;
		Contexts[i] = c;
        
        processes[i].response_time=curr_time-processes[i].arrival_time;
        processes[i].turnaround_time= curr_time-processes[i].arrival_time;
	}
    writeOutput(processes,Contexts,n,contextSwitches);
}

void RoundRobin(struct Process processes[], int n, float timeSlice) {
    qsort(processes, n, sizeof(struct Process), sortByArrivalTime);
    int contextCapacity = 1;
    struct context *Contexts = (struct context *)malloc(sizeof(struct context) * contextCapacity);
    int contextSwitches=0;
    float currentTime = 0;
    struct Queue *queue = createQueue(n);
    int completedProcess=0;
    bool firstProcess=false;
    if(processes[0].arrival_time<currentTime+tolerance){
        enqueue(queue,0);
        processes[0].inqueue=true;
    }
    while (completedProcess<n) {
        
        int p = dequeue(queue);
        if(p==-1){
            float nextArrivalTime=(float)INT_MAX;
            for(int i=0;i<n;i++){
                if(processes[i].arrival_time+tolerance>currentTime){
                    nextArrivalTime=min(nextArrivalTime,processes[i].arrival_time);
                }
            }
            currentTime=nextArrivalTime;
            // printf("Next arrival time: %f\n",nextArrivalTime);
            for (int i = 0; i < n; i++) {
                if (processes[i].remaining_time > tolerance && processes[i].arrival_time <= currentTime+tolerance && !processes[i].inqueue) {
                    enqueue(queue, i);
                    processes[i].inqueue=true;
                }
            }
        }
        else{
            float executionTime = (processes[p].remaining_time < timeSlice+tolerance) ? processes[p].remaining_time : timeSlice;

            if (contextSwitches + 1 >= contextCapacity) {
                contextCapacity *= 2;
                Contexts = (struct context *)realloc(Contexts, sizeof(struct context) * contextCapacity);
            }
            
            strcpy(Contexts[contextSwitches].PID, processes[p].PID);
            Contexts[contextSwitches].start_time = currentTime;
            Contexts[contextSwitches++].end_time = currentTime + executionTime;

            // Process is running for 'executionTime' time units
            processes[p].remaining_time -= executionTime;

            if (processes[p].response_time < 0) {
                processes[p].response_time = currentTime - processes[p].arrival_time;
            }

            currentTime += executionTime;
            for (int i = 0; i < n; i++) {
                if (processes[i].remaining_time > tolerance && processes[i].arrival_time <= currentTime+tolerance && !processes[i].inqueue) {
                    enqueue(queue, i);
                    processes[i].inqueue=true;
                }
            }
            if (processes[p].remaining_time > tolerance) {
                enqueue(queue,p);
            } else {
                processes[p].turnaround_time = currentTime - processes[p].arrival_time;
                completedProcess++;
            }
        
        }     
    }

    writeOutput(processes,Contexts,n,contextSwitches);
    free(queue->array);
    free(queue);
    free(Contexts);
}

int findShortestJob(struct Process processes[],int n, float currentTime){

	 float shortestBurstTime = (float)INT_MAX;
	 int shortestJobIndex = -1;
	 
	 for(int i=0;i<n;i++){
		 if(processes[i].arrival_time <= currentTime+tolerance && processes[i].burst_time < shortestBurstTime && processes[i].remaining_time > tolerance){
			 shortestJobIndex = i;
			 shortestBurstTime = processes[i].burst_time;
		 }
	 }
	 return shortestJobIndex;
}
void SJF(struct Process processes[], int n){
	
	struct context Contexts[n];
	float currentTime = 0;
	int completed = 0;
	int contextSwitches = 0;

	while(completed < n){
		int shortestJobIndex = findShortestJob(processes, n, currentTime);
		if(shortestJobIndex == -1){
			float nextArrivalTime=(float)INT_MAX;
            for(int i=0;i<n;i++){
                if(processes[i].arrival_time+tolerance>currentTime){
                    nextArrivalTime=min(nextArrivalTime,processes[i].arrival_time);
                }
            }
            currentTime=nextArrivalTime;
        }
		else{
			struct context c;
			strcpy(c.PID, processes[shortestJobIndex].PID);
			c.start_time = currentTime;
			c.end_time = currentTime + processes[shortestJobIndex].burst_time;
			Contexts[contextSwitches++] = c;
			
			processes[shortestJobIndex].response_time = currentTime - processes[shortestJobIndex].arrival_time;
			currentTime = currentTime + processes[shortestJobIndex].burst_time;
			processes[shortestJobIndex].turnaround_time = currentTime - processes[shortestJobIndex].arrival_time;
			
			processes[shortestJobIndex].remaining_time = 0;
			completed++;
		}
	}
	writeOutput(processes,Contexts,n,contextSwitches);
}
int findShortestRemainingTime(struct Process processes[], int n, int currentTime) {

    int shortestTime = INT_MAX;
    int shortestIndex = -1;

    for (int i = 0; i < n; i++) {
        if (processes[i].arrival_time <= currentTime && processes[i].remaining_time < shortestTime && processes[i].remaining_time > 0) {
            shortestTime = processes[i].remaining_time;
            shortestIndex = i;
        }
    }

    return shortestIndex;
}
void SRTF(struct Process processes[], int n) {
    
    struct context *Contexts = (struct context *)malloc(sizeof(struct context) * (1));
    int contextCapacity = 1;
    int contextSwitches=-1;
    int currentTime = 0;
    int completedProcesses = 0;
    bool isRunning = false;
    int runningProcess = -1;
    
    while (completedProcesses < n) {
        // Find the process with the shortest remaining time
        int shortestIndex = findShortestRemainingTime(processes, n, currentTime);

        if (shortestIndex == -1) {
            int nextArrivalTime=INT_MAX;
            for(int i=0;i<n;i++){
                if(processes[i].arrival_time>currentTime){
                    nextArrivalTime=min(nextArrivalTime,processes[i].arrival_time);
                }
            }
            currentTime=nextArrivalTime;
        } 
        else {
            if (!isRunning || runningProcess != shortestIndex) {
                if(contextSwitches!=-1){
                    // printf("Context switch is happening\n");
                    Contexts[contextSwitches].end_time=currentTime;
                }
                if (contextSwitches + 1 >= contextCapacity) {
                    // Resize the Contexts array when it reaches its capacity
                    contextCapacity *= 2;
                    Contexts = (struct context *)realloc(Contexts, sizeof(struct context) * contextCapacity);
                }
                strcpy(Contexts[++contextSwitches].PID,processes[shortestIndex].PID);
                Contexts[contextSwitches].start_time=currentTime;
                isRunning = true;
            }
            if (processes[shortestIndex].response_time == -1) {
                processes[shortestIndex].response_time = currentTime - processes[shortestIndex].arrival_time;
            }
            int nextarrivalTime=INT_MAX;
            for(int i=0;i<n;i++){
                if(processes[i].arrival_time>currentTime){
                    nextarrivalTime=min(nextarrivalTime,processes[i].arrival_time);
                }
            }
            int executionTime= min(processes[shortestIndex].remaining_time,nextarrivalTime);
            processes[shortestIndex].remaining_time-=executionTime;
            currentTime+=executionTime;

            if (processes[shortestIndex].remaining_time == 0) {
                // If the process is completed, update completion and turnaround times
                completedProcesses++;
                isRunning = false;
                processes[shortestIndex].turnaround_time = currentTime - processes[shortestIndex].arrival_time;
            }

            runningProcess = shortestIndex;
        }
    }
    Contexts[contextSwitches++].end_time=currentTime;
    writeOutput(processes,Contexts,n,contextSwitches);
    free(Contexts);
}
void MLFQ(struct Process processes[], int n, int TsRR1, int TsRR2, int TsRR3, int boost_time) {
    struct MLFQ *mlfq = createMLFQ(TsRR1, TsRR2, TsRR3,n);
    int contextCapacity = 1; 
    int contextSwitches = 0;
    struct context *Contexts = (struct context *)malloc(sizeof(struct context) * contextCapacity);
    int lastBoostTime = 0; 
    int completedProcesses = 0;

    qsort(processes, n, sizeof(struct Process), sortByArrivalTime);
    int currentTime = 0;

    while (completedProcesses < n) {
        bool processExecuted = false;

        if (!isEmpty(mlfq->queues[0])) {
            int processIndex = dequeue(mlfq->queues[0]);
            struct Process *process = &processes[processIndex];
            
            int executionTime = (process->remaining_time < mlfq->quantum[0]) ? process->remaining_time : mlfq->quantum[0];
            if (contextSwitches + 1 >= contextCapacity) {
                contextCapacity *= 2;
                Contexts = (struct context *)realloc(Contexts, sizeof(struct context) * contextCapacity);
            }
            // printf("Q0 is not empty , id: %s\n",process->PID);
            // Capture the context change
            strcpy(Contexts[contextSwitches].PID, process->PID);
            Contexts[contextSwitches].start_time = currentTime;
            Contexts[contextSwitches++].end_time = currentTime + executionTime;
            process->remaining_time -= executionTime;

            // Update response time if needed
            if (process->response_time<0) {
                process->response_time = currentTime - process->arrival_time;
            }

            currentTime += executionTime;
            if (process->remaining_time == 0) {
                process->turnaround_time = currentTime - process->arrival_time;
                completedProcesses++;
            } 
            else {
                enqueue(mlfq->queues[1], processIndex);
            }
            processExecuted = true;
        } 
        else if (!isEmpty(mlfq->queues[1])) {
    
            int processIndex = dequeue(mlfq->queues[1]);
            struct Process *process = &processes[processIndex];

            int executionTime = (process->remaining_time < mlfq->quantum[1]) ? process->remaining_time : mlfq->quantum[1];

            if (contextSwitches + 1 >= contextCapacity) {
                contextCapacity *= 2;
                Contexts = (struct context *)realloc(Contexts, sizeof(struct context) * contextCapacity);
            }

            // Capture the context change
            strcpy(Contexts[contextSwitches].PID, process->PID);
            Contexts[contextSwitches].start_time = currentTime;
            Contexts[contextSwitches++].end_time = currentTime + executionTime;
            process->remaining_time -= executionTime;

            if (!process->response_time<0) {
                process->response_time = currentTime - process->arrival_time;
            }

            // Check if the process is finished
            currentTime += executionTime;
            if (process->remaining_time == 0) {
                process->turnaround_time = currentTime - process->arrival_time;
                completedProcesses++;
            } 
            else {
                enqueue(mlfq->queues[2], processIndex);
            }

            processExecuted = true;
         } 
        else if (!isEmpty(mlfq->queues[2])) {
    
            int processIndex = dequeue(mlfq->queues[2]);
            struct Process *process = &processes[processIndex];

            int executionTime = (process->remaining_time < mlfq->quantum[2]) ? process->remaining_time : mlfq->quantum[2];

            if (contextSwitches + 1 >= contextCapacity) {
                contextCapacity *= 2;
                Contexts = (struct context *)realloc(Contexts, sizeof(struct context) * contextCapacity);
            }
         
            // Capture the context change
            strcpy(Contexts[contextSwitches].PID, process->PID);
            Contexts[contextSwitches].start_time = currentTime;
            Contexts[contextSwitches++].end_time = currentTime + executionTime;
            process->remaining_time -= executionTime;

            if (process->response_time<0) {
                process->response_time = currentTime - process->arrival_time;
            }

            // Check if the process is finished
            currentTime += executionTime;
            if (process->remaining_time == 0) {
                process->turnaround_time = currentTime - process->arrival_time;
                completedProcesses++;
            } else {
                enqueue(mlfq->queues[2], processIndex);
            }

            processExecuted = true;
        }
        
        if (!processExecuted) {
            int nextArrivalTime=INT_MAX;
            for(int i=0;i<n;i++){
                if(processes[i].arrival_time>=currentTime){
                    nextArrivalTime=min(nextArrivalTime,processes[i].arrival_time);
                }
            }
            // printf("Next arrival time: %d\n",nextArrivalTime);
            currentTime=nextArrivalTime;
        } 
        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time <= currentTime && processes[i].remaining_time > 0 && !processes[i].inqueue) {
                enqueue(mlfq->queues[0], i); 
                processes[i].inqueue=true;
            }
        }
        
        if (currentTime - lastBoostTime >= boost_time) {
            for (int j = 1; j < 3; j++) {
                while (!isEmpty(mlfq->queues[j])) {
                    int processIndex = dequeue(mlfq->queues[j]);
                    enqueue(mlfq->queues[0], processIndex); 
                }
            }
            lastBoostTime = currentTime;
        }
    }

    writeOutput(processes, Contexts, n, contextSwitches);
    free(Contexts);
    destroyMLFQ(mlfq);
}

void createCopy(struct Process p[],struct Process q[],int n){
      for(int i=0;i<n;i++){
           strcpy(q[i].PID,p[i].PID);
           q[i].arrival_time=p[i].arrival_time;
           q[i].burst_time=p[i].burst_time;
           q[i].inqueue=p[i].inqueue;
           q[i].response_time=p[i].response_time;
           q[i].remaining_time=p[i].remaining_time;
      }
}

int stringToInt(char *str){
    int num=0;
     for (int i = 0; str[i] != '\0'; i++) {
        num = num * 10 + (str[i] - 48);
    }
    return num;
}

int main(float argc, char* argv[]) {
    int numOfProcess = 0;
    struct Process *process = NULL;  // Initialize to NULL
    
    readInput(argv[1], &process, &numOfProcess);
    outputFile=fopen(argv[2],"w"); 
    float TsRR = atof(argv[3]);
    float TsMLFQ1= atof(argv[4]);
    float TsMLFQ2= atof(argv[5]);
    float TsMLFQ3= atof(argv[6]);
    float BMFLQ = atof(argv[7]);

    for(int i=0;i<5;i++){
        struct Process tmp[numOfProcess];
        createCopy(process,tmp,numOfProcess);
        if(i==0){
           FCFS(tmp,numOfProcess);
        }
        else if(i==1){
           RoundRobin(tmp,numOfProcess,TsRR); 
        }
        else if(i==2){
            SJF(tmp,numOfProcess);
        }
        else if(i==3){
          SRTF(tmp,numOfProcess);
        }
        else{
            MLFQ(tmp,numOfProcess,TsMLFQ1,TsMLFQ2,TsMLFQ3,BMFLQ);
        }
    }
    
    fclose(outputFile);
    free(process);
    return 0;
}
