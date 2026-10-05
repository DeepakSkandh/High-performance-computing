# Matrix Multiplication: All Six Loop Orders

Matrix multiplication is the classic first problem in performance engineering. The code is simple, the result is easy to check, and the naive version is thousands of times slower than what the hardware can do. This directory starts with the most basic change: **the order of the three loops**.

## The computation

For n × n matrices, each entry of the result is a dot product of a row of A and a column of B:

```
C[i][j] = A[i][0]·B[0][j] + A[i][1]·B[1][j] + … + A[i][n−1]·B[n−1][j]
```

There are three indices, so there are three loops:

| Index | Meaning | Range |
|---|---|---|
| `i` | row of C (and row of A) | 0 … n−1 |
| `j` | column of C (and column of B) | 0 … n−1 |
| `k` | position inside the dot product | 0 … n−1 |

Every version in this directory runs the same statement:

```c
C[i][j] += A[i][k] * B[k][j];
```

That's n³ multiply-adds. For n = 4096, that's about 6.9 × 10¹⁰ iterations, or 1.37 × 10¹¹ floating-point operations.

The three loops can be nested in **3! = 6 orders**. All six compute exactly the same C, because they add the same products into the same cells, just at different times. The difference is **which memory addresses are touched one after another**, and that changes the runtime by up to ~17×.

## How a matrix sits in memory

Memory is one long line of bytes. C stores a 2D array **row by row** (row-major):

```
Grid:                         Memory (one long line):

X[0][0] X[0][1] X[0][2] X[0][3]      position:  0  1  2  3 | 4  5  6  7 | 8  9 10 11 | 12 13 14 15
X[1][0] X[1][1] X[1][2] X[1][3]                 ---row 0---  ---row 1---  ---row 2----  ----row 3----
X[2][0] …
X[3][0] …
```

`X[r][c]` is at position `r * n + c`. So:

- **Along a row** (c + 1): next address. Contiguous. 
- **Down a column** (r + 1): jump of n × 8 bytes, which is **32 KB** for n = 4096. 

## The rule

Only the **innermost loop** matters, since it runs n³ times. For each array, look at where the innermost index appears:

| Inner index appears as… | Access pattern | Cost |
|---|---|---|
| last subscript, `X[..][inner]` | walks along a row |  cheap |
| first subscript, `X[inner][..]` | walks down a column |  expensive |
| doesn't appear | same value every time |  free (stays in a register) |

## The six orders

### 1. i, j, k (the naive version)

```c
for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
        for (int k = 0; k < n; ++k)
            C[i][j] += A[i][k] * B[k][j];
```

Inner index **k**:

- `C[i][j]`: fixed 
- `A[i][k]`: along row i 
- `B[k][j]`: **down column j** 

Computes one full dot product per (i, j). Reads of B jump 32 KB every iteration.

### 2. j, i, k

```c
for (int j = 0; j < n; ++j)
    for (int i = 0; i < n; ++i)
        for (int k = 0; k < n; ++k)
            C[i][j] += A[i][k] * B[k][j];
```

Inner index **k**: same inner loop as i, j, k, so the same single column walk on B. Fills C column by column instead of row by row, which barely matters since C is touched only once per n inner iterations.

### 3. i, k, j  fastest

```c
for (int i = 0; i < n; ++i)
    for (int k = 0; k < n; ++k)
        for (int j = 0; j < n; ++j)
            C[i][j] += A[i][k] * B[k][j];
```

Inner index **j**:

- `C[i][j]`: along row i 
- `A[i][k]`: fixed 
- `B[k][j]`: along row k 

Zero column walks. Each pass of the k loop scales **row k of B** by one number and adds it to **row i of C**:

> row i of C = A[i][0] × (row 0 of B) + A[i][1] × (row 1 of B) + … + A[i][n−1] × (row n−1 of B)

### 4. k, i, j  fastest (tied)

```c
for (int k = 0; k < n; ++k)
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            C[i][j] += A[i][k] * B[k][j];
```

Inner index **j**: same inner loop as i, k, j, so zero column walks. The outer order differs: for each k, it updates **every** row of C using row k of B. Performance is essentially the same as i, k, j.

### 5. j, k, i  slowest

```c
for (int j = 0; j < n; ++j)
    for (int k = 0; k < n; ++k)
        for (int i = 0; i < n; ++i)
            C[i][j] += A[i][k] * B[k][j];
```

Inner index **i**:

- `C[i][j]`: **down column j** 
- `A[i][k]`: **down column k** 
- `B[k][j]`: fixed 

Two column walks per iteration, and one of them is a write.

### 6. k, j, i  slowest (tied)

```c
for (int k = 0; k < n; ++k)
    for (int j = 0; j < n; ++j)
        for (int i = 0; i < n; ++i)
            C[i][j] += A[i][k] * B[k][j];
```

Inner index **i**: same inner loop as j, k, i, so two column walks.

## Summary

| Order | Inner index | C[i][j] | A[i][k] | B[k][j] | Column walks | Group |
|---|---|---|---|---|---|---|
| i, k, j | j | row ✅ | fixed ✅ | row ✅ | 0 | fastest |
| k, i, j | j | row ✅ | fixed ✅ | row ✅ | 0 | fastest |
| i, j, k | k | fixed ✅ | row ✅ | column ❌ | 1 | slow |
| j, i, k | k | fixed ✅ | row ✅ | column ❌ | 1 | slow |
| j, k, i | i | column ❌ | column ❌ | fixed ✅ | 2 | slowest |
| k, j, i | i | column ❌ | column ❌ | fixed ✅ | 2 | slowest |

The six orders collapse into **three groups**, decided entirely by which index is innermost.

## Tracing the inner loop (4 × 4 example)

**j innermost** (i, k, j with i = 0, k = 0):

```
B positions read:   0, 1, 2, 3      ← along a row
C positions read:   0, 1, 2, 3      ← along a row
A:                  A[0][0] every time
```

**k innermost** (i, j, k with i = 0, j = 0):

```
A positions read:   0, 1, 2, 3      ← along a row
B positions read:   0, 4, 8, 12     ← down a column
C:                  C[0][0] every time
```

**i innermost** (j, k, i with j = 0, k = 0):

```
C positions read:   0, 4, 8, 12     ← down a column
A positions read:   0, 4, 8, 12     ← down a column
B:                  B[0][0] every time
```

## Why column walks are so expensive

**Wasted cache lines.** Memory is fetched in 64-byte lines (8 doubles). Walking a row uses all 8. Walking a column uses 1 and wastes 7.

**Data doesn't stay in cache.** Each matrix is 4096² × 8 bytes = **128 MB**, far bigger than L1 (~32 KB), L2 (~hundreds of KB to a few MB) or L3 (~tens of MB). Lines are evicted before they're reused and must come back from DRAM at ~100+ cycles each.

**Conflict misses.** A cache is divided into sets, and an address can only go into one set. A 32 KB stride (a power of two) maps every element of a column to the **same set**, so they evict each other while the rest of the cache sits empty.

**TLB misses.** Pages are 4 KB, so a 32 KB jump lands on a new page every access. The TLB, which caches virtual-to-physical address translations, misses almost every time and the CPU must look up the page table.

**No prefetching help.** The hardware prefetcher handles sequential access easily; large strides are much harder for it.

## Why the j-innermost orders get extra speed

- **Vectorization:** `C[i][j] += a * B[k][j]` over consecutive j is a simple scale-and-add over a row. At `-O3` the compiler can use SIMD to process 4–8 doubles per instruction.
- **No dependency chain:** each `C[i][j]` in the inner loop is independent. In the k-innermost orders, every iteration adds into the same `C[i][j]` and must wait for the previous add.

## Results

### Reference (MIT 6.172, n = 4096)

| Order | Time | Relative to i, k, j |
|---|---|---|
| i, k, j | ~178 s | 1× |
| k, i, j | ~179 s | ~1× |
| j, i, k | ~1080 s | ~6× slower |
| i, j, k | ~1156 s | ~6.5× slower |
| k, j, i | ~3030 s | ~17× slower |
| j, k, i | ~3060 s | ~17× slower |

### My results

Machine: `<CPU model>`, L1 `<size>`, L2 `<size>`, L3 `<size>` (from `lscpu`)
Compiler: `<gcc/clang version>`, flags: `<flags>`

| Order | Time (s) | Relative to i, k, j |
|---|---|---|
| i, k, j | | 1× |
| k, i, j | | |
| i, j, k | | |
| j, i, k | | |
| j, k, i | | |
| k, j, i | | |

## Build and run

```bash
gcc -std=c99 -O0 matmul.c -o matmul
./matmul
```

Change only the loop order between runs. Then repeat with `-O3` to see how much more the compiler gains once the access pattern is cache-friendly.

## What loop order can't fix

Even i, k, j streams through all of B (128 MB) once for every row of C, so B keeps getting evicted and reloaded from DRAM. Further speedups come from:

- **Compiler optimization** (`-O3`): vectorization, registers
- **Parallelism**: using all cores
- **Tiling**: working on small blocks that fit in cache and reusing them before moving on
- **Explicit SIMD** (AVX intrinsics)
