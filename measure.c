// mpicc -o measure measure.c
// mpirun -np 2 ./measure

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

static double measure_ts(int rank, int tag)
{
  MPI_Request send_request, recv_request;
  double start_time, communication_time;
  char *send_buffer;
  char *recv_buffer;
  // Measure communication startup time
  start_time = MPI_Wtime();

  if (rank == 0)
  {
    // Process 0 sends a message to process 1
    MPI_Isend(send_buffer, 0, MPI_CHAR, 1, tag, MPI_COMM_WORLD, &send_request);
  }
  else if (rank == 1)
  {
    // Process 1 receives a message from process 0
    MPI_Irecv(recv_buffer, 0, MPI_CHAR, 0, tag, MPI_COMM_WORLD, &recv_request);
  }

  // Complete the non-blocking communication
  if (rank == 0)
  {
    MPI_Wait(&send_request, MPI_STATUS_IGNORE);
  }
  else if (rank == 1)
  {
    MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
  }

  // Calculate and report communication startup time
  communication_time = MPI_Wtime() - start_time;
  if (rank < 2)
  {
    return communication_time;
  }
  else
  {
    return 0;
  }
}

static double measure_tw(int rank, int tag)
{
  int msg_size = 8000;
  // Allocate memory for send and receive buffers
  char *send_buffer = (char *)malloc(msg_size * sizeof(char));
  char *recv_buffer = (char *)malloc(msg_size * sizeof(char));

  // Ensure the buffers are initialized
  for (int i = 0; i < msg_size; i++)
  {
    send_buffer[i] = 'a';
    recv_buffer[i] = 'b';
  }
  MPI_Request send_request, recv_request;
  double start_time, communication_time;
  // Measure communication startup time
  start_time = MPI_Wtime();
  if (rank == 0)
  {
    // Process 0 sends a message to process 1
    MPI_Isend(send_buffer, msg_size, MPI_CHAR, 1, tag, MPI_COMM_WORLD, &send_request);
  }
  else if (rank == 1)
  {
    // Process 1 receives a message from process 0
    MPI_Irecv(recv_buffer, msg_size, MPI_CHAR, 0, tag, MPI_COMM_WORLD, &recv_request);
  }

  // Complete the non-blocking communication
  if (rank == 0)
  {
    MPI_Wait(&send_request, MPI_STATUS_IGNORE);
  }
  else if (rank == 1)
  {
    MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
  }

  // Calculate and report communication startup time
  communication_time = MPI_Wtime() - start_time;
  if (rank < 2)
  {
    return communication_time;
  }
  else
  {
    return 0;
  }
  free(send_buffer);
  free(recv_buffer);
}

static double measure_tf(int rank)
{
  double num = 2.76;
  double start_time, computation_time;
  start_time = MPI_Wtime();
  for (int i = 0; i < 100000; i++)
  {
    num = num * num;
    num = num + num;
    num = num - num;
    num = num + num;
  }
  computation_time = MPI_Wtime() - start_time;
  if (rank < 2)
  {
    return computation_time;
  }
  else
  {
    return 0;
  }
}

int main(int argc, char *argv[])
{
  int rank, size;
  int tag = 0;

  // Initialize MPI
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (size < 2)
  {
    printf("Please run with at least two processes.\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  // Measure communication startup time
  double ts = 0;
  for (int i = 0; i < 1; i++)
  {
    ts += measure_ts(rank, tag);
  }
  printf("Process %d communication startup time: %f seconds\n", rank, ts);
  // Measure communication per-word time
  double tw = 0;
  for (int i = 0; i < 1; i++)
  {
    tw += measure_tw(rank, tag);
  }
  printf("Process %d communication 8000-word time: %f seconds\n", rank, tw);
  measure_tw(rank, tag);
  // Measure computation time
  double tf = 0;
  for (int i = 0; i < 1; i++)
  {
    tf += measure_tf(rank);
  }
  printf("Process %d 4*100000 float computation time: %f seconds \n", rank, tf);
  // Finalize MPI
  MPI_Finalize();

  return 0;
}
