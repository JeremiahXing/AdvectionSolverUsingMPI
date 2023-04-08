// parallel 2D advection solver module
// written for COMP4300/8300 Assignment 1
// v1.0 25 Feb

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>

#include "serAdvect.h"

#define HALO_TAG 100

int M_loc, N_loc;  // local advection field size (excluding halo)
int M0, N0;        // local field element (0,0) is global element (M0,N0)
static int P0, Q0; // 2D process id (P0, Q0) in P x Q process grid

static int M, N, P, Q; // local store of problem parameters
static int verbosity;
static int rank, nprocs; // MPI values
static MPI_Comm comm;

// sets up parallel parameters above
void initParParams(int M_, int N_, int P_, int Q_, int verb)
{
  M = M_, N = N_;
  P = P_, Q = Q_;
  verbosity = verb;
  comm = MPI_COMM_WORLD;
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &nprocs);

  // original code snips (1D)
  // P0 = rank;
  // M0 = (M / P) * P0; //the starting row (M0)
  // M_loc = (P0 < P-1)? (M / P): (M - M0); //local number of rows (M_loc)
  // // Why this M_loc need ?: operation
  // //-- in case M cant divided by P, e.g. m = 8 and p = 3 then p0 takes 2; p1 takes 2; p2 takes 4

  // assert (Q == 1);
  // Q0 = 0;
  // N0 = 0;
  // N_loc = N;
  // original code snips end

  // 2D
  P0 = rank / Q;
  Q0 = rank % Q;

  M0 = (M / P) * P0;
  M_loc = (P0 < P - 1) ? (M / P) : (M - M0);

  N0 = (N / Q) * Q0;
  N_loc = (Q0 < Q - 1) ? (N / Q) : (N - N0);
  // 2D ends

} // initParParams()

void checkHaloSize(int w)
{
  // Old code
  //  if (w > M_loc || w > N_loc)
  //  {
  //    printf("%d: w=%d too large for %dx%d local field! Exiting...\n",
  //           rank, w, M_loc, N_loc);
  //    exit(1);
  //  }

  // new code
  int error = 0;
  int global_error = 0;
  if (w > M_loc || w > N_loc || w < 1)
    error = 1;
  MPI_Allreduce(&error, &global_error, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
  if (global_error)
  {
    if (rank == 0)
      printf("Error: w=%d invalid for some local fields! Exiting...\n", w);
    exit(1);
  }
}

static void updateBoundary(double *u, int ldu)
{
  int i, j;

  // top and bottom halo
  // note: we get the left/right neighbour's corner elements from each end
  if (P == 1)
  {
    for (j = 1; j < N_loc + 1; j++)
    {
      V(u, 0, j) = V(u, M_loc, j);
      V(u, M_loc + 1, j) = V(u, 1, j);
    }
  }
  else
  {
    // original solution start
    // int topProc = (rank + 1) % nprocs, botProc = (rank - 1 + nprocs) % nprocs;
    // MPI_Send(&V(u, M_loc, 1), N_loc, MPI_DOUBLE, topProc, HALO_TAG, comm);
    // MPI_Recv(&V(u, 0, 1), N_loc, MPI_DOUBLE, botProc, HALO_TAG, comm,
    //    MPI_STATUS_IGNORE);
    // MPI_Send(&V(u, 1, 1), N_loc, MPI_DOUBLE, botProc, HALO_TAG, comm);
    // MPI_Recv(&V(u, M_loc+1, 1), N_loc, MPI_DOUBLE, topProc, HALO_TAG,
    //    comm, MPI_STATUS_IGNORE);
    // original solution end

    // Task_1 solution start
    // int topProc = (rank + 1) % nprocs, botProc = (rank - 1 + nprocs) % nprocs;
    // if (rank % 2 == 0){
    // MPI_Send(&V(u, M_loc, 1), N_loc, MPI_DOUBLE, topProc, HALO_TAG, comm);
    // MPI_Recv(&V(u, 0, 1), N_loc, MPI_DOUBLE, botProc, HALO_TAG, comm,
    //    MPI_STATUS_IGNORE);
    // MPI_Send(&V(u, 1, 1), N_loc, MPI_DOUBLE, botProc, HALO_TAG, comm);
    // MPI_Recv(&V(u, M_loc+1, 1), N_loc, MPI_DOUBLE, topProc, HALO_TAG,
    //    comm, MPI_STATUS_IGNORE);
    // } else {
    //   MPI_Recv(&V(u, 0, 1), N_loc, MPI_DOUBLE, botProc, HALO_TAG, comm,
    //     MPI_STATUS_IGNORE);
    //   MPI_Send(&V(u, M_loc, 1), N_loc, MPI_DOUBLE, topProc, HALO_TAG, comm);
    //   MPI_Recv(&V(u, M_loc+1, 1), N_loc, MPI_DOUBLE, topProc, HALO_TAG,
    //     comm, MPI_STATUS_IGNORE);
    //   MPI_Send(&V(u, 1, 1), N_loc, MPI_DOUBLE, botProc, HALO_TAG, comm);
    // }
    // Task_1 solution end

    // Task_2 solution start
    //  int topProc = (rank + 1) % nprocs;
    //  int botProc = (rank - 1 + nprocs) % nprocs;
    //  MPI_Request req[4];
    //  MPI_Status stat[4];

    // MPI_Isend(&V(u, M_loc, 1), N_loc, MPI_DOUBLE, topProc, HALO_TAG, comm, &req[0]);
    // MPI_Irecv(&V(u, 0, 1), N_loc, MPI_DOUBLE, botProc, HALO_TAG, comm, &req[1]);
    // MPI_Isend(&V(u, 1, 1), N_loc, MPI_DOUBLE, botProc, HALO_TAG, comm, &req[2]);
    // MPI_Irecv(&V(u, M_loc+1, 1), N_loc, MPI_DOUBLE, topProc, HALO_TAG, comm, &req[3]);

    // MPI_Waitall(4, req, stat);
    // Task_2 solution end

    // 2D
    int topProc = (rank - Q + nprocs) % nprocs;
    int botProc = (rank + Q) % nprocs;

    MPI_Request req[4];
    // IT MUST BE bot top top bot!!!!
    MPI_Isend(&V(u, M_loc, 1), N_loc, MPI_DOUBLE, botProc, HALO_TAG, comm, &req[0]);
    MPI_Irecv(&V(u, 0, 1), N_loc, MPI_DOUBLE, topProc, HALO_TAG, comm, &req[1]);
    MPI_Isend(&V(u, 1, 1), N_loc, MPI_DOUBLE, topProc, HALO_TAG, comm, &req[2]);
    MPI_Irecv(&V(u, M_loc + 1, 1), N_loc, MPI_DOUBLE, botProc, HALO_TAG, comm, &req[3]);

    MPI_Waitall(4, req, MPI_STATUS_IGNORE);
  }

  // left and right sides of halo
  if (Q == 1)
  {
    for (i = 0; i < M_loc + 2; i++)
    {
      V(u, i, 0) = V(u, i, N_loc);
      V(u, i, N_loc + 1) = V(u, i, 1);
    }
  }
  else
  {
    // as elements in a column in a 2d array are not location neighboring we need to define a new type column
    MPI_Datatype column_type;
    MPI_Type_vector(M_loc + 2, 1, N_loc + 2, MPI_DOUBLE, &column_type);
    // MPI_Type_vector(count, blocklen, stride, oldtype, newtype)
    MPI_Type_commit(&column_type);

    int col = rank / Q;
    int scaledLeftProc = ((rank - col * Q - 1 + Q) % Q);
    int leftProc = scaledLeftProc + col * Q;
    int scaledRightProc = ((rank - col * Q + 1) % Q);
    int rightProc = scaledRightProc + col * Q;

    MPI_Request req[4];

    MPI_Isend(&V(u, 0, N_loc), 1, column_type, rightProc, HALO_TAG, comm, &req[0]);
    MPI_Irecv(&V(u, 0, 0), 1, column_type, leftProc, HALO_TAG, comm, &req[1]);
    MPI_Isend(&V(u, 0, 1), 1, column_type, leftProc, HALO_TAG, comm, &req[2]);
    MPI_Irecv(&V(u, 0, N_loc + 1), 1, column_type, rightProc, HALO_TAG, comm, &req[3]);

    MPI_Waitall(4, req, MPI_STATUS_IGNORE);
    MPI_Type_free(&column_type);
  }
} // updateBoundary()

static void overlapUpdateBoundary(double *u, int ldu, MPI_Request *req)
{
  int j;

  // top and bottom halo
  // note: we get the left/right neighbour's corner elements from each end
  if (P == 1)
  {
    for (j = 1; j < N_loc + 1; j++)
    {
      V(u, 0, j) = V(u, M_loc, j);
      V(u, M_loc + 1, j) = V(u, 1, j);
    }
  }
  else
  {

    int topProc = (rank - Q + nprocs) % nprocs;
    int botProc = (rank + Q) % nprocs;

    MPI_Isend(&V(u, M_loc, 1), N_loc, MPI_DOUBLE, botProc, HALO_TAG, comm, &req[0]);
    MPI_Irecv(&V(u, 0, 1), N_loc, MPI_DOUBLE, topProc, HALO_TAG, comm, &req[1]);
    MPI_Isend(&V(u, 1, 1), N_loc, MPI_DOUBLE, topProc, HALO_TAG, comm, &req[2]);
    MPI_Irecv(&V(u, M_loc + 1, 1), N_loc, MPI_DOUBLE, botProc, HALO_TAG, comm, &req[3]);
  }
} // overlapUpdateBoundary()

static void wideUpdateBoundary(double *u, int ldu, int w)
{
  int i, j;

  // top and bottom halo
  // note: we get the left/right neighbour's corner elements from each end
  if (P == 1)
  {
    for (j = w; j < N_loc + w; j++)
    {
      V(u, 0, j) = V(u, M_loc + w - 1, j);
      V(u, M_loc + w, j) = V(u, w, j);
    }
  }
  else
  {
    MPI_Datatype wide_row_type;
    MPI_Type_vector(w, N_loc, N_loc + 2 * w, MPI_DOUBLE, &wide_row_type);
    // MPI_Type_vector(count, blocklen, stride, oldtype, newtype)
    MPI_Type_commit(&wide_row_type);

    int topProc = (rank - Q + nprocs) % nprocs;
    int botProc = (rank + Q) % nprocs;

    MPI_Request req[4];

    MPI_Isend(&V(u, M_loc, w), 1, wide_row_type, botProc, HALO_TAG, comm, &req[0]);
    MPI_Irecv(&V(u, 0, w), 1, wide_row_type, topProc, HALO_TAG, comm, &req[1]);
    MPI_Isend(&V(u, w, w), 1, wide_row_type, topProc, HALO_TAG, comm, &req[2]);
    MPI_Irecv(&V(u, M_loc + w, w), 1, wide_row_type, botProc, HALO_TAG, comm, &req[3]);

    MPI_Waitall(4, req, MPI_STATUS_IGNORE);
    MPI_Type_free(&wide_row_type);
  }

  // left and right sides of halo
  if (Q == 1)
  {
    for (i = 0; i < M_loc + 2 * w; i++)
    {
      V(u, i, 0) = V(u, i, N_loc);
      V(u, i, N_loc + w) = V(u, i, w);
    }
  }
  else
  {
    // as elements in a column in a 2d array are not location neighboring we need to define a new type column
    MPI_Datatype wide_column_type;
    MPI_Type_vector(M_loc + 2 * w, w, N_loc + 2 * w, MPI_DOUBLE, &wide_column_type);
    // MPI_Type_vector(count, blocklen, stride, oldtype, newtype)
    MPI_Type_commit(&wide_column_type);

    int col = rank / Q;
    int scaledLeftProc = ((rank - col * Q - 1 + Q) % Q);
    int leftProc = scaledLeftProc + col * Q;
    int scaledRightProc = ((rank - col * Q + 1) % Q);
    int rightProc = scaledRightProc + col * Q;

    MPI_Request req[4];

    MPI_Isend(&V(u, 0, N_loc), 1, wide_column_type, rightProc, HALO_TAG, comm, &req[0]);
    MPI_Irecv(&V(u, 0, 0), 1, wide_column_type, leftProc, HALO_TAG, comm, &req[1]);
    MPI_Isend(&V(u, 0, w), 1, wide_column_type, leftProc, HALO_TAG, comm, &req[2]);
    MPI_Irecv(&V(u, 0, N_loc + w), 1, wide_column_type, rightProc, HALO_TAG, comm, &req[3]);

    MPI_Waitall(4, req, MPI_STATUS_IGNORE);
    MPI_Type_free(&wide_column_type);
  }
} // wideUpdateBoundary()

// evolve advection over r timesteps, with (u,ldu) containing the local field
void parAdvect(int reps, double *u, int ldu)
{
  // printf("P: %d, Q: %d, M_loc: %d, N_loc: %d\n", P, Q, M_loc, N_loc);
  int r;
  double *v;
  int ldv = N_loc + 2;
  v = calloc(ldv * (M_loc + 2), sizeof(double));
  assert(v != NULL);
  assert(ldu == N_loc + 2);

  for (r = 0; r < reps; r++)
  {
    updateBoundary(u, ldu);
    updateAdvectField(M_loc, N_loc, &V(u, 1, 1), ldu, &V(v, 1, 1), ldv);
    copyField(M_loc, N_loc, &V(v, 1, 1), ldv, &V(u, 1, 1), ldu);

    if (verbosity > 2)
    {
      char s[64];
      sprintf(s, "%d reps: u", r + 1);
      printAdvectField(rank, s, M_loc + 2, N_loc + 2, u, ldu);
    }
  }
  free(v);
} // parAdvect()

// overlap communication variant
void parAdvectOverlap(int reps, double *u, int ldu)
{
  int r;
  double *v;
  int ldv = N_loc + 2;
  v = calloc(ldv * (M_loc + 2), sizeof(double));
  assert(Q == 1); // only works for Q=1
  assert(v != NULL);
  assert(ldu == N_loc + 2);
  MPI_Request req[4];

  for (r = 0; r < reps; r++)
  {
    // star communication in u for boundary but not wait msg to arrive
    overlapUpdateBoundary(u, ldu, req);
    // compute v's inner independent part
    updateAdvectField(M_loc - 2, N_loc - 2, &V(u, 2, 2), ldu, &V(v, 2, 2), ldv);
    // wait for boundary msg to arrive
    MPI_Waitall(4, req, MPI_STATUS_IGNORE);

    for (int i = 0; i < M_loc + 2; i++)
    {
      V(u, i, 0) = V(u, i, N_loc);
      V(u, i, N_loc + 1) = V(u, i, 1);
    }
    // compute v's inner dependent part (inner halo) it takes 4 times(top/bottom/left/right)
    updateAdvectField(1, N_loc, &V(u, 1, 1), ldu, &V(v, 1, 1), ldv);             // top
    updateAdvectField(1, N_loc, &V(u, M_loc, 1), ldu, &V(v, M_loc, 1), ldv);     // bottom
    updateAdvectField(M_loc - 2, 1, &V(u, 2, 1), ldu, &V(v, 2, 1), ldv);         // left
    updateAdvectField(M_loc - 2, 1, &V(u, 2, N_loc), ldu, &V(v, 2, N_loc), ldv); // right

    // copy v to u
    copyField(M_loc, N_loc, &V(v, 1, 1), ldv, &V(u, 1, 1), ldu);

    if (verbosity > 2)
    {
      char s[64];
      sprintf(s, "%d reps: u", r + 1);
      printAdvectField(rank, s, M_loc + 2, N_loc + 2, u, ldu);
    }
  }
  free(v);
} // parAdvectOverlap()

// key idea: modify the communication function (updateBoundary())
// wide halo variant
void parAdvectWide(int reps, int w, double *u, int ldu)
{
  int r;
  double *v;
  int ldv = N_loc + 2 * w;
  v = calloc(ldv * (M_loc + 2 * w), sizeof(double));
  assert(v != NULL);
  assert(ldu == N_loc + 2 * w);

  for (r = 0; r < reps; r++)
  {
    wideUpdateBoundary(u, ldu, w);
    for (int i = 1; i <= w - 1 && r < reps; i++, r++)
    {
      updateAdvectField(M_loc + 2 * (w - i), N_loc + 2 * (w - i), &V(u, i, i), ldu, &V(v, i, i), ldv);
      copyField(M_loc + 2 * (w - i), N_loc + 2 * (w - i), &V(v, i, i), ldv, &V(u, i, i), ldu);
    }
    if (r < reps)
    {
      updateAdvectField(M_loc, N_loc, &V(u, w, w), ldu, &V(v, w, w), ldv);
      copyField(M_loc, N_loc, &V(v, w, w), ldv, &V(u, w, w), ldu);
    }
    if (verbosity > 2)
    {
      char s[64];
      sprintf(s, "%d reps: u", r + 1);
      printAdvectField(rank, s, M_loc + 2 * w, N_loc + 2 * w, u, ldu);
    }
  }
} // parAdvectWide()

// extra optimization variant
void parAdvectExtra(int r, double *u, int ldu)
{

} // parAdvectExtra()
