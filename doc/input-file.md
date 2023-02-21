# Input file

## Contents

* [PYON format](#pyon-format)
* [Input file structure](#input-file-structure)
  * [Class `rampack`](#class-rampack)
  * [Simulation pipeline](#simulation-pipeline)
* [Run types](#run-types)
  * [Class `integration`](#class-integration)
  * [Class `overlap_relaxation`](#class-overlaprelaxation)
* [Dynamic parameters](#dynamic-parameters)
  * [Class `linear`](#class-linear)
  * [Class `log`](#class-log)
  * [Class `piecewise`](#class-piecewise)
* [Particle move types](#particle-move-types)
  * [Class `translation`](#class-translation)
  * [Class `rotation`](#class-rotation)
  * [Class `rototranslation`](#class-rototranslation)
  * [Class `flip`](#class-flip)
* [Box move types](#box-move-types)
  * [Class `delta_v`](#class-deltav)
  * [Class `linear`](#class-linear-1)
  * [Class `log`](#class-log)
  * [Class `disabled`](#class-disabled)

## PYON format

RAMPACK uses a custom-designed file format called PYON (PYthon Object Notation, inspired by JSON), which, as the name
suggests, uses Python syntax to represent the data. The main reason that lead us to design this novel format instead of
using existing ones (INI, JSON, YAML, etc.) was the object-oriented nature of the software. PYON input files resembles
creating an object tree with `rampack` object at the top and that closely resembles how the software works internally.

PYON has primitive values and data structures known from Python. Those are:
* Integers:

  ```python
  1
  -5
  0xFF
  0b10110110
  ```

* Floats:

  ```python
  1.3
  -.5
  5.13e-100
  ```

* Boolean:

  ```python
  True
  False
  ```

* None:

  ```python
  None
  ```

* Strings:

  ```python
  "abc"
  "escaped charecters: \" \\ \n \t"
  ```
  
* Arrays:

  ```python
  [1, "abc", None]
  ```

* Dictionaries (only string keys supported):

  ```python
  {"abc": 1, "def": 1.2, "ghi": [1, 2, 3]}
  ```

* Objects

  ```python
  foo(1, 3)
  bar(a=True, b=False)
  baz("abc", param=1)
  ```
  
  When the object construction does not take any arguments, `()` may be omitted

  ```python
  foobar
  foobar()
  ```
  
  Object constructors support default values, variadic arguments and keyword variadic arguments. Argument specification
  depends on the concrete class and is documented in this reference.

* Python `#`-style comments are supported:

  ```python
  [1, 2, 3] # this is array
  # line with only a comment
  "the # character inside a string is not interpreted as a comment start"
  ```

* Whitespace is ignored

  ```python
  data(
    arg1 = "abc",
    arg2 = [
      1,
      2
    ]
  )
  ```

## Input file structure

The input file contains a single rampack object with all necessary data passed as its arguments. Here is an exemplary
input file used in the [Tutorial](tutorial.md)

```python
rampack(
    version = "1.0",
    shape = sphere(r=0.5),
    arrangement = lattice(cell=sc, box_dim=15, n_shapes=1000),
    temperature = 1,
    pressure = 11.5,
    move_types = [translation(step=1)],
    box_move_type = delta_v(step=10),
    seed = 1234,
    handle_signals = True,
    runs = [
        integration(
            run_name = "liquid",
            thermalization_cycles = 500000,
            averaging_cycles = None,
            snapshot_every = 1000,
            output_last_snapshot = [ramsnap("packing_liquid.ramsnap")],
            record_trajectory = [ramtrj("recording_liquid.ramtrj")]
        )
    ]
)
```

### Class `rampack`

```python
rampack(
    version,
    arrangement,
    shape,
    seed,
    runs,
    temperature = None,
    pressure = None,
    move_types = None,
    box_move_type = None,
    walls = [False, False, False],
    box_move_threads = 1,
    domain_divisions = [1, 1, 1],
    handle_signals = True
)
```

Arguments:
* ***version***

  It specifies the minimal required version of RAMPACK the input file can run on. Supported formats:
  ```python
  version = "1"
  version = "1.0"
  version = "1.0.0"
  version = [1, 0, 0]
  ```
  The rule of thumb is that it should be set to a current version (which is 1.0) when creating the file and left
  unchanged. It is introduced as a safeguard to ensure that the input files produce replicable results and remain valid,
  even if breaking changes are introduced in an update, as well as to prevent running it in older versions (where some
  required features may be lacking). As RAMPACK is [versioned semantically](https://semver.org), all future 1.x versions
  should not introduce any breaking changes, and we plan to retain support of 1.x input files if 2.0 is released in the
  future (actually, we still support 0.x input files, even in an old INI format).

* ***arrangement***
  
  Initial arrangement - it specifies the shape of a simulation box and positions and orientations of particles inside
  it. It accepts objects of types: `lattice` and `presimulated`. Detailed description is available in
  [Initial arrangement](initial-arrangement.md).

* ***shape***

  It defines a shape of the particles and interaction between them. A detailed description of all shape classes can be
  found in [Shapes](shapes.md).

* ***seed***

  The integer being the seed of RNG. Simulation using the same domain specification (see `domain_divisions`), the same
  seeds and RAMPACK version should produce identical results.

* ***runs***

  An array of runs that should be performed in a given order. The run can be `integration` or `overlap_relaxation`. For
  example

  ```python
  runs = [overlap_relaxation(...), integration(...)]
  ```
  
  first performs some overlap relaxation run, and then and integration run. Both types of runs:
  [integration](#class-integration), [overlap_relaxation](#class-overlaprelaxation) are described below.

* ***temperature*** (*= None*)

  The temperature of the NpT/NVT simulation. It may be a constant Float or a [dynamic parameter](#dynamic-parameters).
  It is a part of the *simulation environment* (see [Simulation pipeline](#simulation-pipeline)). If `None`, it is left
  unspecified (*incomplete simulation environment*).

* ***pressure*** (*= None*)

  The pressure of the NpT simulation. It may be a constant Float or a [dynamic parameter](#dynamic-parameters). It is a
  part of the *simulation environment* (see [Simulation pipeline](#simulation-pipeline)). If `None`, it is left
  unspecified (*incomplete simulation environment*).

* ***move_types*** (*= None*)

  The array of Monte Carlo particle moves that should be performed during the simulation. Available move types are
  described [below](#particle-move-types). It is a part of the *simulation environment*
  (see [Simulation pipeline](#simulation-pipeline)). If `None`, it is left unspecified
  (*incomplete simulation environment*).

* ***box_move_type*** (*= None*)

  The type of move applied to the simulation box. Available move types are described [below](#box-move-types). Notably,
  using [class `disabled`](#class-disabled) turns off box updating, which means the simulation is of NVT type
  (`pressure` is ignored and may be `None`). It is a part of the *simulation environment*
  (see [Simulation pipeline](#simulation-pipeline)). If `None`, it is left unspecified
  (*incomplete simulation environment*).

* ***walls*** (*= [False, False, False]*)

  An array of Booleans, which specifies which box walls should be hard (non-penetrable by hard particles). `True` means
  that the wall is on, `False` otherwise. If the box vectors are denoted as *v1*, *v2* and *v3*, subsequent array
  indices correspond to box walls spanned by vectors:

  * index 0: *v2* and *v3*
  * index 1: *v3* and *v1*
  * index 2: *v1* and *v2*
  
  For an orthorhombic (cuboidal) box, it reduces to walls orthogonal to, respectively, x, y and z axis.

* ***box_move_threads*** (*= 1*)

  If Integer, is specifies how many OpenMP threads should be used to perform box scaling moves. One can also pass a
  String `"max"`, to use all available OpenMP threads (number of processor threads by default, or a custom value
  specified by `OMP_NUM_THREADS` environment variable).

* ***domain_divisions*** (*= [1, 1, 1]*)

  An array of integers specifysing how many domains in each direction should be used to parallelize particle moves.
  Total number of domains (thus, total number of OpenMP threads used) is a product of the elements. The box should be
  partitioned in such a way, that the domain dimensions are as large as possible to reduce the size of the ghost layers
  between the domains. For example, it is better to partition a cubic box into 2 x 2 x 2 domains instead of 1 x 1 x 8.
  The minimal dimension of a domain in any direction should be at least 2 times the *total interaction range*, which
  is displayed at the start of the simulation in the `casino` mode, or can be checked using
  [`shape-preview` mode](operation-modes.md#shape-preview-mode).

* ***handle_signals*** (*= True*)
  
  If `True`, `SIGINT` and `SIGTERM` will be captured and the simulation will be stopped, but all outputs will be
  gracefully closes, final snapshots will be printed and performance info will be shown, as usual. It comes very handy
  on computing cluster with a walltime, where you can usually send a signal to the job if it's about to be terminated.
  You shouldn't really turn it off, unless you have a good reason for it (for example you are experimenting and don't
  want to overwrite the output files).


### Simulation pipeline

The `casino` mode parses the input file and performs a set simulations based on its content. The following operations
are performed:

1. Initial arrangement is created (simulation box together with particles). It is controlled by `arrangement` argument.
   It can be creared procedurally or loaded from file (see below).
2. Initial *simulation environment* is created. It is described by the arguments `temperature`, `pressure`, `move_types`
   and `box_move_type`. One can specify any combination of those arguments (including none). Environment missing any of
   them is called *incomplete*. The only exception is when `box_move_type = disabled`. Then, simulation is of NVT type
   and parameter `pressure` is ignored, thus the environment may be complete even if `pressure = None`.
3. The first run is performed (the first object in the array passed to `runs` argument). Run can be `integration` for a
   standard Monte Carlo sampling or `overlap_relaxation` for a step-by-step elimination of overlaps:
    * for `integration` run:
        1. Run's simulation environment is prepared. It is defined using the same arguments as in the `rampack` class.
           Their values override the ones appearing in the `rampack` class. It is important that the combined environment
           is *complete*.
        2. Run's thermalization phase is performed (its length is specified by `thermalization_cycles`). During
           the thermalization phase, step sizes of all move types are tuned to reach 0.1-0.2 move acceptance ratio, which
           was proven to be good for hard particles. If specified (`record_trajectory`), one or more trajectory formats
           are saved to files on the fly. The same goes for observable snapshots (`observables`, `observables_out`).
        3. Run's averaging phase is performed. Step size adjustment is turned off (to preserve full detailed balance).
           Trajectory and observable snapshots continue to be recorded. On top of it, observables averages start being
           gathered, if specified (`averages_out`) and bulk observables are collected (`bulk_observables`,
           `bulk_observables_out_pattern`).
        4. When the run is finished, observable averages, bulk observables and final snapshots (`output_last_snapshot`)
           are stored.
    * for `overlap_relaxation` run:
        1. Simulation environment is prepared and combined in the same way as for `integration` run
        2. The run is performed. There is no averaging phase (so neither observable averages nor bulk observables can be
           collected). However, observable snapshots and trajectory recordings can be stored. During the run, moves
           reducing number of overlaps are always accepted and ones introducing new overlaps are always rejected. Thus,
           the number of overlaps declines over time.
        3. The run is finished when all overlaps are eliminated. Final snapshots are stored.
4. The next run from the array is performed:
    1. The final state of the previous run becomes the initial state of this run.
    2. Environment is prepared and combined with the environment from the previous run. Environment elements are reset
       or not, depending on the context:
        * if temperature/pressure is [dynamic](#dynamic-parameters) (depending on cycle number) it starts from the
          beginning (from the  value for cycle 0), even if it was not overriden by `temperature`/`pressure` run argument.
        * if `move_types`/`box_move_type` is not changes since the previous run, it remembers dynamically adjusted step
          sizes from the previous run. If it is overriden, previous step sizes are lost (as they can be meaningless for
          new move types)
    3. The rest is the same as for the first run
5. The previous step is repeated for all remaining runs

## Run types

There are two types of runs supported by RAMPACK:
* [Class `integration`](#class-integration)
* [Class `overlap_relaxation`](#class-overlaprelaxation)

### Class `integration`

```python
integration(
    run_name,
    thermalization_cycles,
    averaging_cycles,
    snapshot_every,
    temperature = None,
    pressure = None,
    move_types = None,
    box_move_type = None,
    averaging_every = 0,
    inline_info_every = 100,
    orientation_fix_every = 10000,
    output_last_snapshot = [],
    record_trajectory = [],
    averages_out = None,
    observables = [],
    observables_out = None,
    bulk_observables = [],
    bulk_observables_out_pattern = None
)
```

Arguments:

...

### Class `overlap_relaxation`


```python
overlap_relaxation(
    run_name,
    snapshot_every,
    temperature = None,
    pressure = None,
    move_types = None,
    box_move_type = None,
    inline_info_every = 100,
    orientation_fix_every = 10000,
    helper_shape = None,
    output_last_snapshot = [],
    record_trajectory = [],
    observables = [],
    observables_out = None
)
```

Arguments:

...


## Dynamic parameters

Parameters such as `temperature` and `pressure` can have static values, or be dynamic. In the latter case, their values
are computed as some function of the current cycle number.

Static parameters are set by passing a single `Float` or using the `const` class:
```python
temperature = 10
temperature = const(10)
temperature = const(value=10)
```

There are the following types of dynamic parameters:
* [Class `linear`](#class-linear)
* [Class `exp`](#class-exp)
* [Class `piecewise`](#class-piecewise)

### Class `linear`

```python
linear(
  slope,
  intercept = 0
)
```

It represents the parameter, which changes linearly with the cycle number, according to the equation

*value* = ***intercept*** + ***slope*** * *cycle number*

### Class `exp`

```python
exp(
  a0,
  rate
)
```

It represents the parameter, which changes exponentially with the cycle number, according to the equation

*value* = ***a0*** * exp(***rate*** * *cycle number*)

### Class `piecewise`

```python
piecewise(
    *args
)
```

It represents the parameter with a piecewise dependence on the cycle number. ***\*args*** is a variadic number of
arguments, which are of type `piece`:

```python
piece(
    start,
    param
)
```

***start*** is an Integer being the cycle number at which this piece begins and ***param*** is any static or dynamic
parameter, that should be used starting from ***start***-th cycle. The cycle number which is seen by ***param*** is
shifted so that it thinks that ***start***-th cycle is 0. The ***start*** of the first `piece` in `piecewise` object
has to start from 0, and others' ***start***-s have to be in ascending order. To give an example:

```python
piecewise(
    piece(0, 10),
    piece(1000, linear(slope=0.01, intercept=10)),
    piece(2000)
)
```

Here, for cycles 0-1000 the value is constant and equal 10, then between cycles 1000-2000 it grows linearly to 20 and
then it once again stays constant.

## Particle move types

There are the following particle move types:
* [Class `translation`](#class-translation)
* [Class `rotation`](#class-rotation)
* [Class `rototranslation`](#class-rototranslation)
* [Class `flip`](#class-flip)

### Class `translation`

### Class `rotation`

### Class `rototranslation`

### Class `flip`

## Box move types

There are the following box move types:
* [Class `delta_v`](#class-deltav)
* [Class `linear`](#class-linear-1)
* [Class `log`](#class-log)
* [Class `disabled`](#class-disabled)

### Class `delta_v`

### Class `linear`

### Class `log`

### Class `disabled`