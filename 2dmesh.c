//Sean Totaram
//MPI 2D mesh

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include"mpi.h"

//total numbers to be summed
#define n 160

int main(int argc, char *argv[]){
    int size, rank, buf, sub;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int localsum, globalsum, i, j, sum[n];
    localsum = 0;
    globalsum = 0;
    //how many numbers each processor gets
    sub = n / (int)sqrt(size);
    //assigns numbers 1 - n
    if(rank == 0)
        for(i = 0; i < n; i++)
            sum[i] = i + 1;
    //sends data to the right of the mesh
    for(i = 0; i < sqrt(size) - 1; i++){
        buf = (int)(n * ((sqrt(size) - (i + 1)) / sqrt(size)));
        if(rank == i){
            MPI_Send(&sum[sub], buf, MPI_INT, i + 1, i, MPI_COMM_WORLD);
            printf("%d sent %d to %d\n", i, buf, i+1);
        }
        else if(rank == i + 1)
            MPI_Recv(&sum, buf, MPI_INT, i, i, MPI_COMM_WORLD, &status);
    }
    sub = n / size;
    //sends data to the bottom of the mesh
    for(i = 0; i < sqrt(size) - 1; i++){
        for(j = 0; j < sqrt(size); j++){
            buf =  n * (1 / sqrt(size)) * ((sqrt(size) - (i + 1)) / sqrt(size));
            if(rank == j + (sqrt(size) * i)){
                MPI_Send(&sum[sub], buf, MPI_INT, j + ((i + 1) * (int)sqrt(size)), i, MPI_COMM_WORLD);
                printf("%d sent %d to %d\n", (int)(j + (sqrt(size) * i)), buf, j + ((i + 1) * (int)sqrt(size)));
            }
            else if(rank == j + ((i + 1) * sqrt(size)))
                MPI_Recv(&sum, buf, MPI_INT, j + (sqrt(size) * i), i, MPI_COMM_WORLD, &status);
        }
    }
    //each proc calculates thier local sum of numbers
    for(i = 0; i < sub; i++){
        localsum = localsum + sum[i];
    }
    printf("processor: %d has a localsum of %d\n", rank, localsum);
    //sends local sum back up the mesh
    for(i = sqrt(size) - 2; i >= 0; i--){
        for(j = sqrt(size) - 1; j >= 0; j--){
            if(rank == j + (i + 1) * sqrt(size)){
                MPI_Send(&localsum, 1, MPI_INT, j + (sqrt(size) * i), i, MPI_COMM_WORLD);
                printf("%d sent to %d\n", (int)(j + ((i + 1) * sqrt(size))), (int)(j + (sqrt(size) * i)));
            }
            else if(rank == j + (sqrt(size) * i)){
                globalsum = localsum;
                MPI_Recv(&localsum, 1, MPI_INT, j + (i + 1) * sqrt(size), i, MPI_COMM_WORLD, &status);
                localsum = globalsum + localsum;
                globalsum = localsum;
            }
        }
    }
    //sends data back to root
    for(i = sqrt(size) - 1; i > 0; i--){
        if(rank == i){
            MPI_Send(&localsum, 1, MPI_INT, i - 1, i, MPI_COMM_WORLD);
            printf("%d sent to %d\n", i, i - 1);
        }
        else if(rank == i - 1){
            globalsum = localsum;
            MPI_Recv(&localsum, 1, MPI_INT, i, i, MPI_COMM_WORLD, &status);
            localsum = globalsum + localsum;
            globalsum = localsum;
        }
    }
    //total sum of all n numbers
    if(rank == 0)
        printf("The globalsum is: %d\n", globalsum);
    
    MPI_Finalize();
    return 0;
}
