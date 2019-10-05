#!/usr/bin/env python3

import argparse
import glob
import os
import subprocess

from check import check

if __name__ == "__main__":
    PARSER = argparse.ArgumentParser(description="Barrier tests.")
    PARSER.add_argument("-a", "--all", action="store_true")
    PARSER.add_argument(
        "-p", "--proc", help="Number of processes/threads", type=int, default=4
    )
    PARSER.add_argument(
        "-n", "--num", help="Number of barriers", type=int, default=2
    )
    PARSER.add_argument(
        "-i", "--impl", help="Barrier implementation", required=True
    )

    ARGS = PARSER.parse_args()

    # Clear out the results directory
    files = glob.glob("results/*")
    for f in files:
        os.remove(f)

    # Clean first
    CALL = ["make", "clean"]

    print(' '.join(CALL))
    subprocess.call(CALL)

    # Compile
    CALL = ["make", ARGS.impl]

    print(' '.join(CALL))
    subprocess.call(CALL)

    # Run
    CALL = []
    if ARGS.impl.startswith("mpi"):
        CALL.append("mpirun")
        CALL.append("-n")
        CALL.append(str(ARGS.proc))
        if ARGS.proc > 4:
            CALL.append("--use-hwthread-cpus")
        CALL.append("./{}".format(ARGS.impl))
        CALL.append(str(ARGS.num))
    else:
        assert ARGS.impl.startswith("openmp")
        CALL.append("./{}".format(ARGS.impl))
        CALL.append(str(ARGS.proc))
        CALL.append(str(ARGS.num))

    if not ARGS.all:
        CALL.append("--fast")

    print(' '.join(CALL))
    subprocess.call(CALL)

    # Check results.
    if ARGS.num < 100000:
        if check():
            print("=== PASSED ===")
        else:
            print("=== FAILED ===")
