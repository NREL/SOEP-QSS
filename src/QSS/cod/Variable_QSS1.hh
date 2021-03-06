// QSS1 Variable
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

#ifndef QSS_cod_Variable_QSS1_hh_INCLUDED
#define QSS_cod_Variable_QSS1_hh_INCLUDED

// QSS Headers
#include <QSS/cod/Variable_QSS.hh>

namespace QSS {
namespace cod {

// QSS1 Variable
template< template< typename > class F >
class Variable_QSS1 final : public Variable_QSS< F >
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
	Variable_QSS1(
	 std::string const & name,
	 Real const rTol = 1.0e-4,
	 Real const aTol = 1.0e-6,
	 Real const zTol = 1.0e-6,
	 Real const xIni = 0.0
	) :
	 Super( 1, name, rTol, aTol, zTol, xIni ),
	 x_0_( xIni ),
	 q_0_( xIni )
	{
		set_qTol();
	}

public: // Property

	// Continuous Value at Time t
	Real
	x( Time const t ) const override
	{
		return x_0_ + ( x_1_ * ( t - tX ) );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const ) const override
	{
		return x_1_;
	}

	// Quantized Value at Time t
	Real
	q( Time const ) const override
	{
		return q_0_;
	}

public: // Methods

	// Initialization
	void
	init() override
	{
		init_0();
		init_1();
	}

	// Initialization to a Value
	void
	init( Real const x ) override
	{
		init_0( x );
		init_1();
	}

	// Initialization: Stage 0
	void
	init_0() override
	{
		x_0_ = q_0_ = xIni;
	}

	// Initialization to a Value: Stage 0
	void
	init_0( Real const x ) override
	{
		x_0_ = q_0_ = x;
	}

	// Initialization: Stage 1
	void
	init_1() override
	{
		init_observers();
		init_observees();
		x_1_ = d_.q( tQ );
		set_qTol();
		set_tE_aligned();
		add_QSS( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << " [q]" << "   = " << x_0_ << x_1_ << x_delta << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
	}

	// QSS Advance
	void
	advance_QSS() override
	{
		x_0_ = q_0_ = x_0_ + ( x_1_ * ( tE - tX ) );
		tX = tQ = tE;
		x_1_ = d_.q( tQ );
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << " [q]" << "   = " << x_0_ << x_1_ << x_delta << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
		if ( observed() ) advance_observers();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0() override
	{
		x_0_ = q_0_ = x_0_ + ( x_1_ * ( tE - tX ) );
		tX = tQ = tE;
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1() override
	{
		x_1_ = d_.q( tQ );
	}

	// QSS Advance: Stage Final
	void
	advance_QSS_F() override
	{
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "!= " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << " [q]" << "   = " << x_0_ << x_1_ << x_delta << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
	}

	// Handler Advance
	void
	advance_handler( Time const t, Real const x ) override
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_0_ = x;
		x_1_ = d_.q( t );
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "*  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << " [q]" << "   = " << x_0_ << x_1_ << x_delta << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
		if ( observed() ) advance_observers();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t, Real const x ) override
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_0_ = x;
	}

	// Handler Advance: Stage 1
	void
	advance_handler_1() override
	{
		x_1_ = d_.q( tQ );
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "*= " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << " [q]" << "   = " << x_0_ << x_1_ << x_delta << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
	}

	// Observer Advance
	void
	advance_observer( Time const t ) override
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		x_0_ += ( x_1_ * ( t - tX ) );
		x_1_ = d_.q( tX = t );
		set_tE_unaligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << " ^ " << name() << '(' << tX << ')' << " = " << std::showpos << q_0_ << " [q(" << std::noshowpos << tQ << std::showpos << ")]" << "   = " << x_0_ << x_1_ << x_delta << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
	}

	// Observer Advance: Parallel
	void
	advance_observer_parallel( Time const t ) override
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		x_0_ += ( x_1_ * ( t - tX ) );
		x_1_ = d_.q( tX = t );
		set_tE_unaligned();
	}

	// Observer Advance: Serial + Diagnostics
	void
	advance_observer_serial_d() override
	{
		assert( options::output::d );
		shift_QSS( tE );
		std::cout << " ^ " << name() << '(' << tX << ')' << " = " << std::showpos << q_0_ << " [q(" << std::noshowpos << tQ << std::showpos << ")]" << "   = " << x_0_ << x_1_ << x_delta << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
	}

private: // Methods

	// Set QSS Tolerance
	void
	set_qTol()
	{
		qTol = std::max( rTol * std::abs( q_0_ ), aTol );
		assert( qTol > 0.0 );
	}

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE_aligned()
	{
		assert( tX <= tQ );
		assert( dt_min <= dt_max );
		Time dt( x_1_ != 0.0 ? qTol / std::abs( x_1_ ) : infinity );
		dt = std::min( std::max( dt_infinity( dt ), dt_min ), dt_max );
		tE = ( dt != infinity ? tQ + dt : infinity );
	}

	// Set End Time: Quantized and Continuous Unaligned
	void
	set_tE_unaligned()
	{
		assert( tQ <= tX );
		assert( dt_min <= dt_max );
		Time dt(
		 ( x_1_ > 0.0 ? ( q_0_ + qTol - x_0_ ) / x_1_ :
		 ( x_1_ < 0.0 ? ( q_0_ - qTol - x_0_ ) / x_1_ :
		 infinity ) ) );
		dt = std::min( std::max( dt_infinity( dt ), dt_min ), dt_max );
		tE = ( dt != infinity ? tX + dt : infinity );
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }; // Continuous rep coefficients
	Real q_0_{ 0.0 }; // Quantized rep coefficients

}; // Variable_QSS1

} // cod
} // QSS

#endif
