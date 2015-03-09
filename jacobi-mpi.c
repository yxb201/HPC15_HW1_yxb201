/*
 *
 *	Parallel Jacobi using MPI
 *
 *	Yuan Xun Bao
 *	March 2015
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include "util.h"

int main (int argc, char *argv[])
{


	int Niter, Ntotal, Nsub, mpisize, mpirank, tag, j, n;
	double h, h2, f, head, tail;
	double *u, *unew;
	double before, next; 	
	timestamp_type start_t, stop_t;
	
	MPI_Init( &argc, &argv );
	MPI_Comm_size(MPI_COMM_WORLD, &mpisize );
	MPI_Comm_rank(MPI_COMM_WORLD, &mpirank );

	if(argc != 3){
		fprintf(stderr, "must have two arguments: matrix size N and max number of iterations!\n");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	Ntotal  = atoi(argv[1]);
	Niter   = atoi(argv[2]);

	if (Ntotal % mpisize !=0 ){
		fprintf(stderr, "Vector size N is not divisible by # of processes!\n");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	
	MPI_Status status;
	Nsub = Ntotal/mpisize; 
	tag = 99;
	h = 1./(Ntotal+1); h2 = h*h;
	f = 1.;
	before = 0.; next = 0.;
	
	// u[1..Nsub] are the actual entries, u[1], u[Nsub+1] are ghost cells
	u    = (double *) malloc( (Nsub+2) * sizeof(double) );
	unew = (double *) malloc( (Nsub+2) * sizeof(double) );
	
	// initialize vectors
	for (j=0 ; j<Nsub+2; j++){
		u[j] = 0; unew[j] = 0;
	}	

	get_timestamp(&start_t);

	// Jacobi iteration
	for (n = 0; n < Niter; n++){
	
		if (mpirank == 0)
		{
			// do work
			for( j=1; j<=Nsub; j++ ){
				unew[j] = (h2*f + u[j-1] + u[j+1])*0.5;
			}
	
			tail = unew[Nsub]; // tail to be sent to rank 1
			MPI_Send( &tail, 1, MPI_DOUBLE, 1, tag, MPI_COMM_WORLD );
			MPI_Recv( &next, 1, MPI_DOUBLE, 1, tag, MPI_COMM_WORLD, &status );
			unew[Nsub+1] = next; // store message from rank 1 in ghost cell
				
		}
		else if (mpirank == mpisize-1)
		{	
			// do work
			for( j=1; j<=Nsub; j++ ){
				unew[j] = (h2*f + u[j-1] + u[j+1])*0.5;
			}
			
			head = unew[1]; // head to be sent to mpirank-1
			MPI_Recv( &before, 1, MPI_DOUBLE, mpirank-1, tag, MPI_COMM_WORLD, &status );
			MPI_Send( &head,   1, MPI_DOUBLE, mpirank-1, tag, MPI_COMM_WORLD );
			unew[0] = before; // store message from rank mpirank-1 in ghost cell
		
		}
		else
		{	
			// do work 
			for( j=1; j<=Nsub; j++ ){
				unew[j] = (h2*f + u[j-1] + u[j+1])*0.5;
			}

			head = unew[1]; tail = unew[Nsub-1];
			MPI_Recv( &before, 1, MPI_DOUBLE, mpirank-1, tag, MPI_COMM_WORLD, &status );
			MPI_Send( &head,   1, MPI_DOUBLE, mpirank-1, tag, MPI_COMM_WORLD );
			MPI_Send( &tail,   1, MPI_DOUBLE, mpirank+1, tag, MPI_COMM_WORLD );
			MPI_Recv( &next,   1, MPI_DOUBLE, mpirank+1, tag, MPI_COMM_WORLD, &status );
	  		unew[0] = before;
			unew[Nsub+1] = next;

				}
		
		// copy unew -> u
		for(j = 0; j<Nsub+2 ; j++){
			u[j] = unew[j];
		}		

	}		
	
	
	get_timestamp(&stop_t);
	
	double elapsed = timestamp_diff_in_seconds(start_t, stop_t);

	if(mpirank == mpisize-1){
		printf("Time elaspsed is %f seconds.\n", elapsed);
	}	
	
	/*
	FILE* fd = NULL;
	char filename[256];
	snprintf(filename, 256, "output%02d.txt", mpirank);
	fd = fopen(filename, "w+");

	if (NULL == fd)
	{
		printf("Error opening file \n");
		return 1;
	}
	
	for(j = 0; j<Nsub+2; j++){
		fprintf(fd, "u[%d] = %.12f\n",j+1, u[j]);
	}

	fclose(fd);
	*/

	free(u); free(unew);
	
	MPI_Finalize();
	return 0;
}
