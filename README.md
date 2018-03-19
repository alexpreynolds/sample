sample
================

[![Build Status](https://travis-ci.org/alexpreynolds/sample.svg?branch=master)](https://travis-ci.org/alexpreynolds/sample)

## About

This tool performs reservoir sampling (Vitter, "Random sampling with a reservoir"; cf. http://dx.doi.org/10.1145/3147.3165 and also: http://en.wikipedia.org/wiki/Reservoir_sampling) on very large text files that are delimited by newline characters. Sampling can be done with or without replacement. The approach used in this application reduces the typical memory usage issue with reservoir sampling by storing a pool of byte offsets to the start of each line, instead of the line elements themselves, thus allowing much larger sample sizes. 

------

## Tip for shuffling entire files

**Tip:** If the line count of the file is known ahead of time (e.g., via `wc -l`) and if we want to shuffle the entire file, we can do so efficiently by storing a pool of bits, one bit for each line offset. This reduces the memory overhead by a factor of 64! Reduced memory usage means we can shuffle even larger files and do so faster through fast bitwise operations.

------

## Advantages over GNU `shuf`

In its current form, this application offers a few advantages over common `shuf`-based approaches:

* On small *k*, it performs roughly 2.25-2.75x faster than `shuf` in informal tests on OS X and Linux hosts.
* It uses much less memory than the usual reservoir sampling approach that stores a pool of sampled elements; instead, `sample` stores the start positions of sampled lines (8 bytes per line).
* Using less memory gives `sample` an advantage over `shuf` for whole-genome scale files, helping avoid `shuf: memory exhausted` errors. For instance, a 2 GB allocation would allow a sample size up to ~268M random elements (sampling without replacement).

The `sample` tool stores a pool of line positions and makes two passes through the input file. One pass generates the sample of random positions, using a Mersenne Twister to generate uniformly random values, while the second pass uses those positions to print the sample to standard output. To minimize the expense of this second pass, we use `mmap` routines to gain random access to data in the (regular) input file on both passes.

The benefit that `mmap` provided was significant. For comparison purposes, we also add a `--cstdio` option to test the performance of the use of standard C I/O routines (`fseek()`, etc.); predictably, this performed worse than the `mmap`-based approach in all tests, but timing results were about identical with `gshuf` on OS X and still an average 1.5x improvement over `shuf` under Linux.

The `sample` tool can be used to sample from any text file delimited by newline characters (BED, SAM, VCF, etc.).

By adding the `--preserve-order` option, the output sample preserves the input order. For example, when sampling from an input BED file that has been sorted by BEDOPS `sort-bed` — which applies a lexicographical sort on chromosome names and a numerical sort on start and stop coordinates — the sample will also have the same ordering applied, with a relatively small *O(k logk)* penalty for a sample of size *k*.

One downside at this time is that `sample` does not process a standard input stream; the input must be a regular file.
