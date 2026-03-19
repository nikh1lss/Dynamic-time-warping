# Dynamic Time Warping (DTW)

A C implementation of Dynamic Time Warping with three variants: standard DTW, windowed (Sakoe-Chiba band) DTW, and path-length constrained DTW.

Dynamic Time Warping is an algorithm for measuring similarity between two temporal sequences that may vary in speed. This project implements three progressively more constrained versions of the algorithm:

- **Part A** — Standard DTW over the full cost matrix
- **Part D** — DTW with a Sakoe-Chiba band (window constraint), limiting how far indices can diverge
- **Part F** — DTW with a maximum total path length constraint, solved via a 3D DP table

Given two numeric sequences A and B, the algorithm computes the minimum-cost alignment between them using absolute difference as the local cost measure. At each cell `(i, j)`, the cost is:

```
cost(i, j) = |A[i] - B[j]| + min(cost(i-1, j), cost(i, j-1), cost(i-1, j-1))
```

### Part A — Standard DTW

Fills the full `|A| × |B|` matrix with no constraints. Time complexity: **O(n × m)**.

### Part D — Windowed DTW

Only computes cells where `|i - j| ≤ windowSize`, reducing work for long sequences that are roughly aligned. Cells outside the band are treated as infinity.

### Part F — Path-Length Constrained DTW

Introduces a third dimension `k` representing the number of steps taken so far. The 3D table `dtw[i][j][k]` stores the minimum cost to align `A[0..i-1]` with `B[0..j-1]` using exactly `k` steps. The final answer is the minimum over all valid `k` values. Time complexity: **O(n × m × L)** where L is the maximum path length.

## Input Format

Each sequence is provided as a comma-separated file of `long double` values:

```
1.0, 2.5, 3.7, 4.1
```

## Building

Requires `problemStruct.c`, `solutionStruct.c`, and `problem.h` (not included here). Compile with a C99-compatible compiler:

```bash
gcc -o dtw problem.c main.c -lm
```

## Usage

The entry points are:

```c
struct problem *readProblemA(FILE *seqAFile, FILE *seqBFile);
struct problem *readProblemD(FILE *seqAFile, FILE *seqBFile, int windowSize);
struct problem *readProblemF(FILE *seqAFile, FILE *seqBFile, int maxPathLength);

struct solution *solveProblemA(struct problem *p);
struct solution *solveProblemD(struct problem *p);
struct solution *solveProblemF(struct problem *p);
```

## Output

Each solution reports:

- **Optimal value** — the minimum DTW distance (printed to 2 decimal places)
- **Cost matrix** (Part A and D only) — the full DP table, with infinity cells shown as blank

## Dependencies

- Standard C library (`stdio.h`, `stdlib.h`, `math.h`, `string.h`, `float.h`, `limits.h`)
- POSIX `getdelim` (for reading input files)
