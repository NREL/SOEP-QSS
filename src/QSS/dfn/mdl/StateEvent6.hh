#ifndef QSS_dnf_mdl_StateEvent6_hh_INCLUDED
#define QSS_dnf_mdl_StateEvent6_hh_INCLUDED

// StateEvent6 Example Setup
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

// StateEvent6 Example Setup
void
StateEvent6( Variables & vars );

} // mdl
} // dfn
} // QSS

#endif
