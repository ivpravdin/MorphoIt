#!/usr/bin/env python3

import argparse
import subprocess as sp
import sys
from pathlib import Path
from time import perf_counter


def morphoit_print(x):
    print("[MORPHOIT]", x, file=sys.stderr)


parser = argparse.ArgumentParser()
parser.add_argument("FILE")
parser.add_argument("-T", "--transpilation-executable")
parser.add_argument("-p", "--profile", action="store_true")
parser.add_argument("-v", "--verbose", action="store_true")

args = parser.parse_args()

exe = Path(args.transpilation_executable or "build/morphoit").resolve()
src = Path(args.FILE).resolve()

if not exe.is_file():
    morphoit_print(f"Missing executable: {exe}")
    sys.exit(1)

if not src.is_file():
    morphoit_print(f"No such file '{args.FILE}'")
    sys.exit(1)

if args.verbose:
    morphoit_print(f"Executable: {exe}")
    morphoit_print(f"Source: {src}")

total_start = perf_counter()
run_start = perf_counter()

proc = sp.run([str(exe), str(src)])

run_elapsed = perf_counter() - run_start
total_elapsed = perf_counter() - total_start

if args.profile:
    print(f"__MI_TOTAL__ {total_elapsed:.9f}", file=sys.stderr)
    print(f"__MI_RUN__ {run_elapsed:.9f}", file=sys.stderr)

sys.exit(proc.returncode)