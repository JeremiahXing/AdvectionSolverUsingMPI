// mpicc -o measure_ts measure_ts.c
// mpirun -np 2 ./measure_ts

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
  int rank, size, tag = 0;
  double communication_time = 0;
  double start_time;
  double end_time;
  double overhead = MPI_Wtime();
  overhead = MPI_Wtime() - overhead;
  MPI_Request req;
  int reps = 1000;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  if (size != 2)
  {
    printf("This program must be run with 2 processes only.\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  if (rank == 0)
  {
    communication_time = 0;
    MPI_Barrier(MPI_COMM_WORLD);
    for (int i = 0; i < reps; i++)
    {
      start_time = MPI_Wtime();
      MPI_Send(NULL, 0, MPI_DOUBLE, 1, tag, MPI_COMM_WORLD);
      MPI_Recv(NULL, 0, MPI_DOUBLE, 1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      end_time = MPI_Wtime();
      communication_time += end_time - start_time;
    }
  }
  else if (rank == 1)
  {
    communication_time = 0;
    MPI_Barrier(MPI_COMM_WORLD);
    for (int i = 0; i < reps; i++)
    {
      start_time = MPI_Wtime();
      MPI_Recv(NULL, 0, MPI_DOUBLE, 0, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Send(NULL, 0, MPI_DOUBLE, 0, tag, MPI_COMM_WORLD);
      end_time = MPI_Wtime();
      communication_time += end_time - start_time;
    }
  }
  if (rank == 0)
  {
    printf("Communication startup time: %.3e seconds (rank %d)\n", communication_time / reps / 2, rank);
  }
  else if (rank == 1)
  {
    printf("Communication startup time: %.3e seconds (rank %d)\n", communication_time / reps / 2, rank);
  }
  MPI_Finalize();
  return 0;
}