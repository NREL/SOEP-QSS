#ifndef QSS_ex_exponential_decay_hh_INCLUDED
#define QSS_ex_exponential_decay_hh_INCLUDED

// Exponential Decay Example Setup
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// C++ Headers
#include <vector>

// Forward
class Variable;

namespace ex {

using Variables = std::vector< Variable * >;

// Exponential Decay Example Setup
void
exponential_decay( Variables & vars );

} // ex

#endif
