// LIQSS2 Variable
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

#ifndef QSS_Variable_LIQSS2_hh_INCLUDED
#define QSS_Variable_LIQSS2_hh_INCLUDED

// QSS Headers
#include <QSS/Variable_QSS.hh>

namespace QSS {

// LIQSS2 Variable
class Variable_LIQSS2 final : public Variable_QSS
{

public: // Types

	using Super = Variable_QSS;

private: // Types

	using Super::c_1;
	using Super::c_2;
	using Super::h_2;

public: // Creation

	// Constructor
	Variable_LIQSS2(
	 FMU_ME * fmu_me,
	 std::string const & name,
	 Real const rTol_ = options::rTol,
	 Real const aTol_ = options::aTol,
	 Real const zTol_ = options::zTol,
	 Real const xIni_ = 0.0,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Super( fmu_me, 2, name, rTol_, aTol_, zTol_, xIni_, var, der ),
	 q_c_( xIni_ ),
	 q_0_( xIni_ ),
	 x_0_( xIni_ )
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
		return x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const t ) const override
	{
		return x_1_ + ( two * x_2_ * ( t - tX ) );
	}

	// Continuous Second Derivative at Time t
	Real
	x2( Time const ) const override
	{
		return two * x_2_;
	}

	// Quantized Value at Time t
	Real
	q( Time const t ) const override
	{
		return q_0_ + ( q_1_ * ( t - tQ ) );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const ) const override
	{
		return q_1_;
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
		init_deferred();
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
		set_qTol();
		if ( self_observer() ) {
			advance_LIQSS_simultaneous();
			x_1_ = d_1_;
			x_2_ = d_2_;
		} else {
			x_2_ = h_2();
		}
	}

	// Initialization: Stage Deferred
	void
	init_deferred() override
	{
		if ( self_observer() ) {
			q_0_ = l_0_;
			q_1_ = x_1_;
		} else {
			q_0_ += signum( x_2_ ) * qTol;
		}
	}

	// Initialization: Stage Final
	void
	init_F() override
	{
		if ( self_observer() ) {
			q_0_ = l_0_;
			q_1_ = x_1_;
		} else {
			q_0_ += signum( x_2_ ) * qTol;
		}
		set_tE_aligned();
		add_QSS( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << " [q]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
	}

	// QSS Advance
	void
	advance_QSS() override
	{
		Time const tDel( tE - tX );
		tQ = tX = tE;
		q_c_ = q_0_ = x_0_ += ( x_1_ + ( x_2_ * tDel ) ) * tDel;
		set_qTol();
		if ( self_observer() ) {
			advance_LIQSS();
		} else {
			q_1_ = x_1_ = h_1();
			x_2_ = h_2();
			q_0_ += signum( x_2_ ) * qTol;
		}
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << " [q]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
		if ( observed() ) advance_observers();
		if ( connected() ) advance_connections();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0() override
	{
		Time const tDel( tE - tX );
		d_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1( Real const x_1 ) override
	{
		d_1_ = x_1;
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2( Real const x_1_p ) override
	{
		set_qTol();
		if ( self_observer() ) {
			advance_LIQSS_simultaneous();
		} else {
			d_2_ = n_2( x_1_p );
		}
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2( Real const x_1_m, Real const x_1_p ) override
	{
		set_qTol();
		if ( self_observer() ) {
			advance_LIQSS_simultaneous();
		} else {
			d_2_ = n_2( x_1_m, x_1_p );
		}
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2_forward( Real const x_1_p, Real const x_1_2p ) override
	{
		set_qTol();
		if ( self_observer() ) {
			advance_LIQSS_simultaneous();
		} else {
			d_2_ = f_2( x_1_p, x_1_2p );
		}
	}

	// QSS Advance: Stage Final
	void
	advance_QSS_F() override
	{
		tQ = tX = tE;
		q_c_ = q_0_ = x_0_ = d_0_;
		q_1_ = x_1_ = d_1_;
		x_2_ = d_2_;
		if ( self_observer() ) {
			q_0_ = l_0_;
			q_1_ = x_1_;
		} else {
			q_0_ += signum( x_2_ ) * qTol;
		}
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "!= " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << " [q]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
		if ( connected() ) advance_connections();
	}

	// Handler Advance
	void
	advance_handler( Time const t ) override
	{
		assert( ( tQ <= t ) && ( tX <= t ) && ( t <= tE ) );
		tQ = tX = t;
		q_c_ = q_0_ = x_0_ = c_0();
		q_1_ = x_1_ = h_1();
		x_2_ = c_2();
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "*  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << " [q]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
		if ( observed() ) advance_observers();
		if ( connected() ) advance_connections();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t, Real const x_0 ) override
	{
		assert( ( tQ <= t ) && ( tX <= t ) && ( t <= tE ) );
		d_0_ = x_0;
	}

	// Handler Advance: Stage 1
	void
	advance_handler_1( Real const x_1 ) override
	{
		d_1_ = x_1;
	}

	// Handler Advance: Stage 2
	void
	advance_handler_2( Real const x_1_p ) override
	{
		d_2_ = n_2( x_1_p );
	}

	// Handler Advance: Stage 2
	void
	advance_handler_2( Real const x_1_m, Real const x_1_p ) override
	{
		d_2_ = n_2( x_1_m, x_1_p );
	}

	// QSS Advance: Stage 2
	void
	advance_handler_2_forward( Real const x_1_p, Real const x_1_2p ) override
	{
		d_2_ = f_2( x_1_p, x_1_2p );
	}

	// Handler Advance: Stage Final
	void
	advance_handler_F( Time const t ) override
	{
		tQ = tX = t;
		q_c_ = q_0_ = x_0_ = d_0_;
		q_1_ = x_1_ = d_1_;
		x_2_ = d_2_;
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "*= " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << " [q]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
		if ( connected() ) advance_connections();
	}

	// Handler No-Advance
	void
	no_advance_handler() override
	{
		shift_QSS( tE );
	}

	// Observer Advance
	void
	advance_observer( Time const t ) override
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		tX = t;
		x_0_ += ( x_1_ + ( x_2_ * tDel ) ) * tDel;
		x_1_ = c_1( t );
		x_2_ = c_2( t );
		set_tE_unaligned();
		shift_QSS( tE );
		if ( connected() ) advance_connections_observer();
	}

	// Observer Advance: Stage 1
	void
	advance_observer_1( Time const t, Real const x_1 ) override
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		// assert( x_1 == p_1() );
		Time const tDel( t - tX );
		d_0_ = x_0_ + ( x_1_ + ( x_2_ * tDel ) ) * tDel;
		d_1_ = x_1;
	}

	// Observer Advance: Stage 2
	void
	advance_observer_2( Real const x_1_p ) override
	{
		d_2_ = n_2( x_1_p );
	}

	// Observer Advance: Stage 2
	void
	advance_observer_2( Real const x_1_m, Real const x_1_p ) override
	{
		d_2_ = n_2( x_1_m, x_1_p );
	}

	// Observer Advance: Stage 2
	void
	advance_observer_2_forward( Real const x_1_p, Real const x_1_2p ) override
	{
		d_2_ = f_2( x_1_p, x_1_2p );
	}

	// Observer Advance: Stage Final
	void
	advance_observer_F( Time const t ) override
	{
		tX = t;
		x_0_ = d_0_;
		x_1_ = d_1_;
		x_2_ = d_2_;
		set_tE_unaligned();
		shift_QSS( tE );
		if ( connected() ) advance_connections_observer();
	}

	// Observer Advance: Stage d
	void
	advance_observer_d() const override
	{
		std::cout << " ^ " << name() << '(' << tX << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << " [q(" << std::noshowpos << tQ << std::showpos << ")]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
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
		assert( tX <= tQ );
		assert( dt_min <= dt_max );
		Time dt( x_2_ != 0.0 ? std::sqrt( qTol / std::abs( x_2_ ) ) : infinity );
		dt = std::min( std::max( dt_infinity( dt ), dt_min ), dt_max );
		tE = ( dt != infinity ? tQ + dt : infinity );
		if ( ( options::inflection ) && ( x_2_ != 0.0 ) && ( signum( x_1_ ) != signum( x_2_ ) ) ) {
			Time const tI( tX - ( x_1_ / ( two * x_2_ ) ) );
			if ( tQ < tI ) tE = std::min( tE, tI );
		}
	}

	// Set End Time: Quantized and Continuous Unaligned
	void
	set_tE_unaligned()
	{
		assert( tQ <= tX );
		assert( dt_min <= dt_max );
		Real const d_0( x_0_ - ( q_c_ + ( q_1_ * ( tX - tQ ) ) ) );
		Real const d_1( x_1_ - q_1_ );
		Time dt;
		if ( ( d_1 >= 0.0 ) && ( x_2_ >= 0.0 ) ) { // Upper boundary crossing
			dt = min_root_quadratic_upper( x_2_, d_1, d_0 - qTol );
		} else if ( ( d_1 <= 0.0 ) && ( x_2_ <= 0.0 ) ) { // Lower boundary crossing
			dt = min_root_quadratic_lower( x_2_, d_1, d_0 + qTol );
		} else { // Both boundaries can have crossings
			dt = min_root_quadratic_both( x_2_, d_1, d_0 + qTol, d_0 - qTol );
		}
		dt = std::min( std::max( dt_infinity( dt ), dt_min ), dt_max );
		tE = ( dt != infinity ? tX + dt : infinity );
		if ( ( options::inflection ) && ( x_2_ != 0.0 ) && ( signum( x_1_ ) != signum( x_2_ ) ) && ( signum( x_1_ ) == signum( q_1_ ) ) ) {
			Time const tI( tX - ( x_1_ / ( two * x_2_ ) ) );
			if ( tX < tI ) tE = std::min( tE, tI );
		}
	}

	// Advance Self-Observing Trigger
	void
	advance_LIQSS();

	// Advance Self-Observing Trigger: Simultaneous
	void
	advance_LIQSS_simultaneous();

	// Coefficient 2 from FMU
	Real
	n_2( Real const x_1_p ) const
	{
		return options::one_over_two_dtND * ( x_1_p - d_1_ ); //ND Forward Euler
	}

	// Coefficient 2 from FMU
	Real
	n_2( Real const x_1_m, Real const x_1_p ) const
	{
		return options::one_over_four_dtND * ( x_1_p - x_1_m ); //ND Centered difference
	}

	// Coefficient 2 from FMU
	Real
	f_2( Real const x_1_p, Real const x_1_2p ) const
	{
		return options::one_over_four_dtND * ( ( three * ( x_1_p - d_1_ ) ) + ( x_1_p - x_1_2p ) ); //ND Forward 3-point
	}

	// Coefficient 2 from FMU at Time tQ
	Real
	c_2() const
	{
		return c_2( tQ );
	}

	// Coefficient 2 from FMU at Time t
	Real
	c_2( Time const t ) const
	{
		return c_2( t, x_1_ );
	}

	// Coefficient 2 from FMU at Time tQ
	Real
	h_2() const
	{
		return h_2( tQ, x_1_ );
	}

private: // Data

	Real q_c_{ 0.0 }, q_0_{ 0.0 }, q_1_{ 0.0 }; // Quantized trajectory coefficients
	Real x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }; // Continuous trajectory coefficients
	Real d_0_{ 0.0 }, d_1_{ 0.0 }, d_2_{ 0.0 }; // Deferred trajectory coefficients
	Real l_0_{ 0.0 }; // LIQSS-adjusted coefficient

}; // Variable_LIQSS2

} // QSS

#endif