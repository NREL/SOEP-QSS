#ifndef QSS_dnf_mdl_nonlinear_hh_INCLUDED
#define QSS_dnf_mdl_nonlinear_hh_INCLUDED

// Nonlinear Derivative Example Setup
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

// Nonlinear Derivative Example Setup
void
nonlinear( Variables & vars );

} // mdl
} // dfn
} // QSS

#endif
