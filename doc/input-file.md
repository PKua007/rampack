# Input file

This reference page describes the simulation pipeline and the format of the input file.

[&larr; back to Reference](reference.md)


## Contents

* [PYON format](#pyon-format)
* [Input file structure](#input-file-structure)
  * [Class `rampack`](#class-rampack)
  * [Simulation environment](#simulation-environment)
  * [Simulation pipeline](#simulation-pipeline)
* [Run types](#run-types)
  * [Class `integration`](#class-integration)
  * [Class `overlap_relaxation`](#class-overlap_relaxation)
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
  * [Class `delta_v`](#class-delta_v)
  * [Class `linear`](#class-linear-1)
  * [Class `log`](#class-log)
  * [Class `delta_triclinic`](#class-delta_triclinic)
  * [Class `disabled`](#class-disabled)


## PYON format

RAMPACK uses a custom-designed file format called PYON (PYthon Object Notation), which, as the name suggests, uses
Python syntax to represent the data. The main reason that led us to designing this custom format instead of using
existing ones (INI, JSON, YAML, etc.) was the object-oriented nature of the software. PYON input files look like Python
code which creates a `rampack` object, whose constructor arguments are further tree-like hierarchies of objects and
structures. This format closely mimics how the software works internally.

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

* Objects

  ```python
  # argument names: foo(arg1, arg2, arg3)
  foo(1, 2, 3)         
  foo(arg3=3, arg1=1, arg2=2)
  foo(1, arg3=3, arg2=2)
  ```
  
  Constructor arguments' semantics is closely modelled on Python callables. Normal arguments are positional, which means
  that they have to be passed in a specific order depending on the constructor's signature. However, when one uses the
  keyword syntax `name=value`, the order is arbitrary as long as all are specified. Positional and keyword styles may be
  mixed - the first few normal positional arguments may be given in a fixed order and the remaining ones in an arbitrary
  order using keyword style. When the object construction does not take any arguments, `()` may be omitted (which is an
  extension compared to Python's syntax).

  ```python
  foobar
  foobar()
  ```
  
  Object constructors support default values, variadic arguments, and keyword variadic arguments. Their signatures are 
  class-specific and are documented in this reference.

* Python `#`-style comments are supported:

  ```python
  [1, 2, 3]  # inline comment
  # line with only a comment
  "the # character inside a string is not interpreted as a comment start"
  ```

* Whitespace is ignored (which is also the case in Python code inside the brackets `()`, `[]`, `{}`)

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

The input file contains a single rampack object. The configuration of simulations is passed as its arguments. Here is an
exemplary input  file used in the [Tutorial](tutorial.md).

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

The rest of this reference document describes all aspects of the input file on a class-by-class basis.


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

It is the main class enclosing all simulation details and all runs. It creates the simulation box with initial positions
and orientations of particles and has its own [simulation environment](#simulation-environment), which is later combined
with first run's simulation environment.

Arguments:

* ***version***

  It specifies the minimal required version of RAMPACK the input file can run on. Supported formats:
  ```python
  version = "1"         # MAJOR (MINOR = 0, PATCH = 0)
  version = "1.0"       # MAJOR.MINOR (PATCH = 0)
  version = "1.0.0"     # MAJOR.MINOR.PATCH
  version = [1, 0, 0]   # [MAJOR, MINOR, PATCH]
  ```
  The rule of thumb is that it should be set to a current version (which can be checked using `rampack --version`) when
  creating the file and left unchanged. It is introduced as a safeguard to ensure that the input files produce
  replicable results and remain valid, even if breaking changes are introduced in a major update (input files are
  [versioned semantically](https://semver.org)), as well as to prevent running it with an older versions of RAMPACK
  where some required features may be lacking.

* ***arrangement*** <a id="rampack_arrangement"></a>
  
  Initial arrangement - it specifies the shape of a simulation box and positions and orientations of particles inside
  it. Available initial arrangements are described in [Arrangement classes](initial-arrangement.md#arrangement-classes).

* ***shape***

  It defines a shape of the particles and interaction between them. A detailed description of all shape classes can be
  found in [Shapes](shapes.md).

* ***seed***

  The integer being the seed of RNG. Simulations using the same domain specification (see
  [`domain_divisions`](#rampack_domaindivisions)), the same seeds and RAMPACK version should produce identical results.

* ***runs*** <a id="rampack_runs"></a>

  An array of runs that should be performed in a given order. For example

  ```python
  runs = [overlap_relaxation(...), integration(...)]
  ```
  
  first performs an overlap relaxation run, and then an integration run. Available run types are described in
  [Run types](#run-types) section.

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
  described in [Particle move types](#particle-move-types) section. It is a part of the
  [Simulation environment](#simulation-environment). If `None`, it is left unspecified (*incomplete simulation
  environment*).

* ***box_move_type*** (*= None*)

  The type of move applied to the simulation box. Available move types are described in
  [Box move types](#box-move-types) section. Notably, using [class `disabled`](#class-disabled) turns off box updating,
  which means the simulation is of NVT type (`pressure` is ignored and may be `None`). It is a part of the
  [Simulation environment](#simulation-environment). If `None`, it is left unspecified (*incomplete simulation
  environment*).

* ***walls*** (*= [False, False, False]*) <a id="rampack_walls"></a>

  An Array of Booleans that specifies which pairs of parallel box walls should be hard (non-penetrable by hard
  particles). `True` means that the wall is on, `False` otherwise. If the box vectors are denoted as **v**<sub>1</sub>,
  **v**<sub>2</sub> and **v**<sub>3</sub>, subsequent array indices correspond to box walls spanned by vectors:

  * index 0: **v**<sub>2</sub> and **v**<sub>3</sub>
  * index 1: **v**<sub>3</sub> and **v**<sub>1</sub>
  * index 2: **v**<sub>1</sub> and **v**<sub>2</sub>
  
  For an orthorhombic (cuboidal) box, it reduces to walls orthogonal to, respectively, x, y and z axis.

  **IMPORTANT**: toggling the wall on **DOES NOT** automatically disable box scaling in this direction. It has to be
  specified manually in `box_move_type` - [class `linear`](#class-linear-1) and [class `log`](#class-log) have
  appropriate options for this purpose.

* ***box_move_threads*** (*= 1*)

  If Integer, is specifies how many OpenMP threads should be used to perform box scaling moves. One can also pass a
  String `"max"`, to use all available OpenMP threads (number of processor threads by default, or a custom value
  specified by `OMP_NUM_THREADS` environment variable).

* ***domain_divisions*** (*= [1, 1, 1]*) <a id="rampack_domaindivisions"></a>

  An Array of integers specifying how many domains in each direction should be used to parallelize particle moves.
  The total number of domains (thus, the total number of OpenMP threads used) is a product of the elements. The box
  should be partitioned in such a way that domain dimensions are as large as possible. That way, the size of the ghost
  layers between the domains where the particles are frozen is reduced to a minimum. For example, it is better to
  partition a cubic box into 2 x 2 x 2 domains instead of 1 x 1 x 8. The minimal dimension of a domain in any direction
  should be at least 2 times the *total interaction range*, which is displayed at the start of the simulation in the
  [`casino` mode](operation-modes.md#casino-mode), or can be checked using
  [`shape-preview` mode](operation-modes.md#shape-preview-mode). When a domain becomes too narrow, RAMPACK automatically
  reduces the number of partitions.

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

These simulation environment arguments are present in [class `rampack`](#class-rampack) and in all
[Run types](#run-types). One can specify any combination of these arguments (including none of them). When at least of
them is `None`, the environment is called *incomplete*. Otherwise, it is *complete*. The only exception is when
`box_move_type = disabled`. Then, simulation is of NVT type and parameter `pressure` is ignored (environment can be
complete even if it is `None`). Simulation environments can be combined. When it is done, all not-`None` components of a
new environment overwrite the ones from the old environment, while for `None` components of the new environment the old
ones are reused.

### Simulation pipeline

The `casino` mode parses the input file and performs a set simulations based on its content. The following operations
are performed:

1. Initial arrangement is created (simulation box together with particles). It is controlled by
   [`rampack.arrangement`](#rampack_arrangement).

2. Initial *simulation environment* is created (based on appropriate [class `rampack`](#class-rampack) arguments).

3. The first run is performed (the first object in the array passed to [`rampack.runs`](#rampack_runs) argument). Run
   can be [`integration`](#class-integration) for a standard Monte Carlo sampling or
   [`overlap_relaxation`](#class-overlap_relaxation) for a step-by-step elimination of overlaps.

   For [`integration`](#class-integration) run:
   1. Run's simulation environment is prepared and combined with [`rampack`](#class-rampack) environment. The combined
      environment has to be *complete*.
   2. Run's **thermalization phase** is performed (its length is specified by
      [`thermalization_cycles`](#integration_thermalizationcycles)). During it, the following operations are performed:
      * step sizes of all move types are tuned to reach 0.1-0.2 move acceptance ratio, which was proven to be good
        for hard particles
      * if specified by [`record_trajectory`](#integration_recordtrajectory), one or more trajectory formats are saved
        to files on the fly
      * if specified by [`observables`](#integration_observables) and [`observables_out`](#integration_observablesout),
        observable snapshots are stored on the fly
   3. Run's **averaging phase** is performed:
      * step size adjustment is turned off (to preserve full detailed balance)
      * trajectory and observable snapshots continue to be recorded
      * if specified by [`averages_out`](#integration_averagesout), observable averages are gathered
      * if specified by [`bulk_observables`](#integration_bulkobservables) and
        [`bulk_observables_out_pattern`](#integration_bulkobservablesoutpattern), bulk observables are collected
   4. When the run is finished, observable averages, bulk observables and final snapshots
      ([`output_last_snapshot`](#integration_outputlastsnapshot)) are stored. 
   
   For [`overlap_relaxation`](#class-overlap_relaxation) run:
   1. Simulation environment is prepared and combined in the same way as for [`integration`](#class-integration) run.
   2. The run is performed:
      * moves reducing the number of overlaps are always accepted, and ones introducing new overlaps are always
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
      * if `move_types = None` or `box_move_type = None`, the move types are "inherited" from the previous run, and
        they remember dynamically adjusted step sizes. If `move_types` or `box_move_type` are overriden, previous step
        sizes are lost (as they can be meaningless for the new move types).
   4. The rest is the same as for the first run.
   
5. The previous step is repeated for all remaining runs in the Array.


## Run types

There are two types of runs supported by RAMPACK:
* [Class `integration`](#class-integration)
* [Class `overlap_relaxation`](#class-overlap_relaxation)


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

* ***run_name*** <a id="integration_runname"></a>

  String with user-specified, unique name of the run.

* ***thermalization_cycles*** <a id="integration_thermalizationcycles"></a>

  Integer representing the number of cycles that should be performed in the thermalization phase. Passing `None` results
  in skipping the thermalization phase.

* ***averaging_cycles*** <a id="integration_averagingcycles"></a>

  Integer representing the number of cycles that should be performed in the averaging phase. Passing `None` results in
  skipping the averaging phase.

* ***snapshot_every*** <a id="integration_snapshotevery"></a>

  Integer representing how often snapshots should be taken (for `record_trajectory` and`observables`) in both simulation
  phases. It should divide `thermalization_cycles` and `averaging_cycles` without remainder.

* ***temperature*** (*= None*) <a id="integration_temperature"></a>

  The temperature of the NpT/NVT simulation. It may be a constant Float or a [dynamic parameter](#dynamic-parameters).
  It is a part of the [Simulation environment](#simulation-environment). If `None`, the value from the previous run (or
  from [class `rampack`](#class-rampack) arguments) is reused.

* ***pressure*** (*= None*) <a id="integration_pressure"></a>

  The pressure of the NpT simulation. It may be a constant Float or a [dynamic parameter](#dynamic-parameters). It is a
  part of the [Simulation environment](#simulation-environment). If `None`, the value from the previous run (or from
  [class `rampack`](#class-rampack) arguments) is reused.

* ***move_types*** (*= None*) <a id="integration_movetypes"></a>

  The Array of Monte Carlo particle moves that should be performed during the simulation. Available move types are
  described [below](#particle-move-types). It is a part of the *simulation environment* (see
  [Simulation pipeline](#simulation-pipeline)). If `None`, the value from the  previous run (or from
  [class `rampack`](#class-rampack) arguments) is reused.

* ***box_move_type*** (*= None*) <a id="integration_boxmovetype"></a>

  The type of move applied to the simulation box. Available move types are described [below](#box-move-types). Notably,
  using [class `disabled`](#class-disabled) turns off box updating, which means the simulation is of NVT type
  (`pressure` is ignored and may be `None`). It is a part of the [Simulation environment](#simulation-environment). If
  `None`, the value from the previous run (or from [class `rampack`](#class-rampack) arguments) is reused.

* ***averaging_every*** (*= 0*) <a id="integration_averagingevery"></a>

  How often average value should be taken (for `averages_out` and `bulk_observables`) in the averaging phase. It should
  divide `averaging_cycles` without remainder. It can be equal 0 if the averaging phase is off
  (`averaging_cycles = None`).

* ***inline_info_every*** (*= 100*) <a id="integration_inlineinfoevery"></a>

  How often inline info should be printed to the standard output. This includes current cycle number and values of
  observables with an `inline` scope.

* ***orientation_fix_every*** (*= 10000*) <a id="integration_orientationfixevery"></a>

  How often rotation matrices should be renormalized. This is needed because they stockpile numerical errors after
  numerous matrix multiplications during the simulation. Unless you have a good reason to change it, the default value
  of `10000` should be left as it is.

* ***output_last_snapshot*** (*= []*) <a id="integration_outputlastsnapshot"></a>

  The array of formats in which the last snapshot should be stored after the simulation. For example

  ```python
  output_last_snapshot = [ramsnap("packing.ramsnap")]
  ```
  
  will store RAMSNAP representation to the file `packing.ramsnap`. See
  [Snapshot formats](output-formats.md#snapshot-formats) for more information.

* ***record_trajectory*** (*= []*) <a id="integration_recordtrajectory"></a>

  The array of formats in which the trajectory should be stored during the simulation. For example

  ```python
  record_trajectory = [ramtrj("trajectory.ramtrj")]
  ```
 
  will store RAMTRJ trajectory to the file `trajectory.ramtrj`. See
  [Trajectory formats](output-formats.md#trajectory-formats) for more information.

* ***averages_out*** (*= None*) <a id="integration_averagesout"></a>

  The name of file to which observable averages should be stored. They are gathered in the averaging phase for
  observables with [`averaging` scope](observables.md#normal-observables). If the file does not exist, it is created.
  Otherwise, a new row with averages is appended at the end. See
  [Observable averages](output-formats.md#observable-averages) for more information.

* ***observables*** (*= []*) <a id="integration_observables"></a>

  The Array of observables which should be computed during the simulation. Observables have 3 scopes: `snapshot`,
  `inline` and `averaging`. `snapshot` observable values are printed out to `observables_out` every `snapshot_every`
  cycles, `inline` observables' values are included in a simulation state line printed to the standard output, while
  `averaging` observables are averaged in the averaging phase and printed to `averages_out`. By default, observable is
  in all three scopes. To manually select the scope, one can use class `scoped`. For example

  ```python
  observables = [packing_fraction, scoped(nematic_order, inline=True)]
  ```
  
  means that [class `packing_fraction`](observables.md#class-packing_fraction) is in all scopes, while
  [class `nematic_order`](observables.md#class-nematic_order) only in the `inline` scope. See
  [Normal observables](observables.md#normal-observables) for more information and a full list of available observables.

* ***observables_out*** (*= None*) <a id="integration_observablesout"></a>

  String with a name of the file where values of `observables` in the `snapshot` scope snapshots will be stored every
  `snapshot_every` cycles.

* ***bulk_observables*** (*= []*) <a id="integration_bulkobservables"></a>

  The Array of bulk observables that will be computed in the averaging phase. Bulk observables are more complex than
  normal observables, thus they are stored in separate files specified by `bulk_observables_out_pattern`. They are
  gathered and averaged in the averaging phase every `averaging_every` cycles. See
  [Bulk observables](observables.md#bulk-observables) for more information and a full list of available bulk
  observables.

* ***bulk_observables_out_pattern*** (*= None*) <a id="integration_bulkobservablesoutpattern"></a>

  Name pattern for files to store bulk observables. If the pattern contains `{}`, it is replaced by the *short name* of 
  bulk observable it is going to store. For example, for `bulk_observables_out_pattern = "{}_out.txt"`, bulk observable
  named `rho` will be stored to a file named `rho_out.txt`. If `{}` is missing, `_{}.txt` is inserted at the end of the
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
output tied to it, such as observable averages and bulk observables). The number of overlaps is displayed in the inline
info on the standard output. Overlaps of all [interaction centers](shapes.md#interaction-centers) are counted
separately.

Arguments:

* ***run_name***

  See [`integration.run_name`](#integration_runname).

* ***snapshot_every***

  See [`integration.snapshot_every`](#integration_snapshotevery).

* ***temperature*** (*= None*)

  See [`integration.temperature`](#integration_temperature).

* ***pressure*** (*= None*)

  See [`integration.pressure`](#integration_pressure).

* ***move_types*** (*= None*)

  See [`integration.move_types`](#integration_movetypes).

* ***box_move_type*** (*= None*)

  See [`integration.box_move_type`](#integration_boxmovetype).

* ***inline_info_every*** (*= 100*)

  See [`integration.inline_info_every`](#integration_inlineinfoevery).

* ***orientation_fix_every*** (*= 10000*)

  See [`integration.orientation_fix_every`](#integration_orientationfixevery).

* ***helper_shape*** (*= None*) <a id="overlaprelaxation_helpershape"></a>

  Helper shape used to speed up overlap relaxation by introducing soft repulsion. Its interaction is imposed on top of
  the original shape's interaction (although the original shape and helper shape DO NOT cross-interact). Technically it
  can be any shape with an arbitrary interaction type, however, to achieve the goal, its interaction should be zero
  whenever the original shapes are not overlapping and be of repulsive soft-core type, when they are overlapping. A good
  choice is a sphere inscribed in the original shape with soft repulsion, such as
  [class `square_inverse_core`](shapes.md#class-square_inverse_core) or [class `wca`](shapes.md#class-wca).

* ***output_last_snapshot*** (*= []*)

  See [`integration.output_last_snapshot`](#integration_outputlastsnapshot).

* ***record_trajectory*** (*= []*)

  See [`integration.record_trajectory`](#integration_recordtrajectory).

* ***observables*** (*= []*)

  See [`integration.observables`](#integration_observables).

* ***observables_out*** (*= None*)

  See [`integration.observables_out`](#integration_observablesout).


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

*value* = `intercept` + `slope` &middot; *cycle_number*


### Class `exp`

```python
exp(
  a0,
  rate
)
```

It represents the parameter, which changes exponentially with the cycle number, according to the equation

*value* = `a0` &middot; exp(`rate` &middot; *cycle_number*)


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

`start` is an Integer being the cycle number at which this piece begins and `param` is any static or dynamic parameter
(excluding `piecewise`), that should be used starting from `start`-th cycle. The cycle number which is seen by `param`
is shifted so that it thinks that is starts from 0. The `start` of the first `piece` in `piecewise` object has to be 0,
and `start` arguments of the following ones have to be in ascending order. To give an example:

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
    rot_step = "auto",
    max_trans_step = None
)
```

Monte Carlo move performing translation and rotation at the same time. Random perturbations are chosen in the same way
as in [class `translation`](#class-translation) and [class `rotation`](#class-rotation), with
`trans_step` and `max_trans_step` having the same meaning as `step` and `max_step` arguments of
[class `translation`](#class-translation) and `rot_step` the same meaning as `step` argument of
[class `rotation`](#class-rotation). It should be noted that the step sizes of translation and rotation components are
adjusted at the same time and their ratio remains constant. If `rot_step = "auto"`, then it will be adjusted
automatically bases on `trans_step` and shape's interaction range.


### Class `flip`

```python
flip(
    every = 10
)
```

Monte Carlo move performing the flip, which is a half rotation around particle's [secondary axis](shapes.md#shape-axes)
attached to the [geometric center](shapes.md#geometric-center) (if only [primary axis](shapes.md#shape-axes) is present,
the flip is performed around an arbitrary axis orthogonal to the primary axis). `every` controls how often the flip is
performed. For example, its default value `10` means that in a full single MC cycle, the flip move will be attempted for
10% of all particles (and accepted according to the Metropolis criterion).


## Box move types

There are the following box move types:
* [Class `delta_v`](#class-delta_v)
* [Class `linear`](#class-linear-1)
* [Class `log`](#class-log)
* [Class `delta_triclinic`](#class-delta_triclinic)
* [Class `disabled`](#class-disabled)

Box move types are able to relax the system to a varying degree. `delta_v` type relaxes only the system pressure
(trace of the stress tensor) realizing the standard NpT ensemble. `linear` and `log` types may be anisotropic, which
additionally relaxes the diagonal part of stress tensor, which is required for structures with macroscopic periods.
`delta_triclinic` type relaxes the whole stress tensor, thus realizing the so-called isobaric-isotension ensemble and is
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
    scale_together = True
)
```

A general anisotropic box move preserving angles between box walls, perturbing it in a linear manner. More precisely, it
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

* ***scale_together*** (*= True*)
  
  Specifies if all independent scaling factors should be sampled in one move (`True`), or only one independent group at
  once (`False`). For example, for `spec = "x(yz)"`:

  * if `independent = False`, a single move is either scaling only x direction or scaling y and z at once (using a
    single *Delta_h* random number)
  * if `independent = True`, a single move is scaling all directions at once, but x is scaled using one *Delta_h* random
    number, while y and z using a different, independently sampled *Delta_h*


### Class `log`

```python
linear(
    spec,
    step,
    scale_together = True
)
```

A general anisotropic box moves preserving angles between box walls, perturbing it in a logarithmic manner. `spec` and
`scale_together` arguments have identical meaning as in [class `linear`](#class-linear), but the way of perturbing
heights is different. Here, *Delta_h* is also sampled from [-*current_step*, *current_step*] interval, but instead of
adding it to the height, the height is multiplied by a factor exp(*Delta_h*).


### Class `delta_triclinic`

```python
delta_triclinic(
    step,
    scale_together = True
)
```

Box move which perturbs both lengths and direction of box vectors. If the current step size is equal *current_step*,
each coordinate of a given box vector is perturbed by an independent random number uniformly sampled from
[-*current_step*, *current_step*] interval. `step` specifies the initial value of *current_step*. If `scale_together` is
`False`, only a single, randomly chosen box vector is perturbed in a single box move. Otherwise, all are perturbed at
once, however using independent random numbers.


### Class `disabled`

```python
disabled( )
```

Disables box scaling. As a consequence, box remains constant, thus NVT simulation is performed. `pressure` becomes
redundant and can be left out of the simulation environment (`pressure = None`).


[&uarr; back to the top](#input-file)