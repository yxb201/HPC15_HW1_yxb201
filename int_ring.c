/* 
 *	MPI send/receive in a ring pattern
 *
 *	Yuan Xun Bao
 *	March 2015
 *
 */

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include "util.h"

int main(int argc, char *argv[])
{

	int N, rank, size, dest, origin, tag, loop;
	int message_in, message_out;
	timestamp_type start_t, stop_t;

	MPI_Init( &argc, &argv );
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	if (argc != 2){
		fprintf(stderr, "Must specify the number of loops N.\n");
		MPI_Abort(MPI_COMM_WORLD,1);
	}
	
	N = atoi(argv[1]);
	message_in = 0;
	MPI_Status status;
	tag = 99;
	loop = 0; 

	get_timestamp(&start_t);
		
	while (loop < N)
	{
		if (rank == 0){
			message_out =  message_in + rank;
			dest = (rank + 1) % size;
			origin = size - 1; 
			MPI_Send(&message_out, 1, MPI_INT, dest  , tag, MPI_COMM_WORLD );
			MPI_Recv(&message_in , 1, MPI_INT, origin, tag, MPI_COMM_WORLD, &status );
		}
		else{
			dest   = (rank + 1 ) % size;
			origin = rank - 1;
			MPI_Recv(&message_in , 1, MPI_INT, origin, tag, MPI_COMM_WORLD, &status );
			message_out = message_in + rank; 
			MPI_Send(&message_out, 1, MPI_INT, dest  , tag, MPI_COMM_WORLD );
		}
		
//		printf("Loop %d: I am rank %d. I received %d from rank %d. I sent %d to rank %d.\n",loop+1, rank, message_in, origin, message_out, dest);
	
	loop++; 
	}
	
	get_timestamp(&stop_t);

	if (rank == 0){
		double elapsed = timestamp_diff_in_seconds(start_t, stop_t);
		printf("Total elapsed time is %.6f seconds.\n", elapsed);
		printf("Latency is %.12f seconds per send/receive.\n", elapsed/N/size);
	}	
	

	MPI_Finalize();
	return 0;

}
