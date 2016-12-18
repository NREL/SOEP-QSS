#ifndef QSS_math_hh_INCLUDED
#define QSS_math_hh_INCLUDED

// QSS Solver Math Functions

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <type_traits>

// Globals
extern double const zero;
extern double const two;
extern double const three;
extern double const four;
extern double const six;
extern double const one_half;
extern double const one_third;
extern double const one_fourth;
extern double const one_sixth;
extern double const one_ninth;
extern double const pi;
extern double const infinity;

// Sign: Returns Passed Type
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
sign( T const x )
{
	return ( std::signbit( x ) ? T( -1 ) : T( +1 ) );
}

// Signum: Returns Passed Type
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
sgn( T const x )
{
	return ( x < T( 0 ) ? T( -1 ) : ( x > T( 0 ) ? T( +1 ) : T( 0 ) ) );
}

// Signum: Returns Passed Type
template< typename T, class = typename std::enable_if< ! std::is_arithmetic< T >::value >::type >
inline
T
sgn( T const & x )
{
	return ( x < T( 0 ) ? T( -1 ) : ( x > T( 0 ) ? T( +1 ) : T( 0 ) ) );
}

// Signum: Returns int
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
int
signum( T const x )
{
	return ( x < T( 0 ) ? -1 : ( x > T( 0 ) ? +1 : 0 ) );
}

// Signum: Returns int
template< typename T, class = typename std::enable_if< ! std::is_arithmetic< T >::value >::type >
inline
int
signum( T const & x )
{
	return ( x < T( 0 ) ? -1 : ( x > T( 0 ) ? +1 : 0 ) );
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

// Min of 3 Values
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
min( T const a, T const b, T const c )
{
	return ( a < b ? ( a < c ? a : c ) : ( b < c ? b : c ) );
}

// Min of 3 Values
template< typename T, class = typename std::enable_if< ! std::is_arithmetic< T >::value >::type >
inline
T const &
min( T const & a, T const & b, T const & c )
{
	return ( a < b ? ( a < c ? a : c ) : ( b < c ? b : c ) );
}

// Min Nonnegative Root of Quadratic Equation a x^2 + b x + c = 0
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
min_root_quadratic( T const a, T const b, T const c )
{
	if ( a == 0.0 ) { // Linear
		if ( b == 0.0 ) { // Parallel
			return infinity;
		} else {
			T const root( -( c / b ) );
			return ( root >= 0.0 ? root : infinity );
		}
	} else { // Quadratic
		T const disc( ( b * b ) - ( 4.0 * a * c ) );
		if ( disc < 0.0 ) { // No real roots
			return infinity;
		} else if ( disc == 0.0 ) { // One real root
			T const root( -( b / ( two * a ) ) );
			return ( root >= 0.0 ? root : infinity );
		} else { // Two real roots: From http://mathworld.wolfram.com/QuadraticEquation.html for precision
			T const q( -0.5 * ( b + ( sign( b ) * std::sqrt( disc ) ) ) );
			T const root1( q / a );
			T const root2( c / q );
			return ( root1 >= 0.0 ? ( root2 >= 0.0 ? std::min( root1, root2 ) : root1 ) : ( root2 >= 0.0 ? root2 : infinity ) );
		}
	}
}

// Min Nonnegative Root of Lower Boundary Quadratic Equation a x^2 + b x + c = 0
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
min_root_quadratic_lower( T const a, T const b, T const c )
{
	assert( a <= 0.0 );
	assert( b <= 0.0 );
	if ( c <= 0.0 ) { // Precision loss: x(tX) < q(tX) - qTol
		return 0.0;
	} else if ( a == 0.0 ) { // Linear
		if ( b == 0.0 ) { // Parallel
			return infinity;
		} else {
			return -( c / b );
		}
	} else { // Quadratic
		T const disc( ( b * b ) - ( 4.0 * a * c ) );
		if ( disc <= 0.0 ) { // Zero or one real root(s) => Precision loss
			return 0.0;
		} else { // Two real roots: From http://mathworld.wolfram.com/QuadraticEquation.html for precision
			T const q( -0.5 * ( b + ( sign( b ) * std::sqrt( disc ) ) ) );
			if ( b + 2.0 * q <= 0.0 ) { // Crossing direction test
				return std::max( q / a, 0.0 );
			} else {
				return std::max( c / q, 0.0 );
			}
		}
	}
}

// Min Nonnegative Root of Upper Boundary Quadratic Equation a x^2 + b x + c = 0
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
min_root_quadratic_upper( T const a, T const b, T const c )
{
	assert( a >= 0.0 );
	assert( b >= 0.0 );
	if ( c >= 0.0 ) { // Precision loss: x(tX) > q(tX) + qTol
		return 0.0;
	} else if ( a == 0.0 ) { // Linear
		if ( b == 0.0 ) { // Parallel
			return infinity;
		} else {
			return -( c / b );
		}
	} else { // Quadratic
		T const disc( ( b * b ) - ( 4.0 * a * c ) );
		if ( disc <= 0.0 ) { // Zero or one real root(s) => Precision loss
			return 0.0;
		} else { // Two real roots: From http://mathworld.wolfram.com/QuadraticEquation.html for precision
			T const q( -0.5 * ( b + ( sign( b ) * std::sqrt( disc ) ) ) );
			if ( b + 2.0 * q >= 0.0 ) { // Crossing direction test
				return std::max( q / a, 0.0 );
			} else {
				return std::max( c / q, 0.0 );
			}
		}
	}
}

// Min Nonnegative Root of Both Boundary Quadratic Equations a x^2 + b x + c = 0
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
min_root_quadratic_both( T const a, T const b, T const cl, T const cu )
{
	if ( ( cl <= 0.0 ) || ( cu >= 0.0 ) ) { // Precision loss: x(tX) < q(tX) - qTol or x(tX) > q(tX) + qTol
		return 0.0;
	} else if ( a == 0.0 ) { // Linear
		if ( b == 0.0 ) { // Parallel
			return infinity;
		} else if ( b <= 0.0 ) {
			return -( cl / b );
		} else {
			return -( cu / b );
		}
	} else { // Quadratic
		T const bb( b * b );
		T const a4( 4.0 * a );

		// Lower boundary
		T const discl( bb - ( a4 * cl ) );
		T rootl;
		if ( discl < 0.0 ) { // No real roots
			rootl = infinity;
		} else if ( discl == 0.0 ) { // One real root
			rootl = -b / ( 2.0 * a );
			if ( rootl < 0.0 ) rootl = infinity;
		} else { // Two real roots: From http://mathworld.wolfram.com/QuadraticEquation.html for precision
			T const q( -0.5 * ( b + ( sign( b ) * std::sqrt( discl ) ) ) );
			if ( b + 2.0 * q <= 0.0 ) { // Crossing direction test
				rootl = q / a;
			} else {
				rootl = cl / q;
			}
		}

		// Upper boundary
		T const discu( bb - ( a4 * cu ) );
		T rootu;
		if ( discu < 0.0 ) { // No real roots
			rootu = infinity;
		} else if ( discu == 0.0 ) { // One real root
			rootu = -b / ( 2.0 * a );
			if ( rootu < 0.0 ) rootu = infinity;
		} else { // Two real roots: From http://mathworld.wolfram.com/QuadraticEquation.html for precision
			T const q( -0.5 * ( b + ( sign( b ) * std::sqrt( discu ) ) ) );
			if ( b + 2.0 * q >= 0.0 ) { // Crossing direction test
				rootu = q / a;
			} else {
				rootu = cu / q;
			}
		}

		if ( ( rootl == infinity ) && ( rootu == infinity ) ) { // Precision loss
			return 0.0;
		} else {
			return std::max( std::min( rootl, rootu ), 0.0 );
		}
	}
}

// Min Nonnegative Root of Cubic Equation a x^3 + b x^2 + c x + d = 0
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
min_root_cubic( T a, T b, T c, T const d )
{
	static T const one_54( 1.0 / 54.0 );
	static T const two_thirds_pi( ( 2.0 / 3.0 ) * pi );
	if ( a == 0.0 ) { // Quadratic
		return min_root_quadratic( b, c, d );
	} else { // Cubic
		T const inv_a( 1.0 / a ); // Normalize to x^3 + a x^2 + b x + c
		a = b * inv_a;
		b = c * inv_a;
		c = d * inv_a;
		T const a_3( one_third * a );
		T const a2( a * a );
		T const q( a2 - ( 3.0 * b ) );
		T const r( ( ( ( 2.0 * a2 ) - ( 9.0 * b ) ) * a ) + ( 27.0 * c ) );
		if ( ( q == 0.0 ) && ( r == 0.0 ) ) {
			return -a_3;
		} else {
			T const Q( one_ninth * q );
			T const R( one_54 * r );
			T const Q3( Q * Q * Q );
			T const R2( R * R );
			if ( R2 < Q3 ) {
				T const norm( -2.0 * std::sqrt( Q ) );
				T const theta_3( one_third * std::acos( sign( R ) * std::sqrt ( R2 / Q3 ) ) );
				T const root1( norm * std::cos( theta_3 - a_3 ) );
				T const root2( norm * std::cos( theta_3 + two_thirds_pi ) - a_3 );
				T const root3( norm * std::cos( theta_3 - two_thirds_pi ) - a_3 );
				return ( root1 >= 0.0 ? ( root2 >= 0.0 ? ( root3 >= 0.0 ?
				 min( root1, root2, root3 ) : // roots 1,2,3 >= 0
				 std::min( root1, root2 ) ) : // roots 1,2 >= 0
				 ( root3 >= 0.0 ?
				 std::min( root1, root3 ) : // roots 1,3 >= 0
				 root1 ) ) : // root 1 >= 0
				 ( root2 >= 0.0 ?
				 ( root3 >= 0.0 ?
				 std::min( root2, root3 ) : // roots 2,3 >= 0
				 root2 ) : // root2 >= 0
				 ( root3 >= 0.0 ?
				 root3 : // root3 >= 0
				 infinity ) ) ); // All roots < 0
			} else {
				T const CR2( 729 * r * r );
				T const CQ3( 2916 * q * q * q );
				if ( CR2 == CQ3 ) {
					T const sqrt_Q( std::sqrt( Q ) );
					if ( R > 0.0 ) {
						T const root1( -( 2.0 * sqrt_Q ) - a_3 );
						T const root2( sqrt_Q - a_3 );
						return ( root1 >= 0.0 ? ( root2 >= 0.0 ? std::min( root1, root2 ) : root1 ) : ( root2 >= 0.0 ? root2 : infinity ) );
					} else {
						T const root1( ( 2.0 * sqrt_Q ) - a_3 );
						T const root2( -sqrt_Q - a_3 );
						return ( root1 >= 0.0 ? ( root2 >= 0.0 ? std::min( root1, root2 ) : root1 ) : ( root2 >= 0.0 ? root2 : infinity ) );
					}
				} else { // One real root
					T const A( -sign( R ) * std::cbrt( std::abs( R ) + std::sqrt( R2 - Q3 ) ) );
					T const B( Q / A );
					T const root( A + B - a_3 );
					return ( root >= 0.0 ? root : infinity );
				}
			}
		}
	}
}

#endif
