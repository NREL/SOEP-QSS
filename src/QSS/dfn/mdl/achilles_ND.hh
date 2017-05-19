#ifndef QSS_dnf_mdl_achilles_ND_hh_INCLUDED
#define QSS_dnf_mdl_achilles_ND_hh_INCLUDED

// Achilles and the Tortoise Numeric Differentiation Example Setup
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/dfn/Variable.fwd.hh>

// C++ Headers
#include <vector>

namespace QSS {
namespace dfn {
namespace mdl {

using Variables = std::vector< Variable * >;

// Achilles and the Tortoise Numeric Differentiation Example Setup
void
achilles_ND( Variables & vars );

} // mdl
} // dfn
} // QSS

#endif
