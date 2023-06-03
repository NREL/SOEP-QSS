// Relaxation xQSS2 Variable
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
#include <QSS/Variable_rxQSS2.hh>

namespace QSS {

	// QSS Advance LIQSS/QSS Step Ratio
	Variable_rxQSS2::Real
	Variable_rxQSS2::
	advance_LIQSS_QSS_step_ratio()
	{
		if ( !self_observer() ) return 1.0; // Same step size

		Time const t_fmu( fmu_get_time() ); // Save FMU time

		Time const tDel( tE - tX );
		Real const x_0( x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel ) );
		Real const q( std::max( rTol * std::abs( x_0 ), aTol ) );

		fmu_set_time( tE );

		// QSS
		Real const x_1( c_1( tE, x_0 ) );
		Real x_2( c_2( tE, x_1 ) );
		Time const dt_QSS( x_2 != 0.0 ? std::sqrt( q / std::abs( x_2 ) ) : infinity );

		// LIQSS /////

		// Value at +/- q
		Real const q_l( x_0 - q );
		Real const q_u( x_0 + q );

		// Derivative at +/- q
		fmu_set_observees_s( tE );
		fmu_set_real( q_l );
		Real const x_1_l( p_1() );
		fmu_set_real( q_u );
		Real const x_1_u( p_1() );

		// Second derivative at +/- q
		Time const dN( options::dtND );
		Time const tN( tE + dN );
		fmu_set_time( tN );
		fmu_set_observees_s( tN );
#ifndef QSS_PROPAGATE_CONTINUOUS
		fmu_set_real( q_l + ( x_1_l * dN ) );
#else
		Real const x_2_dN( x_2_ * dN );
		fmu_set_real( q_l + ( x_1_l + x_2_dN ) * dN );
#endif
		Real const x_2_l( options::one_over_two_dtND * ( p_1() - x_1_l ) ); //ND Forward Euler
		int const x_2_l_s( signum( x_2_l ) );
#ifndef QSS_PROPAGATE_CONTINUOUS
		fmu_set_real( q_u + ( x_1_u * dN ) );
#else
		fmu_set_real( q_u + ( x_1_u + x_2_dN ) * dN );
#endif
		Real const x_2_u( options::one_over_two_dtND * ( p_1() - x_1_u ) ); //ND Forward Euler
		int const x_2_u_s( signum( x_2_u ) );

		// Set coefficients based on second derivative signs
		if ( ( x_2_l_s == -1 ) && ( x_2_u_s == -1 ) ) { // Downward curving trajectory
			x_2 = x_2_l;
		} else if ( ( x_2_l_s == +1 ) && ( x_2_u_s == +1 ) ) { // Upward curving trajectory
			x_2 = x_2_u;
		} else { // Linear trajectory
			x_2 = 0.0;
		}
		Time const dt_LIQSS( x_2 != 0.0 ? std::sqrt( q / std::abs( x_2 ) ) : infinity );

		fmu_set_time( t_fmu ); // Restore FMU time

		return ( dt_QSS > 0.0 ? dt_LIQSS / dt_QSS : ( dt_LIQSS > 0.0 ? infinity : 1.0 ) );
	}

} // QSS
