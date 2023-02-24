# Initial arrangement

This reference page describes all options for `arrangement` argument of [class `rampack`](input-file.md#class-rampack).

## Contents

* [Simulation box](#simulation-box)
* [Arrangement classes](#arrangement-classes)
    * [Class `presimulated`](#class-presimulated)
    * [Class `lattice`](#class-lattice)
* [Unit cell types](#unit-cell-types)
    * [Class `sc`](#class-sc)
    * [Class `bcc`](#class-bcc)
    * [Class `fcc`](#class-fcc)
    * [Class `hcp`](#class-hcp)
    * [Class `hex`](#class-hex)
    * [Class `custom`](#class)
* [Lattice transformers](#lattice-transformers)
    * [Class `optimize_layers`](#class-optimizelayers)
    * [Class `columnar`](#class-columnar)
    * [Class `randomize_flip`](#class-randomizeflip)
    * [Class `layer_rotate`](#class-layerrotate)
* [Lattice populators](#lattice-populators)
    * [Class `serial`](#class-serial)
    * [Class `random`](#class-random)

## Simulation box

RAMPACK support general triclinic boxes, which facilitates all standard symmetries of equilibrium phases of matter. The
box is described be three linearly independent vectors *v1*, *v2*, *v3* which span the simulation box. Each *absolute
position* *r* within it can be expressed as

*r* = *s1 \* v1* + *s2 \* v2* + *s3 \* v3*,

where *si* are from the range [0, 1) and they form the *relative position* *s*. The box is filled with identical
particles characterized by their positions and orientation. Periodic boundary conditions are used. Box's walls can also
be toggled on, which imposes a barrier on hard particles. During the simulation box may be deformed using different
types of [box moves](input-file.md#box-move-types).

## Arrangement classes

Currently, initial arrangement may be prepared using:

* [class `presimulated`](#class-presimulated) - loads the configuration from RAMSNAP file
* [class `lattice`](#class-presimulated) - creates a Bravais lattice with optional transformations

### Class `presimulated`

```python
presimulated(
    file
)
```

Loads the initial configuration from RAMSNAP file with name `file`.

### Class `lattice`

```python
lattice(
    cell,
    **kwargs
)
```

Creates the Bravais lattice of particles, which can then be transformed using
[Lattice transformers](#lattice-transformers). The argument `cell` specifies what type of the unit cell it is built of.
It is important to note that `cell` type specifies only the number of particles in the unit cell and their relative
positions in the cell. The shape of the cell is then specified manually or calculated automatically, depending on
`lattice` arguments. `**kwargs` is a set of keyword arguments which depend on a given call signature. There are 3
available call signatures

1. ```python
   lattice(
       cell,
       cell_dim,
       n_cells,
       transformations,  # optional argument
       fill_partially    # optional argument
   )
   ```
   
   Creates the lattice with manually specified number of cells in each direction and cell dimensions. Simulation box
   dimensions are calculated based on mentioned arguments.

   Arguments:

   * ***cell***
     
     The type of the unit cell (see [Unit cell types](#unit-cell-types)).

   * ***cell_dim*** <a id="celldim"></a>

     Unit cell dimensions. It can be:
     * Float, which specifies a side length of a cubic cell
       ```python
       cell_dim = 10  # creates 10 x 10 x 10 cubic cell
       ```
     * Array of Floats, which specifies 3 side lenghts of an orthorhombic (cuboidal) cell
       ```python
       cell_dim = [5, 10, 15]  # creates 5 x 10 x 15 cuboidal cell
       ```
     * Array of Arrays of Floats, which gives explicit 3 unit cell vectors of a most general triclinic cell.
       ```python
       cell_dim = [[2, 0, 0], [0, 2, 0], [1, 0, 2]]   # creates a monoclinic cell leaning in x direction
       ```
   * ***n_cells*** <a id="ncells"></a>
   
     Array of Integers specifying number of cells in each direction. For example
     ```python
        n_cells = [5, 5, 10]
     ```
     creates 5 x 5 x 10 lattice of cells (together 250 cells).

   * ***transformations*** (optional argument)
     
     An array of [lattice transformations](#lattice-transformers) which should be applied after creating the lattice.

   * ***fill_partially*** (optional argument)

     If specified, not all available slots as specified by the unit cell and the lattice will be filled with particles.
     The way they are filled is specified by [Lattice populators](#lattice-populators).

2. ```python
   lattice(
       cell,
       box_dim,
       n_shapes,
       transformations  # optional argument
   )
   ```
   
   Creates the lattice in a simulation box of fixed `box_dim` dimensions with `n_shapes` in unit cells of type `cell`.
   The number of cells in each direction are chosen automatically in such a way that the height of cell box are as close as
   possible.

   Arguments:
   
   * ***cell***

     The type of the unit cell (see [Unit cell types](#unit-cell-types)).
   
   * ***box_dim***

     Dimensions of the simulation box. They are specified in the same way as [`cell_dim` argument](#celldim) of the
     first call signature.

   * ***n_shapes***
   
     Number of shapes to be distributed in the lattice. Based on this number, number of cells and cell dimensions will
     be optimized.

   * ***transformations*** (optional argument)

     An array of [lattice transformations](#lattice-transformers) which should be applied after creating the lattice.
   
3. ```python
   lattice(
       cell,
       n_cells,
       box_dim,
       transformations,  # optional argument
       fill_partially    # optional argument
   )
   ```
   
   Creates the lattice with a simulation boxed with fixed dimensions and with a fixed number of cells in each direction.
   Cell dimensions are calculated based on those two arguments.

   Arguments:

   * ***cell***

     The type of the unit cell (see [Unit cell types](#unit-cell-types)).
   
   * ***n_cells***

     Array of Integers specifying number of cells in each direction. See [`n_cells` argument](#ncells) of first call
     signature.

   * ***box_dim***

     Dimensions of the simulation box. They are specified in the same way as [`cell_dim` argument](#celldim) of the
     first call signature.

   * ***transformations*** (optional argument)

     An array of [lattice transformations](#lattice-transformers) which should be applied after creating the lattice.

   * ***fill_partially***
   
     If specified, not all available slots as specified by the unit cell and the lattice will be filled with particles.
     The way they are filled is specified by [Lattice populators](#lattice-populators).

## Unit cell types

### Class `sc`

```python
sc ( )
```

### Class `bcc`

```python
bcc ( )
```

### Class `fcc`

```python
fcc ( )
```

### Class `hcp`

```python
hcp (
    axis
)
```

### Class `hex`

```python
hex (
    axis
)
```

### Class `custom`

```python
custom (
    shapes
)
```

## Lattice transformers

### Class `optimize_layers`

```python
optimize_layers (
    spacing,
    axis
)
```

### Class `columnar`

```python
columnar (
    axis,
    seed
)
```

### Class `randomize_flip`

```python
randomize_flip (
    seed
)
```

### Class `layer_rotate`

```python
randomize_flip (
    layer_axis,
    rot_axis,
    rot_angle,
    alternating
)
```

## Lattice populators

### Class `serial`

```python
serial(
    n_shapes,
    axis_order = "auto"
)
```

### Class `random`

```python
random(
    n_shapes,
    seed
)
```