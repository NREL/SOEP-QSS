#ifndef QSS_math_hh_INCLUDED
#define QSS_math_hh_INCLUDED

// C++ Headers
#include <cmath>
#include <type_traits>

// Sign
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
sign( T const x )
{
	return ( std::signbit( x ) ? T( -1 ) : T( +1 ) );
}

// Sign
template< typename T, class = typename std::enable_if< ! std::is_arithmetic< T >::value >::type >
inline
T
sign( T const & x )
{
	return ( std::signbit( x ) ? T( -1 ) : T( +1 ) );
}

// Square
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
square( T const x )
{
	return x * x;
}

// Square
template< typename T, class = typename std::enable_if< ! std::is_arithmetic< T >::value >::type >
inline
T
square( T const & x )
{
	return x * x;
}

// Cube
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
cube( T const x )
{
	return x * x * x;
}

// Cube
template< typename T, class = typename std::enable_if< ! std::is_arithmetic< T >::value >::type >
inline
T
cube( T const & x )
{
	return x * x * x;
}

#endif
