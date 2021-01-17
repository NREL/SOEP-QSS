// LIQSS2 Variable
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2021 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_cod_Variable_LIQSS2_hh_INCLUDED
#define QSS_cod_Variable_LIQSS2_hh_INCLUDED

// QSS Headers
#include <QSS/cod/Variable_QSS.hh>

namespace QSS {
namespace cod {

// LIQSS2 Variable
template< template< typename > class F >
class Variable_LIQSS2 final : public Variable_QSS< F >
{

public: // Types

	using Super = Variable_QSS< F >;

	using Real = Variable::Real;
	using Time = Variable::Time;
	using AdvanceSpecs_LIQSS2 = Variable::AdvanceSpecs_LIQSS2;

	using Super::name;
	using Super::rTol;
	using Super::aTol;
	using Super::qTol;
	using Super::xIni;
	using Super::tQ;
	using Super::tX;
	using Super::tE;
	using Super::dt_min;
	using Super::dt_max;
	using Super::self_observer;
	using Super::observed;

	using Super::add_QSS;
	using Super::advance_observers;
	using Super::dt_infinity;
	using Super::init_observees;
	using Super::init_observers;
	using Super::shift_QSS;

private: // Types

	using Super::d_;

public: // Creation

	// Constructor
	explicit
	Variable_LIQSS2(
	 std::string const & name,
	 Real const rTol = 1.0e-4,
	 Real const aTol = 1.0e-6,
	 Real const xIni = 0.0
	) :
	 Super( 2, name, rTol, aTol, xIni ),
	 x_0_( xIni ),
	 q_c_( xIni ),
	 q_0_( xIni )
	{
		set_qTol();
	}

public: // Predicate

	// LIQSS Variable?
	bool
	is_LIQSS() const
	{
		return true;
	}

public: // Property

	// Continuous Value at Time t
	Real
	x( Time const t ) const
	{
		Time const tDel( t - tX );
		return x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const t ) const
	{
		return x_1_ + ( two * x_2_ * ( t - tX ) );
	}

	// Continuous Second Derivative at Time t
	Real
	x2( Time const ) const
	{
		return two * x_2_;
	}

	// Quantized Value at Time t
	Real
	q( Time const t ) const
	{
		return q_0_ + ( q_1_ * ( t - tQ ) );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const ) const
	{
		return q_1_;
	}

public: // Methods

	// Initialization
	void
	init()
	{
		init_0();
		init_1();
		init_2();
		init_LIQSS();
	}

	// Initialization to a Value
	void
	init( Real const x )
	{
		init_0( x );
		init_1();
		init_2();
		init_LIQSS();
	}

	// Initialization: Stage 0
	void
	init_0()
	{
		x_0_ = q_c_ = q_0_ = xIni;
	}

	// Initialization to a Value: Stage 0
	void
	init_0( Real const x )
	{
		x_0_ = q_c_ = q_0_ = x;
	}

	// Initialization: Stage 1
	void
	init_1()
	{
		init_observers();
		init_observees();
		x_1_ = q_1_ = d_.qs( tQ );
	}

	// Initialization: Stage 2
	void
	init_2()
	{
		set_qTol();
		if ( self_observer() ) {
			advance_LIQSS_s( d_.qlu2( tQ, qTol, q_1_ ) );
		} else {
			x_2_ = one_half * d_.qf1( tQ );
		}
	}

	// Initialization: Stage LIQSS
	void
	init_LIQSS()
	{
		if ( self_observer() ) {
			q_0_ = l_0_;
			q_1_ = x_1_;
		} else {
			q_0_ += signum( x_2_ ) * qTol;
		}
		set_tE_aligned();
		add_QSS( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << " [q]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
	}

	// QSS Advance
	void
	advance_QSS() override final
	{
		Time const tDel( tE - tX );
		tX = tQ = tE;
		x_0_ = q_c_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
		set_qTol();
		if ( self_observer() ) {
			advance_LIQSS( d_.qlu2( tQ, qTol ) );
		} else {
			x_1_ = q_1_ = d_.qs( tQ );
			x_2_ = one_half * d_.qf1( tQ );
			q_0_ += signum( x_2_ ) * qTol;
		}
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << " [q]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( observed() ) advance_observers();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0() override final
	{
		Time const tDel( tE - tX );
		tX = tQ = tE;
		x_0_ = q_c_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1() override final
	{
		x_1_ = q_1_ = d_.qs( tQ );
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2() override final
	{
		set_qTol();
		if ( self_observer() ) {
			advance_LIQSS_s( d_.qlu2( tQ, qTol, q_1_ ) );
		} else {
			x_2_ = one_half * d_.qf1( tQ );
		}
	}

	// QSS Advance: Stage Final
	void
	advance_QSS_F() override final
	{
		if ( self_observer() ) {
			q_0_ = l_0_;
			q_1_ = x_1_;
		} else {
			q_0_ += signum( x_2_ ) * qTol;
		}
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "!= " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << " [q]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
	}

	// Handler Advance
	void
	advance_handler( Time const t, Real const x ) override final
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		x_0_ = q_c_ = q_0_ = x;
		x_1_ = q_1_ = d_.qs( tX = tQ = t );
		x_2_ = one_half * d_.qf1( t );
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "*  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << " [q]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( observed() ) advance_observers();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t, Real const x ) override final
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_c_ = q_0_ = x;
	}

	// Handler Advance: Stage 1
	void
	advance_handler_1() override final
	{
		x_1_ = q_1_ = d_.qs( tQ );
	}

	// Handler Advance: Stage 2
	void
	advance_handler_2() override final
	{
		x_2_ = one_half * d_.qf1( tQ );
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "*= " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << " [q]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
	}

	// Observer Advance
	void
	advance_observer( Time const t ) override final
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		x_0_ += ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
		x_1_ = d_.qs( t );
		x_2_ = one_half * d_.qf1( tX = t );
		set_tE_unaligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << " ▲ " << name() << '(' << tX << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << " [q(" << std::noshowpos << tQ << std::showpos << ")]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
	}

	// Observer Advance: Parallel
	void
	advance_observer_parallel( Time const t ) override final
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		x_0_ += ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
		x_1_ = d_.qs( t );
		x_2_ = one_half * d_.qf1( tX = t );
		set_tE_unaligned();
	}

	// Observer Advance: Serial + Diagnostics
	void
	advance_observer_serial_d() override final
	{
		assert( options::output::d );
		shift_QSS( tE );
		std::cout << " ▲ " << name() << '(' << tX << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << " [q(" << std::noshowpos << tQ << std::showpos << ")]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
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
	advance_LIQSS( AdvanceSpecs_LIQSS2 const & specs )
	{
		assert( qTol > 0.0 );
		assert( self_observer() );
		assert( q_c_ == q_0_ );
		assert( x_0_ == q_0_ );

		// Value at +/- qTol
		Real const q_l( q_c_ - qTol );
		Real const q_u( q_c_ + qTol );

		// Set coefficients based on second derivative signs
		int const dls( signum( specs.l2 ) );
		int const dus( signum( specs.u2 ) );
		if ( ( dls == -1 ) && ( dus == -1 ) ) { // Downward curving trajectory
			q_0_ = q_l;
			x_1_ = q_1_ = specs.l1;
			x_2_ = one_half * specs.l2;
		} else if ( ( dls == +1 ) && ( dus == +1 ) ) { // Upward curving trajectory
			q_0_ = q_u;
			x_1_ = q_1_ = specs.u1;
			x_2_ = one_half * specs.u2;
		} else if ( ( dls == 0 ) && ( dus == 0 ) ) { // Non-curving trajectory
			// Keep q_0_ == q_c_
			x_1_ = q_1_ = one_half * ( specs.l1 + specs.u1 ); // Interpolated 1st deriv at q_0_ == q_c_
			x_2_ = 0.0;
		} else { // Straight trajectory
			q_0_ = std::min( std::max( specs.z0, q_l ), q_u ); // Clipped in case of roundoff
			x_1_ = q_1_ = specs.z1;
			x_2_ = 0.0;
		}
	}

	// Advance Self-Observing Trigger: Simultaneous
	void
	advance_LIQSS_s( AdvanceSpecs_LIQSS2 const & specs )
	{
		assert( qTol > 0.0 );
		assert( self_observer() );
		assert( q_c_ == q_0_ );
		assert( x_0_ == q_0_ );

		// Value at +/- qTol
		Real const q_l( q_c_ - qTol );
		Real const q_u( q_c_ + qTol );

		// Set coefficients based on second derivative signs
		int const dls( signum( specs.l2 ) );
		int const dus( signum( specs.u2 ) );
		if ( ( dls == -1 ) && ( dus == -1 ) ) { // Downward curving trajectory
			l_0_ = q_l;
			x_1_ = specs.l1;
			x_2_ = one_half * specs.l2;
		} else if ( ( dls == +1 ) && ( dus == +1 ) ) { // Upward curving trajectory
			l_0_ = q_u;
			x_1_ = specs.u1;
			x_2_ = one_half * specs.u2;
		} else if ( ( dls == 0 ) && ( dus == 0 ) ) { // Non-curving trajectory
			l_0_ = q_c_;
			x_1_ = one_half * ( specs.l1 + specs.u1 ); // Interpolated 1st deriv at q_0_ == q_c_
			x_2_ = 0.0;
		} else { // Straight trajectory
			l_0_ = std::min( std::max( specs.z0, q_l ), q_u ); // Clipped in case of roundoff
			x_1_ = specs.z1;
			x_2_ = 0.0;
		}
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }; // Continuous rep coefficients
	Real q_c_{ 0.0 }, q_0_{ 0.0 }, q_1_{ 0.0 }; // Quantized rep coefficients
	Real l_0_{ 0.0 }; // LIQSS-adjusted coefficient

}; // Variable_LIQSS2

} // cod
} // QSS

#endif
