// FMU-Based LIQSS2 Variable
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
#include <QSS/fmu/Variable_LIQSS2.hh>

namespace QSS {
namespace fmu {

	// Advance Self-Observing Trigger
	void
	Variable_LIQSS2::
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
		fmu_set_real( q_u );
		Real const x_1_u( p_1() );

		// Second derivative at +/- qTol
		Time const tN( tQ + options::dtND );
		fmu_set_time( tN );
		fmu_set_observees_q( tN );
		fmu_set_real( q_l + ( x_1_l * options::dtND ) );
		Real const x_2_l( options::one_over_two_dtND * ( p_1() - x_1_l ) ); //ND Forward Euler
		int const x_2_l_s( signum( x_2_l ) );
		fmu_set_real( q_u + ( x_1_u * options::dtND ) );
		Real const x_2_u( options::one_over_two_dtND * ( p_1() - x_1_u ) ); //ND Forward Euler
		int const x_2_u_s( signum( x_2_u ) );

		// Reset FMU time
		fmu_set_time( tQ );

		// Set coefficients based on second derivative signs
		if ( ( x_2_l_s == -1 ) && ( x_2_u_s == -1 ) ) { // Downward curving trajectory
			q_0_ = q_l;
			x_1_ = q_1_ = x_1_l;
			x_2_ = x_2_l;
		} else if ( ( x_2_l_s == +1 ) && ( x_2_u_s == +1 ) ) { // Upward curving trajectory
			q_0_ = q_u;
			x_1_ = q_1_ = x_1_u;
			x_2_ = x_2_u;
		} else if ( ( x_2_l_s == 0 ) && ( x_2_u_s == 0 ) ) { // Non-curving trajectory
			// Keep q_0_ == q_c_
			x_1_ = q_1_ = one_half * ( x_1_l + x_1_u ); // Interpolated 1st deriv at q_0_ == q_c_
			x_2_ = 0.0;
		} else { // Straight trajectory
			q_0_ = std::min( std::max( ( ( q_l * x_2_u ) - ( q_u * x_2_l ) ) / ( x_2_u - x_2_l ), q_l ), q_u ); // Value where 2nd deriv is ~ 0 // Clipped in case of roundoff
			x_1_ = q_1_ = ( ( ( q_u - q_0_ ) * x_1_l ) + ( ( q_0_ - q_l ) * x_1_u ) ) / ( two * qTol ); // Interpolated 1st deriv at q_0_
			x_2_ = 0.0;
		}
	}

	// Advance Self-Observing Trigger: Simultaneous
	void
	Variable_LIQSS2::
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
		fmu_set_real( q_l );
		Real const x_1_l( p_1() );
		fmu_set_real( q_u );
		Real const x_1_u( p_1() );

		// Second derivative at +/- qTol
		Time const tN( tQ + options::dtND );
		fmu_set_time( tN );
		fmu_set_observees_q( tN );
		fmu_set_real( q_l + ( x_1_l * options::dtND ) );
		Real const x_2_l( options::one_over_two_dtND * ( p_1() - x_1_l ) ); //ND Forward Euler
		int const x_2_l_s( signum( x_2_l ) );
		fmu_set_real( q_u + ( x_1_u * options::dtND ) );
		Real const x_2_u( options::one_over_two_dtND * ( p_1() - x_1_u ) ); //ND Forward Euler
		int const x_2_u_s( signum( x_2_u ) );

		// Reset FMU time
		fmu_set_time( tQ );

		// Reset FMU values
		fmu_set_observees_q( tQ );
		fmu_set_real( q_c_ );

		// Set coefficients based on second derivative signs
		if ( ( x_2_l_s == -1 ) && ( x_2_u_s == -1 ) ) { // Downward curving trajectory
			l_0_ = q_l;
			x_1_ = x_1_l;
			x_2_ = x_2_l;
		} else if ( ( x_2_l_s == +1 ) && ( x_2_u_s == +1 ) ) { // Upward curving trajectory
			l_0_ = q_u;
			x_1_ = x_1_u;
			x_2_ = x_2_u;
		} else if ( ( x_2_l_s == 0 ) && ( x_2_u_s == 0 ) ) { // Non-curving trajectory
			l_0_ = q_c_;
			x_1_ = one_half * ( x_1_l + x_1_u ); // Interpolated 1st deriv at l_0_ == q_c_
			x_2_ = 0.0;
		} else { // Straight trajectory
			l_0_ = std::min( std::max( ( ( q_l * x_2_u ) - ( q_u * x_2_l ) ) / ( x_2_u - x_2_l ), q_l ), q_u ); // Value where 2nd deriv is ~ 0 // Clipped in case of roundoff
			x_1_ = ( ( ( q_u - l_0_ ) * x_1_l ) + ( ( l_0_ - q_l ) * x_1_u ) ) / ( two * qTol ); // Interpolated 1st deriv at l_0_
			x_2_ = 0.0;
		}
	}

} // fmu
} // QSS
