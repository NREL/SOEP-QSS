// FMU-Based LIQSS1 Variable
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2020 Objexx Engineering, Inc. All rights reserved.
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
#include <QSS/fmu/Variable_LIQSS1.hh>

namespace QSS {
namespace fmu {

	// Advance Self-Observing Trigger
	void
	Variable_LIQSS1::
	advance_LIQSS()
	{
		assert( qTol > 0.0 );
		assert( self_observer() );
		assert( q_c_ == q_0_ );
		assert( x_0_ == q_0_ );

		// Value at +/- qTol
		Real const q_l( q_c_ - qTol );
		Real const q_u( q_c_ + qTol );

		// Derivative at +/- qTol
		fmu_set_observees_q( tQ );
		fmu_set_real( q_l );
		Real const x_1_l( p_1() );
		int const x_1_l_s( signum( x_1_l ) );
		fmu_set_real( q_u );
		Real const x_1_u( p_1() );
		int const x_1_u_s( signum( x_1_u ) );

		// Set coefficients based on derivative signs
		if ( ( x_1_l_s == -1 ) && ( x_1_u_s == -1 ) ) { // Downward trajectory
			q_0_ = q_l;
			x_1_ = x_1_l;
		} else if ( ( x_1_l_s == +1 ) && ( x_1_u_s == +1 ) ) { // Upward trajectory
			q_0_ = q_u;
			x_1_ = x_1_u;
		} else if ( ( x_1_l_s == 0 ) && ( x_1_u_s == 0 ) ) { // Flat trajectory
			// Keep q_0_ == q_c_
			x_1_ = 0.0;
		} else { // Flat trajectory
			q_0_ = std::min( std::max( ( ( q_l * x_1_u ) - ( q_u * x_1_l ) ) / ( x_1_u - x_1_l ), q_l ), q_u ); // Value where deriv is ~ 0 // Clipped in case of roundoff
			x_1_ = 0.0;
		}
	}

	// Advance Self-Observing Trigger: Initialization
	void
	Variable_LIQSS1::
	advance_LIQSS_i()
	{
		assert( qTol > 0.0 );
		assert( self_observer() );
		assert( q_c_ == q_0_ );
		assert( x_0_ == q_0_ );

		// Value at +/- qTol
		Real const q_l( q_c_ - qTol );
		Real const q_u( q_c_ + qTol );

		// Derivative at +/- qTol
		fmu_set_real( q_l );
		Real const x_1_l( p_1() );
		int const x_1_l_s( signum( x_1_l ) );
		fmu_set_real( q_u );
		Real const x_1_u( p_1() );
		int const x_1_u_s( signum( x_1_u ) );
		fmu_set_real( q_c_ );

		// Set coefficients based on derivative signs
		if ( ( x_1_l_s == -1 ) && ( x_1_u_s == -1 ) ) { // Downward trajectory
			l_0_ = q_l;
			x_1_ = x_1_l;
		} else if ( ( x_1_l_s == +1 ) && ( x_1_u_s == +1 ) ) { // Upward trajectory
			l_0_ = q_u;
			x_1_ = x_1_u;
		} else if ( ( x_1_l_s == 0 ) && ( x_1_u_s == 0 ) ) { // Flat trajectory
			l_0_ = q_c_;
			x_1_ = 0.0;
		} else { // Flat trajectory
			l_0_ = std::min( std::max( ( ( q_l * x_1_u ) - ( q_u * x_1_l ) ) / ( x_1_u - x_1_l ), q_l ), q_u ); // Value where deriv is ~ 0 // Clipped in case of roundoff
			x_1_ = 0.0;
		}
	}

	// Advance Self-Observing Trigger: Simultaneous
	void
	Variable_LIQSS1::
	advance_LIQSS_s()
	{
		assert( qTol > 0.0 );
		assert( self_observer() );
		assert( q_c_ == q_0_ );
		assert( x_0_ == q_0_ );

		// Value at +/- qTol
		Real const q_l( q_c_ - qTol );
		Real const q_u( q_c_ + qTol );

		// Derivative at +/- qTol
		fmu_set_observees_q( tQ );
		fmu_set_real( q_l );
		Real const x_1_l( p_1() );
		int const x_1_l_s( signum( x_1_l ) );
		fmu_set_real( q_u );
		Real const x_1_u( p_1() );
		int const x_1_u_s( signum( x_1_u ) );

		// Set coefficients based on derivative signs
		if ( ( x_1_l_s == -1 ) && ( x_1_u_s == -1 ) ) { // Downward trajectory
			l_0_ = q_l;
			x_1_ = x_1_l;
		} else if ( ( x_1_l_s == +1 ) && ( x_1_u_s == +1 ) ) { // Upward trajectory
			l_0_ = q_u;
			x_1_ = x_1_u;
		} else if ( ( x_1_l_s == 0 ) && ( x_1_u_s == 0 ) ) { // Flat trajectory
			l_0_ = q_c_;
			x_1_ = 0.0;
		} else { // Flat trajectory
			l_0_ = std::min( std::max( ( ( q_l * x_1_u ) - ( q_u * x_1_l ) ) / ( x_1_u - x_1_l ), q_l ), q_u ); // Value where deriv is ~ 0 // Clipped in case of roundoff
			x_1_ = 0.0;
		}
	}

} // fmu
} // QSS
