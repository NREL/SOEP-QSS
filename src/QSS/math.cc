// QSS Math Utilities

// QSS Headers
#include <QSS/math.hh>

// C++ Headers
#include <cmath>
#include <limits>

// Globals
double const zero( 0.0 );
double const two( 2.0 );
double const three( 3.0 );
double const four( 4.0 );
double const six( 6.0 );
double const one_half( 0.5 );
double const one_third( 1.0 / 3.0 );
double const one_fourth( 0.25 );
double const one_sixth( 1.0 / 6.0 );
double const one_ninth( 1.0 / 9.0 );
double const two_thirds( 2.0 / 3.0 );
double const pi( 4.0 * std::atan( 1.0 ) );
double const infinity( std::numeric_limits< double >::has_infinity ? std::numeric_limits< double >::infinity() : std::numeric_limits< double >::max() );
