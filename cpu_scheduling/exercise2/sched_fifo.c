#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <sched.h>
#include <time.h>

void delay(double sec){
	clock_t start = clock();
	while((double)(clock() - start)/CLOCKS_PER_SEC < sec);
}

int main(void) {
	
	int a = 0;
	int cpu = 0;
	int scheduler;
	pid_t pid;
	cpu_set_t mask;
	struct sched_param param;

	pid = getpid();
	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);

	if(sched_setaffinity(pid, sizeof(mask), &mask)) {
		printf("failed to set affinity\n");
		exit(1);
	}
	
	param.sched_priority = sched_get_priority_min(SCHED_FIFO);

	if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
		printf("failed to set scheduler\n");
		exit(1);
	}
	
	scheduler = sched_getscheduler(0);
	switch (scheduler) {
		case SCHED_OTHER:
			printf("Default scheduler is being used\n");
			break;
		case SCHED_FIFO:
			printf("FIFO scheduler is being used\n");
			break;
		case SCHED_RR:
			printf("Round robin scheduler is being used\n");
			break;
		default:
			printf("failed to get scheduler\n");
			exit(1);
	}

	pid = fork();
	if (pid == 0) {
		printf("Child process %d with the high priority\n", getpid());
		
		param.sched_priority = sched_get_priority_max(SCHED_FIFO);
		if (sched_setparam(0, &param)) {
			printf("failed to set parameter\n");
			exit(1);
		}
		sleep(2);
		delay(3);
	}
	else {
		printf("Parent process %d with the low priority\n", getpid());
		sleep(1);
		delay(3);
	}

	return 0;
}
