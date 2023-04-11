#!/bin/bash
#PBS -q express
#PBS -j oe
#PBS -l walltime=00:01:00,mem=128GB
#PBS -l wd
#PBS -l ncpus=192
#

e= # echo

r=100
M=1000 # may need to make bigger
N=$M
opts="" # "-o"
ps="2 3 4 6 8 12 16 24 32 48 64"


module load openmpi

for p in $ps; do
    echo ""
    echo mpirun -np 144 ./testAdvect $opts -P $p $M $N $r
    $e mpirun -np 144 ./testAdvect $opts -P $p $M $N $r
    echo ""
done

if [ ! -z "$PBS_NODEFILE" ] ; then
    cat $PBS_NODEFILE
fi

exit