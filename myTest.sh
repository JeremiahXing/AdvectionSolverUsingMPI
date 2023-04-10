#!/bin/bash
#PBS -q express
#PBS -j oe
#PBS -l walltime=00:01:00,mem=128GB
#PBS -l wd
#PBS -l ncpus=192
#

e= # echo

r=100
M=20 # may need to make bigger
N=$M
opts1="" # "-o"
opts2="-o"
opts3="-w 2"
p="9"
P=3

module load openmpi

echo ""
echo mpirun -np $p ./testAdvect $opts1 -P $P $M $N $r
$e mpirun -np $p ./testAdvect $opts1 -P $P $M $N $r
echo ""

echo ""
echo mpirun -np $p ./testAdvect $opts2 $M $N $r
$e mpirun -np $p ./testAdvect $opts2 $M $N $r
echo ""

echo ""
echo mpirun -np $p ./testAdvect $opts3 -P $P $M $N $r
$e mpirun -np $p ./testAdvect $opts3 -P $P $M $N $r
echo ""



if [ ! -z "$PBS_NODEFILE" ] ; then
    cat $PBS_NODEFILE
fi

exit


# mpirun -np 4 ./testAdvect -o -P 2 10 10 100
