#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#define MAX_CHILDREN 5

typedef double MathFunc_t(double);

static volatile int numChildren = 0;

void handleChildTermination(int signo) {
    // Decrement the number of children when a child terminates
    if (signo == SIGCHLD) {
        while (waitpid(-1, NULL, WNOHANG) > 0) {
            numChildren--;
        }
    }
}

double gaussian(double x)
{
	return exp(-(x*x)/2) / (sqrt(2 * M_PI));
}


double chargeDecay(double x)
{
	if (x < 0) {
		return 0;
	} else if (x < 1) {
		return 1 - exp(-5*x);
	} else {
		return exp(-(x-1));
	}
}

#define NUM_FUNCS 3
static MathFunc_t* const FUNCS[NUM_FUNCS] = {&sin, &gaussian, &chargeDecay};





//Integrate using the trapezoid method. 
double integrateTrap(MathFunc_t* func, double rangeStart, double rangeEnd, size_t numSteps)
{
	double rangeSize = rangeEnd - rangeStart;
	double dx = rangeSize / numSteps;

	double area = 0;
	for (size_t i = 0; i < numSteps; i++) {
		double smallx = rangeStart + i*dx;
		double bigx = rangeStart + (i+1)*dx;

		area += dx * ( func(smallx) + func(bigx) ) / 2; //Would be more efficient to multiply area by dx once at the end. 
	}

	return area;
}




bool getValidInput(double* start, double* end, size_t* numSteps, size_t* funcId)
{
	printf("Query: [start] [end] [numSteps] [funcId]\n");

	//Read input numbers and place them in the given addresses:
	size_t numRead = scanf("%lf %lf %zu %zu", start, end, numSteps, funcId);

	//Return whether the given range is valid:
	return (numRead == 4 && *end >= *start && *numSteps > 0 && *funcId < NUM_FUNCS);
}



int main(void)
{
	double rangeStart;
	double rangeEnd;
	size_t numSteps;
	size_t funcId;
	
	while(1){
		while(numChildren >= MAX_CHILDREN){
			pause();
		}
	
	if (getValidInput(&rangeStart, &rangeEnd, &numSteps, &funcId)) {
		pid_t childPid = fork();

		if (childPid ==0) {
			double area = integrateTrap(FUNCS[funcId], rangeStart, rangeEnd, numSteps);

			printf("The integral of function %zu in range %g to %g is %.10g\n", funcId, rangeStart, rangeEnd, area);


			exit(0);
		}else if (childPid >0){
			numChildren++;
		}else{
			fprintf(stderr, "Fork failed\n");
		}
	}else{
		break;
	}
}

while(numChildren >0){
	wait(NULL);
	numChildren--;
	}
	return 0;
}