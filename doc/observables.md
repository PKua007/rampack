# Observables

This reference page give a full walkthrough over observables that can be computed during the simulation or afterwards.


## Contents

* [Observable types](#observable-types)
* [Normal observables](#normal-observables)
  * [Class `number_density`](#class-number_density)
  * [Class `box_dimensions`](#class-box_dimensions)
  * [Class `packing_fraction`](#class-packing_fraction)
  * [Class `compressibility_factor`](#class-compressibility_factor)
  * [Class `energy_per_particle`](#class-energy_per_particle)
  * [Class `energy_fluctuations_per_particle`](#class-energy_fluctuations_per_particle)
  * [Class `nematic_order`](#class-nematic_order)
  * [Class `smectic_order`](#class-smectic_order)
  * [Class `bond_order`](#class-bond_order)
  * [Class `rotation_matrix_drift`](#class-rotation_matrix_drift)
  * [Class `temperature`](#class-temperature)
  * [Class `pressure`](#class-pressure)
* [Bulk observables](#bulk-observables)
  * [Class `pair_density_correlation`](#class-pair_density_correlation)
  * [Class `pair_averaged_correlation`](#class-pair_averaged_correlation)
  * [Class `density_histogram`](#class-density_histogram)
  * [Class `probability_evolution`](#class-probability_evolution)
* [Trackers](#trackers)
  * [Class `fourier_tracker`](#class-fourier_tracker)
* [Binning types](#binning-types)
  * [Class `radial`](#class-radial)
  * [Class `layerwise_radial`](#class-layerwise_radial)
* [Correlation functions](#correlation-functions)
  * [Class `s110`](#class-s110)
  * [Class `axes_angle`](#class-axes_angle)
* [Shape functions](#shape-functions)
  * [Class `const`](#class-const)
  * [Class `axis`](#class-axis)


## Observable types

Observables are (usually numerical) parameters characterizing various aspects of the system. They can be a single number
(such as [number density](#class-number_density)), set of numbers (such as [box dimensions](#class-box_dimensions)) or
even whole matrices of values (such as [density histogram](#class-density_histogram)). There are 2 types of observables
available in the software:

* [(normal) observables](#normal-observables),
* [bulk observables](#bulk-observables).

They are described in corresponding sections of this documentation page.

## Normal observables

**Normal observables** can be calculated for a single snapshot. They consist of a couple of numbers
(**interval values**), which can be added/averaged/etc. and/or a couple of strings (**nominal values**), which cannot be
averaged. They can be computed in both the thermalization and the averaging phase. Normal observables have 3 scopes in
which they can be computed and presented:

1. **Inline scope** - they are computed and presented in simulation standard output together with cycle number and other
   information. Both interval and nominal values are printed.
2. **Snapshot scope** - they are gathered in both the thermalization and the averaging phase every
   [`snapshot_every`](input-file.md#integration_snapshotevery) cycles and printed as a single entry (a single row) in
   the file given by [`observables_out`](input-file.md#integration_observablesout) (the links refer to
   [class `integration`](input-file.md#class-integration), but for
   [class `overlap_relaxation`](input-file.md#class-overlap_relaxation) it is analogous). Both interval and nominal
   values are printed.
3. **Averaging scope** - each separate interval value (nominal values are skipped) is averaged in the averaging phase.
   The values are taken every [`averaging_every`](input-file.md#integration_averagingevery) cycles. When the averaging
   phase is completed, average values are printed on the standard output and appended as a single row to the file given
   by [`averages_out`](input-file.md#integration_averagesout), together with NpT pressure and temperature.

Scopes are **NOT** mutually exclusive and can be combined. By default, the observable has all 3 scopes. To restrict the
scopes, one can use class `scoped`:

```python
scoped(
    obs,
    snapshot = False,
    averaging = False,
    inline = False
)
```

where `obs` is the observable, while `snapshot`, `averaging` and `inline` arguments toggle the scopes on/off. For
example, to print [number density](#class-number_density) only on the standard output, one can use

```python
scoped(number_density, inline=True)
```

Currently, the following observables are supported:

* [Class `number_density`](#class-number_density)
* [Class `box_dimensions`](#class-box_dimensions)
* [Class `packing_fraction`](#class-packing_fraction)
* [Class `compressibility_factor`](#class-compressibility_factor)
* [Class `energy_per_particle`](#class-energy_per_particle)
* [Class `energy_fluctuations_per_particle`](#class-energy_fluctuations_per_particle)
* [Class `nematic_order`](#class-nematic_order)
* [Class `smectic_order`](#class-smectic_order)
* [Class `bond_order`](#class-bond_order)
* [Class `rotation_matrix_drift`](#class-rotation_matrix_drift)
* [Class `temperature`](#class-temperature)
* [Class `pressure`](#class-pressure)

as well as [Trackers](#trackers), which are described in a separate section. All observable have primary name (displayed
when printing averages on the standard output) and one or more named interval/nominal values.


### Class `number_density`

```python
number_density( )
```

* **Primary name**: `Number density`
* **Interval values**:
  * `rho` - number density N/V, where N is number of particle and V is box volume
* **Nominal values**: None


### Class `box_dimensions`

```python
box_dimensions( )
```

* **Primary name**: `Box dimensions`
* **Interval values**:
  * `L_X` - height of the box corresponding to 1<sup>st</sup> vector (orthogonal to 2<sup>nd</sup> and 3<sup>rd</sup>)
  * `L_Y` - height of the box corresponding to 2<sup>nd</sup> vector (orthogonal to 3<sup>rd</sup> and 1<sup>st</sup>)
  * `L_Z` - height of the box corresponding to 3<sup>rd</sup> vector (orthogonal to 1<sup>st</sup> and 2<sup>nd</sup>)
* **Nominal values**: None


### Class `packing_fraction`

```python
packing_fraction( )
```

* **Primary name**: `Packing fraction`
* **Interval values**:
  * `theta` - packing fraction N V<sub>mol</sub> / V, where N is number of particle and V is the box volume and
    V<sub>mol</sub> is the volume of a particle
* **Nominal values**: None


### Class `compressibility_factor`

```python
compressibility_factor( )
```

Factor &ge; 1, which describe how much the state deviates from the ideal gas equation.

* **Primary name**: `Compressibility factor`
* **Interval values**:
  * `Z` - pV/NT, where p - pressure, V - box volume, N - number of molecules, T - temperature
* **Nominal values**: None


### Class `energy_per_particle`

```python
energy_per_particle( )
```

* **Primary name**: `Energy per particle`
* **Interval values**:
  * `E` - average interaction energy of a single particle with the rest of the system
* **Nominal values**: None


### Class `energy_fluctuations_per_particle`

```python
energy_fluctuations_per_particle( )
```

* **Primary name**: `Energy fluctuation per particle`
* **Interval values**:
  * `varE` - variance of the interaction of a single particle with the rest of the system
* **Nominal values**: None


### Class `nematic_order`

```python
nematic_order(
    dump_qtensor = False
)
```

Nematic order parameter, which quantifies ordering of orientations along a preferred direction. It is an eigenvalue of
the symmetric **Q**-tensor with the largest magnitude. The **Q**-tensor is defined as

**Q** = 1/N &sum;<sub>*i*</sub> (3/2 **a**<sub>*i*</sub>&otimes;**a**<sub>*i*</sub> - 1/2),

where index *i* runs over all N particles.

* **Arguments**:
  * ***dump_qtensor*** (*= False*) <br />
    If `True`, in addition to nematic order value, independent entries of the **Q**-tensor will be printed
* **Primary name**: `Nematic order`
* **Interval values**:
  * `P2` - eigenvalue of the **Q**-tensor with the highest magnitude
  * `Q11`, `Q12`, `Q13`, `Q22`, `Q23`, `Q33` - all independent (upper triangular) entries of the **Q**-tensor. Those
    parameters are only present if `dump_qtensor = True`
* **Nominal values**: None


### Class `smectic_order`

```python
smectic_order(
    max_hkl,
    dump_tau_vector = False,
    focal_point = "o",
    function = const
)
```

Smectic order parameter, which in its most general form quantifies modulation of some quantity in the direction of
wavevector **k**. It is defined as

*&tau;<sub>f</sub>* = | 1/N &sum;<sub>*i*</sub> *f<sub>i</sub>* exp(&iota;**k** &middot; **r**<sub>*i*</sub>) |,

where index *i* goes over all N particles, **r**<sub>*i*</sub> is the position of *i*<sup>th</sup> particle,
*f<sub>i</sub>* is the value of `function` for this particle, &iota; is the imaginary unit and |...| is modulus. If
`function = const` (default argument value), it recudes to a standard smectic parameter quantifying the density
modulation:

*&tau;* = | 1/N &sum;<sub>*i*</sub> exp(&iota;**k** &middot; **r**<sub>*i*</sub>) |.

Wavevector **k** has to be compatible with periodic boundary conditions, thus it is specified using Miller indices *h*,
*k*, *l*:

**k** = *h* **g**<sub>1</sub> + *k* **g**<sub>2</sub> + *l* **g**<sub>3</sub>,

where **g**<sub>*i*</sub> are reciprocal box vectors:

**g**<sub>1</sub> = 2&pi;/V (**v**<sub>2</sub> &#10799; **v**<sub>3</sub>), <br/>
**g**<sub>2</sub> = 2&pi;/V (**v**<sub>3</sub> &#10799; **v**<sub>1</sub>), <br/>
**g**<sub>3</sub> = 2&pi;/V (**v**<sub>1</sub> &#10799; **v**<sub>2</sub>).

Here, **v**<sub>*i*</sub> are real-space box vectors and V is the box volume. Values of *h*, *k*, *l* are selected from
the range given by `max_hkl` argument to maximize *&tau;* value.

* **Arguments**:
  * ***max_hkl*** <br />
    An Array of 3 Integers representing maximal magnitudes of subsequent Miller indices *h*, *k*, *l*, which will be
    searched to find the best **k** wavevector, excluding vectors differing only by the sign. For example, for
    `max_hkl = [0, 1, 1]` all independent indices to be checked are `[0, 0, 0]`, `[0, 0, 1]`, `[0, 1, -1]`, `[0, 1, 0]`
    and `[0, 1, 1]`.
  * ***dump_tau_vector*** (*= False*) <br />
    If `True`, additional `tau_[function name]_k_x`, `..._y`, `..._z` interval parameters with wavevector **k**
    components will be printed.
  * ***focal_point*** (*= "o"*) <br />
    [Named point](shapes.md#named-points) on the particle that will be used as position vector **r**.
  * ***function*** (*= const*) <br />
    [Shape function](#shape-functions) *f* that will be used in computations.
* **Primary name**: `Smectic order`
* **Interval values**:
  * `tau_[function name]` - smectic order magnitude *&tau;*
  * `tau_[function name]_k_x`, `..._y`, `..._z` - components of wavevector **k**. Those parameters are only present if
    `dump_tau_vector = True`
* **Nominal values**:
  * `tau_[function name]_hkl` - *h*, *k*, *l* giving the best *&tau;* in the format `h.k.l`, for example `0.-1.5`

`[function name]` describes the [Shape function](#shape-functions) used. If `function = const`, all `[function name]`
tokens are dropped from constituent values' names.


### Class `bond_order`

```python
bond_order(
    hkl,
    ranks,
    layering_point = "o",
    focal_point = "o"
)
```

Bond order parameter, which quantifies local order of angles between nearest neighbours. On a 2D plane, it is defined as

*&psi;<sub>r</sub>* = 1/*n* &sum;<sub>*i*</sub> 1/*r* | &sum;<sub>*j*</sub> exp(*r &iota; &theta;<sub>ij</sub>*) |,

where index *i* goes over all *n* particles lying on the plane, *r* is the rank of the order parameter,
&sum;<sub>*j*</sub> sum goes over *j* = 1, ..., *r* nearest neighbours of the *i*<sup>th</sup> particle, &iota; is the
imaginary unitand |...| is modulus. Finally, *&theta;<sub>ij</sub>* is the angle between the vector joining
*i*<sup>th</sup> and *j*<sup>th</sup> particles and a constant arbitrary direction on the plane. 

To make it applicable to a 3D system, the system is assumed to be a layered smectic (you can also project all particles
on a single plane, see `hkl` argument description). All positions are projected onto nearest layers, *&psi;<sub>r</sub>*
is calculated for each layer separately and then averaged over all layers. Number of layers and their wavevector are
specified upfront by `hkl` Miller indices (see [class `smectic_order`](#class-smectic_order)), while their positions as
well as association of particles to them are inferred automatically.

* **Arguments**:
  * ***hkl*** <br />
    An Array of Integers specifying Miller indices of the wavevector of layers. For example, `hkl = [0, 0, 6]`
    represents 6 layers stacked along the z axis. *Tip*: use `hkl = [0, 0, 1]` to project all particles onto a single XY
    plane.
  * ***ranks*** <br />
    An Integer or Array of Integers with one or more ranks *r* of the bond order parameter to be calculated. For
    example, to quantify hexatic honeycomb order you want to use `ranks = 6`, while for a square lattice `ranks = 4`
    will do the job (`ranks = [4, 6]` will calculate both).
  * ***layering_point*** (*= "o"*) <br />
    [Named point](shapes.md#named-points) on the particle that will be used to associate particles to the nearest
    layers.
  * ***focal_point*** (*= "o"*) <br />
    [Named point](shapes.md#named-points) on the particle that will be used to compute *&theta;<sub>ij</sub>* angles.
* **Primary name**: `Bond order`
* **Interval values**:
  * `psi_[r1]`, `psi_[r2]`, ... - bond order parameters for all `[r1]`, `[r2]`, ... ranks specified by `ranks` argument. 
* **Nominal values**: None


### Class `rotation_matrix_drift`

```python
rotation_matrix_drift( )
```

A rather technical parameter, which measures how rotation matrices accumulate numerical errors during the simulation. It
can be used for debugging purposes, especially when using non-standard particle moves. For a rotation matrix **R**, one
defines

**M** = **R**<sup>T</sup>**R** - **I**,

which is mathematically equal 0 for a rotation matrix. Then, the Frobenius norm of **M** is computed:

*F*<sup>2</sup> = &Vert;**M**&Vert;<sup>2</sup> = &sum;<sub>*i*,*j*</sub> *M<sub>ij</sub>*<sup>2</sup>

For each snapshot, the observable reports the minimal, the maximal and the average value of *F*<sup>2</sup>.

* **Primary name**: `Rotation matrix drift`
* **Interval values**:
  * `F^2` - the average value of *F*<sup>2</sup> over all particles in the snapshot
  * `min(F^2)` - the minimal value of *F*<sup>2</sup> in the snapshot
  * `max(F^2)` - the maximal value of *F*<sup>2</sup> in the snapshot
* **Nominal values**: None


### Class `temperature`

```python
temperature( )
```

The current NpT/NVT temperature of the system. It is useful when the temperature is a
[dynamic parameter](input-file.md#dynamic-parameters), and you want to monitor its instantaneous value.

* **Primary name**: `Temperature`
* **Interval values**:
  * `T` - the current NpT/NVT temperature
* **Nominal values**: None


### Class `pressure`

```python
pressure( )
```

The current NpT pressure of the system. It is useful when the temperature is a
[dynamic parameter](input-file.md#dynamic-parameters), and you want to monitor its instantaneous value. Please note that
it is not the effective pressure computed from a numerical virial - it is the one imposed in the NpT ensemble.
[Virial pressure](http://www.sklogwiki.org/SklogWiki/index.php/Pressure#Virial_pressure) computation is not yet
supported.

* **Primary name**: `Pressure`
* **Interval values**:
  * `p` - the current NpT pressure
* **Nominal values**: None


## Bulk observables

Bulk observables, contrary to [normal observables](#normal-observables), consist of too many values to be meaningfully
presented as a part of a single-line entry on the standard output or other form of the output. They are usually whole
plots, maps, etc. Their data is gathered only in the averaging phase, averaged over many system snapshots and printed to
a separate file (with name generated using
[`bulk_observables_out_pattern`](input-file.md#integration_bulkobservablesoutpattern)), whose format is bulk observable
specific.

The following bulk observables are available:
* [Class `pair_density_correlation`](#class-pair_density_correlation)
* [Class `pair_averaged_correlation`](#class-pair_averaged_correlation)
* [Class `density_histogram`](#class-density_histogram)
* [Class `probability_evolution`](#class-probability_evolution)

Each observable has a **short name**, which is used in the output file name.


### Class `pair_density_correlation`

```python
pair_density_correlation(
    max_r,
    n_bins,
    binning
)
```

General one-dimensional pair density correlation function *&rho;*(*r*). It is equal to the number of particles in
the system with a distance around *r* normalized by the number that would be found in a uniform system. Please note,
that the meaning of *distance* is defined by the `binning` argument. For example, when `binning = radial`, it reduces to
the standard [radial distribution function](https://en.wikipedia.org/wiki/Radial_distribution_function).

* **Arguments**:
  * ***max_r*** <br />
    Maximal distance which is going to be probed. The observable range starts at 0 and ends at this value.
  * ***n_bins*** <br />
    Number of bins to use. The more of them, the higher is the resolution of the plot, but the smaller are the
    statistics in the single bin.
  * ***binning*** <br />
    [Binning type](#binning-types) used. It defines what *distance* means. For example, for `binning = radial`,
    *distance* is the Euclidean distance between the particles.
* **Short name**: `rho_[binning name]`, where `[binning name]` depends on the [binning type](#binning-types) (`binning`
  argument).
* **Output**:
  Rows with space-separated pairs (*r*, *&rho;*(*r*)).


### Class `pair_averaged_correlation`

```python
pair_averaged_correlation(
    max_r,
    n_bins,
    binning,
    function
)
```

Correlations *S*(*r*) between particles as a function of a generalized distance *r* (defined by the `binning` argument).
It is defined as the average of correlation function specified by `function` parameter over all particles with a
distance around *r*.

* **Arguments**:
  * ***max_r*** <br />
    Maximal distance which is going to be probed. The observable range starts at 0 and ends at this value.
  * ***n_bins*** <br />
    Number of bins to use. The more of them, the higher is the resolution of the plot, but the smaller are the
    statistics in the single bin.
  * ***binning*** <br />
    [Binning type](#binning-types) used. It defines what *distance* means. For example, for `binning = radial`,
    *distance* is the Euclidean distance between the particles.
  * ***function*** <br />
    Two-particle [correlation function](#correlation-functions) which is being averaged.
* **Short name**: `[function name]_[binning name]`, where `[function name]` depends on the 
  [correlation function](#correlation-functions) (`function` argument), while `[binning name]` depends on the
  [binning type](#binning-types) (`binning` argument).
* **Output**:
  Rows with space-separated pairs (*r*, *S*(*r*)).


### Class `density_histogram`

```python
density_histogram(
    n_bins_x = None,
    n_bins_y = None,
    n_bins_z = None,
    tracker = None
)
```

Density histogram, which can be 1D, 2D or 3D. It also supports cancelling out the translational Goldstone mode to
prevent softening of the histogram due to zero-energy bulk system movement. Relative positions are used for binning.
Thus, the domain is always [0, 1)<sup>*d*</sup>, where *d* is the dimension. 

* **Arguments**:
  * ***n_bins_x*** (*= None*) <br />
    ***n_bins_y*** (*= None*) <br />
    ***n_bins_z*** (*= None*) <br />
    Number of bins in each direction. If 1 or `None` is specified, the given direction is turned off completely. For
    example, to prepare the density histogram of the system projected on YZ plane (with 100 x 100 bins), you should
    specify `n_bins_x = 1` (or `n_bins_x = None`), `n_bins_y = 100` and `n_bins_z = 100`.
  * ***tracker*** (*= None*) <br />
    [Goldstone tracker](#trackers) used to cancel out system movement. If `None`, no compensation is applied.
* **Short name**: `rho_xyz`
* **Output**:
  Rows with space-separated tuples (*b*<sub>x</sub>, *b*<sub>y</sub>, *b*<sub>z</sub>, *&rho;*(**b**)), where **b** is
  relative middle of the bin. If the direction is turned off, the correspinding bin coordinate is equal 0.5.


### Class `probability_evolution`

```python
probability_evolution(
    max_r,
    n_bins_r,
    binning,
    fun_range,
    n_bins_fun,
    function,
    normalization = None
)
```

Two-dimensional plot of a function

*P*(*r*, *f*) = prob(*f*|*r*)

where prob(*f*|*r*) is a conditional probability density of correlation function *f* for a fixed generalized distance
*r* between particles (defined by the `binning` argument). It represents how the distribution of *f* changes with *r*.

* **Arguments**:
  * ***max_r*** <br />
    Maximal distance which is going to be probed. The distance range starts at 0 and ends at this value.
  * ***n_bins_r*** <br />
    Number of bins to use for distance. The more of them, the higher is the resolution of the plot, but the smaller are
    the statistics in the single bin.
  * ***binning*** <br />
    [Binning type](#binning-types) used. It defines what *distance* means. For example, for `binning = radial`,
    *distance* is the Euclidean distance between the particles.
  * ***fun_range*** <br />
    An Array of 2 Floats representing minimal and maximal value of the function *f* that will be plotted.
  * ***n_bins_fun*** <br />
    Number of bins to use for *f* values. The more of them, the higher is the resolution of the plot, but the smaller
    are the statistics in the single bin.
  * ***function*** <br />
    Two-particle [correlation function](#correlation-functions) which is being averaged.
  * ***normalization*** (*= None*) <br />
    How *P*(*r*, *f*) should be normalized. There are 3 options:
    * `None` <br />
      No normalization is performed. The values are snapshot-averaged counts of particles in the bins. Please
      note than in that case the sum of counts for a fixed distance *r* is proportional to the
      [pair density correlation function](#class-pair_density_correlation).
    * `"pdf"` <br />
      Standard probability density function normalization, for which &int;*P*(*r*, *f*) d*f* = 1.
    * `"unit"` <br />
      Average value in the bin is normalized to 1: &int;*P*(*r*, *f*) d*f* = *f*<sub>max</sub> - *f*<sub>min</sub>
* **Short name**: `prob_[function name]_[binning name]`, where `[function name]` depends on the
  [correlation function](#correlation-functions) (`function` argument), while `[binning name]` depends on the
  [binning type](#binning-types) (`binning` argument).
* **Output**:
  Rows with space-separated 3-tuples (*r*, *f*, *P*(*r*, *f*)), where *r*, *f* are bin middles.


## Trackers

A special class of [normal observables](#normal-observables), with 6 interval values specifying how the system
translates and rotates during its evolution. Those are:

* `[tracker name]_x`, `[...]_y`, `[...]_z` - evolving position of the origin of the system,
* `[tracker name]_ox`, `[...]_oy`, `[...]_oz` - evolving orientation of the system (as Euler angles).

What is considered origin and the identity orientation (zero Euler angles), as well as how the movement is inferred is
specific to a particular tracker type. Some trackers may track only one type of movement (translational or
orientational). Each tracker has its **tracker name**, which is used for example in interval value's names.

The trackers are mainly used by other types of observables, such as
[class `density_histogram`](#class-density_histogram) to cancel out zero-energy movement of the system during the course
of the simulation.

Currently, the following trackers are available:
* [Class `fourier_tracker`](#class-fourier_tracker)


### Class `fourier_tracker`

```python
fourier_tracker(
    wavenumbers,
    function
)
```

A 1D, 2D or 3D tracker, which tracks system movement based on first-order Fourier expansion of a
[shape function](#shape-functions) *f*. More precisely, it tracks how modulations of the shape function move in space.
Most generally, for 3 dimensions, first-order Fourier expansion of *f* in the relative position **s** is

*F*(**s**) = &sum;<sub>trig<sub>1</sub>,trig<sub>2</sub>,trig<sub>3</sub></sub>
*A*<sub>trig<sub>1</sub>,trig<sub>2</sub>,trig<sub>3</sub></sub>
trig<sub>1</sub>(2&pi;*n*<sub>1</sub>*s*<sub>1</sub>)
trig<sub>2</sub>(2&pi;*n*<sub>2</sub>*s*<sub>2</sub>)
trig<sub>3</sub>(2&pi;*n*<sub>3</sub>*s*<sub>3</sub>),

where trig<sub>1,2,3</sub> is sin(...) or cos(...), **s** is a relative position in the box and *n<sub>1,2,3* are
wavenumbers of shape function modulations. The evolving position of the system's origin is taken as the maximum of
F(**s**), converted back to an absolute position. Please note, that *F*(**s**) is oscillatory and has more than one
maximum. Thus, for subsequent snapshots the maximum closest to the previous one is chosen. The 8 coefficients for a
snapshot are calculated using

*A*<sub>trig<sub>1</sub>,trig<sub>2</sub>,trig<sub>3</sub></sub> =
1/N &sum;<sub>*i*</sub> *f<sub>i</sub>*
trig<sub>1</sub>(2&pi;*n*<sub>1</sub>*s*<sub>*i*,1</sub>)
trig<sub>2</sub>(2&pi;*n*<sub>2</sub>*s*<sub>*i*,2</sub>)
trig<sub>3</sub>(2&pi;*n*<sub>3</sub>*s*<sub>*i*,3</sub>),

where *f<sub>i</sub>* and **s**<sub>*i*</sub> are, respectively value of the shape function *f* and relative position of
*i*<sup>th</sup> particle. For 2D and 1D (where some *n<sub>i</sub>* are 0), the number of coefficients decreases to,
respectively 4 and 2.

Please note, that *F*(**s**) is sensitive to density modulation - if *f* = const, it tracks a generalized type of
[smectic order](#class-smectic_order). In fact, if only a single wavenumber is non-zero (1D), it is equivalent to the
smectic order parameter.

* **Arguments**:
  * ***wavenumbers*** <br />
    An Array of Integers (for example `[1, 2, 0`]) representing wavenumbers *n<sub>i</sub>* of modulation. If all are
    non-zero, the modulation is 3-dimensional, while setting 1 or 2 of them to 0 reduces dimensionality.
  * ***function*** <br />
    [Shape function](#shape-functions), whose modulation we are testing. It can be [const](#class-const) - then we are
    probing the density modulation.
* **Tracker name**: `[function name]_fourier`, where `[function name]` depends on the [shape function](#shape-functions)
  (`function` argument)


## Binning types

Binning type dictates how various types of observables are calculated. Most importantly, it defines what *distance*
between particles means. Binning type is chosen for example when using
[class `pair_density_correlation`](#class-pair_density_correlation). There, it decides what type of correlation is
probed - radial, transversal, cylindrical, etc. It can also restrict which pairs of particles should be selected at
all - for example, [class `layerwise_radial`](#class-layerwise_radial) takes into account only particles from the same
layers.

There are the following types of binning:
* [Class `radial`](#class-radial)
* [Class `layerwise_radial`](#class-layerwise_radial)


### Class `radial`

```python
radial(
    focal_point = "o"
)
```

The standard, radial binning type. Here, the *distance* is a standard Euclidean distance and all pairs of particles are
enumerated. `focal_point` is a [named point](shapes.md#named-points), with respect to which the distance should be
calculated.


### Class `layerwise_radial`

```python
layerwise_radial(
    hkl,
    focal_point = "o"
)
```

Layerwise, transversal binning type. Particles are projected on nearest layers as specified by `hkl` Miller indices (see
[class `smectic_order`](#class-smectic_order)) and the *distance* is calculated along the layer (transversally).
Moreover, pairs of particles in different layers are not enumerated. `focal_point` is a
[named point](shapes.md#named-points), which is used to associate particles to layers and with respect to which the
distance should be calculated.


## Correlation functions

Correlation functions take a pair of particles and map it to a single value. They are used in correlation observables,
such as [class `pair_averaged_correlation`](#class-pair_averaged_correlation).

Currently, the following correlation functions are available:
* [Class `s110`](#class-s110)
* [Class `axes_angle`](#class-axes_angle)


### Class `s110`

```python
s110(
    axis
)
```

*S*<sub>110</sub> element of the [S-expansion](https://doi.org/10.1080/00268977800101541), which is defined simply as

*S*<sub>110</sub>(*i*, *j*) = **a**<sub>*i*</sub> &middot; **a**<sub>*j*</sub>,

where **a**<sub>*i*</sub> and **a**<sub>*j*</sub> are (unit) [shape axes](shapes.md#shape-axes) of the *i*<sup>th</sup>
and *j*<sup>th</sup> molecule, determined by the `axis` argument (`"primary"`, `"secondary"` or `"auxiliary"`). 


### Class `axes_angle`

```python
axes_angle(
    axis
)
```

The (smaller) angle between axes of two particles, expressed in degrees:

*&theta;*(*i*, *j*) = (180/&pi;) cos<sup>-1</sup>|**a**<sub>*i*</sub> &middot; **a**<sub>*j*</sub>|.

**a**<sub>*i*</sub> and **a**<sub>*j*</sub> are (unit) [shape axes](shapes.md#shape-axes) of the *i*<sup>th</sup>
and *j*<sup>th</sup> molecule, determined by the `axis` argument (`"primary"`, `"secondary"` or `"auxiliary"`).


## Shape functions

Shape functions take a single particle and map it to a single value. They are used for example in
[class `smectic_order`](#class-smectic_order) and [class `fourier_tracker`](#class-fourier_tracker).

Currently, the following correlation functions are available:
* [Class `const`](#class-const)
* [Class `axis`](#class-axis)


### Class `const`

```python
const(
    value = 1
)
```

Constant shape function always returning `value` (`1` by default).


### Class `axis`

```python
axis(
    which,
    comp
)
```

Shape function returning a specific coordinate of a specific shape axis.

Arguments:

* ***which***

  [Shape axis](shapes.md#shape-axes) whose component is to be taken. It can be either of: `"primary"`, `"secondary"` or
  `"auxiliary"`.

* ***comp***

  Component of the axis vector to be returned. It can be `"x"`, `"y"` or `"z"`.