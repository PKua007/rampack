# Input file

## Contents

* [PYON format](#pyon-format)
* [Simulation pipeline](#simulation-pipeline)
* [Class `rampack`](#class-rampack)
* [Class `integration`](#class-integration)
* [Class `overlap relaxation`](#class-overlaprelaxation)

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

The input file contains a single rampack object with all necessary data passed as its arguments. Here is a valid,
exemplary input file used in the [Tutorial](tutorial.md)

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

## Simulation pipeline

The `casino` mode parses the input file and performs a set simulations based on its content. The following operations
are performed:

1. Initial arrangement is created (simulation box together with particles). It is controlled by `arrangement` argument.
   It can be creared procedurally or loaded from file (see below).
2. Initial *simulation environment* is created. It is described by the arguments `temperature`, `pressure`, `move_types`
   and `box_move_type`. One can specify any combination of those arguments (including none). Environment missing any of
   them is called *incomplete*.
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
      * if temperature/pressure is dynamic (depending on cycle number, see below) it starts from the beginning (from the
        value for cycle 0), even if it was not overriden by `temperature`/`pressure` run argument.
      * if `move_types`/`box_move_type` is not overriden from the previous run, it remembers dynamically adjusted step
        sizes from the prvious run. If it is overriden, previous step sizes are lost (as they can be meaningless for
        new move types)
   3. The rest is the same as for the first run
5. The previous step is repeated for all runs

## Class `rampack`

Synopsis:

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

...

## Class `integration`

Synopsis:

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

## Class `overlap_relaxation`

Synopsis:

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