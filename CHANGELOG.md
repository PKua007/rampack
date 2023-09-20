# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).
The functionality is contained in a binary compiled from a source code.
* [Input file format](docs/input-file.md) adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
* [Command line options](docs/operation-modes.md) do not strictly follow semantic versioning, although breaking changes
  are not introduced unless necessary.
* The source code API will not be versioned semantically until the software is also distributed as a static-linked C++
library (later, with Python bindings as well).


## [Unreleased]

### Fixed

* Corrected [RAMSNAP](docs/output-formats.md#class-ramsnap) format documentation.
* Using `--continue` on a finished run without averaging phases no longer throws `PreconditionException`
  ([#68](https://github.com/PKua007/rampack/issues/68)).

### Changed

* Internal error reports now show a relative path of source location instead of an absolute one.
* Documentation directory `doc/` renamed to `docs/` (needed for GitHub to recognize
  [contributing.md](docs/contributing.md)).
* *Distance* definition is now generalized to *distance vector* in [binning types](docs/observables.md#binning-types).
* [Shape functions](docs/observables.md#shape-functions) can now be multivalued.
* [Class `axis`](docs/observables.md#class-axis) shape function can now also output all three components of the axis.

### Added

* Added global/local switch for [class `bond_order`](docs/observables.md#class-bond_order).
* Added [class `s220`](docs/observables.md#class-s220) and [class `s221`](docs/observables.md#class-s221) correlation functions.
* Added [class `linear`](docs/observables.md#class-linear) binning type.
* Added [class `q_tensor`](docs/observables.md#class-q_tensor) shape function.
* Added [class `bin_averaged_function`](docs/observables.md#class-bin_averaged_function) bulk observable.


## [1.1.0] - 2023-06-21

### Fixed

* Fixed typos in documentation and changelog.
* Fixed corrupted observables' output file when `--continue` is used.

### Added

* Added [v1] and [latest] tags.
* Added MinGW support.
* Added [class `randomize_rotation`](docs/initial-arrangement.md#class-randomize_rotation) lattice transformer.


## [1.0.0] - 2023-06-18

This is the initial stable release of the simulation software.


## Previous versions

During the development stage (0.x versions) no changelog was maintained. The changes can be inferred from git history.

[Unreleased]: https://github.com/PKua007/rampack/compare/latest..dev-minor
[latest]: https://github.com/PKua007/rampack/releases/tag/latest
[v1]: https://github.com/PKua007/rampack/releases/tag/v1
[1.1.0]: https://github.com/PKua007/rampack/releases/tag/v1.0.0
[1.0.0]: https://github.com/PKua007/rampack/releases/tag/v1.0.0


[&uarr; back to the top](#changelog)
