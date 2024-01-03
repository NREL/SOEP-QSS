// LIQSS3 Variable
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
#include <QSS/Variable_LIQSS3.hh>

namespace QSS {

	// Advance Self-Observing Trigger
	void
	Variable_LIQSS3::
	advance_LIQSS()
	{
		assert( qTol > 0.0 );
		assert( self_observer() );

		// Set observee FMU values at q_c_
		fmu_set_observees_s( tE );

		// Set directional derivative seed vector at q_c_
		set_observees_dv( tE );

		// First and second derivatives at -qTol
		Real const q_l( q_c_ - qTol );
		fmu_set_real( q_l );
		Real const x_1_l( p_1() );
		set_self_dv( x_1_l );
		Real const x_2_l( dd_2_use_seed() );

		// First and second derivatives at +qTol
		Real const q_u( q_c_ + qTol );
		fmu_set_real( q_u );
		Real const x_1_u( p_1() );
		set_self_dv( x_1_u );
		Real const x_2_u( dd_2_use_seed() );

		// Third derivative setup
		Time const dN( options::dtND );
		Time tN( tE + dN );
		fmu_set_time( tN );
		fmu_set_observees_s( tN );
		set_observees_dv( tN );

		// Third derivative at -qTol
		fmu_set_trajectory( q_l, x_1_l, x_2_l, dN );
		Real const x_3_l( options::one_over_three_dtND * ( dd_2_use_seed() - x_2_l ) ); //ND Forward Euler
		int const x_3_l_s( signum( x_3_l ) );

		// Third derivative at +qTol
		fmu_set_trajectory( q_u, x_1_u, x_2_u, dN );
		Real const x_3_u( options::one_over_three_dtND * ( dd_2_use_seed() - x_2_u ) ); //ND Forward Euler
		int const x_3_u_s( signum( x_3_u ) );

		// Set coefficients based on third derivative signs
		if ( ( x_3_l_s == -1 ) && ( x_3_u_s == -1 ) ) { // Downward cubic trajectory
			q_0_ = q_l;
			q_1_ = x_1_ = x_1_l;
			q_2_ = x_2_ = x_2_l;
			x_3_ = x_3_l;
		} else if ( ( x_3_l_s == +1 ) && ( x_3_u_s == +1 ) ) { // Upward cubic trajectory
			q_0_ = q_u;
			q_1_ = x_1_ = x_1_u;
			q_2_ = x_2_ = x_2_u;
			x_3_ = x_3_u;
		} else if ( x_3_l_s == x_3_u_s ) { // Quadratic trajectory
			assert( ( x_3_l_s == 0 ) && ( x_3_u_s == 0 ) );
			q_0_ = q_c_;
			fmu_set_time( tE );
			fmu_set_observees_s( tE );
			q_1_ = x_1_ = p_1();
			q_2_ = x_2_ = dd_2( tE );
			x_3_ = 0.0;
		} else { // Cubic trajectory
			q_0_ = std::min( std::max( ( ( q_l * x_3_u ) - ( q_u * x_3_l ) ) / ( x_3_u - x_3_l ), q_l ), q_u ); // Interpolated value where 3rd derivative is ~0 (clipped in case of roundoff)
			fmu_set_time( tE );
			fmu_set_observees_s( tE );
			q_1_ = x_1_ = p_1();
			q_2_ = x_2_ = dd_2( tE );
			fmu_set_time( tN );
			fmu_set_observees_s( tN );
			x_3_ = options::one_over_three_dtND * ( dd_2( tN ) - x_2_ ); //ND Forward Euler
		}

		// Reset FMU time
		fmu_set_time( tE );
	}

	// Advance Self-Observing Trigger: Simultaneous
	void
	Variable_LIQSS3::
	advance_LIQSS_simultaneous()
	{
		assert( qTol > 0.0 );
		assert( self_observer() );

		// Set observee FMU values at q_c_
		fmu_set_observees_s( tE );

		// Set directional derivative seed vector at q_c_
		set_observees_dv( tE );

		// First and second derivatives at -qTol
		Real const q_l( q_c_ - qTol );
		fmu_set_real( q_l );
		Real const x_1_l( p_1() );
		set_self_dv( x_1_l );
		Real const x_2_l( dd_2_use_seed() );

		// First and second derivatives at +qTol
		Real const q_u( q_c_ + qTol );
		fmu_set_real( q_u );
		Real const x_1_u( p_1() );
		set_self_dv( x_1_u );
		Real const x_2_u( dd_2_use_seed() );

		// Third derivative setup
		Time const dN( options::dtND );
		Time tN( tE + dN );
		fmu_set_time( tN );
		fmu_set_observees_s( tN );
		set_observees_dv( tN );

		// Third derivative at -qTol
		fmu_set_trajectory( q_l, x_1_l, x_2_l, dN );
		Real const x_3_l( options::one_over_three_dtND * ( dd_2_use_seed() - x_2_l ) ); //ND Forward Euler
		int const x_3_l_s( signum( x_3_l ) );

		// Third derivative at +qTol
		fmu_set_trajectory( q_u, x_1_u, x_2_u, dN );
		Real const x_3_u( options::one_over_three_dtND * ( dd_2_use_seed() - x_2_u ) ); //ND Forward Euler
		int const x_3_u_s( signum( x_3_u ) );

		// Set coefficients based on third derivative signs
		if ( ( x_3_l_s == -1 ) && ( x_3_u_s == -1 ) ) { // Downward cubic trajectory
			q_0_ = q_l;
			q_1_ = x_1_ = x_1_l;
			q_2_ = x_2_ = x_2_l;
			x_3_ = x_3_l;
			fmu_set_time( tE );
			fmu_set_observees_s( tE );
		} else if ( ( x_3_l_s == +1 ) && ( x_3_u_s == +1 ) ) { // Upward cubic trajectory
			q_0_ = q_u;
			q_1_ = x_1_ = x_1_u;
			q_2_ = x_2_ = x_2_u;
			x_3_ = x_3_u;
			fmu_set_time( tE );
			fmu_set_observees_s( tE );
		} else if ( x_3_l_s == x_3_u_s ) { // Quadratic trajectory
			assert( ( x_3_l_s == 0 ) && ( x_3_u_s == 0 ) );
			q_0_ = q_c_;
			fmu_set_time( tE );
			fmu_set_observees_s( tE );
			q_1_ = x_1_ = p_1();
			q_2_ = x_2_ = dd_2( tE );
			x_3_ = 0.0;
		} else { // Cubic trajectory
			q_0_ = std::min( std::max( ( ( q_l * x_3_u ) - ( q_u * x_3_l ) ) / ( x_3_u - x_3_l ), q_l ), q_u ); // Interpolated value where 3rd derivative is ~0 (clipped in case of roundoff)
			fmu_set_time( tE );
			fmu_set_observees_s( tE );
			q_1_ = x_1_ = p_1();
			q_2_ = x_2_ = dd_2( tE );
			fmu_set_time( tN );
			fmu_set_observees_s( tN );
			x_3_ = options::one_over_three_dtND * ( dd_2( tN ) - x_2_ ); //ND Forward Euler
			fmu_set_time( tE );
			fmu_set_observees_s( tE );
		}
	}

} // QSS
