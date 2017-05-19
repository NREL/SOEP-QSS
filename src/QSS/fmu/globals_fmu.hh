#ifndef QSS_fmu_globals_hh_INCLUDED
#define QSS_fmu_globals_hh_INCLUDED

// FMU-Based QSS Globals
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/fmu/Variable.fwd.hh>
#include <QSS/EventQueue.fwd.hh>

namespace QSS {
namespace fmu {

// QSS Globals
extern EventQueue< Variable > events;

} // fmu
} // QSS

#endif
