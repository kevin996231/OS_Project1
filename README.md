# OS_Project1
## How to Run
After ```make```, you might need ```sudo ./main``` to run the code.
## Description
In this project, I try to simulate CPU scheduling with 4 different policies.
- first in first out
- round robin
- shortest job first
- preemptive shortest job first

A time unit is defined as below.
```
	void time_unit(){
		volatile unsigned long i;
		for (i = 0; i < 1000000UL; i++);	
	}
```
### Program Input
```
  The program will get the input parameters from the standard input.
  S // the scheduling policy, one of the following strings: FIFO, RR, SJF, PSJF.
  N // the number of processes
  N1 R1 T1
  N2 R2 T2
  …
  Nn Rn Tn
  //Ni - a character string with a length less than 32 bytes, specifying the name of the i-th process.
  //Ri - a non-negative integer, specifying the ready time of the i-th process.
  //Ti - a non-negative integer, specifying the execution time of the i-th process.
```
### Program Output
After executing, type ```dmesg``` in terminal to see the output.
```
TAG PID ST FT
//TAG - the tag to identify the message of this project.
//PID - the process ID of this process
//ST - the start time of this process in the format seconds.nanoseconds.
//FT - the finish time of this process in the format seconds.nanoseconds.
```
## Implementation
**I only use one CPU throughout the project.**
At the beginning of each time unit, scheduler will fork the process once it's ready and determine which process should be running at this time unit. Then schedular will yield the CPU usage to that particular process.  
I use sched_setscheduler() to make the actual CPU context switch between scheduler and processes.
### Time Measurement
I implement the kernel function GET_SYS_TIME to get the system time. (See /kernel_files for more details)  
It can be used to check the correctness of scheduler.  
- 500 time units is about 0.75sec in my machine. 

## How to Run
You might need sudo ./main to run the code.
