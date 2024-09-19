// QSS Solver Math Support
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2024 Objexx Engineering, Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
// (3) Neither the name of the copyright holder nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, THE UNITED STATES
// GOVERNMENT, OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef QSS_math_hh_INCLUDED
#define QSS_math_hh_INCLUDED

// C++ Headers
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cmath>
#include <limits>
//#include <numbers> //C++20
#include <type_traits>
#include <utility>

namespace QSS {

// Notes
// - Iterative root finders could be accelerated for multiple roots by detecting linear convergence
//   with same sign x steps but given the rarity of multiple (or near multiple) roots this is not
//   believed to be worth the extra loop overhead so we allow many iterations but require a strictly
//   decreasing value instead.
// - Halley iterative methods were not seen to be faster in testing so far but this can be revisited.

// Globals
static constexpr double zero{ 0.0 };
static constexpr double one{ 1.0 };
static constexpr double two{ 2.0 };
static constexpr double three{ 3.0 };
static constexpr double four{ 4.0 };
static constexpr double six{ 6.0 };
static constexpr double one_half{ 0.5 };
static constexpr double one_third{ 1.0 / 3.0 };
static constexpr double one_fourth{ 0.25 };
static constexpr double one_sixth{ 1.0 / 6.0 };
static constexpr double one_ninth{ 1.0 / 9.0 };
static constexpr double two_thirds{ 2.0 / 3.0 };
static constexpr double pi{ 3.141592653589793115997963 }; //C++20 static constexpr double pi{ std::numbers::pi };  Not working with Intel Classic C++ as of version 2021.9.0 on Windows
static constexpr double infinity{ std::numeric_limits< double >::has_infinity ? std::numeric_limits< double >::infinity() : std::numeric_limits< double >::max() }; //! std::isinf() won't work with Intel C++ with /fp:fast
// static constexpr double infinity{ std::numeric_limits< double >::max() };
static constexpr double neg_infinity{ std::numeric_limits< double >::has_infinity && std::numeric_limits<double>::is_iec559 ? -std::numeric_limits< double >::infinity() : std::numeric_limits< double >::lowest() };
// static constexpr double neg_infinity{ std::numeric_limits< double >::lowest() };
static constexpr double half_infinity{ 0.5 * infinity };

// General

// Sign: Returns Passed Type as Boolean
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
bool
bool_sign( T const x )
{
	return x < T( 0 ) ? false : true;
}

// Sign: Returns Passed Type as Integer
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
int
int_sign( T const x )
{
	return x < T( 0 ) ? -1 : +1;
}

// Sign: Returns Passed Type
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
sign( T const x )
{
	return x < T( 0 ) ? T( -1 ) : T( +1 );
}

// Signs of 2 Values Same?
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
bool
signs_same( T const x, T const y )
{
	return sign( x ) == sign( y );
}

// Signs of 2 Values Differ?
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
bool
signs_differ( T const x, T const y )
{
	return sign( x ) != sign( y );
}

// Signs of 2 Values Same and Values are Non-Zero?
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
bool
nonzero_and_signs_same( T const x, T const y )
{
	return ( ( x < T( 0 ) ) && ( y < T( 0 ) ) ) || ( ( x > T( 0 ) ) && ( y > T( 0 ) ) );
}

// Signs of 2 Values Differ and Values are Non-Zero?
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
bool
nonzero_and_signs_differ( T const x, T const y )
{
	return ( ( x < T( 0 ) ) && ( y > T( 0 ) ) ) || ( ( x > T( 0 ) ) && ( y < T( 0 ) ) );
}

// Signum: Returns Passed Type
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
sgn( T const x )
{
	return x < T( 0 ) ? T( -1 ) : ( x > T( 0 ) ? T( +1 ) : T( 0 ) );
}

// Signum: Returns int
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
int
signum( T const x )
{
	return x < T( 0 ) ? -1 : ( x > T( 0 ) ? +1 : 0 );
}

// Signums of 2 Values Same?
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
bool
signums_same( T const x, T const y )
{
	return signum( x ) == signum( y );
}

// Signums of 2 Values Differ?
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
bool
signums_differ( T const x, T const y )
{
	return signum( x ) != signum( y );
}

// Square
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
square( T const x )
{
	return x * x;
}

// Cube
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
cube( T const x )
{
	return x * x * x;
}

// Quad (4th Power)
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
quad( T const x )
{
	return square( x * x );
}

// Value Clipped to Legal Arc Cosine Range
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
acos_clipped( T const x )
{
	return std::min( std::max( x, T( -1 ) ), T( 1 ) );
}

// Infinity
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
inf()
{
	return std::numeric_limits< T >::has_infinity ? std::numeric_limits< T >::infinity() : std::numeric_limits< T >::max();
	// return std::numeric_limits< T >::max();
}

// Use std::min for 2 arguments
using std::min;

// Min of 3 Values
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
min( T const x, T const y, T const z )
{
	return x < y ? ( x < z ? x : z ) : ( y < z ? y : z );
}

// Min of 4+ Values
template< typename T, typename... Ts, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
min( T const a, T const b, T const c, T const d, Ts const... o )
{
	return min( a < b ? a : b, c < d ? c : d, o... );
}

// Min Nonnegative of 2 Values or Zero
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
min_nonnegative_or_zero( T const x, T const y )
{
	return x >= T( 0 ) ? ( y >= T( 0 ) ? std::min( x, y ) : x ) : ( y >= T( 0 ) ? y : T( 0 ) );
}

// Min Positive of 2 Values or Zero
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
min_positive_or_zero( T const x, T const y )
{
	return x > T( 0 ) ? ( y > T( 0 ) ? std::min( x, y ) : x ) : ( y > T( 0 ) ? y : T( 0 ) );
}

// Min Positive of 3 Values or Zero
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
min_positive_or_zero( T const x, T const y, T const z )
{
	return x > T( 0 ) ?
	 ( y > T( 0 ) ? ( z > T( 0 ) ? min( x, y, z ) : std::min( x, y ) ) : ( z > T( 0 ) ? std::min( x, z ) : x ) ) : // x > 0
	 ( y > T( 0 ) ? ( z > T( 0 ) ? std::min( y, z ) : y ) : ( z > T( 0 ) ? z : T( 0 ) ) ); // x < 0
}

// Min Positive of 2 Values or Infinity
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
min_positive_or_infinity( T const x, T const y )
{
	return x > T( 0 ) ? ( y > T( 0 ) ? std::min( x, y ) : x ) : ( y > T( 0 ) ? y : inf< T >() );
}

// Min Positive of 3 Values or Infinity
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
min_positive_or_infinity( T const x, T const y, T const z )
{
	return x > T( 0 ) ?
	 ( y > T( 0 ) ? ( z > T( 0 ) ? min( x, y, z ) : std::min( x, y ) ) : ( z > T( 0 ) ? std::min( x, z ) : x ) ) : // x > 0
	 ( y > T( 0 ) ? ( z > T( 0 ) ? std::min( y, z ) : y ) : ( z > T( 0 ) ? z : inf< T >() ) ); // x < 0
}

// Use std::max for 2 arguments
using std::max;

// Max of 3 Values
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
max( T const x, T const y, T const z )
{
	return x < y ? ( y < z ? z : y ) : ( x < z ? z : x );
}

// Max of 4+ Values
template< typename T, typename... Ts, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
max( T const a, T const b, T const c, T const & d, Ts const... o )
{
	return max( a < b ? b : a, c < d ? d : c, o... );
}

// Value if Positive or Zero
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
nonnegative( T const r )
{
	return r > T( 0 ) ? r : T( 0 );
}

// Value if Positive or Infinity
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
positive_or_infinity( T const r )
{
	return r > T( 0 ) ? r : inf< T >();
}

// x == y Within Specified Relative or Absolute Tolerances?
template< typename T >
bool
eq_tol( T const & x, T const & y, T const & r_tol, T const & a_tol = T( 0 ) )
{
	assert( r_tol >= T( 0 ) );
	assert( a_tol >= T( 0 ) );
	return std::abs( x - y ) <= std::max( r_tol * std::max( std::abs( x ), std::abs( y ) ), a_tol );
}

// Sorted Array of 3 Values Making Non-Positive Values Infinity
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
std::array< T, 3 >
sorted_positive( T x, T y, T z )
{
	if ( x <= T( 0 ) ) x = inf< T >();
	if ( y <= T( 0 ) ) y = inf< T >();
	if ( z <= T( 0 ) ) z = inf< T >();
	if ( x > y ) std::swap( x, y );
	if ( x > z ) std::swap( x, z );
	if ( y > z ) std::swap( y, z );
	return std::array< T, 3 >{ x, y, z };
}

// Zero-Crossing Root if Given Value Meets Anti-Chatter Zero Crossing Tolerance or Infinity
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
zc_root_cull( T const root, T const val, T const zMag = T( 1e-6 ) )
{
	assert( zMag >= T( 0 ) );
	return std::abs( val ) >= zMag ? root : inf< T >();
}

// Zero-Crossing Root if Given Magnitude Meets Anti-Chatter Zero Crossing Tolerance or Infinity
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
zc_root_cull_mag( T const root, T const mag, T const zMag = T( 1e-6 ) )
{
	assert( mag >= T( 0 ) );
	assert( zMag >= T( 0 ) );
	return mag >= zMag ? root : inf< T >();
}

// Real-Valued Root
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
struct Root
{

	// Data
	T x{ T( 0 ) }; // Root
	T v{ T( 0 ) }; // Value at root
	bool valid{ false }; // Valid?

	// Default Constructor
	Root() = default;

	// Root Constructor
	Root(
	 T const x_,
	 T const v_
	) :
	 x( x_ ),
	 v( v_ ),
	 valid( true )
	{}

	// Root and Tolerance Constructor
	Root(
	 T const x_,
	 T const v_,
	 T const zTol
	) :
	 x( x_ ),
	 v( v_ ),
	 valid( std::abs( v ) <= zTol )
	{
		assert( zTol >= T( 0 ) );
	}

	// bool Conversion
	operator bool() const
	{
		return valid;
	}

}; // Root

// Linear

// Root of Linear Equation a x + b: Returns nonpositive value if no positive roots
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
root_linear( T const a, T const b )
{
	if ( a == T( 0 ) ) { // Constant
		return inf< T >();
	} else {
		return -( b / a );
	}
}

// Zero-Crossing Root of Linear Equation a x + b
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
zc_root_linear( T const a, T const b, T const zTol = T( 1e-6 ), T const xMag = T( 0 ) )
{
	assert( zTol >= T( 0 ) );
	assert( xMag >= T( 0 ) );
	if ( a == T( 0 ) ) { // Constant
		return inf< T >();
	} else if ( b == T( 0 ) ) { // Root at zero
		return inf< T >();
	} else if ( sign( a ) == sign( b ) ) { // Negative root
		return inf< T >();
	} else { // Positive root
		T const zMag( xMag < zTol ? zTol : T( 0 ) );
		return zc_root_cull( -( b / a ), b, zMag );
	}
}

// Quadratic

// Newton Iterative Positive Root Near Given Guess of Quadratic Equation a x^2 + b x + c
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
Root< T >
newton_positive_root_quadratic( T const a, T b, T c, T x, T zTol = T( 1e-6 ) )
{
	assert( x != inf< T >() );
	assert( zTol >= T( 0 ) );
	if ( a == T( 0 ) ) { // Linear
		if ( b == T( 0 ) ) { // Constant
			return Root< T >();
		} else if ( c == T( 0 ) ) { // Root at zero
			return Root< T >();
		} else if ( sign( b ) == sign( c ) ) { // Negative root
			return Root< T >();
		} else {
			return Root( -( c / b ), T( 0 ) );
		}
	} else { // Iterative solution: Monicize quadratic to x^2 + b x + c
		T const inv_a( T( 1 ) / a );
		b *= inv_a;
		c *= inv_a;
		zTol *= std::abs( inv_a );
		T x_p( x ); // Previous (initial) guess
		T xb( x + b );
		T v( ( xb * x ) + c ); // Value at x
		if ( v == T( 0 ) ) return Root( x, T( 0 ) ); // Done
		T v_p( T( 2 ) * v ); // Fake previous value to allow first loop pass
		std::size_t i( 0u );
		while ( ( ++i <= 50u ) && ( std::abs( v ) < std::abs( v_p ) ) ) { // Accept only direct convergence
			T const s( x + xb ); // Slope at current guess
			if ( s == T( 0 ) ) return Root( x, v, zTol ); // Stop
			x_p = x;
			x -= v / s; // New guess
			if ( x <= T( 0 ) ) return ( x_p > T( 0 ) ? Root( x_p, v_p, zTol ) : Root< T >() ); // Stop
			v_p = v;
			xb = x + b;
			v = ( xb * x ) + c; // Value at new guess
			if ( v == T( 0 ) ) return Root( x, T( 0 ) ); // Done
		}
		return std::abs( v ) <= std::abs( v_p ) ? Root( x, v, zTol ) : ( x_p > T( 0 ) ? Root( x_p, v_p, zTol ) : Root< T >() );
	}
}

// Halley Iterative Positive Root Near Given Guess of Quadratic Equation a x^2 + b x + c
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
Root< T >
halley_positive_root_quadratic( T const a, T b, T c, T x, T zTol = T( 1e-6 ) )
{
	assert( x != inf< T >() );
	assert( zTol >= T( 0 ) );
	if ( a == T( 0 ) ) { // Linear
		if ( b == T( 0 ) ) { // Constant
			return Root< T >();
		} else if ( c == T( 0 ) ) { // Root at zero
			return Root< T >();
		} else if ( sign( b ) == sign( c ) ) { // Negative root
			return Root< T >();
		} else {
			return Root( -( c / b ), T( 0 ) );
		}
	} else { // Iterative solution: Monicize quadratic to x^2 + b x + c
		T const inv_a( T( 1 ) / a );
		b *= inv_a;
		c *= inv_a;
		zTol *= std::abs( inv_a );
		T x_p( x ); // Previous (initial) guess
		T xb( x + b );
		T v( ( xb * x ) + c ); // Value at x
		if ( v == T( 0 ) ) return Root( x, T( 0 ) ); // Done
		T v_p( T( 2 ) * v ); // Fake previous value to allow first loop pass
		std::size_t i( 0u );
		while ( ( ++i <= 50u ) && ( std::abs( v ) < std::abs( v_p ) ) ) { // Accept only direct convergence
			T const s( x + xb ); // Slope at current guess
			T const u( ( s * s ) - v );
			if ( u == T( 0 ) ) return Root( x, v, zTol ); // Stop
			x_p = x;
			x -= ( v * s ) / u;
			if ( x <= T( 0 ) ) return ( x_p > T( 0 ) ? Root( x_p, v_p, zTol ) : Root< T >() ); // Stop
			v_p = v;
			xb = x + b;
			v = ( xb * x ) + c; // Value at new guess
			if ( v == T( 0 ) ) return Root( x, T( 0 ) ); // Done
		}
		return std::abs( v ) <= std::abs( v_p ) ? Root( x, v, zTol ) : ( x_p > T( 0 ) ? Root( x_p, v_p, zTol ) : Root< T >() );
	}
}

// Iterative Positive Root Near Given Guess of Quadratic Equation a x^2 + b x + c
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
Root< T >
iterative_positive_root_quadratic( T const a, T b, T c, T x, T const zTol = T( 1e-6 ) )
{
//	return newton_positive_root_quadratic( a, b, c, x, zTol );
	return halley_positive_root_quadratic( a, b, c, x, zTol );
}

// Max Critical Point Magnitude of Quadratic Equation a x^2 + b x + c on x in (0,t)
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
critical_point_magnitude_quadratic( T const a, T const b, T const c, T const t )
{
	assert( t > T( 0 ) );
	if ( a == T( 0 ) ) { // Linear: No critical point
		return T( 0 );
	} else if ( b == T( 0 ) ) { // Critical point at x=0
		return T( 0 );
	} else if ( signs_same( a, b ) ) { // Critical point on x<0
		return T( 0 );
	} else { // Critical point: Root of linear 2 a x + b
		T const a_inv( T( 1 ) / a );
		T const root( -( one_half * b * a_inv ) );
		return root < t ? std::abs( c - ( one_fourth * square( b ) * a_inv ) ) : T( 0 );
	}
}

// Zero-Crossing Root of Quadratic Equation a x^2 + b x + c
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
zc_root_quadratic( T const a, T const b, T const c, T const zTol = T( 1e-6 ), T const xMag = T( 0 ) )
{
	assert( zTol >= T( 0 ) );
	if ( a == T( 0 ) ) { // Linear
		return zc_root_linear( b, c, zTol, xMag );
	} else { // Quadratic
		if ( c == T( 0 ) ) { // Roots at 0 and -b/a
			if ( b == T( 0 ) ) { // No positive root
				return inf< T >();
			} else {
				if ( signs_differ( a, b ) ) {
					T const root( -( b / a ) );
					T const zMag( xMag < zTol ? zTol : T( 0 ) );
					return zc_root_cull( root, root * b * one_fourth, zMag ); // Extremum is at -b/2a == root/2 so on [0,root] with value -b^2/4a
				} else {
					return inf< T >();
				}
			}
		} else if ( b == T( 0 ) ) {
			T const zMag( xMag < zTol ? zTol : T( 0 ) );
			return signs_differ( a, c ) ? zc_root_cull( std::sqrt( -( c / a ) ), c, zMag ) : inf< T >(); // Extremum is at 0 with value c
		} else {
			T const bb( b * b );
			T const four_a( T( 4 ) * a );
			T const disc( bb - ( four_a * c ) );
			if ( disc <= T( 0 ) ) { // Zero or one real root(s)
				if ( disc == T( 0 ) ) { // One real (double) root
					T const zMag( xMag < zTol ? zTol : T( 0 ) );
					return signs_differ( a, b ) ? zc_root_cull( -( b / ( T( 2 ) * a ) ), c, zMag ) : inf< T >(); // Root is also extremum
				} else { // No real roots
					return inf< T >();
				}
			} else { // Two real roots: From https://mathworld.wolfram.com/QuadraticEquation.html for precision
				T const q( -T( 0.5 ) * ( b + ( sign( b ) * std::sqrt( disc ) ) ) );
				assert( q != T( 0 ) );

				// Root(s) if positive or zero
				T const root1( signs_same( q, a ) ? q / a : T( 0 ) );
				T const root2( signs_same( c, q ) ? c / q : T( 0 ) );

				if ( signs_same( a, c ) ) { // Roots have same sign: Extremum between them
					if ( root1 > T( 0 ) ) { // Both roots are positive
						assert( root2 > T( 0 ) );
						assert( signs_differ( a, b ) );
						assert( std::min( root1, root2 ) <= -( b / ( T( 2 ) * a ) ) );
						assert( std::max( root1, root2 ) >= -( b / ( T( 2 ) * a ) ) );
						T const zMag( xMag < zTol ? zTol : T( 0 ) );
						T const rootl( zc_root_cull( std::min( root1, root2 ), c, zMag ) );
						if ( rootl != inf< T >() ) return rootl; // Use min root
						return zc_root_cull( std::max( root1, root2 ), c - ( bb / four_a ), zMag ); // Try max root: Extremum on (0,rootu)
					} else { // Both roots are negative
						return inf< T >();
					}
				} else { // Root signs differ
					T const root( min_positive_or_infinity( root1, root2 ) ); // Select positive root
					assert( root > T( 0 ) );
					assert( root != inf< T >() );
					T const zMag( xMag < zTol ? zTol : T( 0 ) );
					if ( signs_same( a, b ) ) { // Extremum is on x < 0
						return zc_root_cull( root, c, zMag );
					} else { // Extremum is on (0,root) and has greater magnitude than c
						assert( -( b / ( T( 2 ) * a ) ) < root );
						return zc_root_cull( root, c - ( bb / four_a ), zMag );
					}
				}

				// Refine root: Probably not needed
				// if ( root != inf< T >() ) {
				// 	Root< T > const rr( iterative_positive_root_quadratic( a, b, c, root, zTol ) ); // Refined root
				// 	return rr ? rr.x : root;
				// } else {
				// 	return root;
				// }
			}
		}
	}
}

// Min Positive Root of Quadratic Equation a x^2 + b x + c: Returns nonpositive value if no positive roots
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
min_root_quadratic( T const a, T const b, T const c )
{
	if ( a == T( 0 ) ) { // Linear
		if ( b == T( 0 ) ) { // Constant
			return inf< T >();
		} else {
			return -( c / b );
		}
	} else { // Quadratic
		if ( b == T( 0 ) ) {
			if ( c == T( 0 ) ) { // Double root at zero
				return T( 0 );
			} else {
				T const c_a( -( c / a ) );
				if ( c_a > T( 0 ) ) {
					return std::sqrt( c_a );
				} else { // Two imaginary roots
					return T( 0 );
				}
			}
		} else {
			T const disc( ( b * b ) - ( T( 4 ) * a * c ) );
			if ( disc <= T( 0 ) ) { // Zero or one real root(s)
				if ( disc == T( 0 ) ) { // One real root
					return -( b / ( T( 2 ) * a ) );
				} else { // Two imaginary roots
					return T( 0 );
				}
			} else { // Two real roots: From https://mathworld.wolfram.com/QuadraticEquation.html for precision
				T const q( -T( 0.5 ) * ( b + ( sign( b ) * std::sqrt( disc ) ) ) );
				T const r1( c / q );
				T const r2( q / a );
				if ( r1 > T( 0 ) ) {
					return r2 > T( 0 ) ? std::min( r1, r2 ) : r1;
				} else {
					return r2; // Might be nonpositive
				}
			}
		}
	}
}

// Min Nonnegative Root of Lower Boundary Quadratic Equation a x^2 + b x + c
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
min_root_quadratic_lower( T const a, T const b, T const c )
{
	assert( a <= T( 0 ) );
	assert( b <= T( 0 ) );
	// c > 0 with exact precision
	if ( c <= T( 0 ) ) { // Precision loss: x(tX) < q(tX) - qTol
		return T( 0 );
	} else if ( a == T( 0 ) ) { // Linear
		if ( b == T( 0 ) ) { // Constant
			return inf< T >();
		} else {
			assert( -( c / b ) > T( 0 ) );
			return -( c / b );
		}
	} else { // Quadratic
		if ( b == T( 0 ) ) {
			assert( -( c / a ) > T( 0 ) );
			return std::sqrt( -( c / a ) );
		} else {
			T const disc( ( b * b ) - ( T( 4 ) * a * c ) );
			if ( disc <= T( 0 ) ) { // Zero or one real root(s) => Precision loss
				return T( 0 );
			} else { // Two real roots: From https://mathworld.wolfram.com/QuadraticEquation.html for precision
				T const q( -T( 0.5 ) * ( b - std::sqrt( disc ) ) );
				assert( q > T( 0 ) );
				assert( ( T( 2 ) * q ) + b >= T( 0 ) ); // Crossing direction test
				assert( c / q > T( 0 ) );
				assert( q / a < T( 0 ) );
				return c / q;
			}
		}
	}
}

// Min Nonnegative Root of Upper Boundary Quadratic Equation a x^2 + b x + c
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
min_root_quadratic_upper( T const a, T const b, T const c )
{
	assert( a >= T( 0 ) );
	assert( b >= T( 0 ) );
	// c < 0 with exact precision
	if ( c >= T( 0 ) ) { // Precision loss: x(tX) > q(tX) + qTol
		return T( 0 );
	} else if ( a == T( 0 ) ) { // Linear
		if ( b == T( 0 ) ) { // Constant
			return inf< T >();
		} else {
			assert( -( c / b ) > T( 0 ) );
			return -( c / b );
		}
	} else { // Quadratic
		if ( b == T( 0 ) ) {
			assert( -( c / a ) > T( 0 ) );
			return std::sqrt( -( c / a ) );
		} else {
			T const disc( ( b * b ) - ( T( 4 ) * a * c ) );
			if ( disc <= T( 0 ) ) { // Zero or one real root(s) => Precision loss
				return T( 0 );
			} else { // Two real roots: From https://mathworld.wolfram.com/QuadraticEquation.html for precision
				T const q( -T( 0.5 ) * ( b + std::sqrt( disc ) ) );
				assert( q < T( 0 ) );
				assert( ( T( 2 ) * q ) + b <= T( 0 ) ); // Crossing direction test
				assert( c / q > T( 0 ) );
				assert( q / a < T( 0 ) );
				return c / q;
			}
		}
	}
}

// Min Nonnegative Root of Both Boundary Quadratic Equations a x^2 + b x + c
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
min_root_quadratic_both( T const a, T const b, T const cl, T const cu )
{
	// cl > 0 with exact precision
	// cu < 0 with exact precision
	if ( ( cl <= T( 0 ) ) || ( cu >= T( 0 ) ) ) { // Precision loss: x(tX) < q(tX) - qTol or x(tX) > q(tX) + qTol
		return T( 0 );
	} else if ( a == T( 0 ) ) { // Linear
		if ( b == T( 0 ) ) { // Constant
			return inf< T >();
		} else if ( b < T( 0 ) ) {
			return -( cl / b );
		} else {
			return -( cu / b );
		}
	} else { // Quadratic
		if ( b == T( 0 ) ) { // Simple case: Critical point at x=0
			if ( a < T( 0 ) ) {
				assert( -( cl / a ) > T( 0 ) );
				return std::sqrt( -( cl / a ) );
			} else {
				assert( a > T( 0 ) );
				assert( -( cu / a ) > T( 0 ) );
				return std::sqrt( -( cu / a ) );
			}
		} else { // General case: b != 0
			T const bb( b * b );
			T const a4( T( 4 ) * a );

			// Lower boundary
			T const discl( bb - ( a4 * cl ) );
			T rootl;
			if ( discl < T( 0 ) ) { // No real roots
				rootl = inf< T >();
			} else if ( discl == T( 0 ) ) { // One real (double) root
				rootl = positive_or_infinity( -b / ( T( 2 ) * a ) );
			} else { // Two real roots: From https://mathworld.wolfram.com/QuadraticEquation.html for precision
				T const q( -T( 0.5 ) * ( b + ( sign( b ) * std::sqrt( discl ) ) ) );
				if ( ( T( 2 ) * q ) + b <= T( 0 ) ) { // Crossing direction test: Root might be negative
					rootl = q / a;
				} else {
					rootl = cl / q;
				}
			}

			// Upper boundary
			T const discu( bb - ( a4 * cu ) );
			T rootu;
			if ( discu < T( 0 ) ) { // No real roots
				rootu = inf< T >();
			} else if ( discu == T( 0 ) ) { // One real (double) root
				rootu = positive_or_infinity( -b / ( T( 2 ) * a ) );
			} else { // Two real roots: From https://mathworld.wolfram.com/QuadraticEquation.html for precision
				T const q( -T( 0.5 ) * ( b + ( sign( b ) * std::sqrt( discu ) ) ) );
				if ( ( T( 2 ) * q ) + b >= T( 0 ) ) { // Crossing direction test: Root might be negative
					rootu = q / a;
				} else {
					rootu = cu / q;
				}
			}

			// Select root
			if ( ( rootl == inf< T >() ) && ( rootu == inf< T >() ) ) { // Precision loss
				return T( 0 );
			} else {
				return min_nonnegative_or_zero( rootl, rootu );
			}
		}
	}
}

// Min Nonnegative Root of Both Boundary Quadratic Equations a x^2 + b x + c
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
std::pair< T, T >
min_root_quadratic_both_c( T const a, T const b, T const cl, T const cu )
{
	// cl > 0 with exact precision
	// cu < 0 with exact precision
	if ( cl <= T( 0 ) ) { // Precision loss: x(tX) < q(tX) - qTol
		return std::make_pair( T( 0 ), cl );
	} else if ( cu >= T( 0 ) ) { // Precision loss: x(tX) > q(tX) + qTol
		return std::make_pair( T( 0 ), cu );
	} else if ( a == T( 0 ) ) { // Linear
		if ( b == T( 0 ) ) { // Constant
			return std::make_pair( inf< T >(), T( 0 ) );
		} else if ( b < T( 0 ) ) {
			return std::make_pair( -( cl / b ), cl );
		} else {
			return std::make_pair( -( cu / b ), cu );
		}
	} else { // Quadratic
		if ( b == T( 0 ) ) { // Simple case: Critical point at x=0
			if ( a < T( 0 ) ) {
				assert( -( cl / a ) > T( 0 ) );
				return std::make_pair( std::sqrt( -( cl / a ) ), cl );
			} else {
				assert( a > T( 0 ) );
				assert( -( cu / a ) > T( 0 ) );
				return std::make_pair( std::sqrt( -( cu / a ) ), cu );
			}
		} else { // General case: b != 0
			T const bb( b * b );
			T const a4( T( 4 ) * a );

			// Lower boundary
			T const discl( bb - ( a4 * cl ) );
			T rootl;
			if ( discl < T( 0 ) ) { // No real roots
				rootl = inf< T >();
			} else if ( discl == T( 0 ) ) { // One real (double) root
				rootl = positive_or_infinity( -b / ( T( 2 ) * a ) );
			} else { // Two real roots: From https://mathworld.wolfram.com/QuadraticEquation.html for precision
				T const q( -T( 0.5 ) * ( b + ( sign( b ) * std::sqrt( discl ) ) ) );
				if ( ( T( 2 ) * q ) + b <= T( 0 ) ) { // Crossing direction test: Root might be negative
					rootl = q / a;
				} else {
					rootl = cl / q;
				}
			}

			// Upper boundary
			T const discu( bb - ( a4 * cu ) );
			T rootu;
			if ( discu < T( 0 ) ) { // No real roots
				rootu = inf< T >();
			} else if ( discu == T( 0 ) ) { // One real (double) root
				rootu = positive_or_infinity( -b / ( T( 2 ) * a ) );
			} else { // Two real roots: From https://mathworld.wolfram.com/QuadraticEquation.html for precision
				T const q( -T( 0.5 ) * ( b + ( sign( b ) * std::sqrt( discu ) ) ) );
				if ( ( T( 2 ) * q ) + b >= T( 0 ) ) { // Crossing direction test: Root might be negative
					rootu = q / a;
				} else {
					rootu = cu / q;
				}
			}

			// Select root
			if ( ( rootl == inf< T >() ) && ( rootu == inf< T >() ) ) { // Precision loss
				return std::make_pair( T( 0 ), T( 0 ) );
			} else {
				if ( rootl >= T( 0 ) ) {
					if ( rootu >= T( 0 ) ) {
						return rootl <= rootu ? std::make_pair( rootl, cl ) : std::make_pair( rootu, cu );
					} else {
						return std::make_pair( rootl, cl );
					}
				} else if ( rootu >= T( 0 ) ) {
					return std::make_pair( rootu, cu );
				} else {
					return std::make_pair( T( 0 ), T( 0 ) );
				}
			}
		}
	}
}

// Cubic

// Value of Cubic Equation a x^3 + b x^2 + c x + d
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
cubic( T const a, T const b, T const c, T const d, T const x )
{
	return ( ( ( ( ( a * x ) + b ) * x ) + c ) * x ) + d;
}

// Value of Monic Cubic Equation x^3 + a x^2 + b x + c
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
cubic_monic( T const a, T const b, T const c, T const x )
{
	return ( ( ( ( x + a ) * x ) + b ) * x ) + c;
}

// Root of Monic Cubic Equation x^3 + a x^2 + b x + c if it is Positive and Crosses Outward or Zero
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
cubic_cull( T const a, T const b, T const r )
{
	return r > T( 0 ) ? ( ( T( 3 ) * r * r ) + ( T( 2 ) * a * r ) + b >= T( 0 ) ? r : T( 0 ) ) : T( 0 );
}

// Root of Monic Cubic Equation x^3 + a x^2 + b x + c if it is Positive and Crosses Downward or Zero
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
cubic_cull_lower( T const a, T const b, T const r )
{
	return ( r > T( 0 ) ) && ( ( T( 3 ) * r * r ) + ( T( 2 ) * a * r ) + b <= T( 0 ) ) ? r : T( 0 );
}

// Root of Monic Cubic Equation x^3 + a x^2 + b x + c if it is Positive and Crosses Upward or Zero
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
cubic_cull_upper( T const a, T const b, T const r )
{
	return ( r > T( 0 ) ) && ( ( T( 3 ) * r * r ) + ( T( 2 ) * a * r ) + b >= T( 0 ) ) ? r : T( 0 );
}

// Newton Iterative Positive Root Near x=0 of Monic Cubic Equation x^3 + a x^2 + b x + c
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
Root< T >
newton_small_positive_root_cubic_monic( T const a, T const b, T const c, T const zTol = T( 1e-6 ) )
{
	assert( zTol >= T( 0 ) );
	if ( ( c == T( 0 ) ) || ( sign( b ) == sign( c ) ) ) return Root< T >(); // Don't bother looking near x=0
	T x_p( T( 0 ) ); // Previous (initial) guess
	T v_p( c ); // Value at x=0
	T x( T( 0 ) ); // Initial (nonnegative) guess
	if ( b == T( 0 ) ) { // Special case iniital guess
		if ( c < T( 0 ) ) {
			x = std::cbrt( -c );
			if ( a > T( 0 ) ) {
				x = std::min( x, std::sqrt( -( c / a ) ) );
			}
		} else if ( a < T( 0 ) ) {
			assert( c > T( 0 ) );
			x = std::sqrt( -( c / a ) );
		} else {
			assert( a >= T( 0 ) );
			assert( c > T( 0 ) );
			return Root< T >(); // Don't bother looking near x=0
		}
	} else {
		x = -( c / b ); // Initial (nonnegative) guess
	}
	if ( x <= T( 0 ) ) return Root< T >(); // Give up
	T q( x + a );
	T r( ( q * x ) + b );
	T v( ( r * x ) + c ); // Value at x
	std::size_t i( 0u );
	while ( ( ++i <= 50u ) && ( std::abs( v ) < std::abs( v_p ) ) ) { // Accept only direct convergence
		T const s( ( ( x + q ) * x ) + r ); // Slope at current guess
		if ( s == T( 0 ) ) return Root( x, v, zTol ); // Stop
		x_p = x;
		x -= v / s; // New guess
		if ( x <= T( 0 ) ) return ( x_p > T( 0 ) ? Root( x_p, v_p, zTol ) : Root< T >() ); // Stop
		v_p = v;
		q = x + a;
		r = ( q * x ) + b;
		v = ( r * x ) + c; // Value at x
		if ( v == T( 0 ) ) return Root( x, T( 0 ) ); // Done
	}
	return std::abs( v ) <= std::abs( v_p ) ? Root( x, v, zTol ) : ( x_p > T( 0 ) ? Root( x_p, v_p, zTol ) : Root< T >() );
}

// Halley Iterative Positive Root Near x=0 of Monic Cubic Equation x^3 + a x^2 + b x + c
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
Root< T >
halley_small_positive_root_cubic_monic( T const a, T const b, T const c, T const zTol = T( 1e-6 ) )
{
	assert( zTol >= T( 0 ) );
	if ( ( c == T( 0 ) ) || ( sign( b ) == sign( c ) ) ) return Root< T >(); // Don't bother looking near x=0
	T x_p( T( 0 ) ); // Previous (initial) guess
	T v_p( c ); // Value at x=0
	T x( T( 0 ) ); // Initial guess
	if ( b == T( 0 ) ) { // Special case iniital guess
		if ( c < T( 0 ) ) {
			x = std::cbrt( -c );
			if ( a > T( 0 ) ) {
				x = std::min( x, std::sqrt( -( c / a ) ) );
			}
		} else if ( a < T( 0 ) ) {
			assert( c > T( 0 ) );
			x = std::sqrt( -( c / a ) );
		} else {
			assert( a >= T( 0 ) );
			assert( c > T( 0 ) );
			return Root< T >(); // Don't bother looking near x=0
		}
	} else {
		x = -( c * b ) / ( ( b * b ) - ( c * a ) ); // Initial guess
	}
	if ( x <= T( 0 ) ) return Root< T >(); // Give up
	T q( x + a );
	T x_q( x + q );
	T r( ( q * x ) + b );
	T v( ( r * x ) + c ); // Value at x
	std::size_t i( 0u );
	while ( ( ++i <= 50u ) && ( std::abs( v ) < std::abs( v_p ) ) ) { // Accept only direct convergence
		T const s( ( x_q * x ) + r ); // Slope at current guess
		T const u( ( T( 2 ) * s * s ) - ( v * T( 3 ) * x_q ) );
		if ( u == T( 0 ) ) return Root( x, v, zTol ); // Stop
		x_p = x;
		x -= ( T( 2 ) * v * s ) / u; // New guess
		if ( x <= T( 0 ) ) return ( x_p > T( 0 ) ? Root( x_p, v_p, zTol ) : Root< T >() ); // Stop
		v_p = v;
		q = x + a;
		x_q = x + q;
		r = ( q * x ) + b;
		v = ( r * x ) + c; // Value at x
		if ( v == T( 0 ) ) return Root( x, T( 0 ) ); // Done
	}
	return std::abs( v ) <= std::abs( v_p ) ? Root( x, v, zTol ) : ( x_p > T( 0 ) ? Root( x_p, v_p, zTol ) : Root< T >() );
}

// Iterative Positive Root Near x=0 of Monic Cubic Equation x^3 + a x^2 + b x + c
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
Root< T >
iterative_small_positive_root_cubic_monic( T const a, T const b, T const c, T const zTol = T( 1e-6 ) )
{
//	return newton_small_positive_root_cubic_monic( a, b, c, zTol );
	return halley_small_positive_root_cubic_monic( a, b, c, zTol );
}

// Newton Iterative Positive Root Near Given Guess of Monic Cubic Equation x^3 + a x^2 + b x + c
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
Root< T >
newton_positive_root_cubic_monic( T const a, T const b, T const c, T x, T const zTol = T( 1e-6 ) )
{
	assert( x > T( 0 ) );
	assert( x != inf< T >() );
	assert( zTol >= T( 0 ) );
	T x_p( x ); // Previous (initial) guess
	T q( x + a );
	T r( ( q * x ) + b );
	T v( ( r * x ) + c ); // Value at x
	if ( v == T( 0 ) ) return Root( x, T( 0 ) ); // Done
	T v_p( T( 2 ) * v ); // Fake previous value to allow first loop pass
	std::size_t i( 0u );
	while ( ( ++i <= 50u ) && ( std::abs( v ) < std::abs( v_p ) ) ) { // Accept only direct convergence
		T const s( ( ( x + q ) * x ) + r ); // Slope at current guess
		if ( s == T( 0 ) ) return Root( x, v, zTol ); // Stop
		x_p = x;
		x -= v / s; // New guess
		if ( x <= T( 0 ) ) return ( x_p > T( 0 ) ? Root( x_p, v_p, zTol ) : Root< T >() ); // Stop
		v_p = v;
		q = x + a;
		r = ( q * x ) + b;
		v = ( r * x ) + c; // Value at new guess
		if ( v == T( 0 ) ) return Root( x, T( 0 ) ); // Done
	}
	return std::abs( v ) <= std::abs( v_p ) ? Root( x, v, zTol ) : ( x_p > T( 0 ) ? Root( x_p, v_p, zTol ) : Root< T >() );
}

// Halley Iterative Positive Root Near Given Guess of Monic Cubic Equation x^3 + a x^2 + b x + c
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
Root< T >
halley_positive_root_cubic_monic( T const a, T const b, T const c, T x, T const zTol = T( 1e-6 ) )
{
	assert( x > T( 0 ) );
	assert( x != inf< T >() );
	assert( zTol >= T( 0 ) );
	T x_p( x ); // Previous (initial) guess
	T q( x + a );
	T x_q( x + q );
	T r( ( q * x ) + b );
	T v( ( r * x ) + c ); // Value at x
	if ( v == T( 0 ) ) return Root( x, T( 0 ) ); // Done
	T v_p( T( 2 ) * v ); // Fake previous value to allow first loop pass
	std::size_t i( 0u );
	while ( ( ++i <= 50u ) && ( std::abs( v ) < std::abs( v_p ) ) ) { // Accept only direct convergence
		T const s( ( x_q * x ) + r ); // Slope at current guess
		T const u( ( T( 2 ) * s * s ) - ( v * T( 3 ) * x_q ) );
		if ( u == T( 0 ) ) return Root( x, v, zTol ); // Stop
		x_p = x;
		x -= ( T( 2 ) * v * s ) / u; // New guess
		if ( x <= T( 0 ) ) return ( x_p > T( 0 ) ? Root( x_p, v_p, zTol ) : Root< T >() ); // Stop
		v_p = v;
		q = x + a;
		x_q = x + q;
		r = ( q * x ) + b;
		v = ( r * x ) + c; // Value at x
		if ( v == T( 0 ) ) return Root( x, T( 0 ) ); // Done
	}
	return std::abs( v ) <= std::abs( v_p ) ? Root( x, v, zTol ) : ( x_p > T( 0 ) ? Root( x_p, v_p, zTol ) : Root< T >() );
}

// Iterative Positive Root Near Given Guess of Monic Cubic Equation x^3 + a x^2 + b x + c
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
Root< T >
iterative_positive_root_cubic_monic( T const a, T const b, T const c, T x, T const zTol = T( 1e-6 ) )
{
//	return newton_positive_root_cubic_monic( a, b, c, x, zTol );
	return halley_positive_root_cubic_monic( a, b, c, x, zTol );
}

// Max Critical Point Magnitude of Cubic Equation a x^3 + b x^2 + c x + d on x in (0,t)
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
critical_point_magnitude_cubic( T const a, T const b, T const c, T const d, T const t )
{
	assert( t > T( 0 ) );
	if ( a == T( 0 ) ) { // Quadratic
		return critical_point_magnitude_quadratic( b, c, d, t );
	} else if ( ( a > T( 0 ) ) && ( c >= T( 0 ) ) && ( b >= T( 0 ) ) ) { // No critical points on x > 0
		return T( 0 );
	} else if ( ( a < T( 0 ) ) && ( c <= T( 0 ) ) && ( b <= T( 0 ) ) ) { // No critical points on x > 0
		return T( 0 );
	} else {
		T const A( T( 3 ) * a );
		T const B( T( 2 ) * b );
		if ( signs_same( c, ( A * square( t ) ) + ( B * t ) + c ) && signs_same( b, ( A * t ) + b ) ) { // 1st and 2nd derivatives of cubic are the same at x=0 and x=t: No critical point on (0,t)
			return T( 0 );
		} else { // Critical points: Roots of quadratic 3 a x^2 + 2 b x + c == A x^2 + B x + c
			if ( c == T( 0 ) ) {
				if ( nonzero_and_signs_differ( A, B ) ) { // One positive critical point at x=-B/A
					T const root( -( B / A ) );
					return root < t ? std::abs( cubic( a, b, c, d, root ) ) : T( 0 );
				} else { // No positive critical points
					return T( 0 );
				}
			} else if ( B == T( 0 ) ) {
				assert( c != T( 0 ) );
				if ( signs_differ( A, c ) ) { // One positive critical point at x=sqrt(-c/A)
					T const root( std::sqrt( -( c / A ) ) );
					return root < t ? std::abs( cubic( a, b, c, d, root ) ) : T( 0 );
				} else { // No positive critical points
					return T( 0 );
				}
			} else { // General case: B != 0, c != 0
				T const disc( square( B ) - ( T( 4 ) * A * c ) );
				if ( disc < T( 0 ) ) { // No critical points
					return T( 0 );
				} else if ( disc == T( 0 ) ) { // One real (double) root of quadratic
					if ( signs_differ( A, B ) ) { // Positive critical point at x=-B/2A
						T const root( -( B / ( T( 2 ) * A ) ) );
						return root < t ? std::abs( cubic( a, b, c, d, root ) ) : T( 0 );
					} else {
						return T( 0 );
					}
				} else { // Quadratic has two real roots: From https://mathworld.wolfram.com/QuadraticEquation.html for precision
					T const q( -T( 0.5 ) * ( B + ( sign( B ) * std::sqrt( disc ) ) ) );
					T const root1( q / A );
					T const root2( c / q );
					T mag( T( 0 ) );
					if ( ( root1 > T( 0 ) ) && ( root1 < t ) ) mag = std::abs( cubic( a, b, c, d, root1 ) );
					if ( ( root2 > T( 0 ) ) && ( root2 < t ) ) mag = std::max( mag, std::abs( cubic( a, b, c, d, root2 ) ) );
					return mag;
				}
			}
		}
	}
}

// Max Critical Point Magnitude of Monic Cubic Equation x^3 + a x^2 + b x + c on x in (0,t)
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
critical_point_magnitude_cubic_monic( T const a, T const b, T const c, T const t )
{
	assert( t > T( 0 ) );
	if ( ( a >= T( 0 ) ) && ( b >= T( 0 ) ) ) { // No critical points on x > 0
		return T( 0 );
	} else {
		if ( signs_same( b, ( T( 3 ) * square( t ) ) + ( T( 2 ) * a * t ) + b ) && signs_same( a, ( T( 3 ) * t ) + a ) ) { // 1st and 2nd derivatives are the same at x=0 and x=t: No critical point on (0,t)
			return T( 0 );
		} else { // Critical points: Roots of quadratic 3 x^2 + 2 a x + b == roots of monic x^2 + B x + C
			T const B( two_thirds * a );
			T const C( one_third * b );
			if ( C == T( 0 ) ) {
				if ( B < T( 0 ) ) { // One positive critical point at x=-B
					T const root( -B );
					return root < t ? std::abs( cubic_monic( a, b, c, root ) ) : T( 0 );
				} else { // No positive critical points
					return T( 0 );
				}
			} else if ( B == T( 0 ) ) {
				assert( C != T( 0 ) );
				if ( C < T( 0 ) ) { // One positive critical point at x=sqrt(-C)
					T const root( std::sqrt( -C ) );
					return root < t ? std::abs( cubic_monic( a, b, c, root ) ) : T( 0 );
				} else { // No positive critical points
					return T( 0 );
				}
			} else { // General case: B != 0, c != 0
				T const disc( square( B ) - ( T( 4 ) * C ) );
				if ( disc < T( 0 ) ) { // No critical points
					return T( 0 );
				} else if ( disc == T( 0 ) ) { // One real (double) root of quadratic
					if ( B < T( 0 ) ) { // Positive critical point at x=-B/2
						T const root( -( one_half * B ) );
						return root < t ? std::abs( cubic_monic( a, b, c, root ) ) : T( 0 );
					} else {
						return T( 0 );
					}
				} else { // Quadratic has two real roots: From https://mathworld.wolfram.com/QuadraticEquation.html for precision
					T const q( -T( 0.5 ) * ( B + ( sign( B ) * std::sqrt( disc ) ) ) );
					T const root1( q );
					T const root2( C / q );
					T mag( T( 0 ) );
					if ( ( root1 > T( 0 ) ) && ( root1 < t ) ) mag = std::abs( cubic_monic( a, b, c, root1 ) );
					if ( ( root2 > T( 0 ) ) && ( root2 < t ) ) mag = std::max( mag, std::abs( cubic_monic( a, b, c, root2 ) ) );
					return mag;
				}
			}
		}
	}
}

// Cull Zero-Crossing Root of Monic Cubic Equation x^3 + a x^2 + b x + c on x in (0,t) Considering Prior Magnitude and Extrema
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
zc_root_cull_cubic_monic( T const a, T const b, T const c, T const root, T const val, T const zMag )
{
	if ( zMag == T( 0 ) ) { // Extrema don't matter
		return root;
	} else { // Consider extrema
		T const mag( std::max( std::abs( val ), critical_point_magnitude_cubic_monic( a, b, c, root ) ) );
		return zc_root_cull_mag( root, mag, zMag );
	}
}

// Cull Zero-Crossing Root of Monic Cubic Equation x^3 + a x^2 + b x + c on x in (0,t) Considering Prior Magnitude and Extrema
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
zc_positive_root_cull_cubic_monic( T const a, T const b, T const c, T const root, T const val, T const zMag )
{
	if ( root <= T( 0 ) ) {
		return inf< T >();
	} else if ( zMag == T( 0 ) ) { // Extrema don't matter
		return root;
	} else { // Consider extrema
		T const mag( std::max( std::abs( val ), critical_point_magnitude_cubic_monic( a, b, c, root ) ) );
		return zc_root_cull_mag( root, mag, zMag );
	}
}

// Zero-Crossing Root of Cubic Equation a x^3 + b x^2 + c x + d
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
zc_root_cubic( T a, T b, T c, T const d, T zTol = T( 1e-6 ), T xMag = T( 0 ) )
{
	assert( zTol >= T( 0 ) );

	static T const inv_54( T( 1 ) / T( 54 ) );
	static T const inv_1458( T( 1 ) / T( 1458 ) );
	static T const two_thirds_pi( ( T( 2 ) / T( 3 ) ) * pi );

	if ( a == T( 0 ) ) { // Quadratic
		return zc_root_quadratic( b, c, d, zTol, xMag );
	} else { // Cubic

		// Monicize to x^3 + a x^2 + b x + c
		T const inv_a( T( 1 ) / a );
		a = b * inv_a;
		b = c * inv_a;
		c = d * inv_a;
		zTol *= std::abs( inv_a );
		xMag *= std::abs( inv_a );
		T const zMag( xMag < zTol ? zTol : T( 0 ) );

		// Look for positive root near x=0
		Root< T > const small_root( iterative_small_positive_root_cubic_monic( a, b, c, zTol ) );
		if ( small_root ) {
			T const root( zc_root_cull( small_root.x, c, zMag ) ); // Don't expect critical points before a small root
			if ( root != inf< T >() ) return root;
		}

		// Handle near-quadratic case where analytical cubic solution gets degraded
		if ( std::abs( a ) > T( 1e3 ) ) { // Near-quadratic
			T root( zc_root_quadratic( a, b, c, zTol, xMag ) );
			if ( ( root != inf< T >() ) && ( root > T( 0 ) ) ) { // Refine root
				Root< T > const quadratic_root( iterative_positive_root_cubic_monic( a, b, c, root, zTol ) );
				if ( quadratic_root ) return quadratic_root.x; // Assume safe not to cull since zc_root_quadratic culls
			}
		}

		// Analytical root(s)
		T const a_3( one_third * a );
		T const aa( a * a );
		T const q( aa - ( T( 3 ) * b ) );
		T const r( ( ( ( T( 2 ) * aa ) - ( T( 9 ) * b ) ) * a ) + ( T( 27 ) * c ) );
		if ( ( q == T( 0 ) ) && ( r == T( 0 ) ) ) {
			return zc_positive_root_cull_cubic_monic( a, b, c, -a_3, c, zMag );
		} else {
			T const CR2( T( 729 ) * ( r * r ) );
			T const CQ3( T( 2916 ) * ( q * q * q ) );
			if ( CR2 > CQ3 ) { // One real root
				T const A( -sign( r ) * std::cbrt( ( inv_54 * std::abs( r ) ) + ( inv_1458 * std::sqrt( CR2 - CQ3 ) ) ) );
				T const B( q / ( T( 9 ) * A ) );
				T root( A + B - a_3 );
				if ( root <= T( 0 ) ) return inf< T >();
				Root< T > const rr( iterative_positive_root_cubic_monic( a, b, c, root, zTol ) ); // Refine root
				return zc_root_cull_cubic_monic( a, b, c, rr.x, c, zMag );
			} else if ( CR2 < CQ3 ) { // Three real roots
				assert( q > T( 0 ) );
				T const sqrt_q( std::sqrt( q ) );
				T const scl( -two_thirds * sqrt_q );
				T const theta_3( one_third * std::acos( acos_clipped( T( 0.5 ) * r / cube( sqrt_q ) ) ) );
				T root1( ( scl * std::cos( theta_3 ) ) - a_3 );
				T root2( ( scl * std::cos( theta_3 + two_thirds_pi ) ) - a_3 );
				T root3( ( scl * std::cos( theta_3 - two_thirds_pi ) ) - a_3 );
				std::array< T, 3 > const roots( sorted_positive( root1, root2, root3 ) );
				assert( std::is_sorted( roots.begin(), roots.end() ) );
				for ( T root : roots ) {
					if ( root == inf< T >() ) return inf< T >(); // No more positive roots to try
					Root< T > const rr( iterative_positive_root_cubic_monic( a, b, c, root, zTol ) ); // Refine root
					root = zc_root_cull_cubic_monic( a, b, c, rr.x, c, zMag );
					if ( root != inf< T >() ) return root;
					// If root == infinity anti-chatter rejected it: Try next root
				}
				return inf< T >(); // No positive roots that meet anti-chatter
			} else { // Two real roots
				assert( CR2 == CQ3 );
				assert( q >= T( 0 ) );
				T const sqrt_Q( std::sqrt( one_ninth * q ) );
				if ( r > T( 0 ) ) {
					T root( -( T( 2 ) * sqrt_Q ) - a_3 ); // First root
					if ( root > T( 0 ) ) { // Must be smallest positive root
						Root< T > const rr( iterative_positive_root_cubic_monic( a, b, c, root, zTol ) ); // Refine root
						root = zc_root_cull_cubic_monic( a, b, c, rr.x, c, zMag );
						if ( root != inf< T >() ) return root;
						// If root == infinity anti-chatter rejected it: Try second root
					}
					root = sqrt_Q - a_3; // Second root
					if ( root <= T( 0 ) ) return inf< T >();
					Root< T > const rr( iterative_positive_root_cubic_monic( a, b, c, root, zTol ) ); // Refine root
					return zc_root_cull_cubic_monic( a, b, c, rr.x, c, zMag );
				} else {
					T root( -sqrt_Q - a_3 ); // First root
					if ( root > T( 0 ) ) { // Must be smallest positive root
						Root< T > const rr( iterative_positive_root_cubic_monic( a, b, c, root, zTol ) ); // Refine root
						root = zc_root_cull_cubic_monic( a, b, c, rr.x, c, zMag );
						if ( root != inf< T >() ) return root;
						// If root == infinity anti-chatter rejected it: Try second root
					}
					root = ( T( 2 ) * sqrt_Q ) - a_3; // Second root
					if ( root <= T( 0 ) ) return inf< T >();
					Root< T > const rr( iterative_positive_root_cubic_monic( a, b, c, root, zTol ) ); // Refine root
					return zc_root_cull_cubic_monic( a, b, c, rr.x, c, zMag );
				}
			}
		}
	}
}

// Min Nonnegative Analytical Root of Monic Cubic Equation x^3 + a x^2 + b x + c
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
min_root_cubic_monic_boundary_analytical( T const a, T const b, T const c, T const zTol = T( 1e-6 ) )
{
	// Preconditions for lower and upper boundary calls used by QSS to set tE in observer updates
	assert( a >= T( 0 ) );
	assert( b >= T( 0 ) );
	assert( c < T( 0 ) );
	assert( zTol >= T( 0 ) );

	static T const inv_54( T( 1 ) / T( 54 ) );
	static T const inv_1458( T( 1 ) / T( 1458 ) );
	static T const two_thirds_pi( ( T( 2 ) / T( 3 ) ) * pi );

	// Analytical root(s)
	T const a_3( one_third * a );
	T const aa( a * a );
	T const q( aa - ( T( 3 ) * b ) );
	T const r( ( ( ( T( 2 ) * aa ) - ( T( 9 ) * b ) ) * a ) + ( T( 27 ) * c ) );
	assert( ( q != T( 0 ) ) || ( r < T( 0 ) ) ); // q and r can't both be zero given signs of a, b, and c
	T const CR2( T( 729 ) * ( r * r ) );
	T const CQ3( T( 2916 ) * ( q * q * q ) );
	T root;
	if ( CR2 > CQ3 ) { // One real root
		T const A( -sign( r ) * std::cbrt( ( inv_54 * std::abs( r ) ) + ( inv_1458 * std::sqrt( CR2 - CQ3 ) ) ) );
		T const B( q / ( T( 9 ) * A ) );
		root = A + B - a_3;
	} else if ( CR2 < CQ3 ) { // Three real roots
		assert( q > T( 0 ) );
		T const sqrt_q( std::sqrt( q ) );
		T const scl( -two_thirds * sqrt_q );
		T const theta_3( one_third * std::acos( acos_clipped( T( 0.5 ) * r / cube( sqrt_q ) ) ) );

		// Crossing direction tests version: Can help exclude wrong root(s) when analytical solution is inaccurate
//		T const root1( cubic_cull( a, b, ( scl * std::cos( theta_3 ) ) - a_3 ) );
//		T const root2( cubic_cull( a, b, ( scl * std::cos( theta_3 + two_thirds_pi ) ) - a_3 ) );
//		T const root3( cubic_cull( a, b, ( scl * std::cos( theta_3 - two_thirds_pi ) ) - a_3 ) );

		// No crossing direction tests version
		T const root1( ( scl * std::cos( theta_3 ) ) - a_3 );
		T const root2( ( scl * std::cos( theta_3 + two_thirds_pi ) ) - a_3 );
		T const root3( ( scl * std::cos( theta_3 - two_thirds_pi ) ) - a_3 );

		root = min_positive_or_zero( root1, root2, root3 );
	} else { // Two real roots
		assert( CR2 == CQ3 );
		assert( q >= T( 0 ) );
		T const sqrt_Q( std::sqrt( one_ninth * q ) );
		if ( r > T( 0 ) ) {
			assert( -( T( 2 ) * sqrt_Q ) - a_3 < T( 0 ) ); // Other root is negative
			root = sqrt_Q - a_3;
		} else {
			assert( -sqrt_Q - a_3 < T( 0 ) ); // Other root is negative
			root = ( T( 2 ) * sqrt_Q ) - a_3;
		}
	}

	root = nonnegative( root ); // No crossing direction test
//	root = cubic_cull( a, b, root ); // Crossing direction test
	if ( root > T( 0 ) ) root = iterative_positive_root_cubic_monic( a, b, c, root, zTol ).x; // Root refinement
	return root;
}

// Min Nonnegative Root of Lower Boundary Cubic Equation a x^3 + b x^2 + c x + d
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
min_root_cubic_lower( T a, T b, T c, T const d, T zTol = T( 1e-6 ) )
{
	assert( a <= T( 0 ) );
	assert( b <= T( 0 ) );
	assert( c <= T( 0 ) );
	// d > 0 with exact precision
	assert( zTol >= T( 0 ) );

	if ( d <= T( 0 ) ) { // Precision loss
		return T( 0 );
	} else if ( a == T( 0 ) ) { // Quadratic
		return min_root_quadratic_lower( b, c, d );
	} else { // Cubic

		// Monicize to x^3 + a x^2 + b x + c
		T const inv_a( T( 1 ) / a ); // Negative
		a = b * inv_a; // Nonnegative
		b = c * inv_a; // Nonnegative
		c = d * inv_a; // Negative
		zTol *= std::abs( inv_a );

		// Look for positive root near x=0
		Root< T > const small_root( iterative_small_positive_root_cubic_monic( a, b, c, zTol ) );
		if ( small_root ) return small_root.x;

		// Handle near-quadratic case where analytical cubic solution gets degraded
		if ( std::abs( a ) > T( 1e3 ) ) { // Near-quadratic
			T root( min_root_quadratic_upper( a, b, c ) );
			if ( ( root != inf< T >() ) && ( root > T( 0 ) ) )  { // Refine root
				Root< T > const quadratic_root( iterative_positive_root_cubic_monic( a, b, c, root, zTol ) );
				if ( quadratic_root ) return quadratic_root.x;
			}
		}

		// Analytical root(s)
		return min_root_cubic_monic_boundary_analytical( a, b, c, zTol );
	}
}

// Min Nonnegative Root of Upper Boundary Cubic Equation a x^3 + b x^2 + c x + d
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
min_root_cubic_upper( T a, T b, T c, T const d, T zTol = T( 1e-6 ) )
{
	assert( a >= T( 0 ) );
	assert( b >= T( 0 ) );
	assert( c >= T( 0 ) );
	// d < 0 with exact precision
	assert( zTol >= T( 0 ) );

	if ( d >= T( 0 ) ) { // Precision loss
		return T( 0 );
	} else if ( a == T( 0 ) ) { // Quadratic
		return min_root_quadratic_upper( b, c, d );
	} else { // Cubic

		// Monicize to x^3 + a x^2 + b x + c
		T const inv_a( T( 1 ) / a ); // Positive
		a = b * inv_a; // Nonnegative
		b = c * inv_a; // Nonnegative
		c = d * inv_a; // Negative
		zTol *= std::abs( inv_a );

		// Look for positive root near x=0
		Root< T > const small_root( iterative_small_positive_root_cubic_monic( a, b, c, zTol ) );
		if ( small_root ) return small_root.x;

		// Handle near-quadratic case where analytical cubic solution gets degraded
		if ( std::abs( a ) > T( 1e3 ) ) { // Near-quadratic
			T root( min_root_quadratic_upper( a, b, c ) );
			if ( ( root != inf< T >() ) && ( root > T( 0 ) ) )  { // Refine root
				Root< T > const quadratic_root( iterative_positive_root_cubic_monic( a, b, c, root, zTol ) );
				if ( quadratic_root ) return quadratic_root.x;
			}
		}

		// Analytical root(s)
		return min_root_cubic_monic_boundary_analytical( a, b, c, zTol );
	}
}

// Min Nonnegative Root of Both Boundary Cubic Equations a x^3 + b x^2 + c x + d
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
T
min_root_cubic_both( T a, T b, T const c, T const dl, T const du, T zTol = T( 1e-6 ) )
{
	// dl > 0 with exact precision
	// du < 0 with exact precision
	assert( zTol >= T( 0 ) );

	static T const inv_54( T( 1 ) / T( 54 ) );
	static T const inv_1458( T( 1 ) / T( 1458 ) );
	static T const two_thirds_pi( ( T( 2 ) / T( 3 ) ) * pi );

	if ( ( dl <= T( 0 ) ) || ( du >= T( 0 ) ) ) { // Precision loss
		return T( 0 );
	} else if ( a == T( 0 ) ) { // Quadratic
		return min_root_quadratic_both( b, c, dl, du );
	} else { // Cubic

		// Monicize to x^3 + a x^2 + b x + c
		bool const s( bool_sign( a ) );
		T const inv_a( T( 1 ) / a );
		a = b * inv_a;
		b = c * inv_a;
		T const cl( ( s ? dl : du ) * inv_a );
		T const cu( ( s ? du : dl ) * inv_a );
		zTol *= std::abs( inv_a );

		// Look for positive root near x=0
		Root< T > const small_rootl( iterative_small_positive_root_cubic_monic( a, b, cl, zTol ) );
		Root< T > const small_rootu( iterative_small_positive_root_cubic_monic( a, b, cu, zTol ) );
		if ( small_rootl ) {
			if ( small_rootu ) {
				return std::min( small_rootl.x, small_rootu.x );
			} else {
				return small_rootl.x;
			}
		} else if ( small_rootu ) {
			return small_rootu.x;
		}

		// Handle near-quadratic case where analytical cubic solution gets degraded
		if ( std::abs( a ) > T( 1e3 ) ) { // Near-quadratic
			std::pair< T, T > const root_info( min_root_quadratic_both_c( a, b, cl, cu ) );
			T root( root_info.first );
			T cr( root_info.second );
			if ( ( root != inf< T >() ) && ( root > T( 0 ) ) )  { // Refine root
				Root< T > const quadratic_root( iterative_positive_root_cubic_monic( a, b, cr, root, zTol ) );
				if ( quadratic_root ) return quadratic_root.x;
			}
		}

		// Analytical root(s)
		T const a_3( one_third * a );
		T const aa( a * a );
		T const q( aa - ( T( 3 ) * b ) );
		T const rm( ( ( T( 2 ) * aa ) - ( T( 9 ) * b ) ) * a );
		T r;

		// Lower boundary
		T rootl;
		r = rm + ( T( 27 ) * cl );
		if ( ( q == T( 0 ) ) && ( r == T( 0 ) ) ) {
			rootl = std::max( -a_3, T( 0 ) );
		} else {
			T const CR2( T( 729 ) * ( r * r ) );
			T const CQ3( T( 2916 ) * ( q * q * q ) );
			if ( CR2 > CQ3 ) { // One real root
				T const A( -sign( r ) * std::cbrt( ( inv_54 * std::abs( r ) ) + ( inv_1458 * std::sqrt( CR2 - CQ3 ) ) ) );
				T const B( q / ( T( 9 ) * A ) );
				rootl = cubic_cull_lower( a, b, A + B - a_3 ); // No root refinement
				if ( rootl > T( 0 ) ) { // Refine root
					rootl = iterative_positive_root_cubic_monic( a, b, cl, rootl, zTol ).x;
				}
			} else if ( CR2 < CQ3 ) { // Three real roots
				T const sqrt_q( std::sqrt( q ) );
				T const scl( -two_thirds * sqrt_q );
				T const theta_3( one_third * std::acos( acos_clipped( T( 0.5 ) * r / cube( sqrt_q ) ) ) );
				T const root1( cubic_cull_lower( a, b, ( scl * std::cos( theta_3 ) ) - a_3 ) );
				T const root2( cubic_cull_lower( a, b, ( scl * std::cos( theta_3 + two_thirds_pi ) ) - a_3 ) );
				T const root3( cubic_cull_lower( a, b, ( scl * std::cos( theta_3 - two_thirds_pi ) ) - a_3 ) );
				rootl = min_positive_or_infinity( root1, root2, root3 );
				if ( rootl != inf< T >() ) { // Refine root
					rootl = iterative_positive_root_cubic_monic( a, b, cl, rootl, zTol ).x;
				}
			} else { // Two real roots
				assert( CR2 == CQ3 );
				T const sqrt_Q( std::sqrt( one_ninth * q ) );
				if ( r > T( 0 ) ) {
					rootl = cubic_cull_lower( a, b, -( T( 2 ) * sqrt_Q ) - a_3 );
					if ( rootl > T( 0 ) ) { // Must be smallest positive root
						rootl = iterative_positive_root_cubic_monic( a, b, cl, rootl, zTol ).x;
					} else {
						rootl = cubic_cull_lower( a, b, sqrt_Q - a_3 );
						if ( rootl > T( 0 ) ) { // Refine root
							rootl = iterative_positive_root_cubic_monic( a, b, cl, rootl, zTol ).x;
						}
					}
				} else {
					rootl = cubic_cull_lower( a, b, -sqrt_Q - a_3 );
					if ( rootl > T( 0 ) ) { // Must be smallest positive root
						rootl = iterative_positive_root_cubic_monic( a, b, cl, rootl, zTol ).x;
					} else {
						rootl = cubic_cull_lower( a, b, ( T( 2 ) * sqrt_Q ) - a_3 );
						if ( rootl > T( 0 ) ) { // Refine root
							rootl = iterative_positive_root_cubic_monic( a, b, cl, rootl, zTol ).x;
						}
					}
				}
			}
		}

		// Upper boundary
		T rootu;
		r = rm + ( T( 27 ) * cu );
		if ( ( q == T( 0 ) ) && ( r == T( 0 ) ) ) {
			rootu = std::max( -a_3, T( 0 ) );
		} else {
			T const CR2( T( 729 ) * ( r * r ) );
			T const CQ3( T( 2916 ) * ( q * q * q ) );
			if ( CR2 > CQ3 ) { // One real root
				T const A( -sign( r ) * std::cbrt( ( inv_54 * std::abs( r ) ) + ( inv_1458 * std::sqrt( CR2 - CQ3 ) ) ) );
				T const B( q / ( T( 9 ) * A ) );
				rootu = cubic_cull_upper( a, b, A + B - a_3 );
				if ( rootu > T( 0 ) ) { // Refine root
					rootu = iterative_positive_root_cubic_monic( a, b, cu, rootu, zTol ).x;
				}
			} else if ( CR2 < CQ3 ) { // Three real roots
				T const sqrt_q( std::sqrt( q ) );
				T const scl( -two_thirds * sqrt_q );
				T const theta_3( one_third * std::acos( acos_clipped( T( 0.5 ) * r / cube( sqrt_q ) ) ) );
				T const root1( cubic_cull_upper( a, b, ( scl * std::cos( theta_3 ) ) - a_3 ) );
				T const root2( cubic_cull_upper( a, b, ( scl * std::cos( theta_3 + two_thirds_pi ) ) - a_3 ) );
				T const root3( cubic_cull_upper( a, b, ( scl * std::cos( theta_3 - two_thirds_pi ) ) - a_3 ) );
				rootu = min_positive_or_infinity( root1, root2, root3 );
				if ( rootu != inf< T >() ) { // Refine root
					rootu = iterative_positive_root_cubic_monic( a, b, cu, rootu, zTol ).x;
				}
			} else { // Two real roots
				assert( CR2 == CQ3 );
				T const sqrt_Q( std::sqrt( one_ninth * q ) );
				if ( r > T( 0 ) ) {
					rootu = cubic_cull_upper( a, b, -( T( 2 ) * sqrt_Q ) - a_3 );
					if ( rootu > T( 0 ) ) { // Must be smallest positive root
						rootu = iterative_positive_root_cubic_monic( a, b, cu, rootu, zTol ).x;
					} else {
						rootu = cubic_cull_upper( a, b, sqrt_Q - a_3 );
						if ( rootu > T( 0 ) ) { // Refine root
							rootu = iterative_positive_root_cubic_monic( a, b, cu, rootu, zTol ).x;
						}
					}
				} else {
					rootu = cubic_cull_upper( a, b, -sqrt_Q - a_3 );
					if ( rootu > T( 0 ) ) { // Must be smallest positive root
						rootu = iterative_positive_root_cubic_monic( a, b, cu, rootu, zTol ).x;
					} else {
						rootu = cubic_cull_upper( a, b, ( T( 2 ) * sqrt_Q ) - a_3 );
						if ( rootu > T( 0 ) ) { // Refine root
							rootu = iterative_positive_root_cubic_monic( a, b, cu, rootu, zTol ).x;
						}
					}
				}
			}
		}

		return min_positive_or_infinity( rootl, rootu );
	}
}

} // QSS

#endif
