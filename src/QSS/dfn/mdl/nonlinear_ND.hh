#ifndef QSS_dnf_mdl_nonlinear_ND_hh_INCLUDED
#define QSS_dnf_mdl_nonlinear_ND_hh_INCLUDED

// Nonlinear Derivative with Numeric Differentiation Example Setup
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

// Nonlinear Derivative with Numeric Differentiation Example Setup
void
nonlinear_ND( Variables & vars );

} // mdl
} // dfn
} // QSS

#endif
