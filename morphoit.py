#!/usr/bin/env python3

import argparse
import subprocess as sp
import sys
from pathlib import Path
from time import perf_counter

USE_DEFAULT_FILENAME = 1
EXE_EXTENSION = ".exe" if sys.platform == "win32" else ".bin"


def morphoit_print(x):
    print("[MORPHOIT]", x, file=sys.stderr)


parser = argparse.ArgumentParser()
parser.add_argument(
    "FILE",
    help="path to the Morpho source file to transpile; "
    "use '-' to read from stdin",
)
parser.add_argument(
    "-o", "--output",
    help="path to produced executable (default: FILE.bin/.exe)",
)
parser.add_argument(
    "-T", "--transpilation-executable",
    help="path to Morpho-to-C transpiler",
)
parser.add_argument(
    "-t", "--transpilation-file",
    nargs="?",
    const=USE_DEFAULT_FILENAME,
    help="save generated C (default FILE.c if flag given without arg)",
)
parser.add_argument(
    "-d", "--no-run",
    action="store_true",
    help="compile but do not run",
)
parser.add_argument("-v", "--verbose", action="store_true")
parser.add_argument(
    "-p", "--profile",
    action="store_true",
    help="show timing info",
)

args = parser.parse_args()
VERBOSE = args.verbose
PROFILE = args.profile
DRY_RUN = args.no_run

SRC_FILE = args.FILE
TRANSE_EXE = args.transpilation_executable
TRANS_FILE = args.transpilation_file

overall_start = perf_counter() if PROFILE else None

transpilerpath = Path("build/morphoit")
if TRANSE_EXE is not None:
    transpilerpath = Path(TRANSE_EXE)
transpilerpath = transpilerpath.resolve()

if not transpilerpath.is_file():
    morphoit_print(f"Missing transpiler: {transpilerpath}")
    sys.exit(1)

stdin_data = None
if SRC_FILE != "-":
    srcpath = Path(SRC_FILE).resolve()
    basefilename = srcpath.name

    if not srcpath.is_file():
        morphoit_print(f"No such file '{SRC_FILE}'")
        sys.exit(1)
else:
    srcpath = "-"
    basefilename = "_stdin.morpho"
    stdin_data = sys.stdin.buffer.read()

binpath = Path(basefilename + EXE_EXTENSION)
if args.output:
    binpath = Path(args.output)
binpath = binpath.resolve()

ircodepath = None
if TRANS_FILE is not None:
    if TRANS_FILE == USE_DEFAULT_FILENAME:
        ircodepath = Path(basefilename + ".c")
    else:
        ircodepath = Path(TRANS_FILE)
    ircodepath = ircodepath.resolve()

if VERBOSE:
    morphoit_print(f"Transpiler: {transpilerpath}")
    morphoit_print(f"Source: {srcpath}")
    morphoit_print(f"Executable: {binpath}")
    morphoit_print(f"C output: {ircodepath}")

# --- Transpilation ---
trans_cmd = [str(transpilerpath), str(srcpath)]

if PROFILE:
    trans_start = perf_counter()

trans_proc = (
    sp.run(trans_cmd, stdout=sp.PIPE)
    if stdin_data is None
    else sp.run(trans_cmd, input=stdin_data, stdout=sp.PIPE)
)

if PROFILE:
    morphoit_print(f"Transpilation time (s): {perf_counter() - trans_start:.6f}")

if trans_proc.returncode != 0:
    morphoit_print("Transpilation failed")
    sys.exit(1)

if ircodepath:
    with open(ircodepath, "wb") as f:
        f.write(trans_proc.stdout)

# --- Compilation ---
comp_cmd = [
    "cc",
    "-x", "c",
    "-Wno-incompatible-pointer-types",
    "-O3",
    "-",
    "-o", str(binpath),
]

if PROFILE:
    comp_start = perf_counter()

comp_proc = sp.run(comp_cmd, input=trans_proc.stdout)

if PROFILE:
    morphoit_print(f"Compilation time (s): {perf_counter() - comp_start:.6f}")

if comp_proc.returncode != 0:
    morphoit_print("Compilation failed")
    sys.exit(1)

# --- Execution ---
run_elapsed = 0.0
if not DRY_RUN:
    if PROFILE:
        run_start = perf_counter()

    run_proc = sp.run([str(binpath)])

    if PROFILE:
        run_elapsed = perf_counter() - run_start
        morphoit_print(f"Run time (s): {run_elapsed:.6f}")

    rc = run_proc.returncode
else:
    rc = 0

if PROFILE:
    total_elapsed = perf_counter() - overall_start
    # Machine-readable markers for the benchmark script:
    print(f"__MI_TOTAL__ {total_elapsed:.9f}", file=sys.stderr)
    print(f"__MI_RUN__ {run_elapsed:.9f}", file=sys.stderr)

sys.exit(rc)