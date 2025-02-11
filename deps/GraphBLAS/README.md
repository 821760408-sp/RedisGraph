# SuiteSparse:GraphBLAS

SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

VERSION 2.3.0, June 16, 2019

SuiteSparse:GraphBLAS is an full implementation of the GraphBLAS standard,
which defines a set of sparse matrix operations on an extended algebra of
semirings using an almost unlimited variety of operators and types.  When
applied to sparse adjacency matrices, these algebraic operations are equivalent
to computations on graphs.  GraphBLAS provides a powerful and expressive
framework for creating graph algorithms based on the elegant mathematics of
sparse matrix operations on a semiring.

See the user guide in `Doc/GraphBLAS_UserGuide.pdf` for documentation on the
SuiteSparse implementation of GraphBLAS, and how to use it in your
applications.  The doxygen documentation at Doc/html/index.html gives details
on the internals of SuiteSparse:GraphBLAS (use "make dox" first, to create
the doxygen documentation).

See http://graphblas.org for more information on GraphBLAS, including the
GraphBLAS C API (also in `Doc/GraphBLAS_API_C.pdf`).

QUICK START: To compile, run several demos, and install, do these commands in
this directory:

    make
    sudo make install

Please be patient; some files can take several minutes to compile.  Requires an
ANSI C11 compiler, so cmake will fail if your compiler is not C11 compliant.
See the User Guide PDF in Doc/ for directions on how to use another compiler.

For faster compilation, do this instead of just "make", which uses 32
parallel threads to compile the package:

    make JOBS=32

The output of the demo programs will be compared with their expected output.

To remove all compiled files:

    make clean

To compile the library without running the demos or installing it:

    make library

To create the doxygen documentation in Doc/html, first do the following in
MATLAB (while in the Doc/ directory) if you have modified anything in the
Source/ or Include/ folders:

    dox_headers

Then do:

    make dox

--------------------------------------------------------------------------------
## Files and folders in this GraphBLAS directory:

CMakeLists.txt:  cmake instructions to compile GraphBLAS

Config:         version-dependent files used by CMake

Demo:           a set of demos on how to use GraphBLAS

Doc:            SuiteSparse:GraphBLAS User Guide and license

Doxyfile:       instructions for doxygen, and "make dox"

Extras:         parallel methods: triangle counting, k-truss, and a
                massively parallel (MPI) Kronecker product matrix generator.
                These are stand-along package that rely on GraphBLAS.  They
                are not compiled by the cmake script.  See Extras/README.txt
                for more details.

Include:        user-accessible include file, GraphBLAS.h

Makefile:       to compile the SuiteSparse:GraphBLAS library and demos

README.md:      this file

Source:         source files of the SuiteSparse:GraphBLAS library.

Tcov:           test coverage, requires MATLAB

Test:           Extensive tests, not meant for general usage.  To compile
                SuiteSparse:GraphBLAS and test in MATLAB, go to this directory
                and type gbmake;testall in MATLAB.

User:           user-defined objects at compile-time (.m4 files)

build:          build directory, initially empty

--------------------------------------------------------------------------------

## SPEC:

This version fully conforms to the version 1.2.0 (May 18, 2018)
of the GraphBLAS C API Specification.  It includes several additional functions
and features as extensions to the spec.  These extensions are tagged with the
keyword SPEC: in the code and in the User Guide, and in the Include/GraphBLAS.h
file.  All functions, objects, and macros with the prefix GxB are extensions to
the spec.  Functions, objects, and macros with prefix GB must not be accessed
by user code.  They are for internal use in GraphBLAS only.
