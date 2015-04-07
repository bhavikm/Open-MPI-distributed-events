# Open-MPI-distributed-events
Requires the Open Message Passing Interface (MPI) library to use (http://www.open-mpi.org/)  
  
The problem
===
A naval fleet patrols an area comprising 1500 distinct locations. Each vessel can occupy any one of these locations at random. For a vessel to be able to launch a strike, other vessels must accompany it. The rules for launching a strike are as follows:

  1. At least four vessels must share the same location, at a given point in time, for a strike to be counted.
  2. The fleet may generate more than one strike at an instant of time. It will however depend on the number of locations with four or more vessels present at that instant of time.
  3. There is no limit on the number of vessels in the fleet. The objective is to acheive the highest possible strike rate. 

Solutions
===

### Approach 1 (fleet_sim1.c) :
This was the first and simplest approach implemented. It emphasized the repeated assignment of the fleet to random positions to maximize the chance of vessels coinciding in the same location and a strike being launched. This was done by reducing the amount of inter-process communication.

The algorithm used is as follows:
  1.	There is a single master process (the root process) and every other process is a worker (a vessel).
  2.	Each worker process generates a random number between 1 and 1500 (assignment of a vessel to a location) and sends this to the master process.
  3.	The master process counts strikes from locations that have four or more processes allocated to it.
  4.	Steps 1 to 3 are repeated for 60 seconds.

To compile:  
  `mpicc -o fleet_sim1 fleet_sim1.c`  
To run:  
  `mpirun -n [number of processes] fleet_sim1`

### Approach 2 (fleet_sim2.c) :
This approach emphasized parallel work of processes and used a lot of inter-process communication to share state information.

The algorithm used is as follows:
  1.	Each process holds a local variable of the 1500 locations and generates a random number between 1 and 1500 (assignment of a vessel to a location). The location of the random number is recorded in the local variable.
  2.	The local variables of 1500 locations are reduced to a single variable of 1500 locations by summing each corresponding position of the 1500 locations. 
  3.	The summed variable of vessel location counts is broadcast to each process. 
  4.	Each process iterates through a sub-array of the vessel location counts and records the number of locations with four or more vessels (number of strikes).
  5.	Each process’s local strike counts are reduced by summing to a final strike count for the time instant.
  6.	Steps 1 to 5 are repeated for 60 seconds.

To compile:  
  `mpicc -o fleet_sim2 fleet_sim2.c`  
To run:  
  `mpirun -n [number of processes] fleet_sim2`

