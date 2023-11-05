// QSS Zero-Crossing Variable Abstract Base Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2023 Objexx Engineering, Inc. All rights reserved.
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
#include <QSS/Variable_ZC.hh>

namespace QSS {

	// Refine Zero-Crossing Time: Event Indicator Zero-Crossing Variable
	void
	Variable_ZC::
	refine_root_ZC( Time const tBeg )
	{
		assert( options::refine );
		Time t( tZ );
		Time const t_fmu( fmu_get_time() );
		fmu_set_time( tZ );
		Real const vZ( z_0( tZ ) );
		Real v( vZ ), v_p( vZ );
		Real m( 1.0 ); // Multiplier
		std::size_t j( 0u );
		std::size_t const n( 10u ); // Max iterations
		while ( ( ++j <= n ) && ( ( std::abs( v ) > aTol ) || ( std::abs( v ) < std::abs( v_p ) ) ) ) {
			Real const d( X_1( t ) );
			if ( d == 0.0 ) break;
			t -= m * ( v / d );
			fmu_set_time( t );
			v = z_0( t );
			if ( std::abs( v ) >= std::abs( v_p ) ) m *= 0.5; // Non-converging step: Reduce step size
			v_p = v;
		}
		if ( ( t >= tBeg ) && ( std::abs( v ) < std::abs( vZ ) ) ) tZ = t;
		if ( ( j == n ) && ( options::output::d ) ) std::cout << "   " << name() << '(' << t << ')' << " tZ may not have converged" << std::endl;
		fmu_set_time( t_fmu );
	}

} // QSS
