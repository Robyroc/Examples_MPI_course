#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define RADIUS 100


int main(int argc, char** argv)
{
    srand(time(NULL));
    int rank, size;
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    long int samples;
    if(rank == 0)
    {
        printf("Number of samples: ");
        scanf("%ld\n", &samples);
        samples /= size;
    }
    MPI_Bcast(&samples, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    long int i;
    long int inside = 0;
    for(i = 0; i < samples; i++)
    {
        int x = rand() % (RADIUS+1); //[0,RADIUS]
        int y = rand() % (RADIUS+1); //[0,RADIUS]
        if(x*x + y*y <= RADIUS*RADIUS)
            inside++;
    }
    long int total;
    MPI_Reduce(&inside, &total, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    if(rank == 0)
    {
        double ratio = total / ((float) samples * size);
        printf("Value of pi: %f\n", ratio * 4);
    }
    MPI_Finalize()
    return 0;
}