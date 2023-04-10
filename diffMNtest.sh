#!/bin/bash
#PBS -q express
#PBS -j oe
#PBS -l walltime=00:01:00,mem=128GB
#PBS -l wd
#PBS -l ncpus=192
#

e=# echo

r=100
M=1000 # may need to make bigger
N=$M
opts="" # "-o"
p="192"

module load openmpi

echo ""
echo mpirun -np $p ./testAdvect $opts 100 100 $r
$e mpirun -np $p ./testAdvect $opts 100 100 $r
echo ""

echo ""
echo mpirun -np $p ./testAdvect $opts 500 500 $r
$e mpirun -np $p ./testAdvect $opts 500 500 $r
echo ""

echo ""
echo mpirun -np $p ./testAdvect $opts 1000 1000 $r
$e mpirun -np $p ./testAdvect $opts 1000 1000 $r
echo ""

if [ ! -z "$PBS_NODEFILE" ] ; then
    cat $PBS_NODEFILE
fi

exit
