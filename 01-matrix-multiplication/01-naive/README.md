# Matrix Multiplication: Why the Naive Version Is Slow

The naive triple-loop matrix multiply in C takes about **19 minutes** for two 4096 × 4096 matrices. This directory explores why, and how to make it faster.

```c
for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
        for (int k = 0; k < n; ++k)
            C[i][j] += A[i][k] * B[k][j];
```

## How much work is there?

With `n = 4096`, the inner loop runs n³ ≈ 6.9 × 10¹⁰ times. Each iteration does one multiply and one add, so the total is about **1.37 × 10¹¹ floating-point operations**.

## How fast does it actually run?

| Quantity | Value |
|---|---|
| Runtime | ~19 min ≈ 1150 s |
| Throughput | 1.37 × 10¹¹ / 1150 ≈ **0.12 GFLOPS** |
| Cycles per FLOP (at ~3 GHz) | ~25 |
| Cycles per inner-loop iteration | ~50 |
| Theoretical peak (MIT 6.172 machine, all cores) | ~800+ GFLOPS |

A single modern core can do several floating-point operations *per cycle*, so the naive code runs at a tiny fraction of a percent of peak.

**The CPU isn't slow at math. It spends almost all its time waiting for memory.**

## Why it waits: the `B[k][j]` access

C stores 2D arrays **row by row** (row-major), so `B[k][j]` and `B[k+1][j]` are a whole row apart:

```
4096 doubles × 8 bytes = 32 KB apart
```

In the inner `k` loop, the code walks **down a column** of `B`, jumping 32 KB every step. That causes several problems at once.

### 1. Wasted cache lines

Memory moves in 64-byte cache lines, which hold 8 doubles. Each jump uses 1 double and discards the other 7, so **~87% of every memory transfer is wasted**.

### 2. The data doesn't stay in cache

`B` is 4096² × 8 bytes = **128 MB**, far bigger than every level of cache:

| Level | Typical size |
|---|---|
| L1 | ~32 KB |
| L2 | hundreds of KB to a few MB |
| L3 | tens of MB |
| Matrix `B` | **128 MB** |

By the time the code comes back to reuse part of `B`, it has been evicted. Much of it gets fetched again from slow DRAM, at **~100+ cycles per access**.

### 3. Power-of-two stride makes it worse

Caches are split into **sets**, and an address decides which set its line goes into. A stride of exactly 32 KB (a power of two) maps every access in the column to the **same few cache sets**. The lines keep evicting each other even when the cache has plenty of free space elsewhere.

These are called **conflict misses**, and they're why sizes like 4096 often run *slower* than 4000 or 4100.

### 4. TLB misses

Memory pages are typically 4 KB, so a 32 KB jump lands on a **new page every access**. The TLB (the cache of virtual-to-physical address translations) misses constantly, adding even more cycles.

## Who's to blame

| Access | Pattern in inner `k` loop | Cache behavior |
|---|---|---|
| `A[i][k]` | walks along a row (contiguous) | cache-friendly |
| `C[i][j]` | fixed | stays in a register/cache |
| `B[k][j]` | walks down a column (32 KB stride) | **the main culprit** |

## Experiments to try

- **Loop interchange:** change the loop order from `i, j, k` to `i, k, j`. The inner loop then walks `B` along rows, and nothing else in the program changes.
- **Power-of-two effect:** time `n = 4096` against `n = 4100`. If the slightly *bigger* matrix runs faster, that's conflict misses showing up directly.
- **Compiler flags:** compare `-O0` against `-O3`.