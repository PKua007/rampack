# Operation modes

This reference page provides full information about operation modes available in the software.


## Contents
* [Modes overview](#modes-overview)
* [`casino` mode](#casino-mode)
* [`preview` mode](#preview-mode)
* [`shape-preview` mode](#shape-preview-mode)
* [`trajectory` mode](#trajectory-mode)


## Modes overview

RAMPACK software operates as a single compiled binary. As it contains the code for both performing simulations and
analyzing results, the functionality is gathered into a couple of operation modes (subcommands), in a similar fashion to
for example [git](https://git-scm.com/docs/git#_git_commands). The basic syntax is

```shell
rampack [mode] (mode-specific options)
```

The following modes are currently available:
* `rampack casino` - highly configurable Monte Carlo simulation of hard and soft particles performed based on the
  [input file](input-file.md)
* `rampack preview` - previews and metadata regarding the [input file](input-file.md)
* `rampack shape-preview` - previews and metadata regarding [shapes](shapes.md)
* `rampack trajectory` - analyzer of recorded simulation trajectories

Each mode has its specific options. The modes are describe in details in next sections. There are 2 additional
modes

* `rampack help` (alternatively: `--help`, `-h`) - shows list of available modes
* `rampack version` (alternatively: `--version`, `-v`) - shows the version of the software

Moreover, built-in help for a specific mode can be shown by executing `rampack [mode] --help`.


## `casino` mode

The heart and soul of the RAMPACK software. This mode provides highly configurable Monte Carlo simulation of a system
of particles. The course of simulation is dictated by the [input file](input-file.md). Under this link you can find all
information on how the simulations are performed and how to configure them. This section only describes command line
options. They can for example change output verbosity, redirect output to file and skip or prolong some runs.

[//]: # (start casino)
[//]: # (This is automatically generated block, do not edit!!!)
[//]: # (end casino)


## `preview` mode

This mode does basically two things. Firstly, it can export the initial configuration from the input file to all
supported snapshot formats (`-o`, `--output` option). It is especially useful if you are dealing with more complicated
initial states with a complex pipeline of [lattice transformations](initial-arrangement.md#lattice-transformers).
Secondly, it can list the names of runs in the input file (`-r`, `--run-names` option). Below is the full list of
available options.

[//]: # (start preview)
[//]: # (This is automatically generated block, do not edit!!!)
[//]: # (end preview)


## `shape-preview` mode

This mode is dedicated for inspecting the shapes. It can print shape metadata (`-l`, `--log-info` option) or output
shape preview to a supported format (`-o`, `--output` option). It is especially useful when you are hand-crafting a
shape using generic shape classes such as [`polysphere`](shapes.md#class-polysphere),
[`polyspherocylinder`](shapes.md#class-polyspherocylinder) or [`generic_convex`](shapes.md#class-genericconvex). Below
is the full list of available options.

[//]: # (start shape-preview)
[//]: # (This is automatically generated block, do not edit!!!)
[//]: # (end shape-preview)


## `trajectory` mode

The mode can perform various tasks on recorded trajectories, including outputting last snapshots, recalculating
observables or exporting the trajectory to other format. It also contains auto-fixing capability to restore damaged
trajectories (`-f`, `--auto-fix` option). Below is the full list of available options.

[//]: # (start trajectory)
[//]: # (This is automatically generated block, do not edit!!!)
[//]: # (end trajectory)