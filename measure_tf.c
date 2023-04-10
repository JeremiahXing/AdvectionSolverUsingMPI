// mpicc -o measure_tf measure_tf.c
// mpirun -np 1 ./measure_tf

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
  double num = 2.76;
  double start_time, end_time;
  double computation_time = 0;
  int reps = 100000000;
  double overhead = MPI_Wtime();
  overhead = MPI_Wtime() - overhead;
  for (int i = 0; i < reps; i++)
  {
    start_time = MPI_Wtime();
    num = num * num;
    num = num + num;
    num = num - num;
    num = num + num;
    end_time = MPI_Wtime();
    computation_time += end_time - start_time;
  }
  printf("Float computation time: %.3e seconds \n", computation_time / 4 / reps);
  return 0;
}