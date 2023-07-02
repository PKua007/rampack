# Source code

This documentation page describes coding guidelines and sums up the most important code interfaces.

[&larr; back to Reference](reference.md)


## Coding guidelines

RAMPACK is written in C++17. At the moment, the source code does not follow any particular coding style (which may
change). General rules read as follows:

1.  Please try to mimic how the existing codebase is formatted.
2.  Use 4 spaces instead of tabs.
3.  The line length limit is 120 characters.
4.  Naming conventions:
    - classes: `PascalCase`
    - class methods: `camelCase`
    - functions: `snake_case`
    - function/method arguments: `camelCase`
    - class fields: `camelCase`
    - variables: `camelCase`
    - constants/enum values: `SNAKE_CASE_ALL_CAPS`
5.  Usually a single `*.h`/`*.cpp` files pair contains a single class with the same name. Sometimes helper types
    connected with this class (exception classes, enums) may reside in the same file.
6.  Block opening brackets `{` are usually in the same line as the loop/function/conditional etc. The exceptions are
    multiline function signatures - then `{` should always be in a separate line.
7.  Avoid too long functions - they should follow Single Responsibility Principle and can be broken into smaller parts
    if needed.
8.  Speaking of SRP, follow good coding practices - SOLID, DRY, etc.
9.  Avoid manual memory management (`new`, `malloc`, etc.) - use smart pointers. If possible, avoid heap memory
    altogether. Prefer, in this order:
    - automatic objects passed as (const) references
    - `std::unique_ptr`
    - `std::shared_ptr`
10. Prefer references to pointers - references are less likely to point to some gibberish. You want an optional value?
    Use `std::optional`.
11. Use [GSL](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)-like `Expects`, `Ensures` and `Assert`
    macros to express preconditions, postconditions and invariants. There are available in `src/utils/Exceptions.h`
    header.
12. Generally, write a code that you-in-the-six-months will understand. Other developers might then have a slight chance
    of understanding it as well ;-)
13. Write [unit/validation tests](contributing.md#testing) for your shiny new features.
14. Try to make your code in a general fashion. If you add a new shape resembling a star, make the number of rays a 
    parameter.


## Project structure

The project root contains the following files/directories:
- `.github/` - GitHub specific configurations (workflows, etc.)
- `artwork/` - logos
- `docs/` - documentation pages
- `sample_inputs/` - example input files
- `script/` - miscellaneous dev tools, tab completion scripts
- `src/` - source code
- `test/` - unit and validation tests and test infrastructure
- `CMakeLists.txt` - root CMake build configuration file
- `Doxyfile` - Doxygen configuration

The source code is divided into several directories/packages:
- `core/` - all classes doing the heavy-lifting
- `frontend/` - command line frontend code and definitions of PYON classes
- `geometry/` - math and geometry classes
- `pyon/` - PYON foundation framework
- `utils/` - various tools not fitting in other directories

The test code has the following directories:
- `matchers` - [Catch2](https://github.com/catchorg/Catch2) custom matchers
- `mocks` - [trompeloeil](https://github.com/rollbear/trompeloeil) mocks' declarations
- `unit_test` - unit tests with the directory structure mimicking the `src/` directory (JUnit-like)
- `validation_test` - validation tests


## Important interfaces




[&uarr; back to the top](#source-code)