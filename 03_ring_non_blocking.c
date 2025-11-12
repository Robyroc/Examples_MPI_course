#include <stdio.h>
#include "mpi.h"

int main(int argc, char** argv)
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //We are exchanging rank values in a ring format, each process sends its own rank to the one with the following rank in a circular fashion. 
    int value;
    int source = (rank == 0 ? size-1 : rank-1);
    int destination = (rank == size-1 ? 0 : rank+1);
    printf("%d/%d, sending data to %d, expecting from %d\n", rank, size, destination, source);

    MPI_Request req;
    MPI_Isend(&rank, 1, MPI_INT, destination, 0, MPI_COMM_WORLD, &req); //Non-blocking
    MPI_Recv(&value, 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); //Blocking
    MPI_Wait(&req, MPI_STATUS_IGNORE); //Waiting for the send to complete
    printf("%d/%d, received %d\n", rank, size, value);
    MPI_Finalize();
    return 0;
}