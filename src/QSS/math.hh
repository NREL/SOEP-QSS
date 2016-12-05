#ifndef QSS_math_hh_INCLUDED
#define QSS_math_hh_INCLUDED

// QSS Solver Math Functions

// C++ Headers
#include <algorithm>
#include <cmath>
#include <limits>
#include <type_traits>

// Globals
extern double const two;
extern double const three;
extern double const four;
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

// Sign: Returns Passed Type
template< typename T, class = typename std::enable_if< ! std::is_arithmetic< T >::value >::type >
inline
T
sign( T const & x )
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

// Min of 3 doubles
inline
double
min( double const a, double const b, double const c )
{
	return ( a < b ? ( a < c ? a : c ) : ( b < c ? b : c ) );
}

// Min Nonnegative Root of Quadratic Equation a x^2 + b x + c = 0
inline
double
min_root_quadratic( double const a, double const b, double const c )
{
	if ( a == 0.0 ) { // Linear
		if ( b == 0.0 ) { // Degenerate
			return infinity;
		} else {
			double const root( -( c / b ) );
			return ( root >= 0.0 ? root : infinity );
		}
	} else { // Quadratic
		double const disc( ( b * b ) - ( 4.0 * a * c ) );
		if ( disc < 0.0 ) { // No real roots
			return infinity;
		} else if ( disc == 0.0 ) { // One real root
			double const root( -( b / ( two * a ) ) );
			return ( root >= 0.0 ? root : infinity );
		} else { // Two real roots: From http://mathworld.wolfram.com/QuadraticEquation.html for precision
			double const q( -0.5 * ( b + ( sign( b ) * std::sqrt( disc ) ) ) );
			double const root1( q / a );
			double const root2( c / q );
			return ( root1 >= 0.0 ? ( root2 >= 0.0 ? std::min( root1, root2 ) : root1 ) : ( root2 >= 0.0 ? root2 : infinity ) );
		}
	}
}

// Min Nonnegative Root of Cubic Equation a x^3 + b x^2 + c x + d = 0
inline
double
min_root_cubic( double a, double b, double c, double const d )
{
	static double const one_54( 1.0 / 54.0 );
	static double const two_thirds_pi( ( 2.0 / 3.0 ) * pi );
	if ( a == 0.0 ) { // Quadratic
		return min_root_quadratic( b, c, d );
	} else { // Cubic
		double const inv_a( 1.0 / a ); // Normalize to x^3 + a x^2 + b x + c
		a = b * inv_a;
		b = c * inv_a;
		c = d * inv_a;
		double const a_3( one_third * a );
		double const a2( a * a );
		double const q( a2 - ( 3.0 * b ) );
		double const r( ( ( ( 2.0 * a2 ) - ( 9.0 * b ) ) * a ) + ( 27.0 * c ) );
		if ( ( q == 0.0 ) && ( r == 0.0 ) ) {
			return -a_3;
		} else {
			double const Q( one_ninth * q );
			double const R( one_54 * r );
			double const Q3( Q * Q * Q );
			double const R2( R * R );
			if ( R2 < Q3 ) {
				double const norm( -2.0 * std::sqrt( Q ) );
				double const theta_3( one_third * std::acos( sign( R ) * std::sqrt ( R2 / Q3 ) ) );
				double const root1( norm * std::cos( theta_3 - a_3 ) );
				double const root2( norm * std::cos( theta_3 + two_thirds_pi ) - a_3 );
				double const root3( norm * std::cos( theta_3 - two_thirds_pi ) - a_3 );
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
				double const CR2( 729 * r * r );
				double const CQ3( 2916 * q * q * q );
				if ( CR2 == CQ3 ) {
					double const sqrt_Q( std::sqrt( Q ) );
					if ( R > 0.0 ) {
						double const root1( -( 2.0 * sqrt_Q ) - a_3 );
						double const root2( sqrt_Q - a_3 );
						return ( root1 >= 0.0 ? ( root2 >= 0.0 ? std::min( root1, root2 ) : root1 ) : ( root2 >= 0.0 ? root2 : infinity ) );
					} else {
						double const root1( ( 2.0 * sqrt_Q ) - a_3 );
						double const root2( -sqrt_Q - a_3 );
						return ( root1 >= 0.0 ? ( root2 >= 0.0 ? std::min( root1, root2 ) : root1 ) : ( root2 >= 0.0 ? root2 : infinity ) );
					}
				} else { // One real root
					double const A( -sign( R ) * std::cbrt( std::abs( R ) + std::sqrt( R2 - Q3 ) ) );
					double const B( Q / A );
					double const root( A + B - a_3 );
					return ( root >= 0.0 ? root : infinity );
				}
			}
		}
	}
}

#endif
