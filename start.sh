#!/bin/bash
# SIMULATIONS PARAMETERS
default_n=0
default_rep=500
default_threads=0
# END OF SIMULATIONS PARAMETERS
gcc --version
is_integer() {
  [[ "$1" =~ ^[0-9]+$ ]]
}
run_simulations() {
  echo ""; echo "Executing programs..."
  echo ""; echo "sequential.o"; ./sequential.o "$n" "$rep"
  echo ""; echo "vectorization.o"; ./vectorization.o "$n" "$rep"
  echo ""; echo "block_access_pattern.o"; ./block_access_pattern.o "$n" "$rep"
  echo ""; echo "block_access_pattern_prefetching.o"; ./block_access_pattern_prefetching.o "$n" "$rep"
  echo ""; echo "block_access_pattern_O1.o"; ./block_access_pattern_O1.o "$n" "$rep"
  echo ""; echo "block_access_pattern_O2.o"; ./block_access_pattern_O2.o "$n" "$rep"
  echo ""; echo "block_access_pattern_O3.o"; ./block_access_pattern_O3.o "$n" "$rep"
  echo ""; echo "block_access_pattern_Ofast.o"; ./block_access_pattern_Ofast.o "$n" "$rep"
  echo ""; echo "omp.o"; ./omp.o "$n" "$rep" "$threads"
  echo ""; echo "omp_reduction.o"; ./omp_reduction.o "$n" "$rep" "$threads"
  echo ""; echo "omp_block_access_pattern.o"; ./omp_block_access_pattern.o "$n" "$rep" "$threads"
  echo ""; echo "omp_triangular_numbers.o"; ./omp_triangular_numbers.o "$n" "$rep" "$threads"
  echo ""; echo "omp_static_scheduling.o"; ./omp_static_scheduling.o "$n" "$rep" "$threads"
  echo ""; echo "omp_dynamic_scheduling.o"; ./omp_dynamic_scheduling.o "$n" "$rep" "$threads"
  echo ""; echo "omp_dynamic_scheduling_Ofast.o"; ./omp_dynamic_scheduling_Ofast.o "$n" "$rep" "$threads"
}
n=""
rep=""
threads=""
if [[ $# -eq 0 ]]; then
  n=$default_n
  rep=$default_rep
  threads=$default_threads
elif [[ $# -eq 1 ]]; then
  if is_integer "$1" && (($1 >= 4 && $1 <= 12)); then
    n="$1"
  else
    n=$default_n
  fi
  rep=$default_rep
  threads=$default_threads
elif [[ $# -eq 2 ]]; then
  if is_integer "$1" && (($1 >= 4 && $1 <= 12)); then
    n="$1"
  else
    n=$default_n
  fi
  
  if is_integer "$2" && (($2 > 0)); then
    rep="$2"
  else
    rep=$default_rep
  fi
  threads=$default_threads
else 
  if is_integer "$1" && (($1 >= 4 && $1 <= 12)); then
    n="$1"
  else
    n=$default_n
  fi
  
  if is_integer "$2" && (($2 > 0)); then
    rep="$2"
  else
    rep=$default_rep
  fi
  if is_integer "$3" && (($3 > 0)); then
    threads="$3"
  else
    threads=$default_threads
  fi
fi
echo "Simulation variables"
echo "n=$n"
echo "rep=$rep"
echo "threads=$threads"
mkdir -p bin
cd lib
echo ""
echo "Compiling..."
gcc sequential.c -o ../bin/sequential.o -lm
gcc vectorization.c -o ../bin/vectorization.o -mavx2 -lm
gcc block_access_pattern.c -o ../bin/block_access_pattern.o -lm
gcc block_access_pattern_prefetching.c -o ../bin/block_access_pattern_prefetching.o -lm
gcc block_access_pattern.c -o ../bin/block_access_pattern_O1.o -O1 -lm -DO1
gcc block_access_pattern.c -o ../bin/block_access_pattern_O2.o -O2 -lm -DO2
gcc block_access_pattern.c -o ../bin/block_access_pattern_O3.o -O3 -lm -DO3
gcc block_access_pattern.c -o ../bin/block_access_pattern_Ofast.o -Ofast -lm -DOfast
gcc omp.c -o ../bin/omp.o -fopenmp -lm
gcc omp_reduction.c -o ../bin/omp_reduction.o -fopenmp -lm
gcc omp_block_access_pattern.c -o ../bin/omp_block_access_pattern.o -fopenmp -lm
gcc omp_triangular_numbers.c -o ../bin/omp_triangular_numbers.o -fopenmp -lm
gcc omp_static_scheduling.c -o ../bin/omp_static_scheduling.o -fopenmp -lm
gcc omp_dynamic_scheduling.c -o ../bin/omp_dynamic_scheduling.o -fopenmp -lm
gcc omp_dynamic_scheduling.c -o ../bin/omp_dynamic_scheduling_Ofast.o -fopenmp -Ofast -lm -DOfast
echo "Compiling executed correctly!"
cd ../bin
echo ""; echo "Preparing results files..."
rm -f cpu_specs
touch cpu_specs
lscpu > cpu_specs
rm -f results_ilp.csv
touch results_ilp.csv
echo "code,n,flops,bandwidth" > results_ilp.csv
rm -f results_omp.csv
touch results_omp.csv
echo "code,n,threads,speedup1,efficiency1,speedup2,efficiency2,bandwidth" > results_omp.csv
echo "Done!"
if [[ $n -eq 0 ]]; then 
  echo ""; echo "n is set to 0. Running simulations from n=4 to n=12 and rep=$rep"
  for ((i = 4 ; i < 13 ; i++ )); do
    n=$i
    run_simulations
  done
else
  echo ""; echo "Running simulations for n=$n and rep=$rep..."
  run_simulations
fi
echo "Simulations over!"
echo ""; echo "Saving results in \"/results/\"..."
cd ..
mkdir -p results
cd results
rm -f cpu_specs
rm -f results_ilp.csv
rm -f results_omp.csv
mv ../bin/cpu_specs ./cpu_specs
mv ../bin/results_ilp.csv ./results_ilp.csv
mv ../bin/results_omp.csv ./results_omp.csv
echo "All done!"