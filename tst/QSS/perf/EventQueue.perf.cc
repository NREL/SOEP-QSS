// QSS::EventQueue Performance Tests
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

// QSS Headers
#include <QSS/EventQueue.hh>

// C++ Headers
#include <cstddef>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

// Variable Mock
class V {};

// Types
using namespace QSS;
using EventQ = EventQueue< V >;
using Variables = std::vector< V >;
using Time = double;

namespace { // Internal shared global
std::default_random_engine random_generator;
}

int
main()
{
	using namespace std;

	random_generator.seed( 42 );
	size_t const N( 10000 ); // Variable count
	size_t const R( 100000000 ); // Repeat count

	Variables vars;
	vars.reserve( N ); // Prevent reallocation
	EventQ events;
	Time t( 0.0 );
	Time const tE( 10.0 );
	std::uniform_real_distribution< Time > distribution( t, tE );
	for ( Variables::size_type i = 0; i < N; ++i ) {
		vars.emplace_back( V() );
		events.add_QSS( distribution( random_generator ), &vars[ i ] );
	}

	double const time_beg = (double)clock()/CLOCKS_PER_SEC;
	size_t ns( 0u ), nr( 0u ), nl( 0u );
	for ( size_t r = 1; r <= R; ++r ) {
		auto i( events.begin() );
		events.shift_QSS( i->first + ( 0.5 * ( 10.0 - i->first ) ), i ); // Move halfway to tE
	}
	double const time_end = (double)clock()/CLOCKS_PER_SEC;
	cout << std::setprecision( 15 ) << time_end - time_beg << " (s) " << events.top_time() << ' ' << N << ' ' << R << endl << endl;
}
