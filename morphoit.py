#!/usr/bin/env python3

import subprocess as sp
import sys
import argparse
from termcolor import colored
from pathlib import Path
from time import time

USE_DEFAULT_FILENAME=1
EXE_EXTENSION = '.exe' if sys.platform == 'win32' else '.bin'

def morphoit_print(x):
    print(colored('[MORPHOIT]', 'blue', attrs=['bold']), x, file=sys.stderr)


parser = argparse.ArgumentParser()
parser.add_argument("FILE",
                    help="path to the Morpho source file to be transpiled;" 
                         "'-' for stdin, which will cause FILE to be treated"
                         "as _stdin.morpho for default filename purposes")
parser.add_argument("-o", "--output",
                    help="path to the produced binary. Defaults to FILE.bin"
                    "or FILE.exe")
parser.add_argument("-T", "--transpilation-executable", 
                    help="path to the Morpho to C transpiler")
parser.add_argument("-t", "--transpilation-file", 
                    nargs='?', const=USE_DEFAULT_FILENAME,
                    help="location to save the result of the transpilation;" \
                    " by default it is not saved at all. If this option is " \
                    "present with no argument provided, defaults to FILE.c")
parser.add_argument('-d', '--no-run', action='store_true',
                    help='don\'t automatically run the produced binary')
parser.add_argument('-v', '--verbose', action='store_true')
parser.add_argument('-p', '--profile', action='store_true',
                    help='show time taken to transpile/compile/run the program')
# parser.add_argument('-c', '--compiler-args',
#                     help='options to pass to the compiler')




args = parser.parse_args()
VERBOSE = args.verbose
PROFILE = args.profile
DRY_RUN = args.no_run

SRC_FILE = args.FILE
OUTPUT_FILE = args.output
TRANSE_EXE = args.transpilation_executable
TRANS_FILE = args.transpilation_file

transpilerpath = Path('build/morphoit')
# the morphoit binary
if TRANSE_EXE is not None:
    transpilerpath = Path(TRANSE_EXE)
transpilerpath = transpilerpath.resolve()

if not transpilerpath.is_file():
    morphoit_print('Could not find executable for partial evaluator binary ' \
          f'({str(transpilerpath)})')
    exit(1)

# .morpho file to be transpiled
if SRC_FILE != "-":
    srcpath = Path(SRC_FILE).resolve()
    basefilename = srcpath.name

    if not srcpath.is_file():
        morphoit_print('No such file \'' + SRC_FILE + '\', exiting.')
        exit(1)
else:
    srcpath = SRC_FILE
    basefilename = '_stdin.morpho'

# binary produced by compiling transpiled C code
binpath = Path(basefilename + EXE_EXTENSION)
if args.output is not None:
    binpath = Path(args.output)
binpath = binpath.resolve()

# .c file that is the result of the transpilation
ircodepath = None
if args.transpilation_file is not None:
    if TRANS_FILE == USE_DEFAULT_FILENAME:
        ircodepath = Path(basefilename + '.c')
    else:
        ircodepath = Path(args.transpilation_file)
    ircodepath = ircodepath.resolve()

if VERBOSE:
    morphoit_print("Morpho to C transpiler executable path: " + str(transpilerpath))
    morphoit_print("Morpho sourcecode path: " + str(srcpath))
    morphoit_print("Produced executable path: " + str(binpath))
    morphoit_print("Transpiled code path: " + str(ircodepath))
    morphoit_print("Args Object: " + str(args))


trans_cmd = [str(transpilerpath), str(srcpath)]
if VERBOSE:
    morphoit_print('Transpilation command: ' + str(trans_cmd))

if PROFILE:
    trans_start_time = time()
trans_proc = sp.run(trans_cmd, stdout=sp.PIPE)
if PROFILE:
    trans_stop_time = time()
    morphoit_print(f"Transpilation time (s): {trans_stop_time - trans_start_time}")
if trans_proc.returncode != 0:
    morphoit_print(f"Morpho to C transpilation failed. Exiting.")
    exit(1)


if ircodepath is not None:
    with open(str(ircodepath), "wb") as file:
        file.write(trans_proc.stdout)

comp_cmd = ['cc', '-x', 'c', '-Wno-incompatible-pointer-types', '-O3', '-','-o', str(binpath)]

if VERBOSE:
    morphoit_print('Compilation command: ' + str(comp_cmd))
if PROFILE:
    comp_start_time = time()
comp_proc = sp.run(comp_cmd, input=trans_proc.stdout)
if PROFILE:
    comp_stop_time = time()
    morphoit_print(f"Compilation time (s): {comp_stop_time - comp_start_time}")
if comp_proc.returncode != 0:
    morphoit_print("C compilation failed. Exiting.")
    exit(1)


if not DRY_RUN:
    if PROFILE:
        run_start_time = time()
    run_proc = sp.run([binpath])
    if PROFILE:
        run_stop_time = time()
        morphoit_print(f"Run time (s): {run_stop_time - run_stop_time}")
    exit(run_proc.returncode)