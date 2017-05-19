// FMU-Based QSS Globals
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/fmu/globals_fmu.hh>
#include <QSS/fmu/Variable.hh>
#include <QSS/EventQueue.hh>

namespace QSS {
namespace fmu {

// QSS Globals
EventQueue< Variable > events;

} // fmu
} // QSS
