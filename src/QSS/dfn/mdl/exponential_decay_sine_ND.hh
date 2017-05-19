#ifndef QSS_dnf_mdl_exponential_decay_sine_ND_hh_INCLUDED
#define QSS_dnf_mdl_exponential_decay_sine_ND_hh_INCLUDED

// Exponential Decay with Sine Input and Numeric Differentiation Example Setup
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

// Exponential Decay with Sine Input and Numeric Differentiation Example Setup
void
exponential_decay_sine_ND( Variables & vars );

} // mdl
} // dfn
} // QSS

#endif
