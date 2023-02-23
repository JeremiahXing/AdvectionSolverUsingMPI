//parallel 2D advection solver module
// written for COMP4300/8300 Assignment 1 
// v1.0 24 Feb  

extern int M_loc, N_loc; // local advection field size (excluding halo) 
extern int M0, N0;       // local field element (0,0) is global element (M0,N0)
extern int P0, Q0;      // 2D process id (P0, Q0) in P x Q process grid 

//sets up parallel parameters above
void initParParams(int M, int N, int P, int Q, int verbosity);

//check halo size w is not too large for local M_loc x N_loc field
void checkHaloSize(int w);

// evolve advection over r timesteps, with (u,ldu) storing the local filed
// may assume ldu = N_loc+2
void parAdvect(int r, double *u, int ldu);

// overlap communication variant; may assume ldu = N_loc+2
void parAdvectOverlap(int r, double *u, int ldu);

// wide halo variant; ; may assume ldu = N_loc+2*w
void parAdvectWide(int r, int w, double *u, int ldu);

// extra optimization variant; 
void parAdvectExtra(int r, double *u, int ldu);
