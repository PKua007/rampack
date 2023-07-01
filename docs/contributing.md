# Contributing

This document gathers instructions for contributors.

[&larr; back to Reference](reference.md)


## Contents

* [General remarks](#general-remarks)
* [Branches](#branches)
* [Testing](#testing)
* [Code documentation](#code-documentation)
* [User documentation](#user-documentation)
  * [Class `example_class`](#class-example_class)
* [Updating the changelog](#updating-the-changelog) 


## General remarks

Yay! Thank you for your interest in improving the RAMPACK software!

If you want to report an error or propose a new feature,
you can create a [New issue](https://github.com/PKua007/rampack/issues). However, before doing so, please search
existing ones to avoid duplicates.

Code contributions are also highly appreciated! They can be both as small as fixing
typos in the documentation and as large as introducing new [run types](input-file.md#run-types) or
[operation modes](operation-modes.md). The contributions should be made via the
[Pull request](https://github.com/PKua007/rampack/pulls). Excluding trivial changes, it is advisable to create an issue
first in order to discuss with the community what is the best way of implementing it and to ensure it does not interfere
with the roadmap.

Please refer to [Source code](source-code.md) documentation page for coding guidelines and a walkthrough over the
project structure.


## Branches

The changes should be applied on the correct branch (adhering to semantic versioning):
* [`dev-patch`](https://github.com/PKua007/rampack/tree/dev-patch) - for bugfixes, corrected typos, etc.
* [`dev-minor`](https://github.com/PKua007/rampack/tree/dev-patch) - for new features

We generally do not accept interface-breaking changes. Please note that semantic versioning is applied to the
[input file](input-file.md) format and all PYON classes, but not the source code
([yet](https://github.com/PKua007/rampack/milestone/2)). Command-line arguments are kept mostly backwards compatible,
but backwards-incompatible changes (for example renamed options) can be made in some isolated cases.


## Testing

You should write tests for new functionality. There are two types of automated test
* **Unit tests** - short tests (milliseconds) sanitizing single classes, functions, small modules, etc. They reside in
  `<project root>/test/unit_tests` directory and have a runner `<build dir>/test/unit_tests`.
* **Validation tests** - longer tests (up to 10-20 seconds) on larger portions of code, for example performing short
  simulations and comparing the results with known values. They reside in `<project root>/test/valid_tests` directory and
  have a runner `<build dir>/test/valid_tests`.

All tests can be performed by running `ctest` in the build directory.

Test compilation is turned off by default. To enable is, pass `-DRAMPACK_BUILD_TESTS=ON` option when preparing the 
CMake build (see [advanced build options](installation.md#advanced-build-option)).

New features should also be tested manually in research-scale simulations to ensure they work in their go-to
environment.


## Code documentation

New classes should be documented using [Doxygen](https://www.doxygen.nl) comments. Code comments are generally a code
smell (clean code documents itself), but some technical remarks about the algorithms are sometimes needed.

Invoking

```shell
doxygen Doxyfile
```

in project root build the documentation in the `doxygen-doc` folder. The index is located at
`doxygen-doc/html/index.html`.


## User documentation

The public-facing API resides in the [input file](input-file.md) and all PYON classes that are used to build the
simulation pipeline. All new PYON classes and extensions of existing ones should be documented in the appropriate
[Reference](reference.md) sections. The styling should be concise with the rest of the documentation. The general
layout is as follows:

1. Class name as a `### section`
2. Version in which it was introduced
3. Constructor signature
4. Class description
5. List of arguments with their description
6. Additional comments regarding changes in given versions

The styling is as follows:

``````markdown
### Class `example_class`

> Since v1.1.0

```python
example_class(
    arg1,
    arg2 = "default value"
)
```

This is an example PYON class.

Arguments:

* ***arg1***

  This is the first argument.
  
* `since v1.2.0` ***arg2*** (*= "default value"*)

  This is the second argument with the default value. It was introduced in version 1.2.0.
  
> Since v1.2.0, some additional comments were added here, for example telling what was improved.
``````

Quite often, when the arguments are not too complicated, you can just document them in the class description and omit
the "Arguments" section. Please note that we point out in which version the new features or their updates were
introduced. As you cannot be sure in which version your changes will be applied, it is advised to use a placeholder
`vX.Y.Z`.

The above example will be rendered as follows:


### Class `example_class`

> Since v1.1.0

```python
example_class(
    arg1,
    arg2 = "default value"
)
```

This is an example PYON class.

Arguments:

* ***arg1***

  This is the first argument.

* `since v1.2.0` ***arg2*** (*= "default value"*)

  This is the second argument with the default value. It was introduced in version 1.2.0.

> Since v1.2.0, some additional comments were added here, for example telling what was improved.


## Commandline documentation

RAMPACK uses [cxxopts](https://github.com/jarro2783/cxxopts/) to handle commandline options. The framework
auto-generates the help, which is mirrored to [operation-modes.md](operation-modes.md) documentation page. The mirroring
is done by the Python script `scripts/regenerate_modes_help.py`. It should be used to regenerate the documentation after
every change in commandline options. Usage:

```
./regenerate_modes_help.py [rampack executable] [operation-modes.md path]
```

Of course `[rampack executable]` should be the one with updated options. If you want to add a new program mode, you
should add it to the list of modes `regenerate_modes_help.py` and mark the place in
[operation-modes.md](operation-modes.md) where the auto-generated help should be inserted. Assuming that the new mode is
called `foo`, the markers are:

```markdown
[//]: # (start foo)
Here the auto-generated help will be inserted.
[//]: # (end foo)
```


## Updating the changelog

You should reflect your changes in the changelog within the
[Unreleased](https://github.com/PKua007/rampack/blob/main/CHANGELOG.md#unreleased) section. You should only document
changes visible to the end user. Code refactoring is not visible, but a 20% performance increase due to optimizations
definitely is.


## Additional tools

The Python script `scripts/markdown_link_check.py` validates cross-links in the documentation directory. Usage:

```
./markdown_link_check.py [docs path]
```

It will report all dead links, if found. The script is included as a part of CI and is invoked by `ctest`.


[&uarr; back to the top](#contributing)