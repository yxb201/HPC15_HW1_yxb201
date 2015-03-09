CC=mpicc

all: int_ring jacobi-mpi

int_ring: int_ring.c
	$(CC) int_ring.c -o int_ring

jacobi-mpi: jacobi-mpi.c
	$(CC) jacobi-mpi.c -o jacobi-mpi

clean:
	rm int_ring jacobi-mpi
