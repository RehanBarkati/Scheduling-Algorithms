import random
import matplotlib.pyplot as plt

output = 'output.txt'
indexToProcess={0:"FCFS",2:"Round_Robin",4:"SJF",6:"SRTF",8:"MLFQ"}

with open(output, 'r') as file:
    lines = file.readlines()
    for i in range(0, len(lines), 2): 
        context= (lines[i].strip()).split()
        dictionary={"PID":[],"Start Time":[],"End Time": []}    
        k=0
        while(k<len(context)):
            dictionary["PID"].append(context[k])
            dictionary["Start Time"].append(float(context[k+1]))
            dictionary["End Time"].append(float(context[k+2]))
            k+=3
        pid_colors = {}
        unique_pids = list(set(dictionary["PID"]))
        colors = [f'#{random.randint(0, 0xFFFFFF):06X}' for _ in unique_pids]

        for pid, color in zip(unique_pids, colors):
            pid_colors[pid] = color
        fig, ax = plt.subplots(figsize=(10, 5))
        for j in range(len(dictionary["PID"])):
           ax.barh(dictionary["PID"][j], dictionary["End Time"][j] - dictionary["Start Time"][j], left=dictionary["Start Time"][j], color=pid_colors[dictionary["PID"][j]],height=0.2, label=f'{dictionary["PID"][j]}')
        
        ax.set_xlabel('Time (ms)')
        ax.set_ylabel('Processes')
        ax.set_title("Gaant Chart for " + indexToProcess[i])

        plt.show()

