#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>

typedef struct Process{
	char name[35];
	int ready_t;
	int exec_t;
	int pid;
	long long start_t;
	long long finish_t;
} Proc;

int compare(const void *a, const void *b);
void scheduler(char *policy, Proc *proc, int process_num);
void unit_of_time();
int new_proc_exec(Proc *proc);
int is_empty();
void push(int element);
void pop();
void block_proc(int pid);
void run_proc(int pid);
int choose_next(char *policy, int process_num, int running, Proc *proc);