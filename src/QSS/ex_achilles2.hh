#ifndef QSS_ex_achilles2_hh_INCLUDED
#define QSS_ex_achilles2_hh_INCLUDED

// Achilles and the Tortoise Symmetric Example Setup
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

// Achilles and the Tortoise Symmetric Example Setup
void
achilles2( Variables & vars );

} // ex

#endif
