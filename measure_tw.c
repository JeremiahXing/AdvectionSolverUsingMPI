// mpicc -o measure_tw measure_tw.c
// mpirun -np 2 ./measure_tw

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
  int rank, size, tag = 0;
  double communication_time;
  double overhead = MPI_Wtime();
  overhead = MPI_Wtime() - overhead;
  MPI_Request req;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  if (size != 2)
  {
    printf("This program must be run with 2 processes only.\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
  int msg_size = 1000000;
  double *send_buffer = (double *)malloc(msg_size * sizeof(double));
  double *recv_buffer = (double *)malloc(msg_size * sizeof(double));

  if (rank == 0)
  {
    MPI_Barrier(MPI_COMM_WORLD);
    communication_time = MPI_Wtime();
    MPI_Send(send_buffer, msg_size, MPI_DOUBLE, 1, tag, MPI_COMM_WORLD);
    MPI_Recv(recv_buffer, msg_size, MPI_DOUBLE, 1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    communication_time = MPI_Wtime() - communication_time - overhead;
  }
  else if (rank == 1)
  {
    MPI_Barrier(MPI_COMM_WORLD);
    communication_time = MPI_Wtime();
    MPI_Recv(recv_buffer, msg_size, MPI_DOUBLE, 0, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Send(send_buffer, msg_size, MPI_DOUBLE, 0, tag, MPI_COMM_WORLD);
    communication_time = MPI_Wtime() - communication_time - overhead;
  }
  free(send_buffer);
  free(recv_buffer);
  if (rank == 0)
  {
    printf("Communication per-word time: %.3e seconds (rank %d)\n", communication_time / msg_size / 2, rank);
  }
  else if (rank == 1)
  {
    printf("Communication per-word time: %.3e seconds (rank %d)\n", communication_time / msg_size / 2, rank);
  }
  MPI_Finalize();
  return 0;
}