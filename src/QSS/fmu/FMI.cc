// FMI API Wrappers
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/fmu/FMI.hh>

namespace QSS {
namespace fmu {

// Globals
fmi2_import_t * fmu( nullptr ); // FMU instance
std::size_t n_ders( 0 ); // Number of derivatives
fmi2_real_t * derivatives( nullptr ); // Derivatives

} // fmu
} // QSS
