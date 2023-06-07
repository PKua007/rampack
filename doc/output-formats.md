# Output formats

This reference page described all output formats supported by the software.


## Contents

* [Snapshot formats](#snapshot-formats)
  * [Class `ramsnap`](#class-ramsnap)
  * [Class `wolfram`](#class-wolfram)
  * [Class `xyz`](#class-xyz)
* [Trajectory formats](#trajectory-formats)
  * [Class `ramtrj`](#class-ramtrj)
  * [Class `xyz`](#class-xyz-1)
* [Shape model formats](#shape-model-formats)
  * [Class `wolfram`](#class-wolfram-1)
  * [Class `obj`](#class-obj)


## Snapshot formats

Snapshots of the system are outputted at the end of simulation runs (see
[Simulation pipeline](input-file.md#simulation-pipeline),
[integration.output_last_snapshot](input-file.md#integration_outputlastsnapshot)). The amount of information stored
(positions and orientation, box dimensions, simulation metadata) varies between the formats. The native representation
is the [RAMSNAP](#class-ramsnap) format, which contains metadata such as the number of cycles or step sizes. Currently,
the following snapshot formats are supported:

* [Class `ramsnap`](#class-ramsnap)
* [Class `wolfram`](#class-wolfram)
* [Class `xyz`](#class-xyz)


### Class `ramsnap`

```python
ramsnap(
    filename
)
```

Internal RAMPACK text representation of a simulation snapshot, stored to a file with the name given by the `filename`
argument. Apart from box dimensions and particles' positions and orientations, it contains simulation metadata (which
is used to resume an interrupted simulation) as key=value pairs. Format's specification is technically not a part of the
public interface; thus it may change in the future (retaining the software support of older versions). Currently, is has
the following structure:

```text
[k]
[key 1]=[value 1]
...
[key k]=[value k]
[v11] [v21] [v31] [v12] [v22] [v32] [v13] [v23] [v33]
[N]
[r11] [r12] [r13] [O111] [O112] [O113] [O121] [O122] [O123] [O131] [O132] [O133]
...
[rN1] [rN2] [rN3] [ON11] [ON12] [ON13] [ON21] [ON22] [ON23] [ON31] [ON32] [ON33]
```

where:

* `[k]` <br />
   number of key=value pairs
* `[vij]` <br />
   j<sup>th</sup> component of i<sup>th</sup> [box vector](initial-arrangement.md#simulation-box) **v**<sub>*i*</sub>
* `[N]` <br />
   number of particles
* `[rij]` <br />
   j<sup>th</sup> component of the absolute position **r**<sub>*i*</sub> of i<sup>th</sup> particle
* `[Oijk]` <br />
   j<sup>th</sup> row and k<sup>th</sup> column of the orientation matrix **O**<sub>*i*</sub> of i<sup>th</sup> particle

Metadata keys' names (depending on the context, some of the entries may not be always present):
* `cycles` <br />
  number of full Monte Carlo cycles performed
* `step.[move sampler].[move]` <br />
  current step size of a move `[move]` from a [particle](input-file.md#particle-move-types) or 
  [box](input-file.md#box-move-types) move sampler `[move sampler]`
  

### Class `wolfram`

```python
wolfram(
    filename,
    style = standard
)
```

[Wolfram Mathematica](https://www.wolfram.com/mathematica/) output of the last snapshot stored to a file with the name
given by the `filename` argument. Simulation box is not drawn. There are 2 `style` options:

* class `standard`

  In this format,`Graphics3D` contains a list of individually drawn shapes:

  ```wolfram
  Graphics3D[{
      Sphere[position1, radius],    (* individual shape *)
      ...
      Sphere[positionN, radius]
  }]
  ```
  
  You can easily access the individual shapes by indexing the `Graphics3D` object.

* class `affine_transform`

  The shape is drawn only once, then a list of `AffineTransform`-s is mapped over it:

  ```wolfram
  Graphics3D[
      GeometricTransformation[
          Tube[{{0, 0, -1}, {0, 0, 1}}, radius],    (* default-oriented shape at {0, 0, 0} *)
          AffineTransform@#
      ]& /@ {
          {orientation1, position1},                (* list of positions and orientations of shapes *)
          ...
          {orientationN, positionN}
      }
  ]
  ```
  
  It is the most useful for shapes, for which the Wolfram Mathematica code is very long, for example
  [class `generic_convex`](shapes.md#class-generic_convex).

**NOTE**: It is advisable not to open the output directly in Wolfram Mathematica, but rather copy its content into the
clipboard and paste is into an empty notebook. Alternatively, the output can be loaded directly into a variable using

```wolfram
shapshot = ToExpression[Import[filename, "String"]]
```


### Class `xyz`

```python
xyz(
    filename
)
```

[Extended XYZ](https://www.ovito.org/manual/reference/file_formats/input/xyz.html#extended-xyz-format) snapshot stored
to a file with the name given by the `filename` argument. Unlike
[standard XYZ](https://www.ovito.org/manual/reference/file_formats/input/xyz.html#extended-xyz-format) format, which
contains only positions of atoms, in the extended variant box dimensions and particle orientations are also stored. This
flavor is recognized by [Ovito](https://www.ovito.org) out of the box. You can use
`rampack shape-preview --shape=... --output='obj(...)'`
(see [`shape-preview` mode](operation-modes.md#shape-preview-mode)) to generate a [Wavefront OBJ model](#class-obj) of
the shape and load it into Ovito, so it can render the system properly.


Extended XYZ file structure is the following

```text
[N]
3600
Lattice="[box vectors]" Properties=species:S:1:pos:R:3:orientation:R:4 [key 1]=[value 1] ... [key k]=[value k]
A [r11] [r12] [r13] [q11] [q12] [q13] [q14]
...
A [rN1] [rN2] [rN3] [qN1] [qN2] [qN3] [qN4]
```

where

* `A` <br />
  atom type; it is always `A`, as RAMPACK does not operate on real atoms
* `[N]` <br />
  number of particles
* `[box vectors]` = `[v11] [v12] [v13] [v21] [v22] [v23] [v31] [v32] [v33]` <br />
  where `[vij]` is j<sup>th</sup> component of i<sup>th</sup> [box vector](initial-arrangement.md#simulation-box)
  **v**<sub>*i*</sub>
* `[key i]=[value i]` <br />
  the same key=value pairs as in [class `ramsnap`](#class-ramsnap)
* `[rij]` <br />
  j<sup>th</sup> component of the absolute position **r**<sub>*i*</sub> of i<sup>th</sup> particle
* `[qij]` <br />
  j<sup>th</sup> component of the rotation quaternion **q**<sub>*i*</sub> of i<sup>th</sup> particle


## Trajectory formats

Trajectories consist of many snapshots of the system captured every X full MC snapshots. They are stored on the fly
during the simulation (see [Simulation pipeline](input-file.md#simulation-pipeline),
[integration::record_trajectory](input-file.md#integration_recordtrajectory)). The native representation is
[RAMTRJ](#class-ramtrj), which is a compact binary format. It can be later processed using the
[`trajectory` mode](operation-modes.md#trajectory-mode), for example, to calculate additional observables.

Currently, the following formats are supported:

* [class `ramtrj`](#class-ramtrj)
* [class `xyz`](#class-xyz-1)


### Class `ramtrj`

```python
ramtrj(
    filename
)
```

Trajectory in the internal binary RAMTRJ format stored to a file with the name given by the `filename` argument. It is
designed to be compact with easy access to individual snapshots. It is not part of a public interface, so its format may
change in the future (retaining the software support of older versions).

Currently, RAMTRJ files have the following binary structure (C types of primitive blocks are in `(...)`):

```text
[header] [snapshot 1] ... [snapshot m]

[header] := [magic] [version major(char)] [version minor(char)] [N(unsigned long)] [m(unsigned long)] [s(unsigned long)]
[magic] := "RAMTRJ\n" ASCII characters
[snapshot i] := [box dimensions] [particle 1] ... [particle N]
[box dimensions] := [v11(double)] [v21] [v31] [v12] [v22] [v32] [v13] [v23] [v33]
[particle i] := [ri1(double)] [ri2] [ri3] [ei1(double)] [ei2] [ei3]
```

where

* `[N]` <br />
  number of particles
* `[m]` <br />
  number of snapshots
* `[s]` <br />
  number of cycles between two snapshots
* `[vij]` <br />
   j<sup>th</sup> component of i<sup>th</sup> [box vector](initial-arrangement.md#simulation-box) **v**<sub>*i*</sub>
* `[rij]` <br />
  j<sup>th</sup> component of the absolute position **r**<sub>*i*</sub> of i<sup>th</sup> particle
* `[eij]` <br />
  j<sup>th</sup> [Euler angle](https://en.wikipedia.org/wiki/Euler_angles#Conventions) (in the extrinsic XYZ, 
  convention, in radians) of i<sup>th</sup> particle

The initial configuration is not stored - the first captured snapshot is for cycle number equal step size `[s]`.

### Class `xyz`

```python
xyz(
    filename
)
```

[Extended XYZ](https://www.ovito.org/manual/reference/file_formats/input/xyz.html#extended-xyz-format) trajectory stored
to a file with the name given by the `filename` argument. It is basically a collection of XYZ snapshots, so please refer 
to [snapshot class `xyz`](#class-xyz) documentation. Here, each snapshot has only one auxiliary key=value pair, which is
`cycle=[cycle number]`. This trajetory format is recognized by [Ovito](https://www.ovito.org) out of the box. You can
use `rampack shape-preview --shape=... --output='obj(...)'`
(see [`shape-preview` mode](operation-modes.md#shape-preview-mode)) to generate a [Wavefront OBJ model](#class-obj) of
the shape and load it into Ovito, so it can render the system properly.


## Shape model formats

The [`shape-preview` mode](operation-modes.md#shape-preview-mode) can export the 3D model of a shape for visualization
purposes. The following formats are currently supported:

* [Class `wolfram`](#class-wolfram-1)
* [Class `obj`](#class-obj)


### Class `wolfram`

```python
wolfram(
    filename
)
```

Stores the shape model in [Wolfram Mathematica](https://www.wolfram.com/mathematica/) format to a file with the name
given by the `filename` argument. The output can be passed as an argument of `Graphics3D` object.

**NOTE**: It is advisable not to open the output directly in Wolfram Mathematica, but rather copy its content into the
clipboard and paste is into an empty notebook. Alternatively, the output can be loaded directly into a variable using

```wolfram
shape = ToExpression[Import[filename, "String"]]
```

### Class `obj`

```python
obj(
    filename
)
```

Stores the shape model in [Wavefront OBJ](https://en.wikipedia.org/wiki/Wavefront_.obj_file) format to a file with the
name given by the `filename` argument. The format is recognized by a wide range of visualization tools, including
[Ovito](https://www.ovito.org).