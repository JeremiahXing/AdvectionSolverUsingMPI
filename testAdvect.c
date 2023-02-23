// parallel 2D advection solver test program
// written by Peter Strazdins for COMP4300/8300 Assignment 1
// v1.0 04 Mar 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //getopt()
#include <mpi.h>
#include <assert.h>

#include "serAdvect.h"
#include "parAdvect.h"

#define USAGE   "testAdvect [-P P] [-w w] [-o] [-x] [-v v] M N r"
#define DEFAULTS "P=nprocs w=r=1 v=0"
#define OPTCHARS "P:w:oxv:"

static int M, N;               // advection field size
static int P, Q;             // PxQ logical process grid , Q = nprocs / P
static int w = 1;              // halo width
static int r = 1;              // number of timesteps for the simulation
static int optW = 0, optO = 0; // set if -w , -o specified
static int optX = 0;           // set if -x specified
static int verbosity = 0;      // v, above
static int rank, nprocs;       // MPI values

// print a usage message for this program and exit with a status of 1
void usage(char *msg) {
  if (rank==0) {
    printf("testAdvect: %s\n", msg);
    printf("\tusage: %sn\tdefault values: %s\n", USAGE, DEFAULTS);
    fflush(stdout);
  }
  exit(1);
}

void getArgs(int argc, char *argv[]) {
  extern char *optarg; // points to option argument (for -p option)
  extern int optind;   // index of last option parsed by getopt()
  extern int opterr;
  char optchar;        // option character returned my getopt()
  opterr = 0;          // suppress getopt() error message for invalid option
  P = nprocs;
  while ((optchar = getopt(argc, argv, OPTCHARS)) != -1) {
    // extract next option from the command line     
    switch (optchar) {
    case 'P':
      if (sscanf(optarg, "%d", &P) != 1) // invalid integer 
	usage("bad value for P");
      break;
    case 'w':
      if (sscanf(optarg, "%d", &w) != 1) // invalid integer 
	usage("bad value for w");
      optW = 1;
      break;
    case 'v':
      if (sscanf(optarg, "%d", &verbosity) != 1) // invalid integer 
	usage("bad value for v");
      break;
    case 'o':
      optO = 1;
      break;
    case 'x':
      optX = 1;
      break;
    default:
      usage("unknown option");
      break;
    } //switch 
   } //while

  if (P == 0 || nprocs % P != 0)
    usage("number of processes must be a multiple of P");
  Q = nprocs / P;
  assert (Q > 0);

  if (optind < argc) {
    if (sscanf(argv[optind], "%d", &M) != 1) 
      usage("bad value for M");
  } else
    usage("missing M");
  N = M;
  if (optind+1 < argc)
    if (sscanf(argv[optind+1], "%d", &N) != 1) 
      usage("bad value for N");
  if (optind+2 < argc)
    if (sscanf(argv[optind+2], "%d", &r) != 1) 
      usage("bad value for r");

  if (optO && w != 1)
    usage("-o and -w w are incompatible");
} //getArgs()


static void printLocGlobAvgs(int isMax, char *name, double total, int nlVals, 
			     int ngVals) {
  double v[1];  
  if (verbosity > 0)  
    printf("%d: local avg %s %s is %.3e\n", rank, 
	   isMax? "max": "avg", name, 
	   isMax? total: (nlVals==0? 0.0: total / nlVals));
  MPI_Reduce(&total, v, 1, MPI_DOUBLE,isMax? MPI_MAX: MPI_SUM, 0, 
	     MPI_COMM_WORLD);
  if (rank == 0)
    printf("%s %s %.3e\n", isMax? "Max": "Avg", name, 
	   isMax? v[0]: v[0] / ngVals);
}


typedef struct parParam {
  int rank, M0, M_loc, N0, N_loc;
} parParam;

#define numParParam (sizeof(parParam)/sizeof(int))

// compare par params on M0 and N0 attributes
int compParParam(const void *vp1, const void *vp2) {
  parParam *p1 = (parParam *) vp1, *p2 = (parParam *) vp2;
  if (p1->M0 < p2->M0)
    return(-1);
  else if (p1->M0 > p2->M0)
    return(+1);
  else if (p1->N0 < p2->N0)
    return(-1);
  else if (p1->N0 > p2->N0)  
    return(+1);
  else
    return(0);
}

void gatherParParams() {
  parParam param = {rank, M0, M_loc, N0, N_loc};
  MPI_Send(&param, numParParam, MPI_INT, 0, 0, MPI_COMM_WORLD);
  if (rank == 0) {
    parParam params[nprocs];
    int i; 
    int M0Prev = -1, M_locPrev = -1;
    for (i=0; i < nprocs; i++)
      MPI_Recv(&params[i], numParParam, MPI_INT, i, 0, MPI_COMM_WORLD,
	       MPI_STATUS_IGNORE);
    qsort(params, nprocs, sizeof(parParam), compParParam);
    printf("Global field decomposition:");
    for (i=0; i < nprocs; i++) {
      if (params[i].M0 != M0Prev || params[i].M_loc != M_locPrev) {
	M0Prev = params[i].M0; M_locPrev = params[i].M_loc;
	printf("\nrows %d..%d: ", params[i].M0, 
	     params[i].M0 + params[i].M_loc - 1);
      }
      printf("%d:%d..%d ", params[i].rank, params[i].N0, 
	     params[i].N0 + params[i].N_loc - 1);
    }
    printf("\n");
  }
} //gatherParParams()


int main(int argc, char** argv) {
  double *u; int ldu; //local advection field
  double t; //time

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  getArgs(argc, argv);

  if (rank == 0) {
    printf("Advection of a %dx%d global field over %dx%d processes" 
	   " for %d steps.\n", M, N, P, Q, r);
    if (optO)
      printf("Using overlap communication/computation\n");
    else if (optW)
      printf("Using wide halo technique, width=%d\n", w);
    else if (optX)
      printf("Using extra optimization methods\n");
  }

  initAdvectParams(M, N);  
  initParParams(M, N, P, Q, verbosity);
  if (verbosity > 0)
    gatherParParams();
  checkHaloSize(w);

  ldu = N_loc+2*w;
  u = calloc(ldu*(M_loc+2*w), sizeof(double)); assert (u != NULL);
  initAdvectField(M0, N0, M_loc, N_loc, &V(u,w,w), ldu);
  if (verbosity > 1)
    printAdvectField(rank, "init u", M_loc, N_loc, &V(u,w,w), ldu);

  MPI_Barrier(MPI_COMM_WORLD);
  t = MPI_Wtime();

  if (optO)    
    parAdvectOverlap(r, u, ldu); 
  else if (optW)
    parAdvectWide(r, w, u, ldu);
  else if (optX)
    parAdvectExtra(r, u, ldu);
  else
    parAdvect(r, u, ldu);

  MPI_Barrier(MPI_COMM_WORLD);
  t = MPI_Wtime() - t;
  if (rank == 0) {
    double gflops = 1.0e-09 * AdvFLOPsPerElt * M * N * r;
    printf("Advection time %.2es, GFLOPs rate=%.2e (per core %.2e)\n",
	   t, gflops / t,  gflops / t / (P*Q)); 
  }

  if (verbosity > 1)
    printAdvectField(rank, "final u", M_loc+2*w, N_loc+2*w, u, ldu);
  printLocGlobAvgs(0, "error of final field: ", 
		   errAdvectField(r, M0, N0, M_loc, N_loc, &V(u,w,w), ldu),
		   M_loc*N_loc, M*N);
  printLocGlobAvgs(1, "error of final field: ", 
		   errMaxAdvectField(r, M0, N0, M_loc, N_loc, &V(u,w,w), ldu),
		   M_loc*N_loc, M*N);

  free(u);
  MPI_Finalize();
  return 0;
} //main()

