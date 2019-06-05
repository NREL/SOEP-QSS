// xQSS3 Variable
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2019 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_cod_Variable_xQSS3_hh_INCLUDED
#define QSS_cod_Variable_xQSS3_hh_INCLUDED

// QSS Headers
#include <QSS/cod/Variable_QSS.hh>

namespace QSS {
namespace cod {

// xQSS3 Variable
template< template< typename > class F >
class Variable_xQSS3 final : public Variable_QSS< F >
{

public: // Types

	using Super = Variable_QSS< F >;

	using Real = Variable::Real;
	using Time = Variable::Time;

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
	using Super::dt_inf;
	using Super::self_observer;
	using Super::have_observers_;

	using Super::add_QSS;
	using Super::advance_observers;
	using Super::event;
	using Super::shift_QSS;
	using Super::init_observees;
	using Super::init_observers;
	using Super::tE_infinity_tQ;
	using Super::tE_infinity_tX;

private: // Types

	using Super::d_;
	using Super::event_;
	using Super::observers_;

public: // Creation

	// Constructor
	explicit
	Variable_xQSS3(
	 std::string const & name,
	 Real const rTol = 1.0e-4,
	 Real const aTol = 1.0e-6,
	 Real const xIni = 0.0
	) :
	 Super( 3, name, rTol, aTol, xIni ),
	 x_0_( xIni ),
	 q_0_( xIni )
	{
		set_qTol();
	}

public: // Properties

	// Continuous Value at Time t
	Real
	x( Time const t ) const
	{
		Time const tDel( t - tX );
		return x_0_ + ( ( x_1_ + ( ( x_2_ + ( x_3_ * tDel ) ) * tDel ) ) * tDel );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const t ) const
	{
		Time const tDel( t - tX );
		return x_1_ + ( ( ( two * x_2_ ) + ( three * x_3_ * tDel ) ) * tDel );
	}

	// Continuous Second Derivative at Time t
	Real
	x2( Time const t ) const
	{
		return ( two * x_2_ ) + ( six * x_3_ * ( t - tX ) );
	}

	// Continuous Third Derivative at Time t
	Real
	x3( Time const ) const
	{
		return six * x_3_;
	}

	// Quantized Value at Time t
	Real
	q( Time const t ) const
	{
		Time const tDel( t - tQ );
		return q_0_ + ( ( q_1_ + ( ( q_2_ + ( q_3_ * tDel ) ) * tDel ) ) * tDel );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const t ) const
	{
		Time const tDel( t - tQ );
		return q_1_ + ( ( ( two * q_2_ ) + ( three * q_3_ * tDel ) ) * tDel );
	}

	// Quantized Second Derivative at Time t
	Real
	q2( Time const t ) const
	{
		return ( two * q_2_ ) + ( six * q_3_ * ( t - tQ ) );
	}

	// Quantized Third Derivative at Time t
	Real
	q3( Time const ) const
	{
		return six * q_3_;
	}

	// Simultaneous Value at Time t
	Real
	s( Time const t ) const
	{
		Time const tDel( t - tQ );
		return q_0_ + ( ( q_1_ + ( ( q_2_ + ( q_3_ * tDel ) ) * tDel ) ) * tDel );
	}

	// Simultaneous Numeric Differentiation Value at Time t
	Real
	sn( Time const t ) const
	{
		Time const tDel( t - tQ );
		return q_0_ + ( ( q_1_ + ( ( q_2_ + ( q_3_ * tDel ) ) * tDel ) ) * tDel );
	}

	// Simultaneous First Derivative at Time t
	Real
	s1( Time const t ) const
	{
		Time const tDel( t - tQ );
		return q_1_ + ( ( ( two * q_2_ ) + ( three * q_3_ * tDel ) ) * tDel );
	}

	// Simultaneous Second Derivative at Time t
	Real
	s2( Time const t ) const
	{
		return ( two * q_2_ ) + ( six * q_3_ * ( t - tQ ) );
	}

	// Simultaneous Third Derivative at Time t
	Real
	s3( Time const ) const
	{
		return six * q_3_;
	}

public: // Methods

	// Initialization
	void
	init()
	{
		init_0();
		init_1();
		init_2();
		init_3();
	}

	// Initialization to a Value
	void
	init( Real const x )
	{
		init_0( x );
		init_1();
		init_2();
		init_3();
	}

	// Initialization: Stage 0
	void
	init_0()
	{
		x_0_ = q_0_ = xIni;
		set_qTol();
	}

	// Initialization to a Value: Stage 0
	void
	init_0( Real const x )
	{
		x_0_ = q_0_ = x;
		set_qTol();
	}

	// Initialization: Stage 1
	void
	init_1()
	{
		init_observers();
		init_observees();
		x_1_ = q_1_ = d_.ss( tQ );
	}

	// Initialization: Stage 2
	void
	init_2()
	{
		x_2_ = q_2_ = one_half * d_.sc1( tQ );
	}

	// Initialization: Stage 3
	void
	init_3()
	{
		x_3_ = q_3_ = one_sixth * d_.sc2( tQ );
		set_tE_aligned();
		add_QSS( tE );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << q_2_ << "*t^2" << q_3_ << "*t^3" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
	}

	// Set Current Tolerance
	void
	set_qTol()
	{
		qTol = std::max( rTol * std::abs( q_0_ ), aTol );
		assert( qTol > 0.0 );
	}

	// QSS Advance
	void
	advance_QSS()
	{
		Time const tDel( tE - tX );
		tX = tQ = tE;
		x_0_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
		set_qTol();
		x_1_ = q_1_ = d_.qs( tQ );
		x_2_ = q_2_ = one_half * d_.qc1( tQ );
		x_3_ = q_3_ = one_sixth * d_.qc2( tQ );
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << q_2_ << "*t^2" << q_3_ << "*t^3" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( have_observers_ ) advance_observers();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0()
	{
		Time const tDel( tE - tX );
		tX = tQ = tE;
		x_0_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
		set_qTol();
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1()
	{
		x_1_ = q_1_ = d_.ss( tQ );
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2()
	{
		x_2_ = q_2_ = one_half * d_.sc1( tQ );
	}

	// QSS Advance: Stage 3
	void
	advance_QSS_3()
	{
		x_3_ = q_3_ = one_sixth * d_.sc2( tQ );
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << q_2_ << "*t^2" << q_3_ << "*t^3" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
	}

	// Observer Advance
	void
	advance_observer( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		x_0_ = x_0_ + ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
		x_1_ = d_.qs( t );
		x_2_ = one_half * d_.qc1( t );
		x_3_ = one_sixth * d_.qc2( tX = t );
		set_tE_unaligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "  " << name << '(' << tX << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << q_2_ << "*t^2" << q_3_ << "*t^3" << " [q]" << '(' << std::noshowpos << tQ << std::showpos << ')' << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
	}

	// Observer Advance: Parallel
	void
	advance_observer_parallel( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		x_0_ = x_0_ + ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
		x_1_ = d_.qs( t );
		x_2_ = one_half * d_.qc1( t );
		x_3_ = one_sixth * d_.qc2( tX = t );
		set_tE_unaligned();
	}

	// Observer Advance: Serial + Diagnostics
	void
	advance_observer_serial_d()
	{
		assert( options::output::d );
		shift_QSS( tE );
		std::cout << "  " << name << '(' << tX << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << q_2_ << "*t^2" << q_3_ << "*t^3" << " [q]" << '(' << std::noshowpos << tQ << std::showpos << ')' << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
	}

	// Handler Advance
	void
	advance_handler( Time const t, Real const x )
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_0_ = x;
		set_qTol();
		x_1_ = q_1_ = d_.qs( t );
		x_2_ = q_2_ = one_half * d_.qc1( t );
		x_3_ = q_3_ = one_sixth * d_.qc2( t );
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << q_2_ << "*t^2" << q_3_ << "*t^3" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( have_observers_ ) advance_observers();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t, Real const x )
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_0_ = x;
		set_qTol();
	}

	// Handler Advance: Stage 1
	void
	advance_handler_1()
	{
		x_1_ = q_1_ = d_.qs( tQ );
	}

	// Handler Advance: Stage 2
	void
	advance_handler_2()
	{
		x_2_ = q_2_ = one_half * d_.qc1( tQ );
	}

	// Handler Advance: Stage 3
	void
	advance_handler_3()
	{
		x_3_ = q_3_ = one_sixth * d_.qc2( tQ );
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << q_2_ << "*t^2" << q_3_ << "*t^3" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
	}

private: // Methods

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE_aligned()
	{
		assert( tX <= tQ );
		assert( dt_min <= dt_max );
		Time dt( x_3_ != 0.0 ? std::cbrt( qTol / std::abs( x_3_ ) ) : infinity );
		dt = std::min( std::max( dt, dt_min ), dt_max );
		tE = ( dt != infinity ? tQ + dt : infinity );
		if ( ( options::inflection ) && ( x_3_ != 0.0 ) && ( signum( x_2_ ) != signum( x_3_ ) ) ) {
			Time const tI( tX - ( x_2_ / ( three * x_3_ ) ) );
			if ( tQ < tI ) tE = std::min( tE, tI );
		}
		tE_infinity_tQ();
	}

	// Set End Time: Quantized and Continuous Unaligned
	void
	set_tE_unaligned()
	{
		assert( tQ <= tX );
		assert( dt_min <= dt_max );
		Time const tXQ( tX - tQ );
		Real const d_0( x_0_ - ( q_0_ + ( q_1_ + ( q_2_ * tXQ ) ) * tXQ ) );
		Real const d_1( x_1_ - ( q_1_ + ( two * q_2_ * tXQ ) ) );
		Real const d_2( x_2_ - q_2_ );
		Time dt;
		if ( ( x_3_ >= 0.0 ) && ( d_2 >= 0.0 ) && ( d_1 >= 0.0 ) ) { // Upper boundary crossing
			dt = min_root_cubic_upper( x_3_, d_2, d_1, d_0 - qTol );
		} else if ( ( x_3_ <= 0.0 ) && ( d_2 <= 0.0 ) && ( d_1 <= 0.0 ) ) { // Lower boundary crossing
			dt = min_root_cubic_lower( x_3_, d_2, d_1, d_0 + qTol );
		} else { // Both boundaries can have crossings
			dt = min_root_cubic_both( x_3_, d_2, d_1, d_0 + qTol, d_0 - qTol );
		}
		dt = std::min( std::max( dt, dt_min ), dt_max );
		tE = ( dt != infinity ? tX + dt : infinity );
		if ( ( options::inflection ) && ( x_3_ != 0.0 ) && ( signum( x_2_ ) != signum( x_3_ ) ) && ( signum( x_2_ ) == signum( q_2_ ) ) ) {
			Time const tI( tX - ( x_2_ / ( three * x_3_ ) ) );
			if ( tX < tI ) tE = std::min( tE, tI );
		}
		tE_infinity_tX();
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }, x_3_{ 0.0 }; // Continuous rep coefficients
	Real q_0_{ 0.0 }, q_1_{ 0.0 }, q_2_{ 0.0 }, q_3_{ 0.0 }; // Quantized rep coefficients

};

} // cod
} // QSS

#endif
