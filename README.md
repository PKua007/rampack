# rampack

Random And Maximal PACKing PACKage - the software enabling simulation of particle systems using different packing
methods. Currently, it supports elastic and extensible monodisperse Monte Carlo sampling (with features such as full
isobaric-isotension relaxation in a triclinic box, many types of molecule moves, overlap relaxation, etc.) and more
algorithms are coming the future (eg. Torquato-Jiao MRJ scheme). The package includes also some utility tools. The
current interface is not final and is subject to change, so this description will provide only a general overview and
often instruct to inspect the example input file `integration.ini` and other sample input files in `sample_inputs`
directory, as well as the `--help` option.

## Operation modes

The whole functionality is provided within a single compiled binary. A C++ static linked library and Python bindings are
coming in the future. The operation mode can be selected using the first command line argument:

```shell
./rampack [mode] (mode-specific arguments and options)
```

Operation modes are:
* `casino` - Monte Carlo simulation facility
* `preview` - preview of the initial configuration
* `optimize-distance` - the optimization of lattice cell dimensions
* `trajectory` - operations on recorded simulation trajectories

and they are described in detail below.

A general built-in help is available under `./rampack --help`. Mode-specific guides can be displayed using
`./rampack [mode] --help`.

### casino

The `casino` mode is used to perform Monte Carlo simulations. Currently, available simulation types are NpT integration
and reduction of overlaps. The submodule is optimized for hard-core repulsion, however some soft interaction potentials
are also available. The parameters of the simulation are provided within an INI input file and are passed using
`-i [input file]` option. All details regarding the input file and NpT integration are described in an exemplary input 
file `sample_input/integration.ini`. Overlap reduction is described in `sample_input/overlap_reduction.ini`. The
anonymous INI section at the beginning describes the initial conditions of the system, particle model and interaction
model as well as specifies technical parameters such as number of threads and initial Monte Carlo step extents. Then,
one or more Monte Carlo runs are specified. Each run can be either NpT integration or overlap reduction. The runs are performed
sequentially in the order specified in the input file and the final state of a finished run is used as a starting point
of the next one (apart from the first run, whose initial configuration is specified in at the beginning). Each run
corresponds to an INI section named (including the brackets) `[integration.run_name]` for integration and
`[overlaps.run_name]` for overlaps reduction. Each of these sections includes the parameters for the specific run.
Currently, when the run is finished the software can output the following data:

* internal representation of the packing, which can be used for example as a starting point for another run (both run
  types)
* Mathematica notebook representing the packing (for best performance it is advisable to open it in a text editor, copy
  the contents and manually paste in an empty Mathematica notebook) (both run types)
* a csv-like table containing values of specified observables taken at given intervals of time (both run types)
* ensemble-averaged values of some observables (only NpT integration)
* a compact, binary recording of a simulation (trajectories), which can be used later for example to recalculate  
  observable averages (both run types)

The example input file `sample_inputs/integration.ini` describes the simulation of hard-core balls. It starts with a
gaseous phase, which is then compressed to a degenerate liquid in the second run and in the third one is freezes into
hcp crystalline structure. Another example input file `sample_inputs/overlap_reduction.ini` performs  reduction of
overlaps of too tightly packed hard spheres followed by an NpT run to gather averages.

The behavior of the `casino` mode can be altered using command-line options described in `./rampack casino --help`. For
example, one can continue a finished run for more cycles or start from any run if the previous run has been finished
in the past and the internal packing representation `*.dat` file has been stored.

### preview

The `preview` mode enables one to create the initial configuration from a given input file specified by `-i` option
and export it to internal and/or Wolfram Mathematica format. It may prove itself useful if one wants to eg. tweak the
lattice parameters using a visual inspection. The options are described in `./rampack preview --help`.

### optimize-distance

The `optimize-distance` mode is used to find the minimal distances in one or more directions between two particles. It
may help to choose the lattice spacing. Using appropriate options (see `./rampack optimize-distance --help`) one can
produce a clean output making it easier to incorporate into an automated workflow. Please note that long particles may
interact not only with the nearest neighbors, meaning that the values calculated by this mode may be too low. More
intelligent optimization, similar to the one done by `initialArrangement` input parameter will be added in the future.

### trajectory

The `trajectory` mode is used to analyze trajectories recorded during the simulation (see `recordingFilename` parameter
description in `sample_inputs/integration.ini`). Using appropriate options (see `./rampack optimize-distance --help`)
one can for example replay the simulation and calculate some observables which were not computed during the original
run.

## Compilation

The project uses CMake build system. To prepare build files for the Release build in the `build` folder, go to the
project's root folder and run:

```shell
mkdir build
cd build
cmake ../ -DCMAKE_BUILD_TYPE=Release
```

GCC with C++17 support is required (at least version 7). If gcc and g++ commands are aliased to a lower version, or to
clang (the default behaviour on macOS), you have to specify the path manually using cmake options
`-DCMAKE_C_COMPILER=...`, `-DCMAKE_CXX_COMPILER=...`.

Now, in the build folder, you can use `make rampack` to build the program (the binary will appear in `src` subfolder).
You can use `-j` option with a number of threads for a parallel compilation. 

### Automated tests

It is advised to run a suite of unit and validation tests before using the program. The tests can be performed by
compiling and running the two binaries:

* `[build folder]/unit_tests/unit_tests` for short unit tests with sanity-check assertions
* `[build folder]/validation_tests/valid_tests` for longer validation tests performing full MC sampling and comparing
  the results with the ones from the literature

It is advised to build the validation tests using a Release build configuration, since compiler optimizations
significantly reduce the run time.

## Code extensions

The package is designed in an OOP fashion and many elements are tweakable and expandable by implementing appropriate
interfaces. The examples are:

* additional shapes can be added by extending `core/ShapeTraits` class. The class itself presents an interface
  `core/Interaction` to further abstract away the interaction (and enables the support of many of them), 
  `core/ShapeGeometry` to represent geometric properties of a shape and `core/ShapePrinter` for Wolfram Mathematica
  output. One can implement all three interfaces in a one class. The shape parsing is done by `frontend/ShapeFactory`,
  so new shapes have to be registered there
* observables implement `core/Observable` interface. They should be registered in `frontend/ObservableCollectorFactory`
* bulk observables implement `core/BulkObservable` interface. `core/observable/correlation/PairDensityCorrelation` and 
  `core/observable/correlation/PairAveragedCorrelation` use interface `core/observable/correlation/PairEnumerator`
  to sample pairs and define the distance. `PairDensityCorrelation` has programmable correlation function to be computed
  given by the interface `core/observable/correlation/CorrelationFunction`. Everything is registered in
  `frontend/ObservableCollectorFactory`.
* molecule move sampling schemes (translation, rotation, flip, etc.) are provided by `core/MoveSampler` interface. They
  are registered in`frontend/MoveSamplerFactory`
* volume scaling scheme (all dimensions at once vs a single one a time, linear vs logarithmic, etc.) is provided by
  `core/VolumeScaler` and `core/TriclinicBoxScaler` interfaces. They are registered in
  `frontend/TriclinicVolumeScalerFactory`
* dynamic parameters (changing with a current cycle number) used for non-constant temperature and/or pressure are
  provided by `core/DynamicParameter` interface. They are registered in `frontend/DynamicParameterFactory`

### Documentation

The code is documented using Doxygen comments. A user-friendly HTML documentation can be generated using Doxygen:
`doxygen Doxyfile`

### Contribution

The contribution to the project is highly appreciated and can be done via the pull request (it should originate from the
newest commit in the `main` branch). All new functionality should be documented and unit-tested. No detailed code
guidelines are provided at the moment, but the general rules are:

* the code design should follow standard SOLID rules
* no `new`/`delete` manual memory management (or a minimal amount) - RAII is your friend (`std::unique_ptr`,
  `std::shared_ptr`, `std::vector`). In most cases no dynamic memory is needed at all - non-runtime-polymorphic objects
  can be automatic variables passed around as references
* references should be used instead of pointers. References lacking a value ('`NULL` pointers') can be achieved using
  `std::optional`
* methods not altering the objects should always be marked `const`. Conversely, objects that should not to be modified
  should be passed using constant references. Not following it in one place can cause problems in many other parts of
  the code down the line
* symbols should be named expressively
* one should use GSL-like preconditions and postconditions (`Expects` and `Ensures`) to validate method parameters
  and/or method output. Macros are available in `utils/Assertions.h` header. There is also `Validate` macro, which can
  be conveniently used to verify user input
* some performance may be sacrificed for readability, however with C++ it is almost always possible to write code which
  is both rapid and clean