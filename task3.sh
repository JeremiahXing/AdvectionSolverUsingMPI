#!/bin/bash
#PBS -q express
#PBS -j oe
#PBS -l walltime=00:01:00,mem=128GB
#PBS -l wd
#PBS -l ncpus=192
#

e= # echo

r=100
M=2 # may need to make bigger
N=$M
opts="" # "-o"
p=1
P=1

module load openmpi

mpirun -np $p ./testAdvect $opts -P $P $M $N $r

exit
