#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define MATRIX_LENGTH 100

int main(int argc, char** argv)
{
    srand(time(NULL));
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int dimensions[2];
    MPI_Dims_create(size, 2, dimensions);
    int periods[2];
    periods[0] = 1; periods[1] = 1;

    MPI_Comm topo;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dimensions, periods, 0, &topo);

    //Initialise data, we put one additional row at top and bottom, left & right for halos
    int data[MATRIX_LENGTH+2][MATRIX_LENGTH+2];
    long int sum = 0;
    //We initialise everything except for border area
    for(int i = 1; i < MATRIX_LENGTH + 1; i++)
        for(int j = 1; j < MATRIX_LENGTH + 1; j++)
        {
            data[i][j] = rand() % 500;
            sum += data[i][j];
        }

    if(rank == 0)
    {
        printf("dimensions %d %d\n", dimensions[0], dimensions[1]);
        long int total;
        MPI_Comm_reduce(&sum, &total, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD)
        float average = ((float) total) / MATRIX_LENGTH / MATRIX_LENGTH / size;
        printf("Average: %f\n", average);
    }
    else
    {
        MPI_Comm_reduce(&sum, NULL, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    }

    for(int iteration = 0; iteration < 100; iteration++)
    {
        //Now we send the topmost row to the process above, and we receive from the one below us, and we put it inside the halo
        int source, dest;
        MPI_Cart_shift(topo, 0, -1, &source, &dest)
        printf("%d N, sending to %d, receiving from %d\n", rank, source, dest);
        MPI_Sendrecv(&(data[1][1]), MATRIX_LENGTH, MPI_INT, dest, 0,
                    &(data[MATRIX_LENGTH+1][1]), MATRIX_LENGTH, MPI_INT, source, 0, topo, MPI_STATUS_IGNORE);

        //Same for bottom row
        MPI_Cart_shift(topo, 0, 1, &source, &dest)
        printf("%d S, sending to %d, receiving from %d\n", rank, source, dest);
        MPI_Sendrecv(&(data[MATRIX_LENGTH][1]), MATRIX_LENGTH, MPI_INT, dest, 1,
                    &(data[0][1]), MATRIX_LENGTH, MPI_INT, source, 1, topo, MPI_STATUS_IGNORE);

        //for left and right, we must first move elements around
        int temp[2][MATRIX_LENGTH];
        for(int i = 0; i < MATRIX_LENGTH; i++)
            temp[0][i] = data[1][i+1];
        MPI_Cart_shift(topo, 1, -1, &source, &dest)
        printf("%d W, sending to %d, receiving from %d\n", rank, source, dest);
        MPI_Sendrecv(temp[0], MATRIX_LENGTH, MPI_INT, dest, 2,
                    temp[1], MATRIX_LENGTH, MPI_INT, source, 2, topo, MPI_STATUS_IGNORE);
        for(int i = 0; i < MATRIX_LENGTH; i++)
            data[MATRIX_LENGTH+1][i+1] = temp[1][i];

        for(int i = 0; i < MATRIX_LENGTH; i++)
            temp[0][i] = data[MATRIX_LENGTH][i+1];
        MPI_Cart_shift(topo, 1, 1, &source, &dest)
        printf("%d E, sending to %d, receiving from %d\n", rank, source, dest);
        MPI_Sendrecv(temp[0], MATRIX_LENGTH, MPI_INT, dest, 2,
                    temp[1], MATRIX_LENGTH, MPI_INT, source, 2, topo, MPI_STATUS_IGNORE);
        for(int i = 0; i < MATRIX_LENGTH; i++)
            data[0][i+1] = temp[1][i];

        MPI_Waitall(8, requests, MPI_STATUSES_IGNORE);

        int changes[MATRIX_LENGTH][MATRIX_LENGTH]
        //Now we can operate on each singular process separately
        for(int i = 1; i < MATRIX_LENGTH + 1; i++)
            for(int j = 1; j < MATRIX_LENGTH + 1; j++)
            {
                int sum = 0;
                sum += data[i][j];
                sum += data[i+1][j];
                sum += data[i-1][j];
                sum += data[i][j+1];
                sum += data[i][j-1];
                sum /= 5;
                sum -= data[i][j];
                changes[i-1][j-1] = sum;
            }
        
        for(int i = 0; i < MATRIX_LENGTH; i++)
            for(int j = 0; j < MATRIX_LENGTH; j++)
                data[i+1][j+1] += changes[i][j];

        printf("%d random element: %d\n", rank, data[rand() % MATRIX_LENGTH + 1][rand() % MATRIX_LENGTH + 1]);
        MPI_Barrier(topo);
    }

    MPI_Finalize();
    return 0;
}
