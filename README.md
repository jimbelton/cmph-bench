This package (*cmph-bench*) implements a benchmark for *cmph* is based on the sample program in the cmph README.
The open source software package *cmph* includes implementations in C of several minimal perfect hashing algorithms.

You can download cmph and build it as follows:

    git clone https://github.com/zvelo/cmph
    cd cmph
    configure
    make
    make check

You can download cmph-bench and build it as follows:

    git clone https://github.com/jimbelton/baker
    git clone https://github.com/jimbelton/cmph-bench
    cd cmph-bench
    ../baker/baker
    
Note that the latest version of *baker* from 2017-06-25 is required, as cmph uses libm.a (the math library) and baker wasn't resolving symbols in libraries under /usr/lib prior to that version.

After building, the progam *cmph-bench* is dropped in the target directory and can be run with: *target/cmph-bench*. The bench-mark creates an MPH from *corpus-words*, which is a copy of the file */usr/share/dict/words*, a corpus of almost 100000 unique words used in the Easy Perfect Minimal Hashing benchmark.

The benchmark creates a MPH from the corpus, reloads it from disk, and looks up every word in the corpus in it. Running without an algorithm gives you the list of supported algorithms:

    usage: cmph-bench [-c] bdz|bdz-ph|bmz|bmz8|brz|chd|chd-ph|chm|fch)
        -c: Output CSV line for graphing

The benchmark tracks the amount of time taken for each step, as well as the amount of memory being allocated (using the linux function *mallinfo*). Sample results from running: *target/cmph-bench chd*

    Constructed chd MPH in 0.078931 seconds, memory 52720
    Loaded MPH in 0.000054 seconds, memory 52112
    Looked up 99171 keys in MPH in 0.068846 seconds, corpus size 938848

Note that the corpus size is the actual size of the corpus including just the words separated by newlines. Here's the CSV output you get if you run: *target/cmph-bench -c chd*

    "chd", 99171, 79634, 52720, 52, 52112, 66359, 938848

You can generate CSV lines for a range of sizes by running: *./cmph-graph.py*
