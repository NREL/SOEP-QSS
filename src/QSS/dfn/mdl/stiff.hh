#ifndef QSS_dnf_mdl_stiff_hh_INCLUDED
#define QSS_dnf_mdl_stiff_hh_INCLUDED

// Stiff System Example Setup
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

// Stiff System Example Setup
void
stiff( Variables & vars );

} // mdl
} // dfn
} // QSS

#endif
