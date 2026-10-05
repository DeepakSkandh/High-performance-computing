# 02 – Loop Order: i, k, j

The only change from the naive version is the **order of the loops**. The math, the number of operations, and the result are identical. The runtime is not.

```c
// Naive (01-naive): i, j, k
for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
        for (int k = 0; k < n; ++k)
            C[i][j] += A[i][k] * B[k][j];

// This version: i, k, j
for (int i = 0; i < n; ++i)
    for (int k = 0; k < n; ++k)
        for (int j = 0; j < n; ++j)
            C[i][j] += A[i][k] * B[k][j];
```

## The key idea: look at the innermost loop

The innermost loop runs n³ ≈ 6.9 × 10¹⁰ times, so its memory access pattern decides the performance.

C stores 2D arrays **row by row** (row-major). `X[r][c]` sits at position `r * n + c` in memory, so:

- moving along a **row** (`c + 1`) is the next address,
- moving down a **column** (`r + 1`) jumps `n * 8` bytes, which is **32 KB** for n = 4096.

## Tracing a 4 × 4 example

Memory layout of B as one long line:

```
position:  0  1  2  3 | 4  5  6  7 | 8  9 10 11 | 12 13 14 15
           ---row 0---  ---row 1---  ---row 2----  ----row 3----
```

**i, j, k** with i = 0, j = 0, inner loop over k:

| k | reads | B position |
|---|---|---|
| 0 | B[0][0] | 0 |
| 1 | B[1][0] | 4 |
| 2 | B[2][0] | 8 |
| 3 | B[3][0] | 12 |

B is read at 0, 4, 8, 12: **jumping down a column.**

**i, k, j** with i = 0, k = 0, inner loop over j:

| j | reads B | B pos | writes C | C pos | A |
|---|---|---|---|---|---|
| 0 | B[0][0] | 0 | C[0][0] | 0 | A[0][0] |
| 1 | B[0][1] | 1 | C[0][1] | 1 | A[0][0] |
| 2 | B[0][2] | 2 | C[0][2] | 2 | A[0][0] |
| 3 | B[0][3] | 3 | C[0][3] | 3 | A[0][0] |

B and C are read at 0, 1, 2, 3: **walking along a row.** A is the same value every time, so it stays in a register.

## Why walking in order is faster

**Full cache lines.** Memory is fetched in 64-byte cache lines (8 doubles). In i, k, j one fetch serves 8 iterations. In i, j, k each iteration needs a new line and 7 of the 8 doubles are wasted.

**Prefetching.** The CPU's hardware prefetcher detects sequential access and loads the next lines before they're needed, hiding memory latency.

**No conflict misses.** Consecutive addresses map to different cache sets, so lines don't keep evicting each other. See [cache sets](../concepts/cache-sets.md).

**Fewer TLB misses.** One 4 KB page holds 512 doubles, so one address translation serves 512 accesses instead of one. See [TLB](../concepts/tlb.md).

**Easy to vectorize.** The inner loop is `C[i][j] += a * B[k][j]` with `a` constant: scale one row and add it to another. At `-O3` the compiler can turn this into SIMD instructions that process 4–8 doubles at once.

**No dependency chain.** Each `C[i][j]` in the inner loop is independent, so the CPU can work on several at the same time. In i, j, k every iteration adds into the same `C[i][j]` and must wait for the previous add.

## Why it's still correct

Both orders compute the same products and add them into the same cells; only the timing of the additions changes.

Another way to read i, k, j:

> **row i of C = A[i][0] × (row 0 of B) + A[i][1] × (row 1 of B) + … + A[i][n−1] × (row n−1 of B)**

Each pass of the k loop scales one whole row of B by a single number and adds it to row i of C.

## All six loop orders

Statement: `C[i][j] += A[i][k] * B[k][j]`

| Order | Inner index | C[i][j] | A[i][k] | B[k][j] | Column walks |
|---|---|---|---|---|---|
| i, k, **j** | j | row ✅ | fixed ✅ | row ✅ | **0** |
| k, i, **j** | j | row ✅ | fixed ✅ | row ✅ | **0** |
| i, j, **k** | k | fixed ✅ | row ✅ | column ❌ | 1 |
| j, i, **k** | k | fixed ✅ | row ✅ | column ❌ | 1 |
| j, k, **i** | i | column ❌ | column ❌ | fixed ✅ | **2** |
| k, j, **i** | i | column ❌ | column ❌ | fixed ✅ | **2** |

Rule for row-major C: the innermost index should be the **last** subscript of the arrays it touches.

### Reference times (MIT 6.172, n = 4096)

| Order | Time |
|---|---|
| i, k, j | ~178 s |
| k, i, j | ~179 s |
| j, i, k | ~1080 s |
| i, j, k | ~1156 s |
| k, j, i | ~3030 s |
| j, k, i | ~3060 s |

### My results

Machine: `<CPU model>`, L1 `<size>`, L2 `<size>`, L3 `<size>` (from `lscpu`)
Compiler: `<gcc/clang version>`, flags: `<flags>`

| Order | Time (s) | Speedup vs i, j, k |
|---|---|---|
| i, j, k | | 1× |
| i, k, j | | |
| k, i, j | | |
| j, i, k | | |
| j, k, i | | |
| k, j, i | | |

## Build and run

```bash
gcc -std=c99 -O0 matmul.c -o matmul
./matmul
```

Try `-O3` as well to see how much the compiler adds once the access pattern is cache-friendly.

## What's still left

i, k, j fixes the **access pattern**, but for every row `i` it still streams through all of B (128 MB), which doesn't fit in any cache. B keeps getting evicted and reloaded from DRAM.

Next steps:

- **Compiler flags** (`03-compiler-flags`): let the compiler vectorize and keep values in registers.
- **Parallel loops** (`04-parallel`): use all cores.
- **Tiling** (`05-tiling`): work on small blocks that fit in cache and reuse them many times before moving on.