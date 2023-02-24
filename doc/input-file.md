# Input file

This reference page described the input file format and simulation pipeline.

## Contents

* [PYON format](#pyon-format)
* [Input file structure](#input-file-structure)
  * [Class `rampack`](#class-rampack)
  * [Simulation environment](#simulation-environment)
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
  * [Class `delta_triclinic`](#class-deltatriclinic)
  * [Class `disabled`](#class-disabled)

## PYON format

RAMPACK uses a custom-designed file format called PYON (PYthon Object Notation, inspired by JSON), which, as the name
suggests, uses Python syntax to represent the data. The main reason that lead us to design this custom format instead of
using existing ones (INI, JSON, YAML, etc.) was the object-oriented nature of the software. PYON input files resembles
creating an object tree with `rampack` object at the top and that closely resembles how the software works internally.

PYON has primitive values and data structures known from Python. Those are:

* Integer:

  ```python
  1
  -5
  0xFF
  0b10110110
  ```

* Float:

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
  
* Array:

  ```python
  [1, "abc", None]
  ```

* Dictionary (only string keys supported):

  ```python
  {"abc": 1, "def": 1.2, "ghi": [1, 2, 3]}
  ```

* Object (class)

  ```python
  # argument names: foo(arg1, arg2, arg3)
  foo(1, 2, 3)         
  foo(arg3=3, arg1=1, arg2=2)
  foo(1, arg3=3, arg2=2)
  ```
  
  Arguments' semantics is closely modelled on Python object constructors. Normal arguments are positional, which means
  that they have to be passed in a specific order depending on the class if the name is not specified explicitly.
  However, when one uses the keyword syntax `key=value`, the order is arbitrary as long as all are specified. Positional
  and keyword styles may be mixed - first few positional arguments may be given in a fixed order without their names,
  and the remaining ones in any order using keyword style. When the object construction does not take any arguments,
  `()` may be  omitted

  ```python
  foobar
  foobar()
  ```
  
  Object constructors support default values, variadic arguments and keyword variadic arguments. Argument specification
  depends on a concrete class and is documented in this reference.

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
input file used in the [Tutorial](tutorial.md).

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

The rest of this reference document describes all aspects of the input file class by class.

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

It is the main class enclosing all simulation details and all runs. It creates the initial box with particles and
has its own *simulation environment* (see [Simulation pipeline](#simulation-pipeline)), which is later combined with
first run's *simulation environment*.

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
  even if breaking changes are introduced in an update, as well as to prevent running it in older versions of RAMPACK
  (where some required features may be lacking). As RAMPACK is [versioned semantically](https://semver.org), all future
  1.x versions should not introduce any breaking changes, and we plan to retain support of 1.x input files if 2.0 is
  released in the future (actually, we still support 0.x input files, even in an old INI format).

* ***arrangement***
  
  Initial arrangement - it specifies the shape of a simulation box and positions and orientations of particles inside
  it. It accepts objects of types: `lattice` and `presimulated`. A detailed description is available in
  [Initial arrangement](initial-arrangement.md).

* ***shape***

  It defines a shape of the particles and interaction between them. A detailed description of all shape classes can be
  found in [Shapes](shapes.md).

* ***seed***

  The integer being the seed of RNG. Simulations using the same domain specification (see `domain_divisions`), the same
  seeds and RAMPACK version should produce identical results.

* ***runs***

  An array of runs that should be performed in a given order. The run can be `integration` or `overlap_relaxation`. For
  example

  ```python
  runs = [overlap_relaxation(...), integration(...)]
  ```
  
  first performs an overlap relaxation run, and then an integration run. Both types of runs:
  [class `integration`](#class-integration), [class `overlap_relaxation`](#class-overlaprelaxation) are described below.

* ***temperature*** (*= None*)

  The temperature of the NpT/NVT simulation. It may be a constant Float or a [dynamic parameter](#dynamic-parameters).
  It is a part of the [Simulation environment](#simulation-environment). If `None`, it is left unspecified (*incomplete
  simulation environment*).

* ***pressure*** (*= None*)

  The pressure of the NpT simulation. It may be a constant Float or a [dynamic parameter](#dynamic-parameters). It is a
  part of the [Simulation environment](#simulation-environment). If `None`, it is left unspecified (*incomplete
  simulation environment*). It is ignored in the NVT run - when `box_move_type` is [class `disabled`](#class-disabled).

* ***move_types*** (*= None*)

  The Array of Monte Carlo particle moves that should be performed during the simulation. Available move types are
  described [below](#particle-move-types). It is a part of the [Simulation environment](#simulation-environment). If
  `None`, it is left unspecified (*incomplete simulation environment*).

* ***box_move_type*** (*= None*)

  The type of move applied to the simulation box. Available move types are described [below](#box-move-types). Notably,
  using [class `disabled`](#class-disabled) turns off box updating, which means the simulation is of NVT type
  (`pressure` is ignored and may be `None`). It is a part of the [Simulation environment](#simulation-environment). If
  `None`, it is left unspecified (*incomplete simulation environment*).

* ***walls*** (*= [False, False, False]*)

  An aArray of Booleans that specifies which box walls should be hard (non-penetrable by hard particles). `True` means
  that the wall is on, `False` otherwise. If the box vectors are denoted as *v1*, *v2* and *v3*, subsequent array
  indices correspond to box walls spanned by vectors:

  * index 0: *v2* and *v3*
  * index 1: *v3* and *v1*
  * index 2: *v1* and *v2*
  
  For an orthorhombic (cuboidal) box, it reduces to walls orthogonal to, respectively, x, y and z axis.

  **IMPORTANT**: toggling the wall on **DOES NOT** automatically disable box scaling in this direction. It has to be
  specified manually in `box_move_type` (see [class `linear`](#class-linear-1) nad [class `log`](#class-log)).

* ***box_move_threads*** (*= 1*)

  If Integer, is specifies how many OpenMP threads should be used to perform box scaling moves. One can also pass a
  String `"max"`, to use all available OpenMP threads (number of processor threads by default, or a custom value
  specified by `OMP_NUM_THREADS` environment variable).

* ***domain_divisions*** (*= [1, 1, 1]*)

  An Array of integers specifysing how many domains in each direction should be used to parallelize particle moves.
  Total number of domains (thus, total number of OpenMP threads used) is a product of the elements. The box should be
  partitioned in such a way, that the domain dimensions are as large as possible. That way, the size of the ghost layers
  between the domains where the particles are frozen is reduced to minimum. For example, it is better to partition a
  cubic box into 2 x 2 x 2 domains instead of 1 x 1 x 8. The minimal dimension of a domain in any direction should be at
  least 2 times the *total interaction range*, which is displayed at the start of the simulation in the `casino` mode,
  or can be checked using [`shape-preview` mode](operation-modes.md#shape-preview-mode). When a domain becomes too
  narrow, RAMPACK automatically reduces the number of partitions.

* ***handle_signals*** (*= True*)
  
  If `True`, `SIGINT` and `SIGTERM` will be captured and the simulation will be stopped, but all outputs will be
  finalized and closed, final snapshots will be printed and performance info will be shown. It comes very handy on
  computing cluster with a walltime, where you can usually send a signal to the job if it's about to be terminated. You
  shouldn't really turn it off, unless you have a good reason for it (for example you are experimenting and don't
  want to overwrite the output files).


### Simulation environment

*Simulation environment* is composed of thermodynamic parameters and types of moves that are performed:

* `temperature`
* `pressure`
* `move_types`
* `box_move_type`

Simulation environment is present in [class `rampack`](#class-rampack) and in all [Run types](#run-types). One can 
specify any combination of those arguments (including none of them). When at least of them is `None`, the environment is
called *incomplete*. Otherwise, it is *complete*. The only exception is when `box_move_type = disabled`. Then,
simulation is of NVT type and parameter `pressure` is ignored (environment can be complet even if it is `None`). 
Simulation environments can be combined. When it is done, all not-`None` components of a new environment overwrite the
ones from the old environment, while for the `None` components the old ones are reused.

### Simulation pipeline

The `casino` mode parses the input file and performs a set simulations based on its content. The following operations
are performed:

1. Initial arrangement is created (simulation box together with particles). It is controlled by `arrangement` argument.
2. Initial *simulation environment* is created (based on [class `rampack`](#class-rampack) arguments).
3. The first run is performed (the first object in the array passed to `runs` argument). Run can be `integration` for a
   standard Monte Carlo sampling or `overlap_relaxation` for a step-by-step elimination of overlaps.

   For `integration` run:
   1. Run's simulation environment is prepared and combined with `rampack` environment. The combined environment
      has to be *complete*.
   2. Run's **thermalization phase** is performed (its length is specified by `thermalization_cycles`). During it,
      the following operations are performed:
      * step sizes of all move types are tuned to reach 0.1-0.2 move acceptance ratio, which was proven to be good
        for hard particles
      * if specified by `record_trajectory`, one or more trajectory formats are saved to files on the fly
      * if specified by `observables` an `observables_out`, observable snapshots are stored on the fly
   3. Run's **averaging phase** is performed:
      * step size adjustment is turned off (to preserve full detailed balance)
      * trajectory and observable snapshots continue to be recorded
      * if specified by `averages_out`, observables averages are gathered
      * if specified by `bulk_observables` and `bulk_observables_out_pattern`, bulk observables are collected
   4. When the run is finished, observable averages, bulk observables and final snapshots (`output_last_snapshot`)
      are stored. 
   
   For `overlap_relaxation` run:
   1. Simulation environment is prepared and combined in the same way as for `integration` run.
   2. The run is performed:
      * moves reducing number of overlaps are always accepted and ones introducing new overlaps are always
        rejected. Thus, the number of overlaps declines over time
      * there is no averaging phase (so neither observable averages nor bulk observables can be
        collected)
      * observable snapshots and trajectory recordings can be stored
   3. The run is finished when all overlaps are eliminated. Final snapshots are stored.
4. The next run from the Array is performed:
   1. The final state of the previous run becomes the initial state of this run.
   2. Environment is prepared and combined with the environment from the previous run.
   3. Environment elements are reset or not, depending on the context:
      * if temperature/pressure is [dynamic](#dynamic-parameters) (depending on cycle number) it starts from the
        beginning (from the value for cycle 0), even if it was not overriden by `temperature`/`pressure` run argument
      * if `move_types`/`box_move_type` is not changed since the previous run, it remembers dynamically adjusted step
        sizes from the previous run. If it is overriden, previous step sizes are lost (as they can be meaningless for
        new move types)
   4. The rest is the same as for the first run.
5. The previous step is repeated for all remaining runs in the Array.

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

Run type performing Monte Carlo sampling. It consists of the thermalization phase and the averaging phase. During both
phases, various types of auxiliary data, such as simulation trajectory or observable values are gathered and processed.
Many parameters of the Monte Carlo algorithm may be tweaked, including particle's shape, interaction type, presence of
hard walls and types of particle or box perturbation moves.

Arguments:

* ***run_name***

  String with user-specified, unique name of the run.

* ***thermalization_cycles***

  Integer representing number of cycles that should be performed in the thermalization phase. If the thermalization
  phase should be skipped, you should pass `None`.

* ***averaging_cycles***

  Integer representing number of cycles that should be performed in the averaging phase. If the averaging phase should
  be skipped, you should pass `None`.

* ***snapshot_every***

  Integer representing how often snapshots should be taken (for `record_trajectory` and`observables`) in both simulation
  phases. It should divide `thermalization_cycles` and `averaging_cycles` without remainder.

* ***temperature*** (*= None*)

  The temperature of the NpT/NVT simulation. It may be a constant Float or a [dynamic parameter](#dynamic-parameters).
  It is a part of the [Simulation environment](#simulation-environment). If `None`, the value from the previous run (or
  from [class `rampack`](#class-rampack) arguments) is reused.

* ***pressure*** (*= None*)

  The pressure of the NpT simulation. It may be a constant Float or a [dynamic parameter](#dynamic-parameters). It is a
  part of the [Simulation environment](#simulation-environment). If `None`, the value from the previous run (or from
  [class `rampack`](#class-rampack) arguments) is reused.

* ***move_types*** (*= None*)

  The Array of Monte Carlo particle moves that should be performed during the simulation. Available move types are
  described [below](#particle-move-types). It is a part of the *simulation environment* (see
  [Simulation pipeline](#simulation-pipeline)). If `None`, the value from the  previous run (or from
  [class `rampack`](#class-rampack) arguments) is reused.

* ***box_move_type*** (*= None*)

  The type of move applied to the simulation box. Available move types are described [below](#box-move-types). Notably,
  using [class `disabled`](#class-disabled) turns off box updating, which means the simulation is of NVT type
  (`pressure` is ignored and may be `None`). It is a part of the [Simulation environment](#simulation-environment). If
  `None`, the value from the previous run (or from [class `rampack`](#class-rampack) arguments) is reused.

* ***averaging_every*** (*= 0*)

  How often average value should be taken (for `averages_out` and `bulk_observables`) in the averaging phase. It should
  divide `averaging_cycles` without remainder. It can be equal 0 if the averaging phase is off
  (`averaging_cycles = None`).

* ***inline_info_every*** (*= 100*)

  How often inline info should be printed to the standard output. This includes current cycle number and values of
  observables with an `inline` scope.

* ***orientation_fix_every*** (*= 10000*)

  How often rotation matrices should be renormalized. This is needed because they stockpile numerical errors after
  numerous matrix multiplications during the simulation. Unless you have a good reason to change it, the default value
  of `10000` should be left as it is.

* ***output_last_snapshot*** (*= []*)

  The array of formats in which the last snapshot should be stored after the simulation. For example

  ```python
  output_last_snapshot = [ramsnap("packing.ramsnap")]
  ```
  
  will store RAMSNAP representation to the file `packing.ramsnap`.
  See [Last snapshot writers](output-formats.md#last-snapshot-writers) for more information.

* ***record_trajectory*** (*= []*)

  The array of formats in which the trajectory should be stored during the simulation. For example

  ```python
  record_trajectory = [ramtrj("trajectory.ramtrj")]
  ```
 
  will store RAMTRJ trajectory to the file `trajectory.ramtrj`. See
  [Trajectory writers](output-formats.md#trajectory-writers) for more information.

* ***averages_out*** (*= None*)

  The name of file to which observable averages should be stored. They are gathered in the averaging phase for
  observables with `averaging` scope. If the file does not exist, it is created. Otherwise, a new row with
  averages is appended at the end. See [Observable averages](output-formats.md#observable-averages) for more
  information.

* ***observables*** (*= []*)

  The Array of observables which should be computed during the simulation. Observables have 3 scopes: `snapshot`,
  `inline` and `averaging`. `snapshot` observable values are printed out to `observables_out` every `snapshot_every`
  cycles, `inline` observables' values are included in a simulation state line printed to the standard output, while
  `averaging` observables are averaged in the averaging phase and printed to `averages_out`. By default, observable is
  in all three scopes. To manually select the scope, one can use [class `scoped`](observables.md#class-scoped). For
  example

  ```python
  observables = [packing_fraction, scoped(nematic_order, inline=True)]
  ```
  
  means that [class `packing_fraction`](observables.md#class-packingfraction) is in all scopes, while
  [class `nematic_order`](observables.md#class-nematicorder) only in the `inline` scope. See
  [Observables](observables.md) for more information and a full list of available observables.

* ***observables_out*** (*= None*)

  String with a name of the file where values of `observables` in the `snapshot` scope snapshots will be stored every
  `snapshot_every` cycles.

* ***bulk_observables*** (*= []*)

  The Array of bulk observables that will be computed in the averaging phase. Bulk observables are more complex than
  normal observables, thus they are stored in separate files specified by `bulk_observables_out_pattern`. They are
  gathered and averaged in the averaging phase every `averaging_every` cycles. See [Observables](observables.md) for
  more information and a full list of available bulk observables.

* ***bulk_observables_out_pattern*** (*= None*)

  Name pattern for files to store bulk observables. If the pattern contains `{}`, it is replaced by the name of bulk
  observable it is going to store. For example, for `bulk_observables_out_pattern = "{}_out.txt"`, bulk observable named
  `rho` will be stored to a file named `rho_out.txt`. If `{}` is missing, `_{}.txt` is inserted at the end of the
  pattern. Thus, if `bulk_observables_out_pattern = "out"`, `rho` bulk observable will be stored to `out_rho.txt`.

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

Run type where the overlaps of particles are slowly eliminated by rewarding Monte Carlo moves which decrease the number
of overlaps and penalizing ones which introduce them. A large part of the specification overlaps (pun not intended) with
[class `integration`](#class-integration), however, most notably, the averaging phase is missing (so do all types of
output tied to it, such as observable averages and bulk observables).

Arguments:

* ***run_name***

  See [class `integration`](#class-integration).

* ***snapshot_every***

  See [class `integration`](#class-integration).

* ***temperature*** (*= None*)

  See [class `integration`](#class-integration).

* ***pressure*** (*= None*)

  See [class `integration`](#class-integration).

* ***move_types*** (*= None*)

  See [class `integration`](#class-integration).

* ***box_move_type*** (*= None*)

  See [class `integration`](#class-integration).

* ***inline_info_every*** (*= 100*)

  See [class `integration`](#class-integration).

* ***orientation_fix_every*** (*= 10000*)

  See [class `integration`](#class-integration).

* ***helper_shape*** (*= None*)

  Helper shape used to speed up overlap relaxation by introducing soft repulsion. Its interaction is imposed on top of
  original shape's interaction (although the original shape and helper shape DO NOT cross-interact). Technically it can
  be any shape, however, to achieve the goal, its interaction should be zero whenever the original shapes are not
  overlapping and be of repulsive soft-core type, when they are overlapping. A good choice is a sphere inscribed in the
  original shape with soft repulsion, such as [class `square_inverse_core`](shapes.md#class-squareinversecore)
  or [class `repulsive_lj`](shapes.md#class-repulsive_lj).

* ***output_last_snapshot*** (*= []*)

  See [class `integration`](#class-integration).

* ***record_trajectory*** (*= []*)

  See [class `integration`](#class-integration).

* ***observables*** (*= []*)

  See [class `integration`](#class-integration).

* ***observables_out*** (*= None*)

  See [class `integration`](#class-integration).

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
parameter (excluding `piecewise`), that should be used starting from ***start***-th cycle. The cycle number which is
seen by ***param*** is shifted so that it thinks that ***start***-th cycle is 0. The ***start*** of the first `piece` in
`piecewise` object has to be 0, and ***start*** arguments of the following ones have to be in ascending order. To give
an example:

```python
piecewise(
    piece(0, const(10)),  # could also be simply `piece(0, 10)`
    piece(1000, linear(slope=0.01, intercept=10)),
    piece(2000, const(20))
)
```

Here, for cycles 0-1000 the value is constant and equal 10, then between cycles 1000-2000 it grows linearly to 20, and
then it once again is constant, this time indefinitely.

## Particle move types

There are the following particle move types:
* [Class `translation`](#class-translation)
* [Class `rotation`](#class-rotation)
* [Class `rototranslation`](#class-rototranslation)
* [Class `flip`](#class-flip)

### Class `translation`

```python
translation(
    step,
    max_step = None
)
```

Monte Carlo move performing only translations of particles. If current step size is equal *current_step*, random
translation is constructed by sampling random coordinates of the translation vector from the uniform interval
[-*current_step*, *current_step*] (each coordinate is independent). `step` controls the initial value of *current_step*,
while `max_step` imposes an upper limit on it. If `None`, the upper limit is half of the smallest height of the
simulation box.

### Class `rotation`

```python
rotation(
    step
)
```

Monte Carlo move performing only rotations of particles. If current step size is equal *current_step*, random rotation
is constructed by selecting a random rotation axis and performing the rotation around this axis with the rotation angle
selected uniformly from the interval [-*current_step*, *current_step*]. `step` is the initial value of *current_step*.

### Class `rototranslation`

```python
rototranslation(
    trans_step,
    rot_step,
    max_trans_step = None
)
```

Monte Carlo move performing translation and rotation at the same time. Random perturbations are chosen in the same way
as in [class `translation`](#class-translation) and [class `rotation`](#class-rotation), with
`trans_step` and `max_trans_step` having the same meaning as `step` and `max_step` arguments of
[class `translation`](#class-translation) and `rot_step` the same meaning as `step` argument of
[class `rotation`](#class-rotation). It should be noted that the step sizes of translation and rotation components are
adjusted at the same time and their ratio remains constant.

### Class `flip`

```python
flip(
    every = 10
)
```

Monte Carlo move performing the flip, which is a half rotation around particle's [secondary axis](shapes.md#geometry).
`every` controls how often the flip is performed. For example, its default value `10` means that in a full single MC
cycle for 10% of all particles flip move will be attempted (and accepted according to the Metropolis criterion).

## Box move types

There are the following box move types:
* [Class `delta_v`](#class-deltav)
* [Class `linear`](#class-linear-1)
* [Class `log`](#class-log)
* [Class `delta_triclinic`](#class-deltatriclinic)
* [Class `disabled`](#class-disabled)

Box move types are able to relax the system to a varying degree. `delta_v` type relaxes only the system pressure
(trace of the stress tensor) realizing the standard NpT ensemble. `linear` and `log` types may be anisotropic, which
additionally relaxes the diagonal part of stress tensor, which is required for structures with macroscopic periods.
`delta_triclinic` type relaxes the whole stress tensor, thus realizing the so called isobaric-isotension ensemble and is
very useful when simulating crystalline structures.

### Class `delta_v`

```python
delta_v(
    step
)
```

Monte Carlo move performing isotropic box scaling. The scaling factor is calculated based on an absolute box volume
change chosen at random. If current step size is equal *current_step*, volume change is sampled uniformly from the
interval [-*current_step*, *current_step*].

### Class `linear`

```python
linear(
    spec,
    step,
    independent = False
)
```

General anisotropic scaler preserving angles between box walls, perturbing it in a linear manner. More precisely, it
operates only on the heights of box vectors. If current step size is equal *current_step*, a random number *Delta_h* is
sampled uniformly from [-*current_step*, *current_step*] interval and the given box height is changed by *Delta_h*.

Arguments:

* ***spec***
 
  A String which specifies which box heights are perturbed and if the perturbations are independent or not. Independent
  heights are perturbed using independent *Delta_h* random numbers, while dependent heights using the same one. There
  are 5 predefined values of `spec`:

  * `spec = "isotropic"` - all heights are dependent and scaled at one (a cubic box remains cubic)
  * `spec = "anisotropic x"`, `"anisotropic y"`, `"anisotropic z"` - one direction is scaled independently of the two
    other, dependent ones (tetrahedral box remains tetrahedral)
  * `spec = "anisotropic xyz"` - all directions are independent

  One can also manually specify the scaling directions. `spec` should then be a string containing all 3 axes: `"x"`,
  `"y"`, `"z"`. By default, all are independent. To make 2 axes dependent, they should be put into parentheses `"()"`.
  If the direction should not be scaled at all, it must be put into square brackets `"[]"`. For example:

  * `spec = "xyz"` - the same as `"anisotropic xyz"`
  * `spec = "(xyz)"` - the same as `"isotropic"`
  * `spec = "x(yz)"` - the same as `"anisotropic x"`
  * `spec = "(zx)[y]"` - x and z direction are dependent, and y is not scaled at all

* ***step***

  Initial value of *current_step*.

* ***independent*** (*= False*)
  
  Specifies if independent scaling factors should be sampled in one move (`False`), or only one independent group at
  once (`True`). For example, for `spec = "x(yz)"`:

  * if `independent = True`, a single move is either scaling only x direction or scaling y and z at one (using a single
    *Delta_h*)
  * if `independent = False`, a single move is scaling all directions at one, but x is scaled using one *Delta_h* random
    number, while y and z using a different, independently sampled *Delta_h*

### Class `log`

```python
linear(
    spec,
    step,
    independent = False
)
```

General anisotropic scaler preserving angles between box walls, perturbing it in a logarithmic manner. `spec` and
`independent` arguments have identical meanings as [class `linear`](#class-linear), but the way of perturbing heights is
different. Here, *Delta_h* is also sampled from [-*current_step*, *current_step*] interval, but instead of adding it
to the height, it is multiplied by a factor exp(*Delta_h*).

### Class `delta_triclinic`

```python
delta_triclinic(
    step,
    independent = False
)
```

Box updater which perturbs both lengths and direction of box vectors. If current step size is equal *current_step*, each
coordinate of a given box vector is perturbed by an independent random number uniformly sampled from
[-*current_step*, *current_step*] interval. `step` specifies the initial value of *current_step*. If `independent` is
`True`, only a single, randomly chosen box vector is perturbed in a single box move. Otherwise, all are perturbed at
once, however using independent random numbers.

### Class `disabled`

```python
disabled( )
```

Disables box scaling. As a consequence, box remains constant, thus NVT simulation is performed. `pressure` becomes
redundant and can be left out of the simulation environment.