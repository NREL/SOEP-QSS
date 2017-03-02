#ifndef QSS_options_hh_INCLUDED
#define QSS_options_hh_INCLUDED

// Options Support
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// C++ Headers
#include <string>

namespace options {

// QSS Method Enumerator
enum class QSS {
 QSS1,
 QSS2,
 QSS3,
 LIQSS1,
 LIQSS2,
 LIQSS3
};

extern QSS qss; // QSS method: (LI)QSS1|2|3  [QSS2]
extern int qss_order; // QSS method order  [computed]
extern bool inflection; // Requantize at inflections?  [F]
extern double rTol; // Relative tolerance  [1e-4|FMU]
extern double aTol; // Absolute tolerance  [1e-6]
extern bool rTol_set; // Relative tolerance set?
extern double dtOut; // Sampled & FMU output time step (s)  [1e-3]
extern double dtND; // Numeric differentiation time step (s)  [1e-6]
extern double one_half_over_dtND; // 0.5 / dtND  [computed]
extern double tEnd; // End time (s)  [1|FMU]
extern bool tEnd_set; // End time set?
extern std::string out; // Outputs: r, a, s, x, q, f  [rx]
extern std::string model; // Name of model or FMU

namespace output { // Output selections

extern bool r; // Requantization events?  [T]
extern bool a; // All variables at requantization events?  [F]
extern bool s; // Sampled output?  [F]
extern bool f; // FMU outputs?  [T]
extern bool d; // Diagnostic output?  [F]
extern bool x; // Continuous trajectories?  [T]
extern bool q; // Quantized trajectories?  [F]

} // out

// Process command line arguments
void
process_args( int argc, char * argv[] );

} // options

#endif
