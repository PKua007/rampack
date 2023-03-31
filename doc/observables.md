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

as well as [Trackers](#trackers), which are in a separate section.

### Class `number_density`

### Class `box_dimensions`

### Class `packing_fraction`

### Class `compressibility_factor`

### Class `energy_per_particle`

### Class `energy_fluctuations_per_particle`

### Class `nematic_order`

### Class `smectic_order`

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