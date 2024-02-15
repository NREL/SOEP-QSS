# QSS Solver Prototype/Experimental Code with FMU Support

This is a QSS solver being developed for integration into Modelon's Optimica Compiler Toolkit (OCT) as part of the "Spawn of EnergyPlus" project.

## Status

Currently the code has:
* QSS1/2/3 and LIQSS1/2/3 solvers.
* fQSS solvers that broadcast full-order quantized representations.
* Experimental continuous trajectory broadcasting (x builds).
* Experimental relaxation solvers for the highly sensitive derivatives as seen in many Buildings Library models.
* Linear and nonlinear derivative function support.
* Input variables/functions.
* Discrete-valued variables.
* Numeric differentiation support with specified or automatic ND time step.
* Zero-crossing event support via OCT event indicator variables.
* Conditional if and when block framework.
* A simple "baseline" event queue built on `std::multimap`.
* Simultaneous event support that produces deterministic (non-order-dependent) results.
* Numeric bulletproofing of root solvers.
* A master algorithm with sampling and diagnostic output controls.
* FMU for Model Exchange simulation support.
* FMU-QSS generation and simulation support.
* Connected FMU-ME and FMU-QSS simulation support.
* Binned-QSS support.
* Experimental Python wrapping with Pybind11.

## Plan

Planned development in anticipated sequence order are:
* FMU support extensions: Modelica annotations, OCT/FMIL API migration, higher derivatives, ...
* Extended precision time handling for large time span simulation.
* Performance improvements.
* Parallelization and vectorization.

## Goals

* High-performance QSS-over-FMU solver.
* Support a mix of different QSS solvers.
* Modular, object-oriented code.
* Identify potential future FMI API extensions for more efficient and robust QSS support.

## Solvers

QSS has a number of solvers that are variations on the base 1st, 2nd, and 3rd order solvers:
* QSS1/2/3:  QSS solvers for non-stiff models
* LIQSS1/2/3:  Linearly-implicit QSS solvers for stiff models

The variation categories are:
* f: Full-order quantized representation propagation
* n: Numerical differentiation for state second derivatives (instead of analytical values from directional derivatives) for models that have high dependency density (causing slow directional derivative calls) and that are sufficiently numerically stable
* i: LIQSS solvers using interpolation for derivatives at trajectories starting within the +/- Q tolerance band in place of more expensive directional derivatives
* r: Relaxation solvers for models with high derivative sensitivity that can cause traditional QSS solver inefficient "yo-yoing" behavior with very small time steps
  * Inflection point steps are enabled with the relaxation solvers

The current solvers, grouped by order, are shown below.

### Order 1 Solvers
* QSS1
* fQSS1
* LIQSS1
* fLIQSS1
* iLIQSS1
* ifLIQSS1

### Order 2 Solvers
* QSS2
* fQSS2
* rQSS2
* rfQSS2
* nQSS2
* nfQSS2
* nrQSS2
* nrfQSS2
* LIQSS2
* fLIQSS2
* iLIQSS2
* ifLIQSS2
* nLIQSS2
* nfLIQSS2
* niLIQSS2
* nifLIQSS2

### Order 3 Solvers
* QSS3
* fQSS3
* rQSS3
* rfQSS3
* nQSS3
* nfQSS3
* nrQSS3
* nrfQSS3
* LIQSS3
* fLIQSS3
* iLIQSS3
* ifLIQSS3
* nLIQSS3
* nfLIQSS3
* niLIQSS3
* nifLIQSS3

## Design

The basic design concepts for a fast QSS-over-FMU solver seem to be:
* Variables for each QSS method (QSS1/2/3, LIQSS1/2/3, ...).
* Event queue to find the next "trigger" variable to advance.
* Input variables/functions.
* Continuous and discrete valued variables.
* Call pooling to amortize the expensive FMU operations.
* Binning: Performing simultaneous requantization of variables due to requantize near the current requantization event time to amortize the cost of FMU operations.

### Notes

* For efficiency variables handle their own integration and quantization operations so we don't consider those as separate entities.
* Parallel updating of observer variables on trigger variable events has the potential for performance benefits.
* Priority queues with good concurrency and cache efficiency is a wide research topic: if the event queue is found to be a bottleneck experiments with advanced concepts are planned.

### Variable

* Hierarchy typed by the QSS solver method.
* Integration and quantization are handled internally to avoid the cost of calls to other objects and passing of data packets between them.
* Holds the iterator of its entry in the event queue to save one _O_( log N ) lookup.
* Supports mix of different QSS method variables in the same model.
* Input, zero-crossing, and other non-continuous-state variable classes fit under the Variable hierarchy so that they can be processed along with QSS continuous-state variables.
* Variables use the FMI 2.0 API to get derivatives.

### Time Steps

#### Deactivation

The QSS time stepping logic depends on the magnitude of the highest order continuous representation derivative to set the next trigger time based on when the quantized and continuous representations will diverge by the selected quantum threshold. This does not always give a good indication of the divergence of the quantized representation from the actual/analytical value of QSS (ODE) state variables or input variables. For a QSS variable that is "isolated" (does not appear in many or any other variable's derivatives) or any input variable an artificially large time step causes the variable to "deactivate", becoming stuck in a fixed quantized and continuous representation.

Some model variables may have intrinsic deactivation where derivatives above some order are zero. For example, the velocity of a falling object in a (drag-free) gravitational field is linear so its derivatives of order 2 and up are zero, and the displacement is quadratic so its order 3 and up derivatives are zero. Such variables are represented accurately without any requantization so any approach to addressing deactivation should avoid wasting significant effort requantizing them.

Other variables might have zero derivatives at certain points, such as a variables with a derivative of `cos(t)` that has a zero second derivative at `t=0`. The situation of a zero derivative at model startup is probably not uncommon. Normally such variables will be observers of other variables that get requantized soon enough to avoid a significant solution glitch but this may not always be the case.

Deactivation is more of an issue for LIQSS methods because they set the highest derivative to zero when an interior point quantized representation start value occurs.

Deactivation appears to be a fairly serious flaw in the QSS approach. To address this a time step to use as a threshold for detecting deactivation was added as an option (`dtInf`) and its use, especially for LIQSS methods, is highly recommended. (Due to numeric differentiation deactivation may cause a very smll, but not zero, highest derivative so we use a time step threshold.) To minimize effort expended on intrinsically deactivated variables a "relaxation" approach is used where the `dtInf` time step is doubled on each successive requantization while deactivation is detected. The concept of falling back to a lower-order QSS method for computing the next requantization time when deactivation occurs was found to be much less efficient and effective.

#### Time Step Limits

QSS time stepping can be ridigly limited with the `dtMin` and `dtMax` options. While these should not be needed with typical QSS simulations they can be useful for investigating unexpected simulation results. Unlike `dtInf` the `dtMin` and `dtMax` options do not "relax" but remain as fixed limits.

#### Inflection Points

QSS methods can have trouble converging tightly on asymptotic values such as tails of exponential decay functions. This can be caused by the continuous representation reversing course and moving away from the quantized representation before the next quantum-based requantization event. An option to add inflection point requantization time steps has been built in to this package for QSS methods of order 2 and above to address this limitation. This will move up the next requantization time to the time when the continuous representation's next to highest derivative will pass through zero if the sign of that derivative in the quantized and continuous representations is the same at the start of the continuous representation time segment. (If the signs differ at the segment start then an inflection point would only improve the fit of the continuous and quantized representations, so we leave this case alone and let the normal quantization limits control the next time step.) So for (LI)QSS2 this finds the time where the slope is zero, and for QSS3 this finds the time when the second derivative is zero. If this time is positive and less than the computed quantum-based next requantization time then it is used as the next requantization time. In testing so far this has proven very effective in improving convergence at a modest cost of a few extra time steps but this should be evaluated in real-world cases.

Note that this differs from the literature. In some papers by the original QSS authors there is an additional time step recommended at the point when the QSS order derivative is zero. As noted by David Lorenzetti this is confusing because the Nth derivative of the continuous representation in a QSS order N method is a constant. Nevertheless, there may be some room for alternative approaches to improving QSS convergence.

### LIQSS

LIQSS as described in the literature is somewhat under-defined and inconsistent in some details. Some of the key issues and how they are addressed in this code are detailed below.

#### Cyclic/Sequence Dependency

At startup and simultaneous requantization trigger events the LIQSS approach defined in the literature is inadequate because the quantized values depend on derivatives which, in turn, depend on other quantized values. When multiple variables' quantized values need to be set at the same time point there is, in general, a cyclic dependency among them. Approaches that were considered:
* Single pass in arbitrary order: Leaves different representations of the same variable in the system and thus has a variable processing sequence dependency so results are dependent on the (arbitrary) ordering of variables in their containers.
* Multiple passes hoping for a fixed point: May not find a consistent fixed point and is still potentially order-dependent.
* Use derivatives based on a stable LIQSS state at these events to eliminate the sequence dependency. This is the approach chosen and a "simultaneous" representation is used that contains the stable coefficients determined for each order pass of the algorithm, without the updates made by the highest-order LIQSS pass. This approach has the potential for solution value and derivative discontinuities at transitions between simultaneous and non-simultaneous events but no clearly better alternative is apparent.

#### Computed vs. Interpolated

The default LIQSS solvers compute, rather than interpolate, the derivatives at the quantized trajectory values that the LIQSS algorithms select. While this has accuracy and performance benefits, it adds some overhead that is worse in higher order LIQSS solvers, especially for those that use directional second derivatives. Because these are per-variable computations, FMU call pooling cannot be used to amortize their cost. Additional `iLIQSS` solvers that use interpolation are provided and can be used when their lower accuracy is a beneficial tradeoff for the faster requantizations.

### fQSS Variant

A variant of the QSS method with methods named `fQSS1`, `fQSS2`, and `fQSS3` has been implemented. In this variant the "full-order" broadcast (quantized) representation has the same order as the internal (continuous) representation. The concept is based on using all available knowledge of the variable trajectory in the broadcast representation. This has been found to provide significantly higher accuracy solutions at the same tolerance (or, equivalently, faster solutions at the same accuracy) for some models.

More experiments are needed to develop best practices for when to try fQSS but experience to date has yielded these preliminary guidelines:
* Smooth trajectory models with precise or analytical derivatives tend to benefit most from fQSS methods
* Numeric differentiation that the current FMU-based simulation requires injects some noise into the extra top order term of the fQSS quantized representations, which tends to shorten the time steps and thus hurt performance.
* LIQSS methods on stiff systems did not benefit from fLIQSS since it tends to put the quantum-shifted quantized representation farther from the continuous representation.

### Event Queue

* C++ `std::priority_queue` doesn't support changing the key value so it isn't a suitable out-of-the-box solution: It may be worth trying to work around this limitation with supplementary methods.
* A simple version built on `std::multimap` was added as a starter/baseline but more efficient approaches are planned.
* Simultaneous trigger events are handled as a special case since correct operation sequencing requires more virtual method calls.
* Boost `mutable_queue` and `d_ary_hoop_indirect` may be worth experimenting with.
* There are many research papers about priority queues with good scalability, concurrency, and/or cache efficiency, with a seeming preference for skip list based designs: These should be evaluated once we have large-scale real-world cases to test.

### Function

For the code-defined modeling a few function types are provided:
* Linear time-invariant functions for QSS and LIQSS solvers.
* Numeric differentiating linear time-invariant functions for QSS solvers.
* Sample nonlinear functions.
* Sample input variable functions with analytical and numeric derivatives.

### Input Functions

Input functions are external system "drivers" that do not depend on the state of other variables within the DAE system. The QSS solver implements input functions for continuous real-valued variables with QSS-like quantized and continuous representations and logic for selecting the next time at which to update these representations. Input variables using this approach are smoothly integrated with the QSS variable system.

Input functions can also have discrete events at which their value changes. To provide accurate behavior with QSS solvers, where time steps can be large and variable, these discrete events are assumed predictable by the input functions. These predicted discrete events are placed in the QSS event queue to assure they are processed at their exact event time. We don't currently support discrete events for continuous input functions but this is easily added if needed.

Input functions may be outputs from other models/FMUs. This is discussed in the Connected Models section.

### Zero-Crossing Functions

So-called zero-crossing functions are used by Modelica for conditional behavior: when the function crosses zero an event needs to be carried out. This might be, for instance, a thermostat control reaching a trigger temperature that needs to turn on an A/C system. Zero crossings cause (discontinuous) changes to (otherwise) continuous and discrete variables via "handler" functions.

In traditional solvers a zero crossing is detected after it occurs and time backtracking may be used to find its precise time of crossing. With QSS we can do better by predicting the zero crossing using the QSS-style polynomial representation, optionally refining that prediction with an iterative root finder. The zero-crossing support is integrated with the QSS system as zero-crossing variables that are somewhat analogous to the QSS variables in their use of polynomial trajectories that requantize based on specified tolerances. The predicted zero crossings are events on the QSS event queue so they are handled efficiently without a need for backtracking.

The QSS literature indicates that zero-crossing variables should use the quantized, not continuous, representation of their dependent variables. Such zero-crossing representations are less accurate than the variables they depend on and can have discontinuities which make it possible for crossings to occur at unpredicted times. Also, with some zero-crossing functions (such as LTI functions) this approach causes the continuous zero-crossing representation to have a zero highest order coefficient, deactivating its own requantization. QSS simulation of FMUs has the additional complexity of getting the QSs-predicted zero-crossing event to align with the FMU event detection, which benefits from using highly accurate QSS zero-crossing representations. The approach used here has been shifted to base zero-crossing variables on the continuous representation of their dependent variables. This "shadowing" approach requires observer updates whenever a dependent variable has observer (as well as requantization) updates, so there can be a modest performance impact. Since zero-crossing variables don't have observers there is no propagation of these extra updates. The more accurate zero-crossings should improve solution accuracy and reduce FMU zero-crossing event detection misses and it should make zero-crossing root refinement unnecessary in most simulations. More experience with this approach is needed but it is expected to generally provide improved solution accuracy with little or no performance penalty. With zero-crossing functions tracking the continuous representations of their dependent variables it is often most efficient and sufficiently accurate to let the dependent variable requantizations trigger most of the zero-crossing variable updates: the `--zFac` option is provided to adjust the zero-crossing requantization tolerance. Using too large a `zFac` value can cause zero crossings to be missed: a sign that this may be happening is if the statistics report shows that unpredicted zero crossings were detected.

QSS needs to have variables that represent the zero-crossing functions to predict the crossings but they are not provided as FMU variables in standard Modelica implementations. The SOEP QSS solver provides two mechanisms to address this:
* Modelon's OPTIMICA Compiler Toolkit (OCT) has been extended with compiler options `event_indicator_structure` and `event_output_vars` to generate "event indicator" variables for the zero-crossing functions. These variables should have both their dependencies and the "reverse dependencies" of the variables modified within the "if" or "when" block where the zero-crossing function appears encoded in the modelDescription.xml file. When this is fully functional the support for zero-crossing functions with the SOEP QSS solver will be fully automatic but not all of the event indicator dependencies are captured in the current OCT release so manual checking and addition of dependencies is required for now.
* For JModelica and other standard Modelica implemntations explicit zero-crossing output variables and their derivatives must be added with names of the form \_\_zc\_*name* and \_\_zc\_der\_*name*. The dependencies of these variables should appear in the Outputs secion of the modelDescription.xml file extracted from the FMU. Variables modified by the "if" or "when" block where a zero-crossing function appears need to be added to the InitialUnknowns or, for discrete variables, a DiscreteStates section of the xml file. The xml file can be updated in the FMU using the `zip -f` command. This approach is not very practical for models with many zero-crossing functions but was useful for development with smaller models before the OCT solution was available.

Zero crossings introduce the potential for cyclic dependencies to cause a cascade of updates that occur at the same time point. Zero crossing handler events change some variable values. Because those changes can be discontinuous this is treated like a requantization event that must update both the quantized and continuous representation of those modified variables, which, in turn, can cause zero-crossing variables to update, possibly triggering another round of zero crossings, and so on. All of these events happen at the same (clock) time but they are logically sequentially triggered so we cannot know or process them simultaneously. To create a deterministic simulation with zero crossings the following approach is used:
* The phases of zero crossing, handler updates, and resulting requantizations are clustered together by the use of a "superdense time", which is a time value paired with a pass index and an event type ordering offset.
* In any pass with both new zero crossings and new requantizations the zero crossings are processed first (via superdense time indexing).
* In each phase multiple events of the same (zero crossing, handler, or requantization) type are handled together as simultaneous events: the processing for simultaneous events is phased so that the changes to interdependent variables are propagated deterministically.

With cyclic dependencies it is possible for an infinite cascade loop of such changes to occur at the same (clock) time. Detecting such dependencies and the occurrence of these infinite loops is necessary with zero crossing support.
* The `--cycles` option will cause the QSS solver to report cyclic dependencies among the variables, including dependencies created *via* conditional clause handlers.
* The `--pass` option sets a limit for event passes at the same (clock) time after which increasing minimum time steps are used to advance the time and avoid a (possibly infinite) cascade of events preventing the simulation from advancing. If 100 times the pass limit is reached the simulation will terminate.

Zero-crossing based conditional logic can also introduce "chattering" when their handlers change the model state such that another conditional is triggered almost immediately. In some models this occurs with the same zero-crossing function crossing in the opposite direction. This can cause many very small time steps that bog a simulation down. The best solution to chattering is to build the necessary "smooth" control logic and/or hysteresis into the model's conditional logic. Automatic chattering prevention can be effective in some cases, typically ignoring zero crossings until the variable's magnitude has reached some threshold level since the last zero crossing. The QSS solver implements this using the `--zTol` option that can set a global threshold value (the code-defined models can also set per-variable thresholds). OCT and JModelica FMUs may have a parameter named `_events_default_tol`, and that is used as the default `zTol` value if present. The threshold method is fairly simplistic and can cause meaningful zero crossings to be ignored and so should be used with care: more sophisticated approaches can be implemented in the QSS solver if/when needed.

### Conditionals

Modelica supports conditional behavior via "if" and "when" blocks. The QSS solver has classes representing if and when conditional blocks to provide this behavior. Each conditional block has a sequence of clauses that represent the if/elseif/else and when/elsewhen structure of the corresponding Modelica conditional. Each clause (except the else clause) contains one or more boolean or zero-crossing variables. When fully realized clauses would be able to represent logical "trees" over their variables: for now only OR logic is supported.

Since FMUs are not going expose the full conditional block and clause structure to QSS via the model description XML file the conditional support has been simplified for FMUs to treat each Conditional object as a single-Variable clause.

### Connected Models

Simulation of multiple subsystems models with some outputs connected to inputs in other models is important for SOEP. For this purpose the QSS application was extended to support multiple models and support for a `--con` option to specify interconnections was added. When connections are specified the models run in a synched mode to provide the "current" inputs.

Each connection is specified via a command line options of this form:
`--con=`_model1_`.`_inp\_var_`:`_model2_`.`_out\_var_

Connected model simulations are supported for FMU-QSS and FMU-ME models.

#### Connected FMU-QSS Models

Connected FMU-QSS models are treated as loosely coupled. Inputs manage their own state and trajectory independent of the corresponding outputs, getting the output state via "smooth tokens" that are packets containing the output value and derivative state. This loose coupling means there is some potential for discrepancy between outputs and inputs. The "smooth tokens" used to communicate output state to inputs has a next discrete even time field so that predicted discrete events will force a refresh of the inputs.

There are two modes for synching the FMU-QSS models:
1. Specifying a `--dtCon` connection sync time step will simulate each model for that time span in loops until finished. This limits the worst-case time sync error.
2. Simulating without `--dtCon` causes a model event queue based sync that simulates each model until its next event past the first event time will modify a connected output. This allows other models to catch up before their inputs change. While the recommended approach, this does not assure perfect sync because output events do not trigger events in the corresponding inputs at the event time. It also will more accurate but less efficient than using a large enough `dtCon` to allow more events to be processed in each model simulation pass.

#### Connected FMU-ME Models

There are two methods available for simulating connected FMU-ME models: a loosely coupled approach analogous to the the FMU-QSS method and a "perfect sync" approach.

The loosely coupled approach allows the input variable updates to lag changes to the corresponding outputs by a user-specifiable time step. Inputs also do not generate output file entries at their output variable event times. This approach is provided as a demonstration of how connected FMU-ME could be simulated from a master algorithm that doesn't allow direct communication between the FMUs: the perfect sync approach is recommended instead and should provide better accuracy and efficiency.

The perfect sync approach builds a direct connection from the output variable to its inputs in other FMUs. When the output variable has a requantization, discrete, or handler event, changing its quantized state, its connection variables update their state and do observer advances on their own observers. The connection variable is a proxy rather than holding its own trajectory and getting smooth token packet updates. The master simulation loop runs the FMU with the soonest next event time and each FMU simulation runs until it completes an event pass that changes an output variable. This approach assures that all the models are using the same representation for the connected variables so no accuracy loss should occur. It also allows inputs to generate entries to their output files at output variable event times. Perfect sync is enabled by using the `--perfect` option with connected FMU-ME models.

### Binned-QSS

Hybrid/DAE Modelica models do not obtain the performance benefit of a pure QSS solver applied to an ODE since event processing for each requantizing variable can trigger an expensive algebraic solution. We developed a Binned-QSS variant to address this situation.
Binned-QSS requantizes a "bin" of variables with events near the top of the queue together, amortizing the overhead of the algebraic solution over more solution progress at the cost of requantizing some variables sooner than necessary.
Efficient observer/observee container setup and pooled FMU calls are used to optimize the Binned-QSS performance.
The Binned-QSS approach is consistent with recent research findings that show advancing the fast-changing variables separately (using otherwise traditional ODE solvers) can bring large performance gains to building models.
The Binned-QSS goes beyond this research by exploiting QSS to dynamically identify the currently fast-changing variables, and has the potential for automatic, adaptive binning size/logic.

#### Notes

* Early performance experiments with Binned-QSS are promising, showing higher performance than normal QSS even for most non-DAE (pure ODE) models.
* LIQSS is not fully optimized in this first Binned-QSS implementation: this will be more practical when automatic differentiation becomes available.
* The `--bin=`_SIZE_`:`_FRAC_`:`_AUTO_ option can be used to specify the bin size and min time step fraction for binning and whether to use automatic bin size optimization.
* A simple automatic bin size optimizer was developed for use until more large-scale models can be run to refine the design.
* The bin size optimizer is controlled by the optional _AUTO_ field of the `--bin` option and is off by default.
* The bin size optimizer considers CPU timing both to determine how often to compute a new recommended bin size and to compute the bin size. Since CPU timing varies with system load, the bin size optimizer causes some solution variation between runs (non-deterministic). A fixed simulation time step specified by the user could be added to allow bin size optimization without solution non-determinism but a good time step for this would need to be long enough for a CPU elapsed time that is long enough relative to the CPU clock resolution for robust solution "velocity" computation, which will depend on the system.

## FMU Support

Models defined by FMUs following the FMI 2.0 API and built by OCT can be run by this QSS solver. Constraints on the models that are currently supported include:
- Pure SI unit system models are assumed: QSS does not perform any unit conversions that could be needed such as between a state variable and its derivative variable that are in incompatible units.

### FMU Notes

* Mixing QSS methods in an FMU simulation is not yet supported and will require a Modelica annotation to indicate QSS methods on a per-variable basis.
* The FMU support is performance-limited by the FMI 2.0 API, which requires expensive get-all-derivatives calls where QSS needs individual derivatives.
* Performance is limited by the use of numeric differentiation: the FMI ME 2.0 API doesn't provide higher derivatives but they may become available via FMI extensions.
* Zero crossings are problematic because the FMI spec doesn't expose the dependency of variables that are modified when each zero crossing occurs. Our initial approach was to add the zero crossing variables to the Modelica models and to add their dependencies to the FMU's modelDescription.xml file. Additionally, we now support the OCT event indicator system that defines the zero crossing variables and their dependencies.
  * The OCT event indicator system is under ongoing development to refine the treatment of dependencies and reverse (handler) dependencies, such as when to short-circuit non-state variables from the dependencies.
* Input function evaluations will be provided by OCT when QSS is integrated. For stand-alone QSS testing purposes a few input functions are provided for use with FMUs.
* Only SI units are supported in FMUs at this time as per LBNL specifications. Support for other units could be added in the future.

### Next-Gen FMU Support

In preparation for anticipated FMI API extensions to OCT's FMI Library a NextGen branch of this repository has been developed.
NextGen assumes that the FMU can provide higher derivatives directly for a variable when its observee variables are set to their values at the evaluation time.
This significantly simplifies the QSS code and makes it more practical to implement 3rd order QSS methods.
It will also eliminate the variable processing sequence dependency flaw with QSS3+ and fQSS2+ methods.
The purpose of the NextGen branch is to show approximately how the code will look when this advanced support becomes available and to simplify migration at that time.
Using placeholder calls to current the FMI API enables this NextGen branch code to compile and run.

## FMU-QSS

An FMU-QSS is an FMU that wraps an FMU for Model Exchange (FMU-ME) and only exposes inputs and outputs (including FMU-ME integrated state variables) using "SmoothToken" objects that contain values and necessary derivatives.

## Implementation

### Code Structure

The source code is in `src/QSS` so that with include search paths set to `src` the includes can be of the form `#include <QSS/...>`.

The test code is in `tst/QSS` and the unit tests are in `unit`.

### Numeric Differentiation

Second order and higher derivatives are needed for QSS2+ methods. For FMUs we currently must use some numeric differentiation:
* Continuous state variable second derivatives are obtained using FMU directional derivative calls and third derivatives are computed numerically in the default QSS solvers. Alternative "n" solvers (such as nQSS2) that use numerical second and third derivatives may be useful for denser dependency models for which the current directional derivative overhead is high.
* Zero-crossing and (non-state) real-valued variable first derivatives are obtained using FMU directional derivative calls and second and third derivatives are computed numerically.

At variable initialization and other simultaneous requantization or zero-crossing handler events, the numeric differentiation currently required for QSS3 solvers has a variable processing sequence dependency problem: the derivative evaluations at time step offsets used to compute the numeric derivatives can depend on QSS trajectory coefficients being computed in other variables being updated, so the results can depend on the sequence with which variables are processed. Deferring variable trajectory updates until after all such computations can eliminate this sequence dependency but was found to significantly hurt solution accuracy and performance. To avoid these issues, QSS is best implemented with non-numeric higher derivatives: automatic differentiation support is being considered for future OCT releases.

### Numeric Bulletproofing

The time advance functions solve for the roots of polynomials of the QSS method order to see where the continuous representation next crosses the quantized representation +/-Q boundaries. The naive approach is to just pick the smallest positive root that the root solver finds, but with finite precision computations it is possible for this to select the "wrong" root that allows the continuous representation trajectory to fall outside the quantum band. To protect against this we take a few steps:
* Roots that cross the boundary in the wrong direction (moving from outside to inside) are culled out. The crossing direction tests have some computational cost: in the unlikely event that this is significant a "fast and dangerous" mode could be provided that omits them.
* When a numeric precision loss causes a bad solution we exploit knowledge of the actual behavior to produce a zero time step rather than an infinite step.
* When the polynomial coefficients indicate which boundary must be hit first we save time by only root solving on that boundary and we exploit the known coefficient signs.
* When the polynomial coefficients don't clearly show which boundary will be hit first we process the boundaries together to exploit knowledge that at least one of them should have a positive root with the correct, outward-crossing direction.

### Zero-Crossing Support

Some of the tasks remaining for a robust, production-quality implementation are:
* Detect and warn about cyclic dependencies that could lead to infinite update cascades.
* Detect the occurrence of infinite update cascades and terminate with an error.
* Zero-crossing root finding is complicated for the case of large tolerance and/or complex zero-crossing functions, where multiple zero crossings might occur within the active quantization time segment. A combination of long and short range algorithms is needed to minimize the possibility of missing roots. Careful development of such an approach is recommended for production use. The current code is only robust for small tolerances and sufficiently simple zero-crossing functions that the continuous trajectory representation is able to localize the root to good accuracy and only one root is present in any quantized time segment.
* LIQSS variables present a problem with zero crossings since the quantized and continuous representations do not generally start at the same value in a requantization event. To avoid this LIQSS handler advances set the zero order quantized and continuous coefficients to the same value rather than using the normal LIQSS behavior.
* Startup behavior is not well-handled by zero-crossings: the initial model state may put the zero-crossing function in any of its conditions but no zero crossing occurred to get there so it isn't detected. For example, a bouncing ball model where the ball starts on or below the floor: no crossing from above the floor occurs so the ball will fall to negative infinity unless a startup procedure is used. A set of rules and a mechanism for dealing with startup state of zero crossings needs to be developed.

Bulletproofing present in the implementation:
* Attempting to change the same variable to different values in two zero-crossing handlers in the same superdense time pass is detected as a (non-fatal) error.
* Crossing type/direction is part of the zero-crossing event so that handlers can ignore irrelevant crossings.
* When zero crossing events cluster up closer and closer (such as a bouncing ball) eventually time may not progress and an infinite loop can occur. The bball model shows how a handler should be designed to detect and address this. For Modelica-based models such a set of rules may not be possible in which case the chattering prevention will be needed.

FMU zero crossing support has some additional complications and limitations:
* Zero-crossing functions are implicitly defined by if and when blocks in the Modelica file and are not directly exposed via the FMI API.
The OCT addresses this by generating event indicator variables that are documented as XML file annotations with the "reverse" (handler) dependencies.
(Before this OCT capability was added we created a convention of defining output variables and their derivatives for each zero-crossing function with names of the form \_\_zc\__name_ and \_\_zc\_der\__name_ and added the reverse dependencies to the XML file `DiscreteStates` block for discrete variables and `InitialUnknowns` for continuous state variables.)
* The FMI API doesn't expose crossing directions of interest so we enable all of them. If this will never be available we should eliminate crossing check logic to avoid wasted effort.
* There is no FMI API to directly trigger the zero-crossing handlers to run when the QSS solver reaches zero-crossing events. Instead we set the relevant FMU variables to a time slightly beyond the zero-crossing time with the hope that the zero crossing will be detected by the FMU. The `--zTol`, `--zMul`, and `--dtZC` options allows control over this time "bump". OCT and JModelica FMUs may have a parameter called `_events_default_tol` that will be used as the default `zTol` value to match the QSS time bump to the FMU behavior. The `zMul` value multiplies the `zTol` when setting the bump step. The `dtZC` value is only a fallback when no `zTol` is present or when the trajectory doesn't allow a time bump step to be computed. Using the uniform `dtZC` bump step is not robust as it doesn't adjust for solution behavior. This is also not highly robust because the output variables used to track the zero crossing derivatives are (at least for Dymola-generated FMUs) numerically, not analytically, based so the QSS zero-crossing function does not track the actual FMU zero-crossing function to high precision.
* Zero-crossing root refinement is expensive due to the overhead of FMU operations so it is disabled by default (the `--refine` option enables it). Once atomic FMU variable get/set operations are provided the overhead will be lower.

#### Notes

* Zero-crossing variables should not need and are required to have no observers (but they do have reverse dependents: the variables modified by their handlers).
* Zero-crossing variables must initialize and advance after other variables because their zero-order coefficients come from their function evaluation that depends on the zero-order coefficients of other variables being advanced.
* Zero-crossing variables use the continuous, not quantized, representations of variables appearing in their functions, in contrast with the QSS literature, to provide more accurate and full order representations.

#### Root Refinement

Finding accurate zero-crossing times is important for simulation correctness and efficiency. If the crossing time is not accurate the model may not carry out the correct logic in the conditional clause handler function or it may detect the same actual crossing multiple times. With FMU-based models there is the additional complication that the QSS solver needs to know the crossing time accurately so that it can tell the FMU when to check for crossing events.

The QSS-style continuous trajectory of zero-crossing functions will give accurate crossing times when the QSS tolerance is small enough to make the trajectory very close to the actual zero-crossing function. With larger QSS tolerances or fast-changing variables this may not be accurate enough. The current QSS solver can perform Newton iterative refinement of zero-crossing roots (using the `--refine` option), typically only requiring 1-2 iterations to converge. Root refinement is expensive and is probably not needed with most models. With FMU-based models root refinement is even more expensive: all observees (dependencies) of the zero-crossing variable must be set to the value at each iteration time and then the variable value and derivative must be evaluated by the FMU. This root refinement approach works well when the initial guess provided by the continuous trajectory tightly represents the actual zero-crossing function. When larger tolerances are in use this may not hold and a more robust approach will be needed.

The zero-crossing method now used in this solver that bases the zero-crossing variables' representation on the continuous representation of their dependent variables provides more accurate crossing times and probably does not require root refinement in most situations with typical tolerances.

A robust approach to large-tolerance zero-crossing root finding will probably have these characteristics:
* Setting a `dtZ` time step for the zero-crossing variable that is sufficiently small to avoid missing zero crossings.
* Sampling the function value out from the interval start time with `dtZ` steps until a crossing is localized or the trajectory-based root estimate is reached.
* Refinement with a Newton or Brent type algorithm from the root estimate and restricted to the interval of localization/interest.
* A option to enable the large-tolerance root finding either directly or when a specified tolerance level is exceeded.

Since this process can be expensive, the alternative of depending on the modeler to use sufficiently small relative and absolute tolerances may be preferable. Robust root finding in hybrid continuous-discrete systems is challenging and further experimentation and literature review is needed.

Some References:
* [Zero-Crossing Location and Detection Algorithms For Hybrid System Simulation](https://pdfs.semanticscholar.org/08e4/53ea25b1fa44b4d550f552a9db41bbaa09ff.pdf)
* [Non-standard semantics of hybrid systems modelers](http://www.irisa.fr/s4/download/papers/nsshs-jcss-2011.pdf)

### Conditionals

The QSS solver behavior for processing conditionals is:
* At events when zero-crossing or boolean variables in clauses become true (and false for if block clauses) then notify their clauses to put the conditional event on the queue at that time.
* When conditional events are processed the clauses are evaluated to see which of them are active at that time (and superdense time pass).
* Active clauses place events for their handlers on the queue at the same time and pass.
* Handler events are processed to make the necessary variable updates.

For FMUs a simpler conditional support is used because the FMUs don't expose the conditional block and clause structure.

Conditional handler functions can cause variable changes that cause other (zero-crossing, requantization, ...) events so the processing order can affect results. To assure deterministic results, each event type has an offset number that is used as an additional field in the superdense time to assure that discrete, zero-crossing, conditional, and handler events are processed in groups in that order in each pass. Variables modified inconsistently by handlers in the same pass are detected and warnings generated since this indicates a problematic model that can produce processing order-dependent results.

## Performance

Performance assessments are ongoing as larger-scale models become available. Preliminary profiling, tuning, and parallelization findings are described here.

### Performance Findings

#### FMU Model Performance

* FMU model performance is currently dominated by operations inside the FMU: no QSS solver function registers even 1% in the profile. This performance is severely hobbled by the FMI 2.0 API that is ill-suited to QSS simulation and by the need for numeric differentiation. Once the planned "atomic" (per-variable) API and higher derivatives become available via FMI extensions this performance profile should significantly improve.
* The `advance_observers` operation is also a candidate for performance gains in the FMU models.
  * The first approach is to used pooled lookup operations for the observers' derivatives (and, for zero-crossing variables, value) to reduce the FMU call overhead. This was done as a first pass for the `advance_observers` first phase operation. This provided a 15% speedup for the 4-zone ScaleTest model but no speedup for the Case600 room air model. There are other places in the code that could be refactored to use pooled FMU calls: this should improve performance but these are non-trivial code changes.
  * FMUs are not thread-safe, inhibiting parallelization, but by pooling the observer FMU calls we can explore parallelizing the rest of the `advance_observers` operation. This was tried for the same 4-zone ScaleTest model and results with OpenMP controls tried to date yielded slowdowns, indicating that the OpenMP threading overhead dominates the loop time, so this code has been commented out for now. More FMU model parallelization experimentation is warranted.

### Performance: Future

* Run time comparisons _vs._ [Qss Solver](https://sourceforge.net/projects/qssengine/), [Ptolemy](http://ptolemy.eecs.berkeley.edu/), and other implementations will be useful.
* Evaluation of alternative event queue designs is likely to be worthwhile, especially once parallel processing is added.
* Simultaneous requantization triggering: Could skip continuous representation update if a variable is not an observer of any of the requantizing variables. This would save assignments but more importantly evaluation of the highest derivative. There is some overhead in determining whether a variable qualifies. Testing so far doesn't show a significant benefit for this optimization but it should be reevaluated with real-world cases where simultaneous triggering is common.

## Testing

* Case runs are being compared with results from [Qss Solver](https://sourceforge.net/projects/qssengine/) and [Ptolemy](http://ptolemy.eecs.berkeley.edu/) for now.
* Unit tests are included and will be extended for wider coverage as the code progresses.

## Building QSS

Instructions for building the QSS application on different platforms follows.

### Linux

FMIL:
* See the FMIL directory for full information on building and using the FMIL with QSS.
* You will need a build of the latest [FMI Library](http://www.jmodelica.org/FMILibrary).
* If your FMI Library is not installed to directories the `setFMIL` scripts expect (such as `/opt/FMIL.GCC.r`) copy the corresponding `bin/Linux/<compiler>/setFMIL` to a directory in your PATH and adapt it to the location of you FMI Library installation.
* FMU-QSS support needs extra FMIL headers added to the FMIL installation. For Linux this would be installed with commands of the form (adjusted for your configuration):
  * `sudo cp <path>/FMIL/trunk/src/CAPI/include/FMI2/fmi2_capi.h /opt/FMIL.<Compiler>.<Build>/include/FMI2`
  * `sudo cp <path>/FMIL/trunk/src/CAPI/src/FMI2/fmi2_capi_impl.h /opt/FMIL.<Compiler>.<Build>/include/src/FMI2`
  * `sudo cp <path>/FMIL/trunk/src/Import/src/FMI/fmi_import_context_impl.h /opt/FMIL.<Compiler>.<Build>/include/FMI`
  * `sudo cp <path>/FMIL/trunk/src/Import/src/FMI2/fmi2_import_impl.h /opt/FMIL.<Compiler>.<Build>/include/FMI2`
  * `sudo cp <path>/FMIL/trunk/src/XML/include/FMI/*.h /opt/FMIL.<Compiler>.<Build>/include/FMI`
  * `sudo cp <path>/FMIL/trunk/src/XML/include/FMI1/*.h /opt/FMIL.<Compiler>.<Build>/include/FMI1`
  * `sudo cp <path>/FMIL/trunk/src/XML/include/FMI2/*.h /opt/FMIL.<Compiler>.<Build>/include/FMI2`
  * `sudo cp <path>/FMIL/trunk/ThirdParty/Expat/expat-2.1.0/lib/expat*.h /opt/FMIL.<Compiler>.<Build>/include`

Googletest:
* The unit tests use googletest. The `setGTest` scripts under `bin` set up the necessary environment variables to find googletest when you run the `setQSS` script for your compiler: put a custom version of `setGTest` in your `PATH` to adapt it to your system.

To build the QSS application on Linux:
* `cd <repository_directory>`
* `source bin/Linux/GCC/r/setQSS` for release builds or `source bin/Linux/GCC/d/setQSS` for debug builds
* `cd src/QSS/app`
* `mak` (add a -j*N* make argument to override the number of parallel compile jobs)
* Note that `-fPIC` is used to compile with GCC on Linux to share the build configuration with the FMU-QSS shared/dynamic library generation: this may add a small performance penalty for the QSS application and should not be used for production application builds.

To run the QSS application:
* `QSS` from any directory with a console configured with `setQSS`
* `QSS --help` will show the command line usage/options.

To build and run the unit tests on Linux:
* The unit tests are in the `tst/QSS/unit` directory and can be built and run with the command `mak run`.

To run the tests that exercise an FMU place a binary compatible build of the tested FMUs in the `unit/Linux/<compiler>/d` directory before running the unit tests.
At this time an Achilles.fmu FMU built from Achilles.mo in the SOEP-QSS-Test repository can be used by the tests.

### Windows

#### Note

FMIL:
* You will need a build of the latest [FMI Library](http://www.jmodelica.org/FMILibrary).
* If your FMI Library is not installed to directories the `setFMIL` scripts expect (such as `C:\FMIL.VC.r`) copy the corresponding `bin\Windows\<compiler>\setFMIL.bat` to a directory in your PATH and adapt it to the location of you FMI Library installation.
* FMU-QSS support needs extra FMIL headers added to the FMIL installation. For Linux this would be installed with commands of the form (adjusted for your configuration):
  * `copy \Projects\FMIL\trunk\src\CAPI\include\FMI2\fmi2_capi.h C:\FMIL.<Compiler>.<Build>\include\FMI2`
  * `copy \Projects\FMIL\trunk\src\CAPI\src\FMI2\fmi2_capi_impl.h C:\FMIL.<Compiler>.<Build>\include\src\FMI2`
  * `copy \Projects\FMIL\trunk\src\Import\src\FMI\fmi_import_context_impl.h C:\FMIL.<Compiler>.<Build>\include\FMI`
  * `copy \Projects\FMIL\trunk\src\Import\src\FMI2\fmi2_import_impl.h C:\FMIL.<Compiler>.<Build>\include\FMI2`
  * `copy \Projects\FMIL\trunk\src\XML\include\FMI\*.h C:\FMIL.<Compiler>.<Build>\include\FMI`
  * `copy \Projects\FMIL\trunk\src\XML\include\FMI1\*.h C:\FMIL.<Compiler>.<Build>\include\FMI1`
  * `copy \Projects\FMIL\trunk\src\XML\include\FMI2\*.h C:\FMIL.<Compiler>.<Build>\include\FMI2`

Googletest:
* The unit tests use googletest. The `setGTest` scripts under `bin` set up the necessary environment variables to find googletest when you run the `setQSS` script for your compiler: put a custom version of `setGTest.bat` in your `PATH` to adapt it to your system.

To build the QSS application on Windows:
* `cd <repository_directory>`
* `bin\Windows\VC\r\setQSS` for VC++ release builds or `bin\Windows\VC\d\setQSS` for VC++ debug builds (or similarly with GCC or with IC for Intel C++)
* `cd src\QSS\app`
* `mak` (add a -j*N* make argument to override the number of parallel compile jobs)

To run the QSS application:
* `QSS` from any directory with a console configured with `setQSS`
* `QSS --help` will show the command line usage/options.

To build and run the unit tests on Windows:
* The unit tests are in the `tst\QSS\unit` directory and can be built and run with the command `mak run`.

To run the tests that exercise an FMU place a binary compatible build of the tested FMUs in the `unit\Windows\<compiler>\d` directory before running the unit tests.
At this time an Achilles.fmu FMU built from Achilles.mo in the SOEP-QSS-Test repository can be used by the tests.

## Running QSS

The QSS solver application can run both code-defined and FMU-based test cases. FMU for Model Exchange .fmu files can be run if properly adapted for QSS with respect to zero-crossing variables and dependencies. FMU-QSS .fmu files can also be run but they must have names of the form `<FMU=ME_name>_QSS.fmu` to be recognized as FMU-QSS FMUs.

There are command line options to select the QSS method, set quantization tolerances, output and differentiation time steps, and output selection controls.
* Relative tolerance is taken from the FMU if available by default but can be overridden with the `--rTol` command line option.
* Absolute tolerances are computed by default from the FMU variable nominal values and the `--aFac` command line option but can be overridden with a constant value using the `--aTol` command line option.
* QSS variable continuous and/or quantized trajectory values can be output at their requantization events.
* QSS variable continuous and/or quantized trajectory output at a regular sampling time step interval can be enabled.
* FMU outputs can be generated for FMU model runs.
* Diagnostic output can be enabled, which includes a line for each quantization-related variable update.

To run QSS with one of the code-defined models:
* `QSS <model> [options]`

To run QSS with an FMU:
* `QSS <model>.fmu [options]`

Run `QSS --help` to see the command line usage.

## Building an FMU-QSS

Instructions for building an FMU-QSS from an FMU-ME (FMU for Model Exchange) follows.
* The [fmu-uuid](https://github.com/viproma/fmu-uuid) application must be on your PATH to generate and FMU-QSS with GUID checking.
* The same FMIL setup and `setQSS` console configuration as for the QSS application is assumed.
* The `FMU-QSS.gen.py` (Python 2.7 or 3.x) script generates the FMU-QSS from an FMU-ME adapted for QSS use.
* From a console configured with `setQSS` move to an empty working directory and copy in the FMU-ME .fmu file.
* Run `FMU-QSS.gen.py <FMU-ME_name>.fmu` to start the generation process.
  * Additional options can be seen by running `FMU-QSS.gen.py --help`.
* This will generate the FMU-QSS `modelDescription.xml` file, build the FMU-QSS shared/dynamic library, and zip the FMU-QSS into a file with a name of the form `<FMU=ME_name>_QSS.fmu`. The FMU-ME fmu is embedded in the `resources` folder within the FMU-QSS .fmu zip file.

## Running an FMU-QSS

To run an FMU-QSS:
* `QSS <FMU-QSS>.fmu [options]`

Run `QSS --help` to see the command line usage.

## Running Multiple FMU-QSS

To run multiple FMU-QSS simply enter all of them on the command line:
* `QSS <FMU-QSS_1>.fmu <FMU-QSS_2>.fmu ... [options]`

Input/output connections between models can be specified via command line options of the form:
`--con=`_model1_`.`_inp\_var_`:`_model2_`.`_out\_var_

Run `QSS --help` to see the command line usage.

## PyQSS

To prepare for integration into PyFMI support for building a Python wrapper for QSS was developed and is included.
A Python front end to QSS can be built by running the `mak pyd` command from the `app` subdirectory of `src`.
This builds the `PyQSS` shared/dynamic library that can be imported as a Python module.
(`PyQSS` is not built by the `bld` or default `mak` commands.)
A simple front end script, `QSS.py`, is included.
To run QSS from the Python front end use:
* `QSS.py <arguments> [options]`

### Notes

* Pybind11 does not work with the Windows MinGW GCC compilers with the stock Python binary distribution that is built with Visual C++.
* To use `PyQSS` with PyFMI it must be built with the same Python version that PyFMI runs under.
  To support this usage the Windows environment setup uses the OCT Python installation, if present, in favor of the primary system Python.
