reservoir-sample
================

This tool performs reservoir sampling (Vitter, "Random sampling with a reservoir"; cf. http://dx.doi.org/10.1145/3147.3165) on very large text files that are delimited by newline characters. The approach used in this application reduces memory usage by storing a pool of byte offsets to the start of each line, instead of the line elements themselves.

In its current form, this application offers a few advantages over common `shuf`-based approaches:

* It performs roughly 2.25-2.75x faster than shuf in informal tests on OS X and Linux hosts.
* It uses much less memory than the usual reservoir sampling approach that stores a pool of sampled elements; instead, `reservoir-sample` stores the start positions of sampled lines (roughly 16 bytes per line).
* Using less memory also gives `reservoir-sample` two advantages:
  - Helps avoid `shuf: memory exhausted` errors for whole-genome-scale input files.
  - Allows a larger pool of samples, before running out of system memory.
  
To give an idea of the speed improvement, we compare the time performance of `shuf` and `reservoir-sample` for sampling three elements from a ~12M element, five-column BED file:

```
$ wc -l ~/fBrain.chr1.bed
 12462524 ~/fBrain.chr1.bed
```
 
Here is how `shuf` (`gshuf` v8.22 via Homebrew) does:

```
$ /usr/bin/time -p sh -c 'gshuf ~/fBrain.chr1.bed | head -3'
chr1    163361805       163361825       id      0.67
chr1    49746225        49746245        id      0.44
chr1    170730085       170730105       id      0.56
real         2.48
user         2.08
sys          0.36
```

Here is how `reservoir-sample` does with the same parameters, after refreshing cache:

```
$ /usr/bin/time -p sh -c '../bin/reservoir-sample --shuffle --sample-size=3 ~/fBrain.chr1.bed'
chr1    146798685       146798705       id      0.22
chr1    114519145       114519165       id      1.11
chr1    52810805        52810825        id      0.78
real         1.16
user         1.08
sys          0.08
```

The `reservoir-sample` tool stores a pool of line positions and makes two passes through the input file. One pass generates the sample of random positions, while the second pass uses those positions to print the sample to standard output. To minimize the expense of this second pass, we use `mmap` routines to gain random access to data in the (regular) input file on both passes.

The benefit that `mmap` provided was significant. For comparison purposes, we also add a `--cstdio` option to test the performance of the use of standard C I/O routines (`fseek()`, etc.); predictably, this performed worse than the `mmap`-based approach in all tests, but timing results were about identical with `gshuf` on OS X and still an average 1.5x improvement over `shuf` under Linux.

While we use a BED file here as an example input file, `reservoir-sample` can be used to sample from any text file delimited by newline characters (BED, SAM, VCF, etc.).

By leaving out the `--shuffle` option, the output sample preserves the input order. For example, when sampling from an input BED file that has been sorted by `sort-bed`, which applies a lexicographical sort on chromosome names and a numerical sort on start and stop coordinates, the sample will also have the same ordering applied, with a relatively small *O(klogk)* penalty for a sample of size *k*:

```
$ /usr/bin/time -p sh -c '../bin/reservoir-sample --sample-size=3 ~/fBrain.chr1.bed'
chr1    127931285       127931305       id      0.00
chr1    189937345       189937365       id      1.11
chr1    216440725       216440745       id      0.89
real         1.19
user         1.09
sys          0.08
```

One downside at this time is that `reservoir-sample` does not process a standard input stream; the input must be a regular file.
