// LIQSS3 Variable
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

#ifndef QSS_cod_Variable_LIQSS3_hh_INCLUDED
#define QSS_cod_Variable_LIQSS3_hh_INCLUDED

// QSS Headers
#include <QSS/cod/Variable_QSS.hh>

namespace QSS {
namespace cod {

// LIQSS3 Variable
template< template< typename > class F >
class Variable_LIQSS3 final : public Variable_QSS< F >
{

public: // Types

	using Super = Variable_QSS< F >;

	using Real = Variable::Real;
	using Time = Variable::Time;
	using AdvanceSpecs_LIQSS3 = Variable::AdvanceSpecs_LIQSS3;

	using Super::name;
	using Super::rTol;
	using Super::aTol;
	using Super::qTol;
	using Super::zTol;
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
	Variable_LIQSS3(
	 std::string const & name,
	 Real const rTol = 1.0e-4,
	 Real const aTol = 1.0e-6,
	 Real const zTol = 1.0e-6,
	 Real const xIni = 0.0
	) :
	 Super( 3, name, rTol, aTol, zTol, xIni ),
	 x_0_( xIni ),
	 q_c_( xIni ),
	 q_0_( xIni )
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
		return q_0_ + ( ( q_1_ + ( q_2_ * tDel ) ) * tDel );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const t ) const override
	{
		return q_1_ + ( two * q_2_ * ( t - tQ ) );
	}

	// Quantized Second Derivative at Time t
	Real
	q2( Time const ) const override
	{
		return two * q_2_;
	}

public: // Methods

	// Initialization
	void
	init() override
	{
		init_0();
		init_1();
		init_2();
		init_3();
		init_LIQSS();
	}

	// Initialization to a Value
	void
	init( Real const x ) override
	{
		init_0( x );
		init_1();
		init_2();
		init_3();
		init_LIQSS();
	}

	// Initialization: Stage 0
	void
	init_0() override
	{
		x_0_ = q_c_ = q_0_ = xIni;
	}

	// Initialization to a Value: Stage 0
	void
	init_0( Real const x ) override
	{
		x_0_ = q_c_ = q_0_ = x;
	}

	// Initialization: Stage 1
	void
	init_1() override
	{
		init_observers();
		init_observees();
		x_1_ = q_1_ = d_.qs( tQ );
	}

	// Initialization: Stage 2
	void
	init_2() override
	{
		x_2_ = q_2_ = one_half * d_.qc1( tQ );
	}

	// Initialization: Stage 3
	void
	init_3() override
	{
		set_qTol();
		if ( self_observer() ) {
			advance_LIQSS_s( d_.qlu3( tQ, qTol, q_1_, two * q_2_ ) );
		} else {
			x_3_ = one_sixth * d_.qc2( tQ );
		}
	}

	// Initialization: Stage LIQSS
	void
	init_LIQSS() override
	{
		if ( self_observer() ) {
			q_0_ = l_0_;
			q_1_ = x_1_;
			q_2_ = x_2_;
		} else {
			q_0_ += signum( x_3_ ) * qTol;
		}
		set_tE_aligned();
		add_QSS( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << " [q]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
	}

	// QSS Advance
	void
	advance_QSS() override
	{
		Time const tDel( tE - tX );
		tX = tQ = tE;
		x_0_ = q_c_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
		set_qTol();
		if ( self_observer() ) {
			advance_LIQSS( d_.qlu3( tQ, qTol ) );
		} else {
			x_1_ = q_1_ = d_.qs( tQ );
			x_2_ = q_2_ = one_half * d_.qc1( tQ );
			x_3_ = one_sixth * d_.qc2( tQ );
			q_0_ += signum( x_3_ ) * qTol;
		}
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << " [q]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
		if ( observed() ) advance_observers();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0() override
	{
		Time const tDel( tE - tX );
		tX = tQ = tE;
		x_0_ = q_c_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1() override
	{
		x_1_ = q_1_ = d_.qs( tQ );
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2() override
	{
		x_2_ = q_2_ = one_half * d_.qc1( tQ );
	}

	// QSS Advance: Stage 3
	void
	advance_QSS_3() override
	{
		set_qTol();
		if ( self_observer() ) {
			advance_LIQSS_s( d_.qlu3( tQ, qTol, q_1_, two * q_2_ ) );
		} else {
			x_3_ = one_sixth * d_.qc2( tQ );
		}
	}

	// QSS Advance: Stage Final
	void
	advance_QSS_F() override
	{
		if ( self_observer() ) {
			q_0_ = l_0_;
			q_1_ = x_1_;
			q_2_ = x_2_;
		} else {
			q_0_ += signum( x_3_ ) * qTol;
		}
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "!= " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << " [q]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
	}

	// Handler Advance
	void
	advance_handler( Time const t, Real const x ) override
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_c_ = q_0_ = x;
		x_1_ = q_1_ = d_.qs( t );
		x_2_ = q_2_ = one_half * d_.qc1( t );
		x_3_ = one_sixth * d_.qc2( t );
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "*  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << " [q]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
		if ( observed() ) advance_observers();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t, Real const x ) override
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_c_ = q_0_ = x;
	}

	// Handler Advance: Stage 1
	void
	advance_handler_1() override
	{
		x_1_ = q_1_ = d_.qs( tQ );
	}

	// Handler Advance: Stage 2
	void
	advance_handler_2() override
	{
		x_2_ = q_2_ = one_half * d_.qc1( tQ );
	}

	// Handler Advance: Stage 3
	void
	advance_handler_3() override
	{
		x_3_ = one_sixth * d_.qc2( tQ );
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "*= " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << " [q]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
	}

	// Observer Advance
	void
	advance_observer( Time const t ) override
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		x_0_ += ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
		x_1_ = d_.qs( t );
		x_2_ = one_half * d_.qc1( t );
		x_3_ = one_sixth * d_.qc2( tX = t );
		set_tE_unaligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << " ^ " << name() << '(' << tX << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << " [q(" << std::noshowpos << tQ << std::showpos << ")]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
	}

	// Observer Advance: Parallel
	void
	advance_observer_parallel( Time const t ) override
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		x_0_ += ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
		x_1_ = d_.qs( t );
		x_2_ = one_half * d_.qc1( t );
		x_3_ = one_sixth * d_.qc2( tX = t );
		set_tE_unaligned();
	}

	// Observer Advance: Serial + Diagnostics
	void
	advance_observer_serial_d() override
	{
		assert( options::output::d );
		shift_QSS( tE );
		std::cout << " ^ " << name() << '(' << tX << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << " [q(" << std::noshowpos << tQ << std::showpos << ")]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
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
		Time dt( x_3_ != 0.0 ? std::cbrt( qTol / std::abs( x_3_ ) ) : infinity );
		dt = std::min( std::max( dt_infinity( dt ), dt_min ), dt_max );
		tE = ( dt != infinity ? tQ + dt : infinity );
		if ( ( options::inflection ) && ( x_3_ != 0.0 ) && ( signum( x_2_ ) != signum( x_3_ ) ) ) {
			Time const tI( tX - ( x_2_ / ( three * x_3_ ) ) );
			if ( tQ < tI ) tE = std::min( tE, tI );
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
		dt = std::min( std::max( dt_infinity( dt ), dt_min ), dt_max );
		tE = ( dt != infinity ? tX + dt : infinity );
		if ( ( options::inflection ) && ( x_3_ != 0.0 ) && ( signum( x_2_ ) != signum( x_3_ ) ) && ( signum( x_2_ ) == signum( q_2_ ) ) ) {
			Time const tI( tX - ( x_2_ / ( three * x_3_ ) ) );
			if ( tX < tI ) tE = std::min( tE, tI );
		}
	}

	// Advance Self-Observing Trigger
	void
	advance_LIQSS( AdvanceSpecs_LIQSS3 const & specs )
	{
		assert( qTol > 0.0 );
		assert( self_observer() );
		assert( q_c_ == q_0_ );
		assert( x_0_ == q_0_ );

		// Value at +/- qTol
		Real const q_l( q_c_ - qTol );
		Real const q_u( q_c_ + qTol );

		// Set coefficients based on third derivative signs
		int const dls( signum( specs.l3 ) );
		int const dus( signum( specs.u3 ) );
		if ( ( dls == -1 ) && ( dus == -1 ) ) { // Downward curve-changing trajectory
			q_0_ = q_l;
			x_1_ = q_1_ = specs.l1;
			x_2_ = q_2_ = one_half * specs.l2;
			x_3_ = one_sixth * specs.l3;
		} else if ( ( dls == +1 ) && ( dus == +1 ) ) { // Upward curve-changing trajectory
			q_0_ = q_u;
			x_1_ = q_1_ = specs.u1;
			x_2_ = q_2_ = one_half * specs.u2;
			x_3_ = one_sixth * specs.u3;
		} else if ( ( dls == 0 ) && ( dus == 0 ) ) { // Non-curve-changing trajectory
			// Keep q_0_ == q_c_
			x_1_ = q_1_ = one_half * ( specs.l1 + specs.u1 ); // Interpolated 1st deriv at q_0_ == q_c_
			x_2_ = q_2_ = one_half * specs.z2;
			x_3_ = 0.0;
		} else { // Quadratic trajectory
			q_0_ = std::min( std::max( specs.z0, q_l ), q_u ); // Clipped in case of roundoff
			x_1_ = q_1_ = specs.z1;
			x_2_ = q_2_ = one_half * specs.z2;
			x_3_ = 0.0;
		}
	}

	// Advance Self-Observing Trigger: Simultaneous
	void
	advance_LIQSS_s( AdvanceSpecs_LIQSS3 const & specs )
	{
		assert( qTol > 0.0 );
		assert( self_observer() );
		assert( q_c_ == q_0_ );
		assert( x_0_ == q_0_ );

		// Value at +/- qTol
		Real const q_l( q_c_ - qTol );
		Real const q_u( q_c_ + qTol );

		// Set coefficients based on third derivative signs
		int const dls( signum( specs.l3 ) );
		int const dus( signum( specs.u3 ) );
		if ( ( dls == -1 ) && ( dus == -1 ) ) { // Downward curve-changing trajectory
			l_0_ = q_l;
			x_1_ = specs.l1;
			x_2_ = one_half * specs.l2;
			x_3_ = one_sixth * specs.l3;
		} else if ( ( dls == +1 ) && ( dus == +1 ) ) { // Upward curve-changing trajectory
			l_0_ = q_u;
			x_1_ = specs.u1;
			x_2_ = one_half * specs.u2;
			x_3_ = one_sixth * specs.u3;
		} else if ( ( dls == 0 ) && ( dus == 0 ) ) { // Non-curve-changing trajectory
			l_0_ = q_c_;
			x_1_ = one_half * ( specs.l1 + specs.u1 ); // Interpolated 1st deriv at q_0_ == q_c_
			x_2_ = one_half * specs.z2;
			x_3_ = 0.0;
		} else { // Quadratic trajectory
			l_0_ = std::min( std::max( specs.z0, q_l ), q_u ); // Clipped in case of roundoff
			x_1_ = specs.z1;
			x_2_ = one_half * specs.z2;
			x_3_ = 0.0;
		}
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }, x_3_{ 0.0 }; // Continuous rep coefficients
	Real q_c_{ 0.0 }, q_0_{ 0.0 }, q_1_{ 0.0 }, q_2_{ 0.0 }; // Quantized rep coefficients
	Real l_0_{ 0.0 }; // LIQSS-adjusted coefficient

}; // Variable_LIQSS3

} // cod
} // QSS

#endif
