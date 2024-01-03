// Vector Delete Duplicates Performance Tests
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

// C++ Headers
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <unordered_set>
#include <vector>

namespace { // Internal shared global
std::default_random_engine random_generator;
}

// Uniform-Random Integer on [0,(2^15)-1]
template< typename T >
T
uniform_random_integer( T const l = T( 0 ), T const u = T( 1 ) )
{
	assert( l <= u );
	T const b( std::numeric_limits< T >::max() );
	static std::uniform_int_distribution< T > distribution( T( 0 ), b ); // [0,b]
	return static_cast< T >( distribution( random_generator ) * ( double( u - l + 1 ) / ( double( b ) + 1.0 ) ) ) + l; // Shift from [0,b] to [l,u]
}

int
main()
{
	std::size_t const N( 200000000 ); // Variable count
	std::size_t const R( 1 ); // Repeat count

	random_generator.seed( std::chrono::system_clock::now().time_since_epoch().count() );

	{ // Vector in-place sort+unique
		using V = std::vector< std::int64_t >;
		V v;
		v.reserve( N );
		for ( V::size_type i = 0; i < N; ++i ) {
			v.push_back( uniform_random_integer< std::int64_t >( 0, N/2 ) );
		}

		// Sort + unique method: Faster for N up to 2e8
		double const time_beg = (double)clock()/CLOCKS_PER_SEC;
		for ( std::size_t r = 1; r <= R; ++r ) {
			std::sort( v.begin(), v.end() ); // Sort by value
			v.erase( std::unique( v.begin(), v.end() ), v.end() ); // Remove duplicates
			v.shrink_to_fit();
		}
		double const time_end = (double)clock()/CLOCKS_PER_SEC;
		std::cout << std::setprecision( 16 ) << "sort+unique" << time_end - time_beg << " (s)  " << v.size() << ' ' << v.front() << ' ' << v.back() << ' ' << std::endl;
	}

	{ // Unordered set
		using V = std::vector< std::int64_t >;
		V v;
		v.reserve( N );
		for ( V::size_type i = 0; i < N; ++i ) {
			v.push_back( uniform_random_integer< std::int64_t >( 0, N/2 ) );
		}

		// Unordered set method: Maybe faster for N > 1e10 or so
		double const time_beg = (double)clock()/CLOCKS_PER_SEC;
		for ( std::size_t r = 1; r <= R; ++r ) {
			std::unordered_set< std::int64_t > s( v.begin(), v.end() ); // Remove duplicates
			v.clear();
			v.reserve( s.size() );
			v.assign( s.begin(), s.end() );
			//v.shrink_to_fit(); // Instead of clear() and reserve(): Not sig faster
		}
		double const time_end = (double)clock()/CLOCKS_PER_SEC;
		std::cout << std::setprecision( 16 ) << "unordered_set " << time_end - time_beg << " (s)  " << v.size() << ' ' << v.front() << ' ' << v.back() << ' ' << std::endl;
	}
}
