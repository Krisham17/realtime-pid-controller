#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <_time.h>

struct thread_info {
	double a, b; // The numbers a and b
	char op; // The math iteration to be performed: +,-,*,/
	int maxitr; // The number of iterations to be performed
	double c; // The result: c = a <op> b
	double exec_time; // The execution time per iteration in nsec
};

typedef struct thread_info thread_info_t;

void *func(void *arg) {
	struct timespec time_1, time_2;
	double exec_time;
	thread_info_t *info;
	int i, maxitr;
	volatile double a, b, c;

	info = (thread_info_t *)arg;
	maxitr = info->maxitr;
	char op = info->op;

	b = 2.3; c = 4.5;

	exec_time = 0.0;

	clock_gettime(CLOCK_REALTIME, &time_1);

	for (i = 0; i < maxitr; i++) {
    	switch (op) {
        	case '+':
            	a = b + c;
            	break;
        	case '-':
            	a = b - c;
            	break;
        	case '*':
            	a = b * c;
            	break;
        	case '/':
            	if (c != 0) {
                	a = b / c;
            	}
            	break;
    	}
	}

	clock_gettime(CLOCK_REALTIME, &time_2);

	exec_time = (time_2.tv_sec - time_1.tv_sec)* 1.0e9;
	exec_time += (time_2.tv_nsec - time_1.tv_nsec);
	exec_time = exec_time/maxitr;

	info->exec_time = exec_time;

	pthread_exit(NULL);
}

int main(void) {
	pthread_t threads[4];
	thread_info_t infos[4];
	char operations[4] = {'+', '-', '*', '/'};
	int i;

	int maxitr = 5.0e8;

	for (i = 0; i < 4; i++) {
    	infos[i].maxitr = maxitr;
    	infos[i].op = operations[i];

    	if (pthread_create(&threads[i], NULL, &func, &infos[i]) != 0) {
        	printf("Error creating thread %d\n", i + 1);
        	exit(1);
    	}

    	pthread_join(threads[i], NULL);

    	printf("Execution time for operation '%c': %lf seconds\n", infos[i].op, infos[i].exec_time);
	}

	pthread_exit(NULL);
}
