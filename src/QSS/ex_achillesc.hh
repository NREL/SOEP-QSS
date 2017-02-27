#ifndef QSS_ex_achillesc_hh_INCLUDED
#define QSS_ex_achillesc_hh_INCLUDED

// Achilles and the Tortoise Custom Function Example Setup
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

// Achilles and the Tortoise Custom Function Example Setup
void
achillesc( Variables & vars );

} // ex

#endif
