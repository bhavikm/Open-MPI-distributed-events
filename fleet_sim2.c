#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

int main(argc,argv)
int argc;
char **argv;
{
	static int positions = 1500; //how many different positions can vessels occupy
	static int averaging_loops = 4; //how many times to run the 60 second program for averaging the strike count

	int rank, size; //rank of process and number of processes

	int summed_total_strike_counts = 0; //for averaging the strike counts
	unsigned long summed_total_loop_counts = 0;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);

	int num_elements_per_proc = positions / size; //for dividing up counting work in global_counter copy for each process

	srand((unsigned int)(MPI_Wtime()) + (unsigned)(rank*size)); //seed the pseudo-random number generator uniquely for each process

	for (int a = 0; a < averaging_loops; a++)
	{

		int total_strike_count = 0;
		double tmr = MPI_Wtime(); //time the iterations
		int loop_count = 0; //keep track of number of loops (generation of positions and counting of strikes) as another metric

		while ((60.0 - (MPI_Wtime() - tmr)) > 0.0)
		{
			//each process has a local counter of positions that vessels can end up at
			int local_counters[positions];
			for (int i = 0; i < positions; i++) {
				local_counters[i] = 0;
			}

			//each process calculates its position based on random number
			int random_number = rand() % positions;
			local_counters[random_number] = 1;

			//sum each position of all local_counters into corresponding position of a global_counter
			//then also re-broadcast this summed global_counter to each process
			int global_counters[positions];
			MPI_Allreduce(local_counters, global_counters, positions, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

			//each process has copy of global_counter now
			//each process counts number of vessels in an a offset sub-array of positions from the global_counters 
			int local_strike_count = 0;
			for (int j = (num_elements_per_proc*rank); j < ((num_elements_per_proc*rank) + num_elements_per_proc); j++)
			{
				if (j == positions) {
					break;
				}

				if (global_counters[j] >= 4) {
					local_strike_count++;
				}
			}

			//each process sends its local_strike_count (from going over its own subarray) to master process
			//master process thus has the total number of strikes after a SUM Reduce on all the local_strike_counts
			int global_strike_count;
			MPI_Reduce(&local_strike_count, &global_strike_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

			//add this to the overall strike count for duration of program run
			if (rank == 0) {
				total_strike_count += global_strike_count;
			}

			loop_count++;
		}

		if (rank == 0) {
			printf("Round %d: Total strike count: %d Loop Count: %d Total time: %f\n", a, total_strike_count, loop_count, (MPI_Wtime() - tmr));
		}

		summed_total_strike_counts += total_strike_count; //for averaging sum the total strike count
		summed_total_loop_counts += loop_count;
	}

	if (rank == 0) {
		printf("Averaged Strike Count: %d, Averaged Loop Count: %lu, Number Processes: %d\n", ( summed_total_strike_counts / averaging_loops ), ( summed_total_loop_counts / averaging_loops ), size);
	}

	MPI_Finalize();

	return 0;
}