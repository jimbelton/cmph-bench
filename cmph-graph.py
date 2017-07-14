#!/usr/bin/python

import os
import subprocess
import sys

def usage():
    sys.stderr.write("usage: [-c|-l|-m|-t] bdz|brz-ph|bmz|mz8|brz|chd|chd-ph|chm|fch [...]\n")
    sys.exit(1)

stats = {"-c": 2, "-l": 4, "-m": 5, "-t": 6};
stat  = None;

if len(sys.argv) > 1:
    stat = stats.get(sys.argv[1])

    if stat == None:
        usage()

if not os.path.exists("corpus-words.bak"):
    os.rename("corpus-words", "corpus-words.bak")

if stat:
    print '"algo", 0,',

    for size in range(1000000, 11000000, 1000000):
        print "{},".format(size),

    print

for algo in sys.argv[2:]:
    if stat:
        print '"{}", 0,'.format(algo),

    for size in range(1000000, 11000000, 1000000):
        with open("corpus-words", "w") as output:
            for key in range (0, size):
                output.write("{}\n".format(key))

        if stat:
            output = subprocess.check_output(["target/cmph-bench", "-c", "chd"])
            atoms  = output.split(", ")
            print "{},".format(atoms[stat]),
        else:
            subprocess.check_call(["target/cmph-bench", "-c", "chd"])

    if stat:
        print

os.rename("corpus-words.bak", "corpus-words")
