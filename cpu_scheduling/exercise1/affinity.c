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

void print_affinity(int pid) {
	cpu_set_t mask;
	long ncores = sysconf(_SC_NPROCESSORS_ONLN);

	if(sched_getaffinity(pid, sizeof(mask), &mask)) {
		printf("failed to get affinity\n");
		exit(1);
	}
	printf("Process[%d]'s affinity mask is ", pid);

	for(int i = 0; i < ncores; i++){
		printf("%d ", CPU_ISSET(i, &mask));
	}
	printf("\n");
}

void delay(double sec){
	clock_t start = clock(); 
	while((double)(clock() - start)/CLOCKS_PER_SEC < sec);
}

int main(){
	int a;
	int cpu = 0;
	pid_t pid;
	cpu_set_t mask;

	pid = getpid();
	print_affinity(pid);

	delay(5);
	
	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);

	if(sched_setaffinity(pid, sizeof(mask), &mask)) {
		printf("failed to set affinity\n");
		exit(1);
	}
	print_affinity(pid);

	delay(5);

	return 0;
}
