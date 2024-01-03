// QSS Observers Collection Performance Tests
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
#include <cstddef>
#include <ctime>
#include <functional>
#include <iostream>
#include <limits>
#include <vector>

// Results
//
// The # triggers above which binary search became faster is shown below
//
// The low crossovers are surprising since linear search is usually faster up to more items
//
// This test case is somewhat contrived because we never find the observer in the trigger list so the
// linear search never ends early but in QSS observers that are triggers is also probably infrequent
//
//						# Triggers Crossover
// # Observers		VC		VCnew	IC		IX		GCC
// 1					40				25		  		33
// 5					16		75		17		56		17
// 10					14		60		11		40		13
// 20					11				8		30		13
// 40					11				6		20		13
// 100				6				6		10		13
//
// It probably isn't worth measuring the average number of observers at run time since the speed
// difference is not great until you get to many triggers and since there is variation among
// compilers and hardware: a reasonable crossover (say, 16) will give near-optimal performance
//
// Binary search with copying is not much slower than without copying and avoids side effects


struct Variable {
	using Variables = std::vector< Variable * >;
	Variables observers;
};

int
main()
{
	using Variables = std::vector< Variable * >;

	std::size_t const N( 20u ); // Triggers count
	std::size_t const O( 10u ); // Observers count
	std::size_t const R( 5000000u ); // Repeat count
	std::cout << N << " triggers" << std::endl;
	std::cout << O << " observers" << std::endl;

	Variables triggers;
	Variables triggers_reversed;
	for ( Variables::size_type i = 0; i < N; ++i ) {
		triggers.push_back( new Variable() );
	}
	for ( Variables::size_type i = 0; i < N; ++i ) { // Add observers
		for ( Variables::size_type j = 0; j < O; j += 2 ) {
			triggers[i]->observers.push_back( new Variable() );
			triggers[i]->observers.push_back( triggers[(i+N/2)%N] );
		}
	}
	std::sort( triggers.begin(), triggers.end() );
	triggers_reversed = triggers;
	std::sort( triggers_reversed.begin(), triggers_reversed.end(), std::greater< Variable * >() ); // Make sort do some work

	{ // Linear
		Variables observers; observers.reserve( N * O );
		std::size_t size( 0u );
		double const time_beg = (double)clock()/CLOCKS_PER_SEC;
		for ( std::size_t r = 0; r < R; ++r ) { observers.clear();
			for ( Variable * trigger : triggers ) {
				for ( Variable * observer : trigger->observers ) {
					if ( std::find( triggers.begin(), triggers.end(), observer ) == triggers.end() ) observers.push_back( observer );
				}
			}
			size += observers.size();
		}
		double const time_end = (double)clock()/CLOCKS_PER_SEC;
		std::cout << "Linear: " << time_end - time_beg << " (s)  " << size << std::endl;
	}

	// { // Reverse: Check the cost of assigning the reversed triggers
	// 	Variables observers;observers.reserve( N * O );
	// 	std::size_t size( 0u );
	// 	double const time_beg = (double)clock()/CLOCKS_PER_SEC;
	// 	for ( std::size_t r = 0; r < R; ++r ) { observers.clear();
	// 		triggers = triggers_reversed; // Make sort do work
	// 		size += triggers.size()*r;
	// 	}
	// 	double const time_end = (double)clock()/CLOCKS_PER_SEC;
	// 	std::cout << "Reverse: " << time_end - time_beg << " (s)  " << size << std::endl;
	// }

	{ // Binary
		Variables observers;observers.reserve( N * O );
		std::size_t size( 0u );
		double const time_beg = (double)clock()/CLOCKS_PER_SEC;
		for ( std::size_t r = 0; r < R; ++r ) { observers.clear();
			triggers = triggers_reversed; // Make sort do work
			std::sort( triggers.begin(), triggers.end() );
			for ( Variable * trigger : triggers ) {
				for ( Variable * observer : trigger->observers ) {
					if ( !std::binary_search( triggers.begin(), triggers.end(), observer ) ) observers.push_back( observer );
				}
			}
			size += observers.size();
		}
		double const time_end = (double)clock()/CLOCKS_PER_SEC;
		std::cout << "Binary: " << time_end - time_beg << " (s)  " << size << std::endl;
	}

	// { // BinCopy
	// 	Variables observers;observers.reserve( N * O );
	// 	std::size_t size( 0u );
	// 	double const time_beg = (double)clock()/CLOCKS_PER_SEC;
	// 	for ( std::size_t r = 0; r < R; ++r ) { observers.clear();
	// 		Variables triggers_copy( triggers_reversed ); // Make sort do work
	// 		std::sort( triggers_copy.begin(), triggers_copy.end() );
	// 		for ( Variable * trigger : triggers_copy ) {
	// 			for ( Variable * observer : trigger->observers ) {
	// 				if ( !std::binary_search( triggers_copy.begin(), triggers_copy.end(), observer ) ) observers.push_back( observer );
	// 			}
	// 		}
	// 		size += observers.size();
	// 	}
	// 	double const time_end = (double)clock()/CLOCKS_PER_SEC;
	// 	std::cout << "BinCopy: " << time_end - time_beg << " (s)  " << size << std::endl;
	// }
}
