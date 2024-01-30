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
#define NUM_THREADS 4

typedef double MathFunc_t(double);

typedef struct {
    MathFunc_t* func;
    double rangeStart;
    double rangeEnd;
    size_t numSteps;
    double* result;  // Pointer to result variable
    pthread_mutex_t* mutex;  // Mutex for result variable
} ThreadArgs;

double gaussian(double x) {
    return exp(-(x * x) / 2) / (sqrt(2 * M_PI));
}

double chargeDecay(double x) {
    if (x < 0) {
        return 0;
    } else if (x < 1) {
        return 1 - exp(-5 * x);
    } else {
        return exp(-(x - 1));
    }
}

#define NUM_FUNCS 3
static MathFunc_t* const FUNCS[NUM_FUNCS] = {&sin, &gaussian, &chargeDecay};

void* integrateTrap(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;

    double rangeSize = args->rangeEnd - args->rangeStart;
    double dx = rangeSize / args->numSteps;

    double area = 0;
    for (size_t i = 0; i < args->numSteps; i++) {
        double smallx = args->rangeStart + i * dx;
        double bigx = args->rangeStart + (i + 1) * dx;

        area += dx * (args->func(smallx) + args->func(bigx)) / 2;
    }

    // Lock the mutex before updating the result
    pthread_mutex_lock(args->mutex);
    *(args->result) += area;
    // Unlock the mutex
    pthread_mutex_unlock(args->mutex);

    pthread_exit(NULL);
}

bool getValidInput(double* start, double* end, size_t* numSteps, size_t* funcId) {
    printf("Query: [start] [end] [numSteps] [funcId]\n");

    size_t numRead = scanf("%lf %lf %zu %zu", start, end, numSteps, funcId);

    return (numRead == 4 && *end >= *start && *numSteps > 0 && *funcId < NUM_FUNCS);
}

static volatile int numChildren = 0;

void handleChildTermination(int signo) {
    if (signo == SIGCHLD) {
        while (waitpid(-1, NULL, WNOHANG) > 0) {
            numChildren--;
        }
    }
}

int main(void) {
    signal(SIGCHLD, handleChildTermination);

    double rangeStart;
    double rangeEnd;
    size_t numSteps;
    size_t funcId;

    // Initialize the result and mutex
    double result = 0;
    pthread_mutex_t resultMutex = PTHREAD_MUTEX_INITIALIZER;

    // Array of thread arguments
    ThreadArgs threadArgs[NUM_THREADS];

    pthread_t threads[NUM_THREADS];

    while (1) {
        // Check if the maximum number of children is reached
        while (numChildren >= MAX_CHILDREN) {
            // Wait for a child to terminate
            pause();
        }

        for (int i = 0; i < NUM_THREADS; i++) {
            if (getValidInput(&rangeStart, &rangeEnd, &numSteps, &funcId)) {
                // Set up thread arguments
                threadArgs[i].func = FUNCS[funcId];
                threadArgs[i].rangeStart = rangeStart;
                threadArgs[i].rangeEnd = rangeEnd;
                threadArgs[i].numSteps = numSteps;
                threadArgs[i].result = &result;
                threadArgs[i].mutex = &resultMutex;

                // Create threads
                if (pthread_create(&threads[i], NULL, integrateTrap, (void*)&threadArgs[i]) != 0) {
                    fprintf(stderr, "Thread creation failed\n");
                    exit(EXIT_FAILURE);
                }
            } else {
                // No more input or invalid input
                break;
            }
        }

        // Wait for all threads to finish
        for (int i = 0; i < NUM_THREADS; i++) {
            if (pthread_join(threads[i], NULL) != 0) {
                fprintf(stderr, "Thread join failed\n");
                exit(EXIT_FAILURE);
            }
        }

        // Print the result after all threads finish
        printf("The total integral is %.10g\n", result);
    }

    return 0;
}
