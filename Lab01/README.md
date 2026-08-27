# Lab01 - Performance Profiling and Optimization using GPROF and PERF

## Objective
Compare Bubble Sort and Quick Sort using execution time, GPROF, and compiler optimizations.

## Software Used
- Ubuntu 24.04.4 LTS
- GCC 13.3.0
- GNU GPROF 2.42
- PERF (Not available on WSL2 kernel)

## Files
- mysort.c
- myreport.txt
- report_O2.txt
- report_O3.txt
- screenshots/

## Compilation

Without optimization

gcc -O0 -pg mysort.c -o mysort

With O2

gcc -O2 -pg mysort.c -o mysort_O2

With O3

gcc -O3 -pg mysort.c -o mysort_O3

## Execution

./mysort

time ./mysort

gprof mysort gmon.out > myreport.txt

perf stat ./mysort
