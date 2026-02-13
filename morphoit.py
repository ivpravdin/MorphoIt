#!/usr/bin/env python3

import subprocess as sp
import sys
import argparse
from pathlib import Path

USE_DEFAULT_FILENAME=1
EXE_EXTENSION = '.exe' if sys.platform == 'win32' else '.bin'


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
# parser.add_argument('-c', '--compiler-args',
#                     help='options to pass to the compiler')




args = parser.parse_args()

transpilerpath = Path('build/morphoit')
# the morphoit binary
if args.transpilation_executable is not None:
    transpilerpath = Path(args.transpilation_executable)
transpilerpath = transpilerpath.resolve()

if not transpilerpath.is_file():
    print('Could not find executable for partial evaluator binary ' \
          f'({str(transpilerpath)})',
          file=sys.stderr)
    exit(1)

# .morpho file to be transpiled
if args.FILE != "-":
    srcpath = Path(args.FILE).resolve()
    basefilename = srcpath.name

    if not srcpath.is_file():
        print('No such file \'' + args.FILE + '\', exiting.', file=sys.stderr)
        exit(1)
else:
    srcpath = args.FILE
    basefilename = '_stdin.morpho'

# binary produced by compiling transpiled C code
binpath = Path(basefilename + EXE_EXTENSION)
if args.output is not None:
    binpath = Path(args.output)
binpath = binpath.resolve()

# .c file that is the result of the transpilation
ircodepath = None
if args.transpilation_file is not None:
    if args.transpilation_file == USE_DEFAULT_FILENAME:
        ircodepath = Path(basefilename + '.c')
    else:
        ircodepath = Path(args.transpilation_file)
    ircodepath = ircodepath.resolve()

if args.verbose:
    print("Morpho to C transpiler executable path:", transpilerpath, file=sys.stderr)
    print("Morpho sourcecode path:", srcpath, file=sys.stderr)
    print("Produced executable path:", binpath, file=sys.stderr)
    print("Transpiled code path:", ircodepath, file=sys.stderr)
    print("Args Object:", args, file=sys.stderr)



trans_cmd = [str(transpilerpath), str(srcpath)]
if args.verbose:
    print('Transpilation command:', *trans_cmd, file=sys.stderr)
trans_proc = sp.run(trans_cmd, capture_output=True)
if trans_proc.returncode != 0:
    print(
        'Transpilation exited with a non-zero exit code.' \
        'Dumping stdout and stderr and exiting with its exit code.\n\n'\
        '---------STDOUT----------\n',
        trans_proc.stdout,
        '\n\n---------STDERR----------\n',
        trans_proc.stderr,
        file=sys.stderr
    )
    exit(trans_proc.returncode)

if ircodepath is not None:
    with open(str(ircodepath), "wb") as file:
        file.write(trans_proc.stdout)

comp_cmd = ['cc', '-x', 'c', '-Wno-incompatible-pointer-types', '-O3', '-','-o', str(binpath)]
if args.verbose:
    print('Compilation command:', *comp_cmd, file=sys.stderr)
comp_proc = sp.run(comp_cmd, input=trans_proc.stdout)

if comp_proc.returncode != 0:
    print(
        'Compilation exited with a non-zero exit code. ' \
        'Exiting with its code.',
        file=sys.stderr
    )
    exit(trans_proc.returncode)

if (not args.no_run):
    sp.run([binpath])