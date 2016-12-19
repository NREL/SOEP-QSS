# QSS Solver Prototype/Experimental Code

This is a stand-alone QSS solver being developed for integration into JModelica as part of the "Spawn of EnergyPlus" project.

## Status

Currently the code has:
* QSS1/2/3 and LIQSS1/2 solvers.
* Linear and nonlinear derivative function support.
* Input variables/functions.
* Numeric differentiation support.
* A simple "baseline" event queue built on `std::multimap`.
* Simultaneous requantization event support.
* A master algorithm with sampling and diagnostic output controls.
* A few simple test cases.

Notes:
* No Modelica input file processing is supported: test cases are hard-coded.
* Input functions, algebraic relationship, and discrete variables will be added soon.

## Plan

Planned development in anticipated sequence order are:
* FMI interface.
* Discrete-valued variables (zero-crossing functions).
* Vector-valued variables.
* Numerical bulletproofing:
  * Non-positive time steps or wrong root returned from root solvers.
* Extended precision time handling for large time span simulation.
* Higher performance event queue.
* Parallelization and vectorization.

## Goals

* High-performance QSS solver.
* API suitable for JModelica integration.
* Support a mix of different QSS solvers.
* Support traditional discrete-time solvers.

## Design

The design concepts are still emerging.
The basic constituents of a fast QSS solver seem to be:
* Variables for each QSS method (QSS1/2/3, LIQSS1/2/3, ...).
* Functions for derivative representation: linear and nonlinear.
* Input variables/functions.
* Event queue to find the next "trigger" variable to advance to its requantization time.
* Continuous and discrete valued variables.
* Algebraic relationships between variables including handling of algebraic loops.

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
* Input variable classes fit under the Variable hierarchy so that they can be processed along with QSS state variables.

### Time Steps

#### Inflection Points

QSS methods can have trouble converging tightly on asymptotic values such as tails of exponential decay functions.
This can be caused by the continuous representation reversing course and moving away from the quantized representation before the next quantum-based requantization event.
An option to add inflection point requantization time steps has been built in to this package for QSS methods of order 2 and above to address this limitation.
This will move up the next requantization time to the time when the continuous representation's next to highest derivative will pass through zero if the sign of that derivative in the quantized and continuous representations is the same at the start of the continuous representation time segment.
(If the signs differ at the segment start then an inflection point would only improve the fit of the continuous and quantized representations, so we leave this case alone and let the normal quantization limits control the next time step.)
So for (LI)QSS2 this finds the time where the slope is zero, and for QSS3 this finds the time when the second derivative is zero.
If this time is positive and less than the computed quantum-based next requantization time then it is used as the next requantization time.
In testing so far this has proven very effective in improving convergence at a modest cost of a few extra time steps but this should be evaluated in real-world cases.

Note that this differs from the literature.
In some papers by the original QSS authors there is an additional time step recommended at the point when the QSS order derivative is zero.
As noted by David Lorenzetti this is confusing because the Nth derivative of the continuous representation in a QSS order N method is a constant.
Nevertheless, there may be some room for alternative approaches to improving QSS convergence.

#### Deactivation

The QSS time stepping logic looks at the magnitude of the highest order continuous representation derivative to set the next trigger time based on when the quantized and continuous representations will diverge by the selected quantum threshold.
This does not always give a good indication of the divergence of the quantized representation from the actual/analytical value of QSS (ODE) state variables or input variables.
A trivial example is f(t) = t<sup>n</sup> : at t=0 the value and all derivatives other than the n<sup>th</sup> are zero, so only an n<sup>th</sup> order method will avoid setting the next trigger time to infinity.
For a QSS variable that is "isolated" (does not appear in many or any other variable's derivatives) or any input variable an artificially large time step causes the variable to "deactivate", becoming stuck in a fixed quantized and continuous representation.

This appears to be a fairly serious flaw in the QSS approach.
An elegant, robust and efficient solution to this is not yet obvious.
As a backstop solution a max time step field was added to the implementation and its use is strongly suggested for input and isolated QSS variables.
In a production implementation it would probably make sense for each function class to provide an API to set a max time step that makes sense for that function.
Finding a better, more automated solution would be a worthwhile investigation.

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
* A numeric differentiating linear function is provided for QSS solvers.
* Sample nonlinear functions are included.
* Sample input variable functions with analytical and numeric derivatives are included.
* We'll need a general purpose function approach for the JModelica-generated code: probably a function class that calls back to a provided function.

## Implementation

### Numeric Differentiatation

Higher derivatives are needed for QSS2+ methods.
These are available analytically for linear functions and we provide them for the nonlinear function examples.
In general analytical higher derivatives may not be provided by the model description or via automatic differentiation or may not be available across an FMU interface so we need some support for numeric differentiation.
Numeric differentiation variants of some of the function classes were built to allow emulation of an environment where analytical higher derivatives are not available.
A simple and fast approach suffices for this purpose:
* QSS2 does 2-point forward difference differentiation to allow reuse of one derivative evaulation.
* QSS3 does 2 and 3-point centered difference differentiation to allow reuse of derivative evaluations.
* Input variables of orders 2 and 3 are handled analogously to the corresponding QSS variable.

A mechanism to specify a per-Variable differentiation time step is provided with defaulting to a global differentiation time step.

At startup and requantization events QSS3 with numeric differentiation has a cyclic dependency problem because the derivative function evaluations at time step offsets used to compute the higher derivatives are both used to set the q2 coefficient of the quantized representation but also depend on q2.
The QSS literature does not offer a robust solution and none is implementated at this point.
An iterative approach could be used to find a fixed point solution for a stable q2 value but this would require a number of additional derivative evaluations.
The impact of this flaw will vary across models and could be severe in some situations so it should be addressed if numeric differentiation will be used in the production JModelica+QSS system.

### Numeric Bulletproofing

The time advance functions solve for the roots of polynomials of the QSS method order to see where the continuous representation next crosses the quantized representation +/-Q boundaries.
The naive approach is to just pick the smallest positive root that the root solver finds, but with finite precision computations it is possible for this to select the "wrong" root that allows the continuous representation trajectory to fall outside the quantum band.
To protect against this we take a few steps:
* Roots that cross the boundary in the wrong direction (moving from outside to inside) are culled out.
  The crossing direction tests have some computational cost: in the unlikely event that this is significant a "fast and dangerous" mode could be provided that omits them.
* When a numeric precision loss causes a bad solution we exploit knowledge of the actual behavior to produce a zero time step rather than an infinite step.
* When the polynomial coefficients indicate which boundary must be hit first we save time by only root solving on that boundary and we exploit the known coefficient signs.
* When the polynomial coefficients don't clearly show which boundary will be hit first we process the boundaries together to exploit knowledge that at least one of them should have a positive root with the correct, outward crossing direction.

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

### Performance Notes

* The Variable hierarchy (and thus its virtual calls) can be eliminated by the use of variadic templates and tuples in the Function classes.
  This is probably a worthwhile refactor but should wait for the performance test suite so we can gauge its benefit.

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
