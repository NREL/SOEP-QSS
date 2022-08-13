// QSS Binning Performance Optimizer
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2022 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_BinOptimizer_hh_INCLUDED
#define QSS_BinOptimizer_hh_INCLUDED

// QSS Headers
#include <QSS/math.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <utility>

namespace QSS {

// QSS Binning Performance Optimizer
//
// Simplistic until larger models can be tested
// Design should be reevaluated when automatic differentiation becomes available
class BinOptimizer final
{

public: // Types

	using size_type = std::size_t;
	using Time = double;
	using Velocity = double;

private: // Types

	// Bin Size + Solution Velocity Sample Point Class
	struct Point final
	{

		Point() :
		 bin_size( 0u ),
		 velocity( 0.0 )
		{}

		Point( size_type const bin_size_, Velocity const velocity_ ) :
		 bin_size( bin_size_ ),
		 velocity( velocity_ )
		{}

		void
		swap( Point & a, Point & b )
		{
			std::swap( a.bin_size, b.bin_size );
			std::swap( a.velocity, b.velocity );
		}

		size_type bin_size{ 0u };
		Velocity velocity{ 0.0 };

	}; // Point

public: // Creation

	// Constructor
	BinOptimizer( size_type const max_bin_size ) :
	 max_bin_size_( max_bin_size )
	{}

public: // Predicate

	// Valid Points (3 Distinct Bin Sizes in Order)
	bool
	valid() const
	{
		return ( ( 0u < l_.bin_size ) && ( l_.bin_size < m_.bin_size ) && ( m_.bin_size < u_.bin_size ) );
	}

public: // Property

	// Recommended Bin Size for Max Velocity
	size_type
	rec_bin_size() const
	{
		static size_type const one( 1u );
		size_type bin_size;
		if ( valid() ) {
			if ( m_.velocity > interp( l_, u_, m_.bin_size ) ) { // Choose parabolic max bin size
				size_type const x1( l_.bin_size );
				size_type const x2( m_.bin_size );
				size_type const x3( u_.bin_size );
				double const r1( l_.velocity / static_cast< Velocity >( ( x2 - x1 ) * ( x3 - x1 ) ) );
				double const r2( m_.velocity / -static_cast< Velocity >( ( x2 - x1 ) * ( x3 - x2 ) ) );
				double const r3( u_.velocity / static_cast< Velocity >( ( x3 - x2 ) * ( x3 - x1 ) ) );
				double const r_sum( r1 + r2 + r3 );
				double const opt_bin_size( r_sum != 0.0 ? ( ( r1 * ( x2 + x3 ) ) + ( r2 * ( x1 + x3 ) ) + ( r3 * ( x1 + x2 ) ) ) / ( 2.0 * r_sum ) : static_cast< double >( m_.bin_size ) ); // Bin size at velocity parabola max
				bin_size = std::min( std::max( std::min( static_cast< size_type >( opt_bin_size + 0.5 ), max_bin_size_ ), one ), max_bin_size_ );
			} else { // Recommend bin size in direction likely to give better velocity
				if ( ( l_.velocity <= m_.velocity ) && ( m_.velocity <= u_.velocity ) ) { // Velocity increasing with bin size
					bin_size = std::min( std::max( static_cast< size_type >( ( u_.bin_size * bin_fac ) + 0.5 ), u_.bin_size + one ), max_bin_size_ );
				} else if ( ( l_.velocity >= m_.velocity ) && ( m_.velocity >= u_.velocity ) ) { // Velocity decreasing with bin size
					bin_size = std::max( std::min( static_cast< size_type >( ( l_.bin_size / bin_fac ) + 0.5 ), l_.bin_size - one ), one );
				} else if ( ( u_.bin_size - m_.bin_size > m_.bin_size - l_.bin_size ) && ( l_.bin_size > 1u ) ) { // Recommend smaller bin size
					bin_size = std::max( std::min( static_cast< size_type >( ( l_.bin_size / bin_fac ) + 0.5 ), l_.bin_size - one ), one );
				} else { // Recommend larger bin size
					bin_size = std::min( std::max( static_cast< size_type >( ( u_.bin_size * bin_fac ) + 0.5 ), u_.bin_size + one ), max_bin_size_ );
				}
			}
		} else { // Not valid yet: Rec higher bin size
			bin_size = std::min( std::max( static_cast< size_type >( ( u_.bin_size * bin_fac ) + 0.5 ), u_.bin_size + one ), max_bin_size_ );
		}
		if ( ( bin_size == 1u ) && ( min_bin_size_rep_ >= 5u ) ) { // Try larger bin
			bin_size = std::min( static_cast< size_type >( 5u ), max_bin_size_ );
		} else if ( ( bin_size == max_bin_size_ ) && ( max_bin_size_rep_ >= 5 ) ) { // Try smaller bin
			bin_size = std::max( static_cast< size_type >( max_bin_size_ * 0.8 ), one );
		}
		return bin_size;
	}

public: // Methods

	// Add Performance Point
	void
	add( size_type const bin_size, Velocity const velocity )
	{
		// Merge into 3 active performance points
		if ( bin_size < l_.bin_size ) {
			u_ = m_;
			m_ = l_;
			l_ = Point( bin_size, velocity );
		} else if ( bin_size > u_.bin_size ){
			l_ = m_;
			m_ = u_;
			u_ = Point( bin_size, velocity );
		} else if ( bin_size == l_.bin_size ) {
			l_.velocity = velocity;
		} else if ( bin_size == m_.bin_size ) {
			m_.velocity = velocity;
		} else if ( bin_size == u_.bin_size ) {
			u_.velocity = velocity;
		} else if ( bin_size < m_.bin_size ) {
			if ( l_.bin_size == 0u ) {
				l_ = Point( bin_size, velocity );
			} else {
				u_ = m_;
				m_ = Point( bin_size, velocity );
			}
		} else if ( bin_size > m_.bin_size ) {
			if ( m_.bin_size == 0u ) {
				m_ = Point( bin_size, velocity );
			} else {
				l_ = m_;
				m_ = Point( bin_size, velocity );
			}
		} else {
			assert( false ); // Should not get here
		}

		// Update min/max bin size repeat counts
		static size_type const one( 1u );
		static size_type const big( 1000u );
		if ( bin_size == 1u ) {
			min_bin_size_rep_ = std::min( min_bin_size_rep_ + one, big ); // Prevent overflow
			max_bin_size_rep_ = 0u;
		} else if ( bin_size == max_bin_size_ ) {
			max_bin_size_rep_ = std::min( max_bin_size_rep_ + one, big ); // Prevent overflow
			min_bin_size_rep_ = 0u;
		} else {
			min_bin_size_rep_ =  max_bin_size_rep_ = 0u;
		}
	}

private: // Methods

	// Interpolation on 2 Points
	static
	Velocity
	interp( Point const a, Point const b, size_type const s )
	{
		return ( b.bin_size != a.bin_size ? a.velocity + ( ( s - a.bin_size ) * ( b.velocity - a.velocity ) / ( b.bin_size - a.bin_size ) ) : infinity );
	}

private: // Data

	size_type max_bin_size_{ 0u }; // Max bin size
	size_type max_bin_size_rep_{ 0u }; // Max bin size repeat count
	size_type min_bin_size_rep_{ 0u }; // Min bin size repeat count
	Point l_, m_, u_; // Low, middle, upper Points

private: // Static Data

	static double constexpr bin_fac = 1.5; static_assert( bin_fac > 1.0 );

}; // BinOptimizer

} // QSS

#endif
