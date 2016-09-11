// QSS Variable

// QSS Headers
#include <QSS/Variable.hh>

// C++ Headers
#include <limits>

	Variable::Time const Variable::infinity = ( std::numeric_limits< Time >::has_infinity ? std::numeric_limits< Time >::infinity() : std::numeric_limits< Time >::max() );
