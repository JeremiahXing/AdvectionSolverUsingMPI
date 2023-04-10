#!/bin/bash
#PBS -q express
#PBS -j oe
#PBS -l walltime=00:01:00,mem=2GB
#PBS -l wd
#PBS -l ncpus=2

module load openmpi

mpicc -o measure_ts measure_ts.c
echo ""
echo mpirun -np 2 ./measure_ts
mpirun -np 2 ./measure_ts
echo ""

mpicc -o measure_tw measure_tw.c
echo ""
echo mpirun -np 2 ./measure_tw
mpirun -np 2 ./measure_tw
echo ""

mpicc -o measure_tf measure_tf.c
echo ""
echo mpirun -np 1 ./measure_tf
mpirun -np 1 ./measure_tf
echo ""