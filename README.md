![rampack](artwork/rampack.svg)

[![cmake workflow](https://github.com/PKua007/rampack/actions/workflows/cmake.yml/badge.svg)](https://github.com/PKua007/rampack/actions/workflows/cmake.yml)
[![Stand With Ukraine](https://raw.githubusercontent.com/vshymanskyy/StandWithUkraine/main/badges/StandWithUkraine.svg)](https://stand-with-ukraine.pp.ua)


Random And Maximal PACKing PACKage - the software for simulating particle systems using different flavors of Monte Carlo
sampling. Currently, it features, among others, full isobaric-isotension relaxation in a triclinic box, many types of
particle moves, hard walls and elimination of overlaps. Both hard and soft interaction potentials are supported. More
extensions, including the Torquato-Jiao MRJ scheme, are coming in the future. The package also provides data analysis
tools - for example, it can compute observables such as nematic and smectic order as well as more complex ones including
correlation functions and density histograms. It all can be done both at the simulation time and afterwards using
recorded trajectories.

It operates as a single compiled binary with various modes. Additionally, it will be distributed as a C++ static-linked
library (later also with Python bindings) in the near future.

## Quickstart

Each release of RAMPACK contains:
* source core (Linux / macOS / Windows)
* standalone static-linked `rampack` binary (Linux)
* DEB/RPM packages with static-linked `rampack` binary and autocompletion scripts (Linux)

The compiled binary (standalone/DEB/RPM) is ready to be used out of the box (Linux only). However, it is recommended to
compile it from source to fully utilize native CPU optimizations. See [Installation](doc/installation.md) for the
details. `bash` and `zsh` autocompletion scripts are bundled. They are installed automatically when compiled from source
and are included in DEB/RPM packages.

We provided a few sample input files in the `sample_inputs` folder. They can be used to verify that the software is
working correctly and are a good learning materials. For example,

```shell
rampack casino -i integration.pyon
```

will simulate all three phases of hard balls: gas, liquid and crystal and will output many data files, including
trajectories (RAMTRJ, XYZ), final system snapshots (RAMSNAP, XYZ, Wolfram), observable snapshots and ensemble averages.
All types of output are documented in [Output formats](doc/output-formats.md).

## Operation modes

The general execution syntax is as follows:

```shell
rampack [mode] (mode-specific arguments and options)
```

The first argument is operation mode. The following modes are available:

* `casino` - Monte Carlo sampling facility
* `preview` - preview of the initial configuration and input file info
* `shape-preview` - information and preview for shapes
* `trajectory` - operations on recorded simulation trajectories

A general built-in help is available under `rampack --help`. Mode-specific guides can be displayed using
`rampack [mode] --help`. `rampack --version` shows a current version. In-detail descriptions of all modes can be found
in [Operation modes](doc/operation-modes.md).

## Tutorial and full reference

Full documentation can be found in [Reference](doc/reference.md). A summary of the most important aspects can be found
in the [Tutorial](doc/tutorial.md).

## Contribution

The contribution to the project is highly appreciated and can be done via the pull request. Contribution guide can be
found in [Contributing](doc/contributing.md). [Source code](doc/source-code.md) describes how RAMPACK's source code is
organized and walks through all important API-s.
