#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sched.h>
#include <string.h>
#define GET_SYS_TIME 334
#define PRINT_SYS_MESG 335
#define TIME_QUANTUM 500
struct process{
	char pname[10];
	int readyT;
	int execT;
};

void time_unit(){
	volatile unsigned long i;
	for (i = 0; i < 1000000UL; i++);	
}
//set process to specofic cpu
void set_cpu(int coreindex, int pid){

	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(coreindex, &mask);

	if(sched_setaffinity(pid, sizeof(mask), &mask) < 0){
		fprintf(stderr, "affinity\n" );;
	}
}

void set_idle(int pid){
	struct sched_param param;
	param.sched_priority = 1;
	if(sched_setscheduler(pid,SCHED_FIFO,&param) < 0){
		fprintf(stderr, "idle %d\n",pid );;
	}
}

void set_busy(int pid){
	struct sched_param param;
	param.sched_priority = 99;
	if(sched_setscheduler(pid,SCHED_FIFO,&param) < 0){
		fprintf(stderr, "busy %d\n",pid );
	}
}

int cmp(const void *a, const void *b){
	struct process* p1 = (struct process*) a;
	struct process* p2 = (struct process*) b;
	if(p1 -> readyT <= p2 -> readyT)
		return -1;
	else 
		return 1;
}

// for RR
int count = 0;

int schedule(char policy[], struct process *processes,int running, int finish_task_num, int create_task_num){
	if(strcmp(policy,"FIFO") == 0){
		if(finish_task_num < create_task_num)
			return finish_task_num;
		else
			return -1;
	}else if(strcmp(policy,"RR") == 0){
		if(running != -1 && count < 500 && count > 0 && processes[running].execT > 0){
			count+=1;
			return running;
		}else{
			count = 1;
			for (int i = 1; i <= create_task_num; ++i)
				if(processes[(running+i)%create_task_num].execT > 0 )
					return (running+i)%create_task_num;
			return -1;
		}
	}else if(strcmp(policy,"SJF") == 0){
		if(processes[running].execT != 0)
			return running;
		else{
			int min = 2147483647;
			int min_index = -1;
			for (int i = 0; i < create_task_num; ++i){
				if(processes[i].execT > 0 && processes[i].execT < min){
					min = processes[i].execT;
					min_index = i;
				}
			}
			return min_index;
		}
	}else if(strcmp(policy,"PSJF") == 0){
		int min = 2147483647;
		int min_index = -1;
			for (int i = 0; i < create_task_num; ++i){
				if(processes[i].execT > 0 && processes[i].execT < min){
					min = processes[i].execT;
					min_index = i;
				}
			}
		return min_index;
	}else {
		fprintf(stderr, "No such policy.\n" );
	}
	return 0;
}
int main(int argc, char const *argv[])
{
	int N;
	char policy[100];
	scanf("%s",policy);
	scanf("%d",&N);

	struct process *processes = (struct process*) malloc(sizeof(struct process)*N);
	int *pidtable = malloc(sizeof(int)*N);
	
	for (int i = 0; i < N; ++i)
		scanf("%s%d%d",processes[i].pname, &processes[i].readyT, &processes[i].execT);
	
	int schedular_pid = getpid();
	set_cpu(0,schedular_pid);
	// sort by ready time
	qsort((void *)processes, N, sizeof(struct process), cmp);

	int current_time = 0;
	int create_task_num = 0, finish_task_num = 0;
	int running = -1;
	count = 0;

	//set schedular to high priority
	set_busy(schedular_pid);
	while(finish_task_num < N ){
		// check if task is ready
		if(create_task_num < N && processes[create_task_num].readyT == current_time){
			int pid = fork();
			if(pid == 0){
				int child_pid = getpid();
				set_cpu(0, child_pid);
				long start_sec, start_nsec, end_sec, end_nsec;

				syscall(GET_SYS_TIME, &start_sec, &start_nsec);

				while(processes[create_task_num].execT--){
					time_unit();
					if(processes[create_task_num].execT == 0){
						syscall(GET_SYS_TIME, &end_sec, &end_nsec);
						break;
					}
					// return cpu access to schedular
					set_busy(schedular_pid);
					set_idle(child_pid);
				}

				//fprintf(stderr,"%d %ld %ld %ld %ld %ld %ld\n",create_task_num,start_sec,start_nsec, end_sec, end_nsec,end_sec - start_sec, end_nsec - start_nsec );
				fprintf(stderr, "%s %d\n", processes[create_task_num].pname, child_pid );
				char sys_mesg[200];
				sprintf(sys_mesg, "[Project1] %d %ld.%09ld %ld.%09ld", child_pid, start_sec, start_nsec, end_sec, end_nsec);
				//sprintf(sys_mesg, "[Project1] %d %ld.%09ld %ld.%09ld %ld %ld", child_pid, start_sec, start_nsec, end_sec, end_nsec,end_sec - start_sec, end_nsec - start_nsec );
				syscall(PRINT_SYS_MESG, sys_mesg);
				
				exit(0);
			}
			// let child wait until it's their time to run
			set_idle(pid);
			pidtable[create_task_num] = pid;
			create_task_num++;
			continue;
		}
		//find next task
		running = schedule(policy, processes, running, finish_task_num, create_task_num);
		
		//fprintf(stderr, "%d %d %d \n",current_time,running ,processes[running].exect);
		if(running == -1 )
			time_unit(); // no available task
		else{
			//give cpu access to next task
			set_busy(pidtable[running]);
			set_idle(schedular_pid);
			processes[running].execT--;
			if(processes[running].execT== 0){
				waitpid(pidtable[running],NULL,0);
				finish_task_num++;
			}
		}
		current_time++;
	}
	
	return 0;
}
