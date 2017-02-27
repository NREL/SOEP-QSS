#ifndef QSS_ex_nonlinear_ND_hh_INCLUDED
#define QSS_ex_nonlinear_ND_hh_INCLUDED

// Nonlinear Derivative with Numeric Differentiation Example Setup
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

// Nonlinear Derivative with Numeric Differentiation Example Setup
void
nonlinear_ND( Variables & vars );

} // ex

#endif
