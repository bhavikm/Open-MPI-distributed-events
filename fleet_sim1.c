#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int main(argc,argv)
int argc;
char **argv;
{
	static int positions = 1500; //how many different positions can vessels occupy
	static int averaging_loops = 4; //how many times to run the 60 second program for averaging the strike count

	int rank, size; //rank of process and number of processes
	MPI_Status status;

	int summed_total_strike_counts = 0; //for averaging the strike counts
	unsigned long summed_total_loop_counts = 0;

	MPI_Init(&argc,&argv);

	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);

	srand((unsigned int)MPI_Wtime() + (unsigned)rank); //seed the pseudo-random number generator uniquely for each process

	for (int a = 0; a < averaging_loops; a++)
	{

		int total_strike_count = 0;
		double tmr = MPI_Wtime(); //time the iterations
		int loop_count = 0; //also keep track of number of loops

		while ((60.0 - (MPI_Wtime() - tmr)) > 0.0) //stop after 60 seconds as per specification
		{
			int position_counts[positions]; //positions of the vessels (processes)
			for (int k = 0; k < positions; k++)
			{
				position_counts[k] = 0;
			}

			if (rank == 0) { 
				//master process receives the position of each vessel or process and records where the position is
				for (int i = 1; i < size; i++) {
					int recv_rand_numb;
					MPI_Recv(&recv_rand_numb, 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status); //receive the position of each vessel or process
					position_counts[recv_rand_numb] += 1;
				}

			} else {
				//all other processes do the work of randomly assigning vessel position
				int rand_position = rand() % positions; //randomly assign the vessel to a position
				MPI_Send(&rand_position, 1, MPI_INT, 0, rank, MPI_COMM_WORLD); //send the position to the master process
			}
			
			if (rank == 0) { //master process counts how many positions with four or more vessels (a strike)
				int loop_strike_count = 0;
				for (int k = 0; k < positions; k++) //count the strikes in this loop
				{
					if (position_counts[k] >= 4) {
						loop_strike_count++;
					}
				}

				total_strike_count += loop_strike_count; //accumulate the overall strike count
			}

			loop_count++;
		}

		if (rank == 0) {
			printf("Round %d: Total strike count: %d Loop Count: %d Total time: %f\n", a, total_strike_count, loop_count, (MPI_Wtime() - tmr));
		}

		summed_total_strike_counts += total_strike_count; //for averaging sum the total strike count
		summed_total_loop_counts += loop_count; //for averaging the loop counts over each 60 second repeat
	}

	if (rank == 0) {
		printf("Averaged Strike Count: %d, Averaged Loop Count: %lu, Number Processes: %d\n", ( summed_total_strike_counts / averaging_loops ), ( summed_total_loop_counts / averaging_loops ), size);
	}

	MPI_Finalize();

	return 0;
}