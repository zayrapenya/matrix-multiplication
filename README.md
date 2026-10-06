# Matrix Multiplication — Assignment 1

BIG DATA — Academic Year 2026-2027  
Degree in Data Science and Engineering  
Universidad de Las Palmas de Gran Canaria

## Objective

This project investigates how matrix size affects the execution time and memory usage of a basic dense matrix multiplication implemented in Python, Java and C.

## Research Question

How does matrix size affect the execution time and memory usage of the basic triple-loop matrix multiplication implemented in Python, Java and C?

## Implementations

All three implementations use the straightforward triple-loop matrix multiplication algorithm with O(n³) arithmetic work.

- Python
- Java
- C

## Experimental Protocol

The experiment will use the same logical input matrices across implementations, increasing matrix sizes, repeated measurements, and documented execution and memory measurements.

## Project Structure

```text
python/    Python implementation
java/      Java implementation
c/         C implementation
data/      Input data and experimental results
scripts/   Data generation and analysis scripts
report/    Final report and figures