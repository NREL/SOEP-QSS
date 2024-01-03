// ifLIQSS1 Variable
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

// QSS Headers
#include <QSS/Variable_ifLIQSS1.hh>

namespace QSS {

	// Advance Self-Observing Trigger
	void
	Variable_ifLIQSS1::
	advance_LIQSS()
	{
		assert( qTol > 0.0 );
		assert( self_observer() );

		// Set observee FMU values at q_c_
		fmu_set_observees_s( tE );

		// Evaluate at -qTol
		Real const q_l( q_c_ - qTol );
		fmu_set_real( q_l );
		Real const x_1_l( p_1() );
		int const x_1_l_s( signum( x_1_l ) );

		// Evaluate at +qTol
		Real const q_u( q_c_ + qTol );
		fmu_set_real( q_u );
		Real const x_1_u( p_1() );
		int const x_1_u_s( signum( x_1_u ) );

		// Set coefficients based on derivative signs
		if ( ( x_1_l_s == -1 ) && ( x_1_u_s == -1 ) ) { // Downward linear trajectory
			q_0_ = q_l;
			q_1_ = x_1_ = x_1_l;
		} else if ( ( x_1_l_s == +1 ) && ( x_1_u_s == +1 ) ) { // Upward linear trajectory
			q_0_ = q_u;
			q_1_ = x_1_ = x_1_u;
		} else if ( x_1_l_s == x_1_u_s ) { // Constant trajectory
			assert( ( x_1_l_s == 0 ) && ( x_1_u_s == 0 ) );
			q_0_ = q_c_;
			q_1_ = x_1_ = 0.0;
		} else { // Linear trajectory
			q_0_ = std::min( std::max( ( ( q_l * x_1_u ) - ( q_u * x_1_l ) ) / ( x_1_u - x_1_l ), q_l ), q_u ); // Interpolated value where derivative is ~0 (clipped in case of roundoff)
			q_1_ = x_1_ = 0.0;
		}
	}

	// Advance Self-Observing Trigger: Simultaneous
	void
	Variable_ifLIQSS1::
	advance_LIQSS_simultaneous()
	{
		assert( qTol > 0.0 );
		assert( self_observer() );

		// Evaluate at -qTol
		Real const q_l( q_c_ - qTol );
		fmu_set_real( q_l );
		Real const x_1_l( p_1() );
		int const x_1_l_s( signum( x_1_l ) );

		// Evaluate at +qTol
		Real const q_u( q_c_ + qTol );
		fmu_set_real( q_u );
		Real const x_1_u( p_1() );
		int const x_1_u_s( signum( x_1_u ) );

		// Set coefficients based on derivative signs
		if ( ( x_1_l_s == -1 ) && ( x_1_u_s == -1 ) ) { // Downward linear trajectory
			q_0_ = q_l;
			q_1_ = x_1_ = x_1_l;
		} else if ( ( x_1_l_s == +1 ) && ( x_1_u_s == +1 ) ) { // Upward linear trajectory
			q_0_ = q_u;
			q_1_ = x_1_ = x_1_u;
		} else if ( x_1_l_s == x_1_u_s ) { // Constant trajectory
			assert( ( x_1_l_s == 0 ) && ( x_1_u_s == 0 ) );
			q_0_ = q_c_;
			q_1_ = x_1_ = 0.0;
		} else { // Linear trajectory
			q_0_ = std::min( std::max( ( ( q_l * x_1_u ) - ( q_u * x_1_l ) ) / ( x_1_u - x_1_l ), q_l ), q_u ); // Interpolated value where derivative is ~0 (clipped in case of roundoff)
			q_1_ = x_1_ = 0.0;
		}

		// Reset FMU value
		fmu_set_real( q_c_ );
	}

} // QSS
