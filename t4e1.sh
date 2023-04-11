#!/bin/bash
#PBS -q express
#PBS -j oe
#PBS -l walltime=00:01:00,mem=128GB
#PBS -l wd
#PBS -l ncpus=48
#

e= #echo

r=100
M=1000 # may need to be bigger
N=$M
opts="" # "-o" 
ps="1 3 4 6 9 12 18 36"
np=36
module load openmpi

for p in $ps; do
    echo ""
    echo mpirun -np $np ./testAdvect $opts -P $p $M $N $r
    $e mpirun -np $np ./testAdvect $opts -P $p $M $N $r
    echo ""
done

if [ ! -z "$PBS_NODEFILE" ] ; then
    cat $PBS_NODEFILE
fi

exit
