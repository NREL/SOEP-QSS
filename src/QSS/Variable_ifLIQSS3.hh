// ifLIQSS3 Variable
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

#ifndef QSS_Variable_ifLIQSS3_hh_INCLUDED
#define QSS_Variable_ifLIQSS3_hh_INCLUDED

// QSS Headers
#include <QSS/Variable_QSS.hh>

namespace QSS {

// ifLIQSS3 Variable
class Variable_ifLIQSS3 final : public Variable_QSS
{

public: // Types

	using Super = Variable_QSS;

public: // Creation

	// Constructor
	Variable_ifLIQSS3(
	 FMU_ME * fmu_me,
	 std::string const & name,
	 Real const rTol_ = options::rTol,
	 Real const aTol_ = options::aTol,
	 Real const zTol_ = options::zTol,
	 Real const xIni_ = 0.0,
	 FMU_Variable const & var = FMU_Variable(),
	 FMU_Variable const & der = FMU_Variable()
	) :
	 Super( fmu_me, 3, name, rTol_, aTol_, zTol_, xIni_, var, der ),
	 x_0_( xIni_ ),
	 q_c_( xIni_ ),
	 q_0_( xIni_ )
	{
		set_qTol();
	}

public: // Predicate

	// LIQSS Variable?
	bool
	is_LIQSS() const override
	{
		return true;
	}

public: // Property

	// Continuous Value at Time t
	Real
	x( Time const t ) const override
	{
		Time const tDel( t - tX );
		return x_0_ + ( ( x_1_ + ( ( x_2_ + ( x_3_ * tDel ) ) * tDel ) ) * tDel );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const t ) const override
	{
		Time const tDel( t - tX );
		return x_1_ + ( ( ( two * x_2_ ) + ( three * x_3_ * tDel ) ) * tDel );
	}

	// Continuous Second Derivative at Time t
	Real
	x2( Time const t ) const override
	{
		return ( two * x_2_ ) + ( six * x_3_ * ( t - tX ) );
	}

	// Continuous Third Derivative at Time t
	Real
	x3( Time const ) const override
	{
		return six * x_3_;
	}

	// Quantized Value at Time t
	Real
	q( Time const t ) const override
	{
		Time const tDel( t - tQ );
		return q_0_ + ( ( q_1_ + ( ( q_2_ + ( q_3_ * tDel ) ) * tDel ) ) * tDel );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const t ) const override
	{
		Time const tDel( t - tQ );
		return q_1_ + ( ( ( two * q_2_ ) + ( three * q_3_ * tDel ) ) * tDel );
	}

	// Quantized Second Derivative at Time t
	Real
	q2( Time const t ) const override
	{
		return ( two * q_2_ ) + ( six * q_3_ * ( t - tQ ) );
	}

	// Quantized Third Derivative at Time t
	Real
	q3( Time const ) const override
	{
		return six * q_3_;
	}

public: // Methods

	// Initialization
	void
	init() override
	{
		init_0();
		init_observers();
		init_1();
		init_2();
		init_3();
		init_F();
	}

	// Initialization: Stage 0
	void
	init_0() override
	{
		init_observees();
		fmu_set_real( q_c_ = q_0_ = x_0_ = xIni );
	}

	// Initialization: Stage 1
	void
	init_1() override
	{
		q_1_ = x_1_ = p_1();
	}

	// Initialization: Stage 2
	void
	init_2() override
	{
		q_2_ = x_2_ = dd_2();
	}

	// Initialization: Stage 3
	void
	init_3() override
	{
		set_qTol();
		if ( self_observer() ) {
			advance_LIQSS_simultaneous();
		} else {
			q_3_ = x_3_ = n_3();
			fmu_set_observees_x( t0() );
			q_0_ = q_c_ + ( signum( x_3_ ) * qTol );
		}
	}

	// Initialization: Stage Final
	void
	init_F() override
	{
		set_tE_aligned();
		add_QSS( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << q_3_ << x_delta_3 << " [q]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
	}

	// QSS Advance
	void
	advance_QSS() override
	{
		Time const tDel( tE - tX );
		tS = tE - tQ;
		tQ = tX = tE;
		q_c_ = x_0_ += ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel;
		set_qTol();
		if ( self_observer() ) {
			advance_LIQSS();
		} else {
			q_1_ = x_1_ = c_1();
			q_2_ = x_2_ = dd_2();
			q_3_ = x_3_ = n_3();
			q_0_ = q_c_ + ( signum( x_3_ ) * qTol );
		}
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << q_3_ << x_delta_3 << " [q]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
		if ( observed() ) advance_observers();
		if ( connected() ) advance_connections();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0() override
	{
		Time const tDel( tE - tX );
		tS = tE - tQ;
		tQ = tX = tE;
		q_c_ = q_0_ = x_0_ += ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel;
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1( Real const x_1 ) override
	{
		q_1_ = x_1_ = x_1;
	}

	// QSS Advance: Stage 2: Directional 2nd Derivative
	void
	advance_QSS_2_dd2( Real const dd2 ) override
	{
		q_2_ = x_2_ = one_half * dd2;
	}

	// QSS Advance: Stage 3: Directional 2nd Derivative
	void
	advance_QSS_3_dd2( Real const dd2_p ) override
	{
		set_qTol();
		if ( self_observer() ) {
			advance_LIQSS_simultaneous();
		} else {
			q_3_ = x_3_ = n_3( one_half * dd2_p );
			q_0_ = q_c_ + ( signum( x_3_ ) * qTol );
		}
	}

	// QSS Advance: Stage Final
	void
	advance_QSS_F() override
	{
		set_tE_aligned();
		shift_QSS( tE );
		if ( connected() ) advance_connections();
	}

	// QSS Advance: Stage Debug
	void
	advance_QSS_d() override
	{
		assert( options::output::d );
		std::cout << "!= " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << q_3_ << x_delta_3 << " [q]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
	}

	// Handler Advance
	void
	advance_handler( Time const t ) override
	{
		assert( ( tQ <= t ) && ( tX <= t ) && ( t <= tE ) );
		tS = t - tQ;
		tQ = tX = t;
		q_c_ = q_0_ = x_0_ = p_0();
		q_1_ = x_1_ = c_1();
		q_2_ = x_2_ = dd_2();
		q_3_ = x_3_ = n_3();
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "*  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << q_3_ << x_delta_3 << " [q]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
		if ( observed() ) advance_observers();
		if ( connected() ) advance_connections();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t, Real const x_0 ) override
	{
		assert( ( tQ <= t ) && ( tX <= t ) && ( t <= tE ) );
		tS = t - tQ;
		tQ = tX = t;
		q_c_ = q_0_ = x_0_ = x_0;
	}

	// Handler Advance: Stage 1
	void
	advance_handler_1( Real const x_1 ) override
	{
		q_1_ = x_1_ = x_1;
	}

	// Handler Advance: Stage 2: Directional 2nd Derivative
	void
	advance_handler_2_dd2( Real const dd2 ) override
	{
		q_2_ = x_2_ = one_half * dd2;
	}

	// Handler Advance: Stage 3: Directional 2nd Derivative
	void
	advance_handler_3_dd2( Real const dd2_p ) override
	{
		q_3_ = x_3_ = n_3( one_half * dd2_p );
	}

	// Handler Advance: Stage Final
	void
	advance_handler_F() override
	{
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "*= " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << q_3_ << x_delta_3 << " [q]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
		if ( connected() ) advance_connections();
	}

	// Handler No-Advance
	void
	no_advance_handler() override
	{
		shift_QSS( tE );
	}

	// Observer Advance: Stage 1
	void
	advance_observer_1( Time const t, Real const x_1 ) override
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		tX = t;
		x_0_ += ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel;
		x_1_ = x_1;
	}

	// Observer Advance: Stage 2: Directional 2nd Derivative
	void
	advance_observer_2_dd2( Real const dd2 ) override
	{
		x_2_ = one_half * dd2;
	}

	// Observer Advance: Stage 3: Directional 2nd Derivative
	void
	advance_observer_3_dd2( Real const dd2_p ) override
	{
		x_3_ = n_3( one_half * dd2_p );
	}

	// Observer Advance: Stage Final
	void
	advance_observer_F() override
	{
		set_tE_unaligned();
		shift_QSS( tE );
		if ( connected() ) advance_connections_observer();
	}

	// Observer Advance: Stage Final: Parallel
	void
	advance_observer_F_parallel() override
	{
		set_tE_unaligned();
	}

	// Observer Advance: Stage Final: Serial
	void
	advance_observer_F_serial() override
	{
		shift_QSS( tE );
		if ( connected() ) advance_connections_observer();
	}

	// Observer Advance: Stage d
	void
	advance_observer_d() const override
	{
		std::cout << " ^ " << name() << '(' << tX << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << q_3_ << x_delta_3 << " [q(" << std::noshowpos << tQ << std::showpos << ")]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
	}

private: // Methods

	// Set QSS Tolerance
	void
	set_qTol()
	{
		qTol = std::max( rTol * std::abs( q_c_ ), aTol );
		assert( qTol > 0.0 );
	}

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE_aligned()
	{
		assert( tQ == tX );
		assert( dt_min <= dt_max );
		Time dt;
		if ( x_3_ != 0.0 ) {
			Real const x_3_inv( one / x_3_ );
			dt = dt_infinity( std::cbrt( qTol * std::abs( x_3_inv ) ) );
			assert( dt != infinity );
			if ( options::inflection && nonzero_and_signs_differ( x_2_, x_3_ ) ) { // Inflection point
				Time const dtI( -( x_2_ * ( one_third * x_3_inv ) ) );
				dt = ( ( dtI < dt ) && ( dt * options::inflectionFrac < dtI ) ? dtI : dt );
			}
			dt = std::min( std::max( dt, dt_min ), dt_max );
			tE = tQ + dt;
		} else {
			dt = std::min( std::max( dt_infinity_of_infinity(), dt_min ), dt_max );
			tE = ( dt != infinity ? tQ + dt : infinity );
		}
		if ( tQ == tE ) {
			tE = std::nextafter( tE, infinity );
			dt = tE - tQ;
		}
	}

	// Set End Time: Quantized and Continuous Unaligned
	void
	set_tE_unaligned()
	{
		assert( tQ <= tX );
		assert( dt_min <= dt_max );
		Time const tXQ( tX - tQ );
		Real const d_0( x_0_ - ( q_c_ + ( q_1_ + ( q_2_ * tXQ ) ) * tXQ ) );
		Real const d_1( x_1_ - ( q_1_ + ( two * q_2_ * tXQ ) ) );
		Real const d_2( x_2_ - q_2_ );
		Time dt;
		if ( ( x_3_ >= 0.0 ) && ( d_2 >= 0.0 ) && ( d_1 >= 0.0 ) ) { // Upper boundary crossing
			dt = min_root_cubic_upper( x_3_, d_2, d_1, d_0 - qTol, zTol );
		} else if ( ( x_3_ <= 0.0 ) && ( d_2 <= 0.0 ) && ( d_1 <= 0.0 ) ) { // Lower boundary crossing
			dt = min_root_cubic_lower( x_3_, d_2, d_1, d_0 + qTol, zTol );
		} else { // Both boundaries can have crossings
			dt = min_root_cubic_both( x_3_, d_2, d_1, d_0 + qTol, d_0 - qTol, zTol );
		}
		dt = dt_infinity( dt );
		assert( dt > 0.0 );
		if ( options::inflection && nonzero_and_signs_differ( x_2_, x_3_ ) ) { // Inflection point
			Time const dtI( -( x_2_ / ( three * x_3_ ) ) );
			dt = ( ( dtI < dt ) && ( dt * options::inflectionFrac < dtI ) ? dtI : dt );
		}
		dt = std::min( std::max( dt, dt_min ), dt_max );
		tE = ( dt != infinity ? tX + dt : infinity );
		if ( tX == tE ) {
			tE = std::nextafter( tE, infinity );
			dt = tE - tX;
		}
	}

	// Advance Self-Observing Trigger
	void
	advance_LIQSS();

	// Advance Self-Observing Trigger: Simultaneous
	void
	advance_LIQSS_simultaneous();

	// Coefficient 3 at Time tX
	Real
	n_3() const
	{
		Time const tN( tX + options::dtND );
		fmu_set_time( tN );
		fmu_set_observees_s( tN );
		Real const x_2_p( dd_2( tN ) );
		fmu_set_time( tX );
		return options::one_over_three_dtND * ( x_2_p - x_2_ ); //ND Forward Euler
	}

	// Coefficient 3: From Coefficient 2 at +dtND
	Real
	n_3( Real const x_2_p ) const
	{
		return options::one_over_three_dtND * ( x_2_p - x_2_ ); //ND Forward Euler
	}

	// Set FMU Value and Directional Derivative Vector Entry for Specified Trajectory and Time Step
	void
	fmu_set_trajectory(
	 Real const x_0,
	 Real const x_1,
	 Real const x_2,
	 Time const tDel
	)
	{
#ifndef QSS_PROPAGATE_CONTINUOUS
		fmu_set_real( x_0 + ( ( x_1 + ( x_2 * tDel ) ) * tDel ) );
		set_self_dv( x_1 + ( two * x_2 * tDel ) );
#else
		fmu_set_real( x_0 + ( ( x_1 + ( ( x_2 + ( x_3_ * tDel ) ) * tDel ) ) * tDel ) );
		set_self_dv( x_1 + ( ( ( two * x_2 ) + ( three * x_3_ * tDel ) ) * tDel ) );
#endif
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }, x_3_{ 0.0 }; // Continuous trajectory coefficients
	Real q_0_{ 0.0 }, q_1_{ 0.0 }, q_2_{ 0.0 }, q_3_{ 0.0 }; // Quantized trajectory coefficients
	Real q_c_{ 0.0 }; // Quantized trajectory center coefficient

}; // Variable_ifLIQSS3

} // QSS

#endif
