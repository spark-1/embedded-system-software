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

	sleep(1);

	pid = getpid();
	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);

	if(sched_setaffinity(0, sizeof(mask), &mask)) {
		printf("failed to set affinity\n");
		exit(1);
	}

	param.sched_priority = sched_get_priority_max(SCHED_RR);

	if (sched_setscheduler(0, SCHED_RR, &param) == -1) {
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
	if (pid) { // 부모 
		delay(3);
	}
	else { // 자식
		pid = fork();
		delay(3);
	}

	return 0;
}

