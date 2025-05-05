This program elaborates on the reduce-scatter alogrithm through server client interactions using p_threads.

To start the program make sure you run the makefile "make" to create the executable

Then to run the program you need to run "./allreduce <PORT1> <PORT2> <num1> <num2> <num3> <num4>" in the first console

In the second console run "./allreduce <PORT2> <PORT1> <num5> <num6> <num7> <num8>"

An explanation of the arugments here, the PORT1 and PORT2 are switched so that the client connnects to each other's servers
based on what instance of the program you are on. The four numbers after are the array that will be reduced and scattered.

Your output will be something like this: 

Input: /allreduce 8080 8081 0 1 2 3

Output: 
Starting array: 0 1 2 3
Final array after reduce phase: 0 1 8 10
The final array after scatter phase: 4 6 8 10

Input: /allreduce 8081 8080 4 5 6 7

Output:
Starting array: 4 5 6 7
Final array after reduce phase: 4 5 16 18
The final array after scatter phase: 12 14 16 18
