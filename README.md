reservoir-sample
================

Performs reservoir sampling (Vitter, "Random sampling with a reservoir"; cf. http://dx.doi.org/10.1145/3147.3165) on very large text files that are delimited by newline characters. The approach used in this application reduces memory usage by storing a pool of byte offsets to the start of each line, instead of the line elements themselves.
