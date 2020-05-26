#define _GNU_SOURCE

#include <linux/kernel.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>
#include <linux/types.h>
#include <sched.h>
#include <linux/sched.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifndef __NR_sched_setattr
#ifdef __x86_64__
#define __NR_sched_setattr	314
#endif

#ifdef __i386
#define __NR_sched_setattr	351
#endif

#ifdef __arm__
#define __NR_sched_setattr	380
#endif

#ifdef __aarch644__
#define __NR_sched_setattr	274
#endif
#endif

#ifndef __NR_sched_getattr
#ifdef __x86_64__
#define __NR_sched_getattr	315
#endif

#ifdef __i386
#define __NR_sched_getattr	352
#endif

#ifdef __arm__
#define __NR_sched_getattr	381
#endif

#ifdef __aarch644__
#define __NR_sched_getattr	275
#endif
#endif

#ifndef SCHED_DEADLINE
#define SCHED_DEADLINE 6
#endif

typedef unsigned int u32;
typedef unsigned long long u64;
typedef signed int s32;

struct sched_attr {
	u32 size;

	u32 sched_policy;
	u64 sched_flags;

	s32 sched_nice;

	u32 sched_priority;

	u64 sched_runtime;
	u64 sched_deadline;
	u64 sched_period;
};


int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags) {
	return syscall(__NR_sched_setattr, pid, attr, flags);
}

int sched_getattr(pid_t pid, const struct sched_attr *attr, unsigned int size, unsigned int flags) {
	return syscall(__NR_sched_getattr, pid, attr, size, flags);
}

void delay(double sec){
	clock_t start = clock();
	while((double)(clock() - start)/CLOCKS_PER_SEC < sec);
}

int main(void) {

	int a = 0;
	int scheduler;
	struct sched_attr param;

	if (sched_getattr(0, &param, sizeof(struct sched_attr), 0)) {
		printf("failed to get attr\n");
		exit(1);
	}

	printf("Process %d \n", getpid());

	param.sched_policy = SCHED_DEADLINE;
	param.sched_runtime = 2000000; // 단위 피코
	param.sched_deadline = 10000000;
	param.sched_period = 10000000;

	if (sched_setattr(0, &param, 0) == -1) {
		printf("failed to set attr\n");
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
		case SCHED_DEADLINE:
			printf("Deadline scheduler is being used\n");
			break;
		default:
			printf("failed to get scheduler\n");
			exit(1);
	}

	delay(0.1);

	return 0 ;
}
