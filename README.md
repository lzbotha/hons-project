
## Code Documentation
The use of all exposed methods are documented in the respective header files in the include/ directory. A working example is provided in src/main.cpp which is currently set up to import a mesh called test-area.obj and export it again once finnished. All methods with reasonable parameters are provided, but some are commented out.

When using the mesh extraction libarary provided the order of the method calls is important. A mesh object must be created first. Then the mesh file must be imported. Once a mesh file has been imported it can be exported at any time. However only the walkable triangles in the mesh will be exported (which will be empty until a walkable_by_*) has been run.

Next one the the walkable_by_* methods must be run to determine which triangles are initially walkable. These methods are documented in depth in the report and more briefly again in the header file (mesh.h). The remaining methods may be run in any order but should be run in the following order for sane results:
- remove_* (bottleneck and overhangs)
- proximity_rejoin or graph_rejoin (only one of these methods should be run)
-remove_regions (both of these methods can be run and the order is not relevant)
- keep_largest_region


## Directory Layout
The important directories are as follows:
- *bin* Contains executables.
- *build* Contains .o files.
- *include* Contains all header files
- *src* Contains all source files
- *meshes* Contains meshes used.
- *test* Contains unit tests and other testing code

## External Libraries
The following libraries external to std where used:
- Open Asset Import library (Version 3.0)
- Fast Library for Approximate Nearest Neighbors (Version 1.8.4-4.1)
Both of these libraries where installed system wide from the Debian repositories and are not included with the code.

## Compiling & Running
The source code is set up with a Makefile that can be used to compile the source code. 

### Compiling:
- make: compiles all binaries into an executable.
- make tests: compiles all unit tests into an executable.
- make clean: removes all .o files and executables.

### Running:
The generated executable must be run from the command line.