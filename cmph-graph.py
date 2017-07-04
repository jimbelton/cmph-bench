#!/usr/bin/python

import os
import subprocess

if not os.path.exists("corpus-words.bak"):
    os.rename("corpus-words", "corpus-words.bak")

for size in range(1000000, 11000000, 1000000):
    with open("corpus-words", "w") as output:
        for key in range (0, size):
            output.write("{}\n".format(key))

    subprocess.check_call(["target/cmph-bench", "-c", "chd"])

os.rename("corpus-words.bak", "corpus-words")
