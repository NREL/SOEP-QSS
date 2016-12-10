# QSS Solver Prototype/Experimental Code

This is a stand-alone QSS solver being developed for integration into JModelica as part of the "Spawn of EnergyPlus" project.

## Status

Currently the code has:
* QSS1/2/3 and LIQSS1/2 solvers.
* Linear derivative function support.
* Nonlinear derivative function support/examples for QSS1/2/3.
* A simple "baseline" event queue built on `std::multimap`.
* Simultaneous requantization event support.
* A master algorithm with sampling and diagnostic output controls.
* A few simple test cases.

Notes:
* No Modelica input file processing is supported: test cases are hard-coded.
* Input functions, algebraic relationship, and discrete variables will be added soon.

## Plan

Planned development in anticipated sequence order are:
* LIQSS2/3 with nonlinear function support/example for LIQSS solvers.
* FMI interface.
* Numeric differentiation option to emulate the JModelica environment.
* Discrete-valued variables (zero-crossing functions).
* Vector-valued variables.
* Numerical bulletproofing:
  * (LI)QSS2+ solver requantization at highest derivative sign change events.
  * Non-positive time steps or wrong root returned from root solvers
* Extended precision time handling for large time span simulation
* Higher performance event queue
* Parallelization and vectorization

## Goals

* High-performance QSS solver.
* API suitable for JModelica integration.
* Support a mix of different QSS solvers.
* Support traditional discrete-time solvers.

## Design

Design concepts are just emerging during these early experiments.
The basic constituents of a fast QSS solver seem to be:
* Variables for each QSS method (QSS1/2/3, LIQSS1/2/3, ...).
* Event queue to find the next "trigger" variable to advance to its requantization time.
* Linear and nonlinear derivative function support.
* Continuous and discrete valued variables.
* Input functions.
* Algebraic relationships between variables.

Notes:
* For efficiency variables handle their own integration and quantization operations so we don't consider those as separate entities.
* Parallel updating of variables dependent on the trigger variable is anticipated.
* Priority queues with good concurrency and cache efficiency is a wide research topic: if the event queue is found to be a bottleneck experiments with advanced concepts are planned.

### Variable

* Hierarchy typed by the QSS solver method: This brings in some virtual functions that could be a performance bottleneck.
* Integration and quantization are handled internally to avoid the cost of calls to other objects and passing of data packets between them.
* Holds the iterator of its entry in the event queue to save one _O_( log N ) lookup.
* Supports mix of different QSS method variables in the same model.
* Flags whether its derivative depends on its own value (self-observer) and uses that for efficiency:
  * If not a self-observer the continuous representation trajectory doesn't change at requantization events.
    While the continuous representation could be advanced to re-sync segment start times with the requantized representation some efficiency is gained by not doing so.
    There are different precision impacts of each approach but with bulletproofing against small negative time steps due to finite precision there is probably no benefit to shifting the continuous representation when the trajectory doesn't change.
* Handles self-observer continuous representation updates specially instead of as part of general observer updates for efficiency:
  * Assigns continuous representation coefficients from the corresponding quantized representation during requantization instead of recomputing them.

### LIQSS

LIQSS as described in the literature is somewhat under-defined and inconsistent in some details. Some of the key issues and how they are addressed in this code are detailed below.

#### Cyclic Dependency

At startup and simultaneous requantization trigger events the LIQSS approach defined in the literature is inadequate because the quantized values depend on derivatives which, in turn, depend on other quantized values.
When multiple variables' quantized values need to be set at the same time point there is, in general, a cyclic dependency among them. Approaches that were considered:
* Single pass in arbitrary order: Leaves different representations of the same variable in the system and has a processing order dependency so results can be non-deterministic depending on how variables are held in containers.
* Multiple passes hoping for a fixed point: May not find a consistent fixed point and is still potentially non-deterministic.
* Use derivatives evaluated for the continuous, not quantized, representation at these events: Since the continous representation value is set first this allows a single pass, deterministic treatment. This is the approach used here.

Even when using the continuous representation in the LIQSS derivatives issues remain:
* LIQSS1: The LIQSS1 variables need to be processed first in the first derivative processing pass since it alters their quantized value terms.
* LIQSS2: Because the second derivative pass alters their quantized value and slope terms there is no way to avoid the potential for cyclic dependencies.
  The impact is controlled by setting the neutral (centered) values and first derivatives in those passes but this still allows other variable derivatives to be using these non-final values.
  The exposure is reduced somewhat by processing the LIQSS2 variables first in the second derivative pass.
  No ideal solution has been found for this LIQSS2+ limitation.
  It is possible that when LIQSS2+ variables are present simultaneous triggering would be better handled as a sequence of separate trigger events at very closely spaced time steps: this is a good research topic.

### Event Queue

* C++ `std::priority_queue` doesn't support changing the key value so it isn't a suitable out-of-the-box solution: It may be worth trying to work around this limitation with supplementary methods.
* A simple version built on `std::multimap` was added as a starter/baseline but more efficient approaches are planned.
* Simultaneous trigger events are handled as a special case since correct operation sequencing requires more virtual method calls.
* Boost `mutable_queue` and `d_ary_hoop_indirect` may be worth experimenting with.
* There are many research papers about priority queues with good scalability, concurrency, and/or cache efficiency, with a seeming preference for skip list based designs: These should be evaluated once we have large-scale real-world cases to test.

### Function

* Linear functions are provided for QSS and LIQSS solvers.
* Sample nonlinear functions are included.
* We'll need a general purpose function approach for the JModelica-generated code: probably a function class that calls back to a provided function.

## Performance

Once the code capabilities are sufficient and larger models are built some performance assessments will be carried out.
Run time comparisons _vs._ [Qss Solver](https://sourceforge.net/projects/qssengine/), [Ptolemy](http://ptolemy.eecs.berkeley.edu/), and other implementations will be performed.
Profiling will be used to identify bottlenecks in performance and in scalability as problem size grows.
Performance tuning and experimentation with alternative implementation designs are anticipated.
Specifically, evaluation of alternative event queue designs is likely to be worthwhile, especially once parallel processing is added.

### Performance Findings

Performance findings and observations:
* Simultaneous requantization triggering:
  * Could skip continuous rep update if a variable is not an observer of any of the requantizing variables.
    This would save assignments but more importantly evaluation of the highest derivative.
    There is some overhead in determining whether a variable qualifies.
    Testing so far doesn't show a significant benefit for this optimization but it should be reevaluated with real-world cases where simultaneous triggering is common.

## Testing

* Case runs are being compared with results from [Qss Solver](https://sourceforge.net/projects/qssengine/) and [Ptolemy](http://ptolemy.eecs.berkeley.edu/) for now.
* Unit tests are included and will be extended for wider coverage as the code progresses.

## Running

The QSS solver code can be built to run test cases and unit tests.
Edit QSS.cc to enable or add example cases.
The options near the top control whether uniform sampled time step outputs are generated, the time step size for that, and whether outputs occur for all variables when any variable requantizes.
There is also an option to enable diagnostic output, which includes a line for each update event.
Instructions for building on different platforms follows.

### Linux

To build the QSS prototype on Linux:
* `cd <path_to_repository>`
* `source bin/Linux/GCC/64/r/setProject` for release builds or `source bin/Linux/GCC/64/d/setProject` for debug builds
* `cd src/QSS`
* `mak -j8` to build the demo from the QSS.cc main (adjust -j value for the number of parallel compiles like make)
* `./QSS` to run the demo

To run the unit tests on Linux:
* The unit tests are in the `tst/QSS/unit` directory and can be built and run with a command like `mak -j8 run`.
* The unit tests use googletest. The `setGTest` scripts under `\bin\Windows` set up the necessary environment variables to find googletest: put a custom version of `setGTest` earlier in your `PATH` to adapt it to your system.

### Windows

To build the QSS prototype on Windows:
* `cd <path_to_repository>`
* `bin\Windows\VC\64\r\setProject` for VC++ release builds or `bin\Windows\VC\64\d\setProject` for VC++ debug builds (or similarly for GCC or IC for Intel C++)
* `cd src\QSS`
* `mak -j8` to build the demo from the QSS.cc main (adjust -j value for the number of parallel compiles like make)
* `QSS` to run the demo

To run the unit tests on Windows:
* The unit tests are in the `tst\QSS\unit` directory and can be built and run with a command like `mak -j8 run`.
* The unit tests use googletest. The `setGTest.bat` scripts under `\bin\Windows` set up the necessary environment variables to find googletest: put a custom version of `setGTest.bat` earlier in your `PATH` to adapt it to your system.
