# Operation modes

This reference page provides full information about operation modes available in the software.

[&larr; back to Reference](reference.md)


## Contents
* [Modes overview](#modes-overview)
* [`casino` mode](#casino-mode)
* [`preview` mode](#preview-mode)
* [`shape-preview` mode](#shape-preview-mode)
* [`trajectory` mode](#trajectory-mode)


## Modes overview

RAMPACK software operates as a single compiled binary. As it contains the code for both performing simulations and
analyzing the results, the functionality is gathered into a couple of operation modes (subcommands), in a similar
fashion to, for example, [git](https://git-scm.com/docs/git#_git_commands). The basic syntax is

```shell
rampack [mode] (mode-specific options)
```

The following modes are currently available:
* `rampack casino` - highly flexible Monte Carlo simulation of hard and soft particles performed based on the
  [input file](input-file.md)
* `rampack preview` - previews and metadata regarding the [input file](input-file.md)
* `rampack shape-preview` - previews and metadata regarding [shapes](shapes.md)
* `rampack trajectory` - analyzer of recorded simulation trajectories

Each mode has its specific options. The modes are described in details in next sections. There are 2 additional
modes

* `rampack help` (alternatively: `--help`, `-h`) - shows list of available modes
* `rampack version` (alternatively: `--version`, `-v`) - shows the version of the software

Moreover, built-in help for a specific mode can be shown by executing `rampack [mode] --help`.


## `casino` mode

The heart and soul of the RAMPACK software. This mode performs highly flexible Monte Carlo simulations of a system
of particles. The course of simulation is dictated by the [input file](input-file.md). Under this link, you can find all
information on how the simulations are performed and how to configure them. This section only describes command line
options. They can, for example, change output verbosity, redirect output to a file and skip or prolong some runs.

[//]: # (start casino)
[//]: # (This is automatically generated block, do not edit!!!)

* ***-h***, ***--help***

  prints help for this mode

* ***-i***, ***--input*** *arg*

  a PYON file with parameters. See https://github.com/PKua007/rampack/blob/main/docs/input-file.md for the documentation of the input file

* ***-V***, ***--verbosity*** *arg*

  how verbose the output should be. Allowed values, with increasing verbosity: `fatal`, `error`, `warn`, `info`, `verbose`, `debug`. Defaults to: `info` if `--log-file` not specified, otherwise to: `warn`

* ***-s***, ***--start-from*** *arg*

  when specified, the simulation will be started from the run with the name given. If not used in conjunction with `--continue` option, the packing will be restored from the RAMSNAP file of the preceding run. If `--continue` is used, the current run, but finished or aborted in the past, will be loaded instead and continued. There are also some special values. `.start` and `.end` correspond to, respectively, first and last run in the configuration file. When a special value `.auto` is specified, auto-detection of the starting run will be attempted based on RAMSNAP files (all runs in configuration have to output them). If last attempted run was unfinished, `--continue` option without an argument is implicitly added to finish it

* ***-c***, ***--continue*** *arg (= 0)*

  when specified, the thermalization of previously finished or aborted run will be continued for as many more cycles as specified. It can be used together with `--start-from` to specify which run should be continued. If the thermalization phase is already over, the averaging phase will be immediately started. If 0 is specified (or left blank, since 0 is the implicit value), total number of thermalization cycles from the input file will not be changed

* ***-l***, ***--log-file*** *arg*

  if specified, messages will be logged both on the standard output and to this file. Verbosity defaults then to: `warn` for standard output and to: `info` for log file, unless changed by `--verbosity` and/or `--log-file-verbosity` options

* ***--log-file-verbosity*** *arg*

  how verbose the output to the log file should be. Allowed values, with increasing verbosity: `fatal`, `error`, `warn`, `info`, `verbose`, `debug`. Defaults to: `info`

[//]: # (end casino)


## `preview` mode

This mode does basically two things. Firstly, it can export the initial configuration from the input file to all
supported [snapshot formats](output-formats.md#snapshot-formats) (`-o`, `--output` option). It is especially useful if
you are dealing with more complicated initial states with a complex pipeline of
[lattice transformations](initial-arrangement.md#lattice-transformers). Secondly, it can list the names of runs in the
input file (`-r`, `--run-names` option). Below is the full list of available options.

[//]: # (start preview)
[//]: # (This is automatically generated block, do not edit!!!)

* ***-h***, ***--help***

  prints help for this mode

* ***-i***, ***--input*** *arg*

  a PYON file with parameters. See https://github.com/PKua007/rampack/blob/main/docs/input-file.md for the documentation of the input file

* ***-V***, ***--verbosity*** *arg*

  how verbose the output should be. Allowed values, with increasing verbosity: `fatal`, `error`, `warn`, `info`, `verbose`, `debug`. Defaults to: `info`

* ***-o***, ***--output*** *arg*

  outputs the initial configuration loaded from the input file. Supported formats: `ramsnap`, `wolfram`, `xyz`. More than one format can be chosen by specifying this option multiple times, or in a single one using pipe `|`. It is advisable to put the argument in single quotes `' '` to escape special shell characters `"()|`

* ***-r***, ***--run-names***

  output run names from the input file to the standard output. Use with `-V warn` for a clean output

[//]: # (end preview)


## `shape-preview` mode

This mode is dedicated to inspecting the shapes. It can print shape metadata (`-l`, `--log-info` option) or output
shape preview to a supported [snapshot formats](output-formats.md#snapshot-formats) (`-o`, `--output` option). It is
especially useful when you are hand-crafting a shape using generic shape classes such as
[`polysphere`](shapes.md#class-polysphere), [`polyspherocylinder`](shapes.md#class-polyspherocylinder) or
[`generic_convex`](shapes.md#class-generic_convex). Below is the full list of available options.

[//]: # (start shape-preview)
[//]: # (This is automatically generated block, do not edit!!!)

* ***-h***, ***--help***

  prints help for this mode

* ***-i***, ***--input*** *arg*

  a PYON file with parameters of the shape; it can be used instead of manually creating the shape using -S

* ***-S***, ***--shape*** *arg*

  manually specified shape (instead of reading from input file using `-i`). It is advisable to put the argument in single quotes `' '` to escape special shell characters `"()|`

* ***-l***, ***--log-info***

  prints information about the shape

* ***-o***, ***--output*** *arg*

  stores preview of the shape in a format given as an argument: `wolfram`, `obj` (Wavefront OBJ); multiple formats may be passed using multiple `-o` options or separated by a pipe `|` in a single one. It is advisable to put the argument in `' '` to escape special shell characters `"()|`

[//]: # (end shape-preview)


## `trajectory` mode

This mode can perform various tasks on recorded trajectories, including outputting last snapshots, recalculating
observables or exporting the trajectory to another format. It also contains auto-fixing capability to restore damaged
trajectories (`-f`, `--auto-fix` option). Below is the full list of available options.

[//]: # (start trajectory)
[//]: # (This is automatically generated block, do not edit!!!)

* ***-h***, ***--help***

  prints help for this mode

* ***-i***, ***--input*** *arg*

  a PYON file with parameters. See https://github.com/PKua007/rampack/blob/main/docs/input-file.md for the documentation of the input file

* ***-r***, ***--run-name*** *arg*

  name of the run, for which the trajectory was generated. Special values `.first` and `.last` (for the first and the last run in the configuration file) are also accepted

* ***-f***, ***--auto-fix***

  tries to auto-fix the trajectory if it is broken; fixed trajectory can be stored back using `-t 'ramtrj("filename")'` (please note that `"filename"` must be different than for the source trajectory)

* ***-V***, ***--verbosity*** *arg*

  how verbose the output should be. Allowed values, with increasing verbosity: `fatal`, `error`, `warn`, `info`, `verbose`, `debug`. Defaults to: `info` if `--log-file` not specified, otherwise to: `warn`

* ***-o***, ***--output-obs*** *arg*

  calculates observables and outputs them to a given file. Observables can be specified using `-O` (`--observable`)

* ***-O***, ***--observable*** *arg*

  replays the simulation and calculates specified observables (format as in the input file). Observables can be passed using multiple options (`-O obs1 -O obs2`) or pipe-separated in a single one (`-O 'obs1|obs2'`). It is advisable to put the argument in single quotes `' '` to escape special shell characters `"()|`

* ***-b***, ***--output-bulk-obs*** *arg*

  calculates bulk observables and outputs them to the file with a name given by the specified pattern. In the pattern, every occurrence of `{}` is replaced with observable's signature name. When no occurances are found, `_{}.txt` is appended at the end. Bulk observables are specified using `-B` (`--bulk-observable`)

* ***-B***, ***--bulk-observable*** *arg*

  replays the simulation and calculates specified bulk observables (format as in the input file). Observables can be passed using multiple options (`-B obs1 -B obs2`) or pipe-separated in a single one (`-B 'obs1|obs2'`). It is advisable to put the argument in single quotes `' '` to escape special shell characters `"()|`

* ***-a***, ***--averaging-start*** *arg*

  specifies when the averaging starts. It is used for bulk observables

* ***-T***, ***--max-threads*** *arg*

  specifies maximal number of OpenMP threads that may be used to calculate observables. If 0 is passed, all available threads are used. (default: 1)

* ***-s***, ***--output-snapshot*** *arg*

  reads the last snapshot and outputs it in a given format: `ramsnap`, `wolfram`, `xyz`. More that one output format can be specified using multiple options (`-s out1 -s out2`) or pipe-separated in a single one (`-s 'out1|out2'`). It is advisable to put the argument in single quotes `' '` to escape special shell characters `"()|`

* ***-I***, ***--log-info***

  prints basic information about the recorded trajectory on a standard output

* ***-l***, ***--log-file*** *arg*

  if specified, messages will be logged both on the standard output and to this file. Verbosity defaults then to: `warn` for standard output and to: `info` for log file, unless changed by `--verbosity` and/or `--log-file-verbosity` options

* ***--log-file-verbosity*** *arg*

  how verbose the output to the log file should be. Allowed values, with increasing verbosity: `fatal`, `error`, `warn`, `info`, `verbose`, `debug`. Defaults to: `info`

* ***-t***, ***--output-trajectory*** *arg*

  stores the trajectory in a given format: `ramtrj`, `xyz`. More that one output format can be specified using multiple options (`-t out1 -t out2`) or pipe-separated in a single one (`-t 'out1|out2'`). It is advisable to put the argument in single quotes `' '` to escape special shell characters `"()|`

* ***-x***, ***--truncate*** *arg*

  truncates loaded trajectory to a given number of total cycles; truncated trajectory can be stored to a different RAMTRJ file using `-t 'ramtrj("filename")'`

[//]: # (end trajectory)


[&uarr; back to the top](#operation-modes)