#ifndef QSS_ex_stiff_hh_INCLUDED
#define QSS_ex_stiff_hh_INCLUDED

// Stiff System Example Setup
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

// Stiff System Example Setup
void
stiff( Variables & vars );

} // ex

#endif
