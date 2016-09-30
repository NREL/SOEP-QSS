# QSS Solver Prototype/Experimental Code

This is a prototyping test-bed stand-alone QSS solver being developed for integration into JModelica as part of the "Spawn of EnergyPlus" project.

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
* For efficiency it seems like the variables should handle their own integration and quantization operations so we don't consider those as separate entities.
* Parallel updating of variables dependent on the trigger variable is anticipated.
* Priority queues with good concurrency and cache efficiency is a wide research topic: if the event queue is found to be a bottleneck experiments with advanced concepts are planned.

## Implementation

Currently the code is very young with just QSS1, QSS2, and QSS3 solvers for linear and nonlinear derivative functions and a relatively simple "baseline" event queue built on `std::multimap`.

Notes:
* No Modelica input file processing is supported: test cases are hard-coded.
* Input functions, algebraic relationship, and discrete variables will be added soon.

### Variable

* Hierarchy typed by the QSS solver method: This brings in some virtual functions that could be a performance bottleneck.
* Integration and quantization are handled internally to avoid the cost of calls to other objects and passing of data packets between them.
* Holds the iterator of its entry in the event queue to save one _O_( log N ) lookup.
* Supports mix of different QSS method variables in the same model.

### Event Queue

* C++ `std::priority_queue` doesn't support changing the key value so it isn't a suitable out-of-the-box solution: It may be worth trying to work around this limitation with supplementary methods.
* A simple version built on `std::multimap` was added as a starter/baseline but more efficient approaches are planned.
* Simultaneous trigger events are handled as a special case since correct operation sequencing requires more virtual method calls.
* Boost `mutable_queue` and `d_ary_hoop_indirect` may be worth experimenting with.
* There are many research papers about priority queues with good scalability, concurrency, and/or cache efficiency, with a seeming preference for skip list based designs: These should be evaluated once we have large-scale real-world cases to test.

### Function

* A simple linear function is provided.
* A sample nonlinear function is included.
* We'll need a general purpose function approach for the JModelica-generated code: probably a function class that calls back to a provided function.

## Performance

Once the code capabilities are sufficient and larger models are built some performance assessments will be carried out.
Run time comparisons _vs._ [Qss Solver](https://sourceforge.net/projects/qssengine/) and other implementations will be generated where practical.
Profiling will be used to identify bottlenecks in performance and in scalability as problem size grows.
Performance tuning and experimentation with alternative implementation designs are anticipated.
Specifically, evaluation of alternative event queue designs is likely to be worthwhile, especially once parallel processing is added.

## Testing

* Case runs are being compared with results from [Qss Solver](https://sourceforge.net/projects/qssengine/) for now.
* Unit tests are included and will be extended for wider coverage as the code progresses.