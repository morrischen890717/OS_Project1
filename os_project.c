#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include "os_project.h"
#define queue_size 10000000

int running;
int next_proc;
int now_time;
int continue_time;
int finish_proc;
int RR_queue[queue_size] = {-1};
int front_queue;
int end_queue;

int compare(const void *a, const void *b){ 
	const Proc *fir = (const Proc *) a;
	const Proc *sec = (const Proc *) b;
	return fir->ready_t - sec->ready_t;
}

void unit_of_time(){
	volatile unsigned long i;
	for(i=0;i<1000000UL;i++);
	return;
}

int is_empty(){
	if(front_queue == end_queue) return 1;
	else return 0;
}

void push(int element){
	RR_queue[end_queue] = element;
	end_queue++;
	return;
}

void pop(){
	if(!is_empty()){
		RR_queue[front_queue] = -1;
		front_queue++;
	}
	return;
}

int new_proc_exec(Proc *proc){
	int pid = fork();
	if(pid == 0){ // child process
		struct timespec start;
		struct timespec end;
		syscall(334, &start); // get_time
		for(int i = 0; i < proc->exec_t; i++){
			unit_of_time();
		}
		syscall(334, &end); // get_time
		syscall(335, getpid(), start.tv_sec, start.tv_nsec, end.tv_sec, end.tv_nsec); // printk
		exit(0);
	}
	else if(pid > 0){ // parent process
		cpu_set_t mask;
		CPU_ZERO(&mask);
		CPU_SET(1, &mask); // core 1 for running process
		sched_setaffinity(pid, sizeof(mask), &mask);
	}
	else perror("fork!\n");
	return pid;
}

void block_proc(int pid){
	struct sched_param param;
	param.sched_priority = sched_get_priority_min(SCHED_FIFO);
	sched_setscheduler(pid, SCHED_FIFO, &param);
	return;
}

void run_proc(int pid){
	struct sched_param param;
	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	sched_setscheduler(pid, SCHED_FIFO, &param);
	return;
}

int choose_next(char *policy, int process_num, int running, Proc *proc){
	int next = -1;
	int min_exec_t;
	int cnt;
	switch(policy[0]){
		case 'R':
			if(running == -1){
				if(is_empty()){
					next = -1;
				}
				else next = RR_queue[front_queue];
			}
			else if(proc[running].exec_t <= 0){
				pop();
				if(is_empty()) next = -1;
				else next = RR_queue[front_queue];
			}
			else if((now_time - continue_time) % 500 == 0){
				pop();
				push(running);
				next = RR_queue[front_queue];
			}
			else next = running;
			break;
		case 'F':
			if(running != -1 && proc[running].exec_t > 0){
				next = running;
			}
			else{
				cnt = 0;
				next = (running + 1) % process_num;
				while(proc[next].ready_t > now_time || proc[next].exec_t <= 0){
					cnt++;
					if(cnt == process_num) break;
					next = (next + 1) % process_num;
				}
			}
			break;
		case 'P':
			min_exec_t = 10000000;
			for(int i = 0; i < process_num; i++){
				if(proc[i].ready_t <= now_time && proc[i].exec_t < min_exec_t && proc[i].exec_t > 0){
					next = i;
					min_exec_t = proc[i].exec_t;
				}
			}
			break;
		default:
			if(running == -1 || proc[running].exec_t <= 0){
				min_exec_t = 10000000;
				for(int i = 0; i < process_num; i++){
					if(proc[i].ready_t <= now_time && proc[i].exec_t < min_exec_t && proc[i].exec_t > 0){
						next = i;
						min_exec_t = proc[i].exec_t;
					}
				}
			}
			else next = running;
			break;
	}
	return next;
}

void scheduler(char *policy, Proc *proc, int process_num){
	qsort(proc, process_num, sizeof(Proc), compare);
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(0, &mask); // core 0 for scheduler
	sched_setaffinity(getpid(), sizeof(mask), &mask); 
	running = next_proc = -1;
	while(finish_proc < process_num){
		for(int i = 0; i < process_num; i++){
			if(proc[i].ready_t == now_time){
				proc[i].pid = new_proc_exec(&proc[i]);
				block_proc(proc[i].pid);
				push(i);
			}
		}
		if(running != -1 && proc[running].exec_t <= 0){  // a process finished
			waitpid(proc[running].pid, NULL, 0);
			finish_proc++;
			if(finish_proc == process_num) break;
		}
		next_proc = choose_next(policy, process_num, running, proc);
		if(next_proc != -1){
			if(next_proc != running){
				if(running != -1) block_proc(proc[running].pid);
				run_proc(proc[next_proc].pid);
				running = next_proc;
				continue_time = now_time;
			}
			if(running != -1) proc[running].exec_t--;
		}
		unit_of_time();
		now_time++;
	}
	return;
}

int main(){
	char policy[5];
	int process_num;
	scanf("%s%d", policy, &process_num);
	Proc proc[process_num];
	for(int i = 0; i < process_num; i++){
		scanf("%s%d%d", proc[i].name, &proc[i].ready_t, &proc[i].exec_t);
	}
	scheduler(policy, proc, process_num);
	for(int i = 0; i < process_num; i++){
		printf("%s %d\n", proc[i].name, proc[i].pid);
	}
	return 0;
}