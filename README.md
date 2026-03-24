# Big O Demo: Sorting Algorithms

A benchmark program that compares O(n^2) sorting algorithms (bubble sort, insertion sort, selection sort) against O(n log n) algorithms (merge sort, quick sort) to demonstrate the dramatic performance difference as input size grows.

## What It Shows

- **O(n^2) algorithms** (bubble, insertion, selection): when n doubles, runtime grows ~4x
- **O(n log n) algorithms** (merge sort, quick sort): when n doubles, runtime grows ~2x
- At small sizes the difference is modest; at larger sizes the gap becomes enormous

The program benchmarks each algorithm at sizes 1,000 / 5,000 / 10,000 / 50,000 and prints a summary comparison table at the end.

## How to Run

```bash
cmake -B build -S .
cmake --build build
./build/big-o-demo-sorting
```

## Space Complexity Notes

| Algorithm      | Time (avg) | Space   |
|----------------|------------|---------|
| Bubble Sort    | O(n^2)     | O(1)    |
| Insertion Sort | O(n^2)     | O(1)    |
| Selection Sort | O(n^2)     | O(1)    |
| Merge Sort     | O(n log n) | O(n)    |
| Quick Sort     | O(n log n) | O(log n)|
