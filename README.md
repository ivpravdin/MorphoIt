MorphoIt
=========
MorphoIt is an experimental project that offers a way to compile [morpho](https://github.com/Morpho-lang/morpho) code to a native binary by way of the [BuildIt](https://github.com/BuildIt-lang/buildit) partial evaluation library.

## Usage
```sh
morphoit [-D] FILE
```

* `FILE` should be the path to a morpho script or `-` to read from standard input.
* `-D` is a dev option to print the compiled bytecode of the provided script to standard out, rather than actually running the code.


## Design

### Pipeline
When morphoit is run on a script, the following occurs:
1. morpho script is compiled to bytecode via libmorpho
2. a custom VM is partially evaluated with the bytecode using the BuildIt library to generate C-code (saved to `/tmp/pe_out.c` by default)
3. this C-code is compiled as a shared library (`/tmp/pe_out.so` by default)
4. the library is linked and run using `main_morpho` as an entrypoint, which effectively running the generated code

### Explanation

### Code structure
* `main.cpp`: TODO
* `pe_vm.cpp`: TODO
* `value.cpp`: TODO
* `runtime.cpp`: TODO
* `pe_header.c`: TODO
* `pe_vm_consts.h`: TODO



## Features and Limitations
This implementation covers, so far:
* the primitive types: nil, bool, int, float
* Some limited object support, namely: user-defined functions, built-in functions, some instances of metafunctions, and strings
* standard control flow (`if`/`else if`/`else` statments, `while` loops, `for` loops)
* defining, calling, and returning from functions, including lambdas and built-in (C) functions
* higher order functions

Notable exceptions include:
* complex values
* closures
* invocations
* classes
* methods, and operator overloading
* closures
* optional arguments, and the support for variadic arguments is untested
* exception handling
* All other objects other than strings

## Build
### Build-time Dependencies
* CMake (3.13 or newer)
* C compiler capable of using the C13 standard (? TODO) or newer
  * we use gcc
* C++ compiler capable of C++13 (? TODO) standard or newer
  * we use g++
* libmorpho (TODO: version)

### Instructions
```sh
git clone --recursive 'https://github.com/IvanPrav/MorphoIt'
cd MorphoIt
mkdir build
cd build
cmake ..
cmake --build .
```
The compiled binary will be `build/morphoit`, by default.

## Test
The `tests` directory contains tests and benchmarks for morphoit. 
* `tests/morpho-tests` contains a subset of those found in libmorpho's test directory.
* TODO contains benchmarks

### test.py Usage
```sh
test.py FILE_OR_DIR
```
TODO
