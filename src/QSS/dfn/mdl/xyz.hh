#ifndef QSS_dnf_mdl_xyz_hh_INCLUDED
#define QSS_dnf_mdl_xyz_hh_INCLUDED

// Simple xyz Example Setup
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

// Simple xyz Example Setup
void
xyz( Variables & vars );

} // mdl
} // dfn
} // QSS

#endif
