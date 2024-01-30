# Multiprocessing-and-Threading-project
## process.c
This C program performs multi-process numerical integration using the trapezoidal rule. It dynamically handles input for integration tasks with various parameters and mathematical functions. Child processes are managed to maintain a maximum concurrency limit, each computing the integral for a specified range and function. The results are printed, and the program waits for all child processes to complete before exiting.

## thread.c
This program does much the same, but the integration tasks are distributed among a fixed number of threads, each calculating a portion of the overall integral. The program uses mutexes to protect the shared result variable from race conditions. The final integrated value is continuously updated and printed as new tasks are processed. The program runs indefinitely, accepting input until manually terminated.

## processThread.c
This program combines the previous two and incorporates threading and child processes to minimize the calculation time. 
This C program implements multi-threaded numerical integration using the trapezoidal rule. It dynamically handles input for diverse integration tasks, distributing them among threads. Child processes are managed to maintain concurrency limits. Results are protected with a mutex.

### Running 
To run these programs, compile in the terminal with gcc -o filename filename.c -lm -pthread, and run with ./filename after that. It should prompt for inputs to integrate. 
