# Initial arrangement

This reference page describes all options for `arrangement` argument of [class `rampack`](input-file.md#class-rampack).

[&larr; back to Reference](reference.md)


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
    * [Class `custom`](#class-custom)
* [Lattice transformers](#lattice-transformers)
    * [Class `optimize_cell`](#class-optimize_cell)
    * [Class `optimize_layers`](#class-optimize_layers)
    * [Class `columnar`](#class-columnar)
    * [Class `randomize_flip`](#class-randomize_flip)
    * [Class `layer_rotate`](#class-layer_rotate)
    * [Class `randomize_rotation`](#class-randomize_rotation)
* [Lattice populators](#lattice-populators)
    * [Class `serial`](#class-serial)
    * [Class `random`](#class-random)


## Simulation box

RAMPACK supports general triclinic boxes, which encapsulate all standard symmetries of equilibrium phases of matter. The
box is described be three linearly independent vectors **v**<sub>1</sub>, **v**<sub>2</sub>, **v**<sub>3</sub> which
span the simulation box. Each *absolute position* **r** within it can be expressed as

**r** = *s*<sub>1</sub> &middot; **v**<sub>1</sub> + *s*<sub>2</sub> &middot; **v**<sub>2</sub> + *s*<sub>3</sub> &middot; **v**<sub>3</sub>,

where *s<sub>i</sub>* are in the range [0, 1) and they form the *relative position* **s**. The box is filled with
identical particles characterized by their positions and orientation. Periodic boundary conditions are used. One or more
pairs of parallel box walls, periodic be default, can be toggled impenetrable, which imposes a barrier on hard
particles (see [`rampack.walls`](input-file.md#rampack_walls)). During the simulation, box may be deformed using 
different types of [box moves](input-file.md#box-move-types).


## Arrangement classes

Currently, initial arrangement may be prepared using:

* [class `presimulated`](#class-presimulated) - loads the configuration from the RAMSNAP file
* [class `lattice`](#class-presimulated) - creates a Bravais lattice with optional transformations


### Class `presimulated`

```python
presimulated(
    file
)
```

Loads the initial configuration from RAMSNAP with name given by a String `file`.


### Class `lattice`

```python
lattice(
    cell,
    **kwargs
)
```

Creates the Bravais lattice of particles, which can then be transformed using
[lattice transformers](#lattice-transformers). The argument `cell` specifies what [type](#unit-cell-types) of the unit
cell it is built of. It is important to note that `cell` type specifies only the number of particles in the unit cell,
their orientations and relative positions in the cell. The shape of the cell is then specified manually or calculated
automatically, depending on `lattice` arguments. `**kwargs` is a set of keyword arguments which depend on a given call
signature. There are 3 available call signatures

1. call signature:
   ```python
   lattice(
       cell,
       cell_dim,
       n_cells,
       transformations,  # optional argument
       fill_partially    # optional argument
   )
   ```
   
   Creates the lattice with a manually specified number of cells in each direction and cell dimensions. Simulation box
   dimensions are calculated based on these arguments.

   Arguments:

   * ***cell***
     
     The type of the unit cell (see [Unit cell types](#unit-cell-types)).

   * ***cell_dim*** <a id="celldim"></a>

     Unit cell dimensions. It can be:
     * Float, which specifies a distance between the nearest neighbors.
       ```python
       #cell = sc
       cell_dim = 1  # creates 1 x 1 x 1 simple cubic cell
       
       #cell = hcp(axis="z")
       cell_dim = 1  # creates 1 x sqrt(3) x 2*sqrt(6)/3 hexagonal close packed cell
       ```
       Please note that it does not necessarily describe a cubic cell - the relative side dimensions depend on the
       particular cell type and are chosen in such a way that all nearest neighbours are equidistant.
     * Array of Floats, which specifies 3 side lengths of an orthorhombic (cuboidal) cell.
       ```python
       cell_dim = [0.5, 1, 1.5]  # creates a 0.5 x 1 x 1.5 cuboidal cell
       ```
     * Array of 3 explicitly specified unit cell vectors (each is an Arrays of 3 Floats) of a most general triclinic
       cell.
       ```python
       cell_dim = [[2, 0, 0], [0, 2, 0], [1, 0, 2]]   # creates a monoclinic cell leaning in x direction
       ```
       
   * ***n_cells*** <a id="ncells"></a>
   
     Array of Integers specifying the number of cells in each direction. For example
     ```python
     n_cells = [5, 5, 10]
     ```
     creates a 5 x 5 x 10 lattice of cells (together 250 cells).

   * ***transformations*** (optional argument)
     
     An Array of [lattice transformations](#lattice-transformers) which should be applied after creating the lattice.

   * ***fill_partially*** (optional argument) <a id="fill-partially"></a>

     If specified, not all available particle slots in the lattice will be filled. The way they are filled is specified
     by a [lattice populator](#lattice-populators).

2. call signature:
   
   ```python
   lattice(
       cell,
       box_dim,
       n_shapes,
       transformations  # optional argument
   )
   ```
   
   Creates the lattice in a simulation box of fixed `box_dim` dimensions with `n_shapes` in unit cells of type `cell`.
   The number of cells in each direction is chosen automatically in such a way that the heights of cell box are as
   similar as possible.

   Arguments:
   
   * ***cell***

     The type of the unit cell (see [Unit cell types](#unit-cell-types)).
   
   * ***box_dim*** <a id="boxdim"></a>

     Dimensions of the simulation box. It can be:
       * Float, which specifies a side length of a cubic box.
         ```python
         box_dim = 10  # creates a 10 x 10 x 10 box
         ```
       * Array of Floats, which specifies 3 side lenghts of an orthorhombic (cuboidal) cell.
         ```python
         box_dim = [5, 10, 15]  # creates a 5 x 10 x 15 orthorhombic (cuboidal) box
         ```
       * Array of 3 explicitly specified box vectors (each is an Arrays of 3 Floats) of a most general triclinic box.
         ```python
         box_dim = [[20, 0, 0], [0, 20, 0], [10, 0, 20]]   # creates a monoclinic box leaning in x direction
         ```

   * ***n_shapes***
   
     Number of shapes to be distributed in the lattice. Based on this number, number of cells and cell dimensions will
     be optimized.

   * ***transformations*** (optional argument)

     An Array of [lattice transformations](#lattice-transformers) which should be applied after creating the lattice.
   
3. call signature
   
   ```python
   lattice(
       cell,
       n_cells,
       box_dim,
       transformations,  # optional argument
       fill_partially    # optional argument
   )
   ```
   
   Creates the lattice where the simulation box has fixed dimensions and a fixed number of cells in each direction.
   Cell dimensions are calculated based on these two arguments.

   Arguments:

   * ***cell***

     The type of the unit cell (see [Unit cell types](#unit-cell-types)).
   
   * ***n_cells***

     Array of Integers specifying number of cells in each direction. See [`n_cells` argument](#ncells) of first call
     signature.

   * ***box_dim***

     Dimensions of the simulation box. They are specified in the same way as [`box_dim` argument](#boxdim) of the
     second call signature.

   * ***transformations*** (optional argument)

     An array of [lattice transformations](#lattice-transformers) which should be applied after creating the lattice.

   * ***fill_partially***

   If specified, not all available particle slots in the lattice will be filled. The way they are filled is specified
   by [Lattice populators](#lattice-populators).


## Unit cell types

Unit cell is defined as a set of particles with given positions and orientations in the cell. Unit cell type does not
define the shape of the cell - it is determined by [class `lattice`](#class-lattice) arguments. For example,
[bcc unit cell](#class-bcc) may as well be triclinic. Thus, positions are defined in a cell-shape agnostic manner - by
relative coordinates *s<sub>i</sub>*. See [Simulation box](#simulation-box) section for more information about the
relative coordinates.

There are 5 types of build-in unit cell types
* [class `sc`](#class-sc)
* [class `bcc`](#class-bcc)
* [class `fcc`](#class-fcc)
* [class `hcp`](#class-hcp)
* [class `hex`](#class-hex)

and a user-defined unit cell:
* [class `custom`](#class-custom)

All positions given below in cell classes' descriptions are relative. In the standard convention, known from the
textbooks, one particle is usually in the corner. Here, all particles in the cell are translated in such a way that the
bounding box of the positions lies dead in the middle of the cell.


### Class `sc`

```python
sc( )
```

Creates a *simple cubic* cell with a single particle placed at (relative coordinates):
* (1/2, 1/2, 1/2)


### Class `bcc`

```python
bcc( )
```

Creates a *body centered cubic* cell with 2 particles placed at (relative coordinates):
* (1/4, 1/4, 1/4)
* (3/4, 3/4, 3/4)


### Class `fcc`

```python
fcc( )
```

Creates a *face centered cubic* cell with 4 particles placed at (relative coordinates):
* (1/4, 1/4, 1/4)
* (1/4, 3/4, 3/4)
* (3/4, 1/4, 3/4)
* (3/4, 3/4, 1/4)


### Class `hcp`

```python
hcp(
    axis
)
```

Creates a *hexagonal close packed* cell with 4 particles. Positions depend on `axis` (`"x"`, `"y"` or `"z"`), which
determines the direction of stacking of honeycomb layers. For `axis = "z"`, the positions are (relative coordinates):
* (1/4, 1/12, 1/4)
* (3/4, 7/12, 1/4)
* (1/4, 5/12, 3/4)
* (3/4, 11/12, 3/4)

For `axis = "y"` the coordinates are cyclically shifted left one time, while for `axis = "x"` - two times.


### Class `hex`

```python
hex(
    axis
)
```

Creates a *hexagonal* cell with 2 particles. It forms hexagonal honeycombs placed on top of one another without shifts
as in [class `fcc`](#class-fcc) or [class `hcp`](#class-hcp). Positions depend on `axis` (`"x"`, `"y"` or `"z"`), which
determines the direction of stacking. For `axis = "z"`, the positions are (relative coordinates):
* (1/4, 1/4, 1/2)
* (3/4, 3/4, 1/2)

For `axis = "y"` the coordinates are cyclically shifted one time left, while for `axis = "x"` - two times left.


### Class `custom`

```python
custom(
    shapes
)
```

Lets the user specify a custom unit cell with arbitrary number of particles, their positions and orientations. `shapes`
is an Array of `shape` objects:

```python
shape(
    pos,
    rot = [0, 0, 0]
)
```

`pos` is position in relative coordinates (Array of 3 Floats). `rot` is an array of Tait-Bryan angles ("aircraft" Euler
angles) of counter-clockwise rotations around, respectively, x, y and z axes, performed in this exact order. In a
rotation matrix language, if rotation matrices are denoted as **R**<sub>*x*</sub>, **R**<sub>*y*</sub> and
**R**<sub>*z*</sub>, the net rotation is

**R** = **R**<sub>*z*</sub> **R**<sub>*y*</sub>  **R**<sub>*x*</sub>

Rotation angles are in **degrees**. As an example, to recreate [class `bcc`](#class-bcc), but with the central particle
rotated 90 degrees around y-axis, one could specify

```python
custom(shapes=[
    shape([0.25, 0.25, 0.25]),
    shape([0.75, 0.75, 0.75], [0, 90, 0])
])
```

Notably, relative coordinates do not necessarily have to be in [0, 1) range, although they should as a rule of thumb.
Otherwise, the particles creep out of their corresponding cells, but anyway, RAMPACK correctly wraps them in the box
according to periodic boundary conditions.


## Lattice transformers

Lattice transformations are optional operations, which are performed on a regular lattice. They may be used to create
more complicated structures (for example layers with alternating tilt) without manually defining the unit cell.
Operations can be pipelined, which gives a lot of flexibility. In the documentation of classes the following types of 
lattice appear (which are **NOT** mutually exclusive):

* **regular lattice** <br />
  Bravais lattice fully defined by periodically repeated single unit cell.
  Using [class `lattice`](#class-lattice) without `transformers` produces the regular lattice. On the other hand,
  **irregular lattice** is also build of cells, but they all can be different. Irregular lattice is produced by selected
  transformers, for example [class `columnar`](#class-columnar) or [class `randomize_flip`](#class-randomize_flip).
* **normalized lattice** <br />
  Lattice where coordinates of all relative positions in all cells are in the range [0, 1).

Each lattice transformer denotes the required lattice type to work properly and specifies what is the lattice type after
the transformation if done.

The following transformers are available:
* [class `optimize_cell`](#class-optimize_cell)
* [class `optimize_layers`](#class-optimize_layers)
* [class `columnar`](#class-columnar)
* [class `randomize_flip`](#class-randomize_flip)
* [class `layer_rotate`](#class-layer_rotate)


### Class `optimize_cell`

```python
optimize_cell(
    spacing,
    axis_order = "xyz"
)
```

* **Lattice requirements**: regular
* **Resulting lattice**: regular, normalized if was before

Optimizes unit cell dimensions to pack particles more efficiently. It works axis by axis, in the order specified by
`axis_order`. For a given axis, it shrinks the cell in this direction, preserving angles between cell walls, up until
the particles are tangent. After all 3 axes are optimized, it expands the cell, again preserving angles, so that the
heights of the compressed cell parallelepiped are increased by `spacing`.

It is important to note that relative positions of particles withing the cell remain constant. As a result, in case
of multi-particle cells, there might be gaps between some pairs of particles after the operation. This happens when a
particula pair of particles is already tangent and further shrinking would introduce an overlap, while some other
particle pairs are still disjunctive. For a more flexible cell optimization one can use
[class `optimize_layers`](#class-optimize_layers).


### Class `optimize_layers`

```python
optimize_layers(
    spacing,
    axis
)
```

* **Lattice requirements**: regular, normalized
* **Resulting lattice**: regular, normalized

Performs intelligent, layer-wise optimization of the unit cell and cell dimensions. Contrary to
[class `optimize_cell`](#class-optimize_cell), it optimizes the cell layer by layer, minimizing the gaps in
multi-particle cells. The drawback is that is performs the optimization only in one direction, specified by `axis`
(`"x"`, `"y"`, `"z"`). The axis is defined in box relative coordinates and may not coincide with coordinate system axes.

First, it identifies the layers. The layer is a group of particles in the unit cell, whose relative coordinate
corresponding to `axis` is identical. For example, for [class `fcc`](#class-fcc) cell and `axis = "z"`, one layer
contains particles positioned at (1/4, 1/4, **1/4**), (3/4, 3/4, **1/4**), while the second one particles at
(1/4, 3/4, **3/4**) and (3/4, 1/4, **3/4**) (notice identical z coordinates in both groups). After identifying layers,
it compresses them in the `axis` stacking direction, until they are tangent. Finally, it increases the cell height in
the `axis` direction, to introduce a gap `spacing` between all identified layers.


### Class `columnar`

```python
columnar(
    axis,
    seed
)
```

* **Lattice requirements**: regular, normalized
* **Resulting lattice**: irregular, normalized

Takes a regular lattice and randomly shifts particle columns to form a columnar phase along the axis specified by `axis`
(`"x"`, `"y"`, `"z"`). The axis is defined in box relative coordinates and may not coincide with coordinate system axes.
Columns are recognized a way analogous to how [class `optimize_layers`](#class-optimize_layers) recognizes layers:
the column is a set of particles in the whole lattice, whose relative coordinates differ only on a coordinate
corresponding to `axis`. After the columns are recognized, each one is moved by a random amount along `axis` using RNG
seeded with Integer `seed`.

After "columnarization", the lattice remains normalized (RAMPACK periodically wraps the columns sticking out of their
bounds), however it is no longer regular.


### Class `randomize_flip`

```python
randomize_flip(
    seed
)
```

* **Lattice requirements**: none
* **Resulting lattice**: irregular, normalized if was before

With 1/2 probability, it rotates each particle by 180&deg; around its [secondary axis](shapes.md#shape-axes), attached
at the [geometric center](shapes.md#geometric-center), flipping in turn around half of the particles in the system. It
uses RNG seeded with Integer `seed`. If lattice was normalized prior to using this transformer, it is renormalized again
(normalization may be lost when the geometric center of the particle is not {0, 0, 0}, which is never the case for
[built-in shapes](shapes.md#specific-shape-classes)).


### Class `layer_rotate`

```python
layer_rotate(
    layer_axis,
    rot_axis,
    rot_angle,
    alternating
)
```

* **Lattice requirements**: regular, normalized
* **Resulting lattice**: regular, normalized

It recognizes the layers in the same way as [class `optimize_layers`](#class-optimize_layers) and rotates the particles
in them.

Arguments:

* ***layer_axis***

  `"x"`, `"y"` or `"z"` String representing the axis orthogonal to layers; the axis is defined in box relative
  coordinates and may not coincide with coordinate system axes.

* ***rot_axis***

  `"x"`, `"y"` or `"z"` String representing the coordinate system axis around which the rotation should be performed.

* ***rot_angle***

  The angle of rotation in degrees.

* ***alternating***

  If `False`, all particles in the system will be rotated in the same direction. If `True`, particles in even layers
  will be rotated counterclockwise, and particle in odd layers clockwise. 
  

### Class `randomize_rotation`

> Since v1.1.0

```python
randomize_rotation(
    seed,
    axis = "random",
)
```

* **Lattice requirements**: none
* **Resulting lattice**: irregular, normalized if was before

Randomizes rotations of particles. Rotations may be around a specific or random axis depending on the `axis` parameter.

* ***seed***

  Seed of the RNG used to randomize rotations.

* ***axis*** (*= "random"*)

  Axis of the rotation. The following values are accepted:
  * ***"random"*** <br />
    All shapes are rotated by a random angle around a random axis with a uniform probability of all rotations.
  * Array of Floats (eg. `[1, 1, 0]`) <br />
    Specific constant (lab) axis. Axis normalization is performed automatically. All shapes are rotated around the same
    axis by random angles.
  * ***"x"***, ***"y"***, ***"z"*** <br />
    Shorthands for x, y and z lab axes. All shapes are rotated around the same axis by random angles.
  * ***"primary"***, ***"secondary"***, ***"auxiliary"*** <br />
    Rotation around a [shape axis](shapes.md#shape-axes) by a random angle. Rotation axis is not constant - shape axes
    are defined in shape's coordinate system, thus the axis of rotation depends on the orientation of a shape.


## Lattice populators

Lattice populators determine the way the particles are skipped when using [`fill_partially` argument](#fill-partially)
in [class `lattice`](#class-lattice). The following ones are available:

* [class `serial`](#class-serial)
* [class `random`](#class-random)


### Class `serial`

```python
serial(
    n_shapes,
    axis_order = "auto"
)
```

Fills the vacant spots in the lattice cell by cell, where cells are first grouped in rows, then rows in layers, and
finally the layers are stacked on top of one another. The operation is interrupted when `n_shapes` shapes is already
placed, leaving the rest of spaces empty.

Arguments:

* ***n_shapes***

  Determines how many shapes have to be placed inside the lattice. It has to be smaller or equal the maximal number of
  shapes in the lattice.

* ***axis_order*** (*= "auto"*)
  
  Determines the order of loops filling in the vacant spots. It can be either of:
  * explicitly specified order, for example `"xyz"`. Then the outermost loop is for the x-axis, and the innermost for
    z-axis. More precisely, it means that the cell row on axis z is filled first, then yz layer is populated with those
    rows, and finally layers are stacked one by one along the x-axis
  * `"auto"` - outermost loop is for the axis with the highest number of cells, and the innermost loop for the one with
    the lowest. It means that rows are created along the direction with the smallest number of cells, and layers created
    of such rows are stacked along the axis with the highest number of cells
    

### Class `random`

```python
random(
    n_shapes,
    seed
)
```

Fills the vacant places in lattice with `n_shapes` in a completely random fashion. RNG is seeded with Integer `seed`.

[&uarr; back to the top](#initial-arrangement)