# Observables

This reference page give a full walkthrough over observables that can be computed during the simulation or afterwards.


## Contents

* [Observable types](#observable-types)
* [Normal observables](#normal-observables)
  * [Class `number_density`](#class-numberdensity)
  * [Class `box_dimensions`](#class-boxdimensions)
  * [Class `packing_fraction`](#class-packingfraction)
  * [Class `compressibility_factor`](#class-compressibilityfactor)
  * [Class `energy_per_particle`](#class-energyperparticle)
  * [Class `energy_fluctuations_per_particle`](#class-energyfluctuationsperparticle)
  * [Class `nematic_order`](#class-nematicorder)
  * [Class `smectic_order`](#class-smecticorder)
  * [Class `bond_order`](#class-bondorder)
  * [Class `rotation_matrix_drift`](#class-rotationmatrixdrift)
  * [Class `temperature`](#class-temperature)
  * [Class `pressure`](#class-pressure)
* [Bulk observables](#bulk-observables)
  * [Class `pair_density_correlation`](#class-pairdensitycorrelation)
  * [Class `pair_averaged_correlation`](#class-pairaveragedcorrelation)
  * [Class `density_histogram`](#class-densityhistogram)
* [Trackers](#trackers)
  * [Class `fourier_tracker`](#class-fouriertracker)
* [Binning types](#binning-types)
  * [Class `radial`](#class-radial)
  * [Class `layerwise_radial`](#class-layerwiseradial)
* [Correlation functions](#Correlation-functions)
  * [Class `s110`](#class-s110)
* [Shape functions](#shape-functions)
  * [Class `const`](#class-const)
  * [Class `axis`](#class-axis)


## Observable types

Observables are (usually numerical) parameters characterizing various aspects of the system. They can be a single number
(such as [number density](#class-numberdensity)), set of numbers (such as [box dimensions](#class-boxdimensions)) or
even whole matrices of values (such as [density histogram](#class-densityhistogram)). There are 2 types of observables
available in the software:

* [(normal) observables](#normal-observables),
* [bulk observables](#bulk-observables).

They are described in corresponding sections of this documentation page.

## Normal observables

**Normal observables** can be calculated for a single snapshot. They consist of a couple of numbers
(**interval values**), which can be added/averaged/etc. and/or a couple of strings (**nominal value**), which cannot be
averaged. They can be computed in both thermalization and averaging phase. Normal observables have 3 scopes in which
they can be computed and presented:

1. **Inline scope** - they are computed and presented in simulation standard output together with cycle number and other
   information. Both interval and nominal values are printed.
2. **Snapshot scope** - they are gathered in both the thermalization and the averaging phase every
   [`snapshot_every`](input-file.md#integration_snapshotevery) cycles and printed as a single entry (a single row) in
   the file given by [`observables_out`](input-file.md#integration_observablesout) (the links refer to
   [class `integration`](input-file.md#class-integration), but for
   [class `overlap_relaxation`](input-file.md#class-overlaprelaxation) it is analogous). Both interval and nominal
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
example, to print [number density](#class-numberdensity) only on the standard output, one can use

```python
scoped(number_density, inline=True)
```

Currently, the following observables are supported:

* [Class `number_density`](#class-numberdensity)
* [Class `box_dimensions`](#class-boxdimensions)
* [Class `packing_fraction`](#class-packingfraction)
* [Class `compressibility_factor`](#class-compressibilityfactor)
* [Class `energy_per_particle`](#class-energyperparticle)
* [Class `energy_fluctuations_per_particle`](#class-energyfluctuationsperparticle)
* [Class `nematic_order`](#class-nematicorder)
* [Class `smectic_order`](#class-smecticorder)
* [Class `bond_order`](#class-bondorder)
* [Class `rotation_matrix_drift`](#class-rotationmatrixdrift)
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
  * `theta` - packing fraction N V<sub>mol</sub> / V, where N is number of particle and V is box volume and
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
  * ***dump_qtensor*** (*=False*) <br />
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

Here, **v**<sub>*i*</sub> are box vectors and V is box volume. Values of *h*, *k*, *l* are chosen automatically to
maximize *&tau;* value from the range given by `max_hkl` argument.

* **Arguments**:
  * ***max_hkl*** <br />
    An Array of 3 Integers representing maximal magnitudes of subsequent Miller indices *h*, *k*, *l*, which will be
    searched to find the best **k** wavevector, excluding vectors differing only by the sign. For example
    `max_hkl = [0, 1, 1]` will try `[0, 0, 0]`, `[0, 0, 1]`, `[0, 1, -1]`, `[0, 1, 0]` and `[0, 1, 1]`.
  * ***dump_tau_vector*** (*=False*) <br />
    If `True`, additional `tau_[function name]_k_x`, `..._y`, `..._z` interval parameters with wavevector **k**
    components will be printed.
  * ***focal_point*** (*="o"*) <br />
    [Named point](shapes.md#named-points) on the particle that will be used as position vector **r**.
  * ***function*** (*=const*) <br />
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

### Class `rotation_matrix_drift`

### Class `temperature`

### Class `pressure`


## Bulk observables

### Class `pair_density_correlation`

### Class `pair_averaged_correlation`

### Class `density_histogram`


## Trackers

### Class `fourier_tracker`


## Binning types

### Class `radial`

### Class `layerwise_radial`


## Correlation functions

### Class `s110`


## Shape functions

### Class `const`

### Class `axis`