# QSS Solver Prototype/Experimental Code with FMU Support

This is a stand-alone QSS solver being developed for integration into JModelica as part of the "Spawn of EnergyPlus" project.

## Status

Currently the code has:
* QSS1/2/3 and LIQSS1/2 solvers.
* Linear and nonlinear derivative function support.
* Input variables/functions.
* Discrete-valued variables.
* Numeric differentiation support.
* A simple "baseline" event queue built on `std::multimap`.
* Simultaneous event support.
* Numeric bulletproofing of root solvers.
* A master algorithm with sampling and diagnostic output controls.
* A few simple code-defined example cases.
* Zero-crossing event support.
* Conditional if and when block framework.

Notes:
* Modelica input file processing is not provided: test cases are code-defined or loaded from FMUs.

## Plan

Planned development in anticipated sequence order are:
* FMU support extensions: Modelica annotations, JModelica/FMIL API migration, higher derivatives, ...
* Algebraic relationship/loop support.
* Extended precision time handling for large time span simulation.
* Performance refinements possibly including a higher performance event queue.
* Parallelization and vectorization.

## Goals

* High-performance QSS solver.
* Modular, object-oriented code.
* API suitable for JModelica integration.
* Define FMI extensions for efficient QSS support.
* FMU support.
* Support a mix of different QSS solvers.
* Support traditional discrete-time solvers.

## Design

The design concepts are still emerging. The basic constituents of a fast QSS solver seem to be:
* Variables for each QSS method (QSS1/2/3, LIQSS1/2/3, ...).
* Functions for derivative representation: linear and nonlinear.
* Event queue to find the next "trigger" variable to advance.
* Input variables/functions.
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
  * If not a self-observer the continuous representation trajectory doesn't change at requantization events. While the continuous representation could be advanced to re-sync segment start times with the requantized representation some efficiency is gained by not doing so. There are different precision impacts of each approach but with bulletproofing against small negative time steps due to finite precision there is probably no benefit to shifting the continuous representation when the trajectory doesn't change.
* Handles self-observer continuous representation updates specially instead of as part of general observer updates for efficiency:
  * Assigns continuous representation coefficients from the corresponding quantized representation during requantization instead of recomputing them.
* Input variable classes fit under the Variable hierarchy so that they can be processed along with QSS state variables.
* FMU variables that work through the FMI 2.0 API to get derivatives.

### Time Steps

#### Inflection Points

QSS methods can have trouble converging tightly on asymptotic values such as tails of exponential decay functions. This can be caused by the continuous representation reversing course and moving away from the quantized representation before the next quantum-based requantization event. An option to add inflection point requantization time steps has been built in to this package for QSS methods of order 2 and above to address this limitation. This will move up the next requantization time to the time when the continuous representation's next to highest derivative will pass through zero if the sign of that derivative in the quantized and continuous representations is the same at the start of the continuous representation time segment. (If the signs differ at the segment start then an inflection point would only improve the fit of the continuous and quantized representations, so we leave this case alone and let the normal quantization limits control the next time step.) So for (LI)QSS2 this finds the time where the slope is zero, and for QSS3 this finds the time when the second derivative is zero. If this time is positive and less than the computed quantum-based next requantization time then it is used as the next requantization time. In testing so far this has proven very effective in improving convergence at a modest cost of a few extra time steps but this should be evaluated in real-world cases.

Note that this differs from the literature. In some papers by the original QSS authors there is an additional time step recommended at the point when the QSS order derivative is zero. As noted by David Lorenzetti this is confusing because the Nth derivative of the continuous representation in a QSS order N method is a constant. Nevertheless, there may be some room for alternative approaches to improving QSS convergence.

#### Deactivation

The QSS time stepping logic looks at the magnitude of the highest order continuous representation derivative to set the next trigger time based on when the quantized and continuous representations will diverge by the selected quantum threshold. This does not always give a good indication of the divergence of the quantized representation from the actual/analytical value of QSS (ODE) state variables or input variables. For a QSS variable that is "isolated" (does not appear in many or any other variable's derivatives) or any input variable an artificially large time step causes the variable to "deactivate", becoming stuck in a fixed quantized and continuous representation.

Some model variables may have intrinsic deactivation where derivatives above some order are zero. For example, the velocity of a falling object in a (drag-free) gravitational field is linear so its derivatives of order 2 and up are zero. Such variables are represented accurately without any requantization so any approach to addressing deactivation should avoid wasting significant effort requantizing them.

Other variables might have zero derivatives at certain points, such as a variables with a derivative of `cos(t)` that has a zero second derivative at `t=0`. The situation of a zero derivative at model startup is probably not uncommon. Normally such variables will be observers of other variables that get requantized soon enough to avoid a solution glitch but this may not always be the case.

Deactivation is more of an issue for LIQSS methods because they set the highest derivative to zero when an interior point quantized representation start value occurs.

Deactivation appears to be a fairly serious flaw in the QSS approach. To address this a time step to use when deactivation occurs was added as an option (`dtInf`) and its use, especially for LIQSS methods is highly recommended. To minimize effort expended on intrinsically deactivated variables a "relaxation" approach is used where the `dtInf` time step is doubled on each successive call for the next requantization time that would otherwise give infinity. The concept of falling back to a lower-order QSS method for computing the next requantization time when deactivation occurs was found to be much less efficient and less effective.

A related issue arises when the highest order derivative has a very small magnitude, causing a very large time step to the next requantization to be computed. The `dtMax` value can be used to limit such large time steps but comes at some performance cost when the small derivative magnitude is intrinsic and sustained over large time spans. A different time step option that is like `dtMax` but only active when the highest derivative is non-zero and also relaxes upon successive use would probably be an effective approach.

### LIQSS

LIQSS as described in the literature is somewhat under-defined and inconsistent in some details. Some of the key issues and how they are addressed in this code are detailed below.

#### Cyclic/Order Dependency

At startup and simultaneous requantization trigger events the LIQSS approach defined in the literature is inadequate because the quantized values depend on derivatives which, in turn, depend on other quantized values. When multiple variables' quantized values need to be set at the same time point there is, in general, a cyclic dependency among them. Approaches that were considered:
* Single pass in arbitrary order: Leaves different representations of the same variable in the system and has a processing order dependency so results can be non-deterministic depending on the order of variables in their containers.
* Multiple passes hoping for a fixed point: May not find a consistent fixed point and is still potentially non-deterministic.
* Use derivatives based on a stable LIQSS state at these events to eliminate the order dependency. This is the approach chosen and a "simulanteous" representation is used that contains the stable coefficients determined for each order pass of the algorithm, without the updates made by the highest-order LIQSS pass. This approach has the potential for solution value and derivative discontinuities at transitions between simultaneous and non-simultaneous events but no clearly better alternative is apparent.

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

### Zero-Crossing Functions

So-called zero-crossing functions are used by Modelica for conditional behavior: when the function crosses zero an event needs to be carried out. This might be, for instance, a thermostat control reaching a trigger temperature that needs to turn on an A/C system. Zero crossings cause (discontinuous) changes to (otherwise) continuous and discrete variables via "handler" functions.

In traditional solvers a zero crossing is detected after it occurs and time backtracking may be used to find its precise time of crossing. With QSS we can do better by predicting the zero crossing using the QSS-style polynomial representation, optionally refining that prediction with an iterative root finder. The zero-crossing support is integrated with the QSS system as zero-crossing variables that are somewhat analogous to the QSS variables in their use of polynomial trajectories that requantize based on specified tolerances. The predicted zero crossings are events on the QSS event queue so they are handled efficiently without a need for backtracking.

Zero crossings introduce the potential for cyclic dependencies and a cascade of updates that occur at the same time point. Zero crossing events change some variable values. Because those changes are discontinuous this acts like a requantization event that must update both the quantized and continuous representation of those modified variables, which, in turn, can cause zero-crossing variables to update, possibly triggering another round of zero crossings, and so on. All of these events happen at the same (clock) time but they are logically sequentially triggered so we cannot know or process them simultaneously. To create a deterministic simulation with zero crossings the following approach is used:
* The phases of zero crossing, handler updates, and resulting requantizations are clustered together by the use of a "superdense time", which is a time value paired with a pass index and an event type ordering offset.
* In any pass with both new zero crossings and new requantizations the zero crossings are processed first (via superdense time indexing).
* In each phase multiple events of the same (zero crossing, handler, or requantization) type are handled together as simultaneous events: the processing for simultaneous events is phased so that the changes to interdependent variables are propagated deterministically.

With cyclic dependencies it is possible for an infinite cascade loop of such changes to occur at the same (clock) time. Detecting such dependencies and the occurrence of these infinite loops is necessary with zero crossing support.

### Input Functions

Input functions are external system "drivers" that do not depend on the state of other variables within the DAE system. The QSS solver implements input functions for continuous real-valued variables with QSS-like quantized and continuous representations and logic for selecting the next time at which to update these representations. Input variables using this approach are smoothly integrated with the QSS variable system.

Input functions can also have discrete events at which their value changes. To provide accurate behavior with QSS solvers, where time steps can be large and variable, these discrete events are assumed predictable by the input functions. These predicted discrete events are placed in the QSS event queue to assure they are processed at their exact event time. We don't currently support discrete events for continuous input functions but this is easily added if needed.

### Conditionals

Modelica supports conditional behavior via "if" and "when" blocks. The QSS solver has classes representing if and when conditional blocks to provide this behavior. Each conditional block has a sequence of clauses that represent the if/elseif/else and when/elsewhen structure of the corresponding Modelica conditional. Each clause (except the else clause) contains one or more boolean or zero-crossing variables. When fully realized clauses would be able to represent logical "trees" over their variables: for now only OR logic is supported.

## FMU Support

Models defined by FMUs following the FMI 2.0 API can be run by this QSS solver using QSS1 or QSS2 solvers. Discrete variables and zero crossing functions are supported. This is currently an initial/demonstration capability that cannot yet handle unit conversions (pure SI models are OK), or algebraic relationships. Some simple test model FMUs and a 50-variable room air thermal model have been simulated successfully.

Notes:
* Mixing QSS methods in an FMU simulation is not yet supported and will require a Modelica annotation to indicate QSS methods on a per-variable basis.
* The FMU support is performance-limited by the FMI 2.0 API, which requires expensive get-all-derivatives calls where QSS needs individual derivatives.
* QSS2 performance is limited by the use of numeric differentiation: the FMI ME 2.0 API doesn't provide higher derivatives but they may become available via FMI extensions.
* Zero crossings are problematic because the FMI spec doesn't expose the dependency of variables that are modified when each zero crossing occurs. Until such information is added our approach is to add the dependencies to the xml file. A fallback strategy of assuming that any continuous or discrete variable may have been modified could be used, at some cost to efficiency.
* QSS3 and LIQSS3 solvers can be added when they become more practical with the planned FMI Library FMI 2.0 API extensions.
* Input function evaluations will be provided by JModelica when QSS is integrated. For stand-alone QSS testing purposes a few input functions are provided for use with FMUs.
* Only SI units are supported in FMUs at this time as per LBNL specifications. Support for other units could be added in the future.

## Implementation

### Code Structure

Much of the source code is split into separate directories for the code-defined example models (`/dfn`) and FMU-based models (`/fmu`). While there is significant overlap between these codes there are a number of fundamental differences.

### Numeric Differentiatation

Higher derivatives are needed for QSS2+ methods. These are available analytically for linear functions and we provide them for the nonlinear function examples. In general analytical higher derivatives may not be provided by the model description or via automatic differentiation or may not be available across an FMU interface so we need some support for numeric differentiation. Numeric differentiation variants of some of the function classes were built to allow emulation of an environment where analytical higher derivatives are not available. A simple and fast approach suffices for this purpose:
* QSS2 does 2-point forward difference differentiation to allow reuse of one derivative evaulation.
* QSS3 does 2 and 3-point centered difference differentiation to allow reuse of derivative evaluations.
* Input variables of orders 2 and 3 are handled analogously to the corresponding QSS variable.

A mechanism to specify a per-variable differentiation time step is provided with defaulting to a global differentiation time step.

At startup and requantization events QSS3 with numeric differentiation has a cyclic dependency problem because the derivative function evaluations at time step offsets used to compute the higher derivatives are both used to set the q2 coefficient of the quantized representation but also depend on q2. The QSS literature does not offer a robust solution and none is implementated at this point. An iterative approach could be used to find a fixed point solution for a stable q2 value but this would require a number of additional derivative evaluations. The impact of this flaw will vary across models and could be severe in some situations so it should be addressed if numeric differentiation will be used in the production JModelica+QSS system.

### Numeric Bulletproofing

The time advance functions solve for the roots of polynomials of the QSS method order to see where the continuous representation next crosses the quantized representation +/-Q boundaries. The naive approach is to just pick the smallest positive root that the root solver finds, but with finite precision computations it is possible for this to select the "wrong" root that allows the continuous representation trajectory to fall outside the quantum band. To protect against this we take a few steps:
* Roots that cross the boundary in the wrong direction (moving from outside to inside) are culled out. The crossing direction tests have some computational cost: in the unlikely event that this is significant a "fast and dangerous" mode could be provided that omits them.
* When a numeric precision loss causes a bad solution we exploit knowledge of the actual behavior to produce a zero time step rather than an infinite step.
* When the polynomial coefficients indicate which boundary must be hit first we save time by only root solving on that boundary and we exploit the known coefficient signs.
* When the polynomial coefficients don't clearly show which boundary will be hit first we process the boundaries together to exploit knowledge that at least one of them should have a positive root with the correct, outward-crossing direction.

### Zero-Crossing Support

Zero crossing implementation must deal with some obstacles:
* Some combinations of zero-crossing functions and QSS method orders cannot predict zero crossings, such as a zero-crossing function that is simply the value of a QSS1 variable (whose quantized representation is always a constant). The QSS system will detect zero crossings during observer updates of the zero-crossing variables but these will lag the precise zero crossing times.

Some of the tasks remaining for a robust, production-quality implementation are:
* Detect and warn about cyclic dependencies that could lead to infinite update cascades.
* Detect the occurrence of infinite update cascades and terminate with an error.
* Zero-crossing root finding is a complicated for the case of large tolerance and/or complex zero-crossing functions, where multiple zero crossings might occur within the active quantization time segment. A combination of long and short range algorithms is needed to minimize the possibility of missing roots. Careful development of such an approach is recommended for production use. The current code is only robust for small tolerances and sufficiently simple zero-crossing functions that the continuous trajectory representation is able to localize the root to good accuracy and only one root is present in any quantized time segment.
* LIQSS variables present a problem with zero crossings since the quantized and continuous representations do not generally start at the same value in a requantization event. To avoid this LIQSS handler advances set the zero order quantized and continuous coefficients to the same value rather than using the normal LIQSS behavior.
* Zero-crossing "chattering" can occur for some models so a threshold mechanism to avoid this should be added.
* Startup behavior is not well-handled by zero-crossings: the initial model state may put the zero-crossing function in any of its conditions but no zero crossing occurred to get there so it isn't detected. Think of a bouncing ball model where the ball starts on or below the floor: no crossing from above the floor occurs so the ball will fall to negative infinity unless a startup procedure is used. A set of rules and a mechanism for dealing with startup state of zero crossings needs to be developed.

Bulletproofing present in the implementation:
* Attempting to change the same variable to different values in two zero-crossing handlers in the same superdense time pass is detected as a (non-fatal) error.
* Crossing type/direction is part of the zero-crossing event so that handlers can ignore irrelevant crossings.
* When zero crossing events cluster up closer and closer (such as a bouncing ball) eventually time may not progress and an infinite loop can occur. The bball model shows how a handler should be designed to detect and address this. For Modelica-based models such a set of rules may not be possible in which case the chattering prevention will be needed.

FMU zero crossing support has some additional complications and limitations:
* Zero-crossing functions are implicitly defined by if and when blocks in the Modelica file and are not directly exposed via the FMI API. Until this is addressed we have created a convention of defining output variables and their derivatives for each zero-crossing function with names of the form \_\_zc\__name_ and \_\_zc\_der\__name_.
* The FMI API doesn't expose crossing directions of interest so we enable all of them. If this will never be available we should eliminate crossing check logic to avoid wasted effort.
* When zero crossings occur a "handler" function within the FMU makes the required changes to state and discrete variables. The QSS solver needs to know the dependency of these modified variables on its zero-crossing variable to maintain its state but neither the FMI API nor the generated xml files expose these dependencies. We are adding such dependencies to `DiscreteStates` (dropped from FMI 2.0 but supported by FMI Library) of the xml for discrete variables and `InitialUnknowns` for continuous state variables.
* There is no FMI API to trigger the zero-crossing handlers to run when the QSS solver reaches zero-crossing events. Instead we set the FMU variables to a time slightly beyond the zero-crossing time with the hope that the zero crossing will be detected by the FMU. The `dtZC` option allows control over this "step". This uniform step size is not robust as it doesn't adjust for solution behavior. This is also not highly robust because the output variables used to track the zero crossing derivatives are (at least for Dymola-generated FMUs) numerically, not analytically, based so the QSS zero-crossing function does not track the actual FMU zero-crossing function to high precision.
* Zero-crossing root refinement is not practical with the current FMI limitations so it has been disabled. Once we have atomic variable get/set FMU operations root refinement should be enabled.

Notes:
* Zero-crossing variables should not need and are required to have no observers (but they do have dependents: the variables modified by their handlers).
* Zero-crossing variables must initialize and advance after other variables because their zero-order coefficients come from their function evaluation that depends on the zero-order coefficients of other variables being advanced.
* Zero-crossing variables use the quantized, not continuous, representations of variables appearing in their functions. This is what the literature indicates and it prevents additional updating operations and potential event cascades. But it also provides a lower accuracy representation, making root finding and refinement more critical. It also means that QSS1 variables appearing linearly in zero-crossing functions only contribute a constant to the function at any time: if all variables are QSS1 and/or discrete then no zero crossings can be predicted so zero crossings will only occur as a result of observer updates of the zero-crossing variable. For this reason QSS1 is not suggested when zero crossings are in use.

### Conditionals

The QSS solver behavior for processing conditionals is:
* At events when zero-crossing or boolean variables in clauses become true (and false for if block clauses) then notify their clauses to put the conditional event on the queue at that time.
* When conditional events are processed the clauses are evaluated to see which of them are active at that time (and superdense time pass).
* Active clauses place events for their handlers on the queue at the same time and pass.
* Handler events are processed to make the necessary variable updates.

Conditional handler functions can cause variable changes that cause other (zero-crossing, requantization, ...) events so the processing order can affect results. To assure deterministic results, each event type has an offset number that is used as an additional field in the superdense time to assure that discrete, zero-crossing, conditional, and handler events are processed in groups in that order in each pass. Variables modified inconsistently by handlers in the same pass are detected and warnings generated since this indicates a problematic model that can produce processing order-dependent results.

## Performance

Once the code capabilities are sufficient and larger models are built some performance assessments will be carried out. Run time comparisons _vs._ [Qss Solver](https://sourceforge.net/projects/qssengine/), [Ptolemy](http://ptolemy.eecs.berkeley.edu/), and other implementations will be performed. Profiling will be used to identify bottlenecks in performance and in scalability as problem size grows. Performance tuning and experimentation with alternative implementation designs are anticipated. Specifically, evaluation of alternative event queue designs is likely to be worthwhile, especially once parallel processing is added.

### Performance Findings

Performance findings and observations:
* Simultaneous requantization triggering: Could skip continuous representation update if a variable is not an observer of any of the requantizing variables. This would save assignments but more importantly evaluation of the highest derivative. There is some overhead in determining whether a variable qualifies. Testing so far doesn't show a significant benefit for this optimization but it should be reevaluated with real-world cases where simultaneous triggering is common.

### Performance Notes

* Variable hierarchy virtual calls can be reduced via some refactoring.
* FMU performance is currently severely hobbled by the FMI 2.0 API that is ill-suited to QSS simulation and the need for numeric differentiation until higher derivatives become available via FMI extensions.

## Testing

* Case runs are being compared with results from [Qss Solver](https://sourceforge.net/projects/qssengine/) and [Ptolemy](http://ptolemy.eecs.berkeley.edu/) for now.
* Unit tests are included and will be extended for wider coverage as the code progresses.

## Building

Instructions for building on different platforms follows.

### Linux

Preparation:
* You will need a build of the latest [FMI Library](http://www.jmodelica.org/FMILibrary).
* Copy `bin/Linux/<compiler>/setFMIL` to a directory in your PATH and adapt it to the location of you FMI Library installation.
* The unit tests use googletest. The `setGTest` scripts under `\bin\Windows` set up the necessary environment variables to find googletest: put a custom version of `setGTest` in your `PATH` to adapt it to your system.

To build the QSS prototype on Linux:
* `cd <path_to_repository>`
* `source bin/Linux/GCC/64/r/setProject` for release builds or `source bin/Linux/GCC/64/d/setProject` for debug builds
* `cd src/QSS`
* `mak` to build the demo from the QSS.cc main (add a -j*N* make argument to override the number of parallel compile jobs)
* `./QSS` to run the demo

To run the unit tests on Linux:
* The unit tests are in the `tst/QSS/unit` directory and can be built and run with the command `mak run`.

### Windows

Preparation:
* Copy `bin\Windows\<compiler>\setFMIL.bat` to a directory in your PATH and adapt it to the location of you FMI Library installation.
* The unit tests use googletest so you will need to have a googletest build for your complier available. The `setGTest.bat` scripts under `\bin\Windows` set up the necessary environment variables to find googletest: put a custom version of `setGTest.bat` in your `PATH` to adapt it to your system.

To build the QSS prototype on Windows:
* `cd <path_to_repository>`
* `bin\Windows\VC\64\r\setProject` for VC++ release builds or `bin\Windows\VC\64\d\setProject` for VC++ debug builds (or similarly for GCC or IC for Intel C++)
* `cd src\QSS`
* `mak` to build the demo from the QSS.cc main (add a -j*N* make argument to override the number of parallel compile jobs)
* `QSS` to run the demo

To run the unit tests on Windows:
* The unit tests are in the `tst\QSS\unit` directory and can be built and run with the command `mak run`.

## Running

The QSS solver code can run both code-defined and FMU-based test cases.

There are command line options to select the QSS method, set quantization tolerances, output and differentiation time steps, and output selection controls.
* Relative tolerance is taken from the FMU if available by default but can be overridden with a command line option.
* QSS variable continuous and/or quantized trajectory values can be output at their requantization events.
* QSS variable continuous and/or quantized trajectory output at a regular sampling time step interval can be enabled.
* FMU outputs can be generated for FMU model runs.
* Diagnostic output can be enabled, which includes a line for each quantization-related variable update.

Run `QSS --help` to see the command line usage.
