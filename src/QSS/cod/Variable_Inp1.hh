// QSS1 Input Variable
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

#ifndef QSS_cod_Variable_Inp1_hh_INCLUDED
#define QSS_cod_Variable_Inp1_hh_INCLUDED

// QSS Headers
#include <QSS/cod/Variable_Inp.hh>

namespace QSS {
namespace cod {

// QSS1 Input Variable
template< class F >
class Variable_Inp1 final : public Variable_Inp< F >
{

public: // Types

	using Super = Variable_Inp< F >;

	using Real = Variable::Real;
	using Time = Variable::Time;

	using Super::name;
	using Super::rTol;
	using Super::aTol;
	using Super::qTol;
	using Super::tQ;
	using Super::tX;
	using Super::tE;
	using Super::tD;
	using Super::dt_min;
	using Super::dt_max;
	using Super::dt_inf;
	using Super::have_observers_;
	using Super::observees_;

	using Super::add_discrete;
	using Super::add_QSS;
	using Super::advance_observers;
	using Super::event;
	using Super::shift_discrete;
	using Super::shift_QSS;
	using Super::init_observers;
	using Super::tE_infinity_tQ;

private: // Types

	using Super::f_;

public: // Creation

	// Constructor
	explicit
	Variable_Inp1(
	 std::string const & name,
	 Real const rTol = 1.0e-4,
	 Real const aTol = 1.0e-6
	) :
	 Super( 1, name, rTol, aTol )
	{}

public: // Properties

	// Continuous Value at Time t
	Real
	x( Time const t ) const
	{
		return x_0_ + ( x_1_ * ( t - tX ) );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const ) const
	{
		return x_1_;
	}

	// Quantized Value at Time t
	Real
	q( Time const ) const
	{
		return q_0_;
	}

	// Simultaneous Value at Time t
	Real
	s( Time const ) const
	{
		return q_0_;
	}

	// Simultaneous Numeric Differentiation Value at Time t
	Real
	sn( Time const ) const
	{
		return q_0_;
	}

public: // Methods

	// Initialization
	void
	init()
	{
		init_0();
		init_1();
	}

	// Initialization: Stage 0
	void
	init_0()
	{
		assert( observees_.empty() );
		init_observers();
		x_0_ = q_0_ = f_.vs( tQ );
		set_qTol();
	}

	// Initialization: Stage 1
	void
	init_1()
	{
		x_1_ = f_.df1( tQ );
		set_tE();
		tD = f_.tD( tQ );
		tE < tD ? add_QSS( tE ) : add_discrete( tD );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << " [q]" << "   = " << x_0_ << x_1_ << "*t" << " [x]" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
	}

	// Set Current Tolerance
	void
	set_qTol()
	{
		qTol = std::max( rTol * std::abs( q_0_ ), aTol );
		assert( qTol > 0.0 );
	}

	// Discrete Advance
	void
	advance_discrete()
	{
		x_0_ = q_0_ = f_.vs( tX = tQ = tD );
		set_qTol();
		x_1_ = f_.df1( tD );
		set_tE();
		tD = f_.tD( tD );
		tE < tD ? shift_QSS( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << " [q]" << "   = " << x_0_ << x_1_ << "*t" << " [x]" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
		if ( have_observers_ ) advance_observers();
	}

	// Discrete Advance Simultaneous
	void
	advance_discrete_simultaneous()
	{
		x_0_ = q_0_ = f_.vs( tX = tQ = tD );
		set_qTol();
		x_1_ = f_.df1( tD );
		set_tE();
		tD = f_.tD( tD );
		tE < tD ? shift_QSS( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << " [q]" << "   = " << x_0_ << x_1_ << "*t" << " [x]" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
	}

	// QSS Advance
	void
	advance_QSS()
	{
		x_0_ = q_0_ = f_.vs( tX = tQ = tE );
		set_qTol();
		x_1_ = f_.df1( tQ );
		set_tE();
		tD = f_.tD( tQ );
		tE < tD ? shift_QSS( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << " [q]" << "   = " << x_0_ << x_1_ << "*t" << " [x]" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
		if ( have_observers_ ) advance_observers();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0()
	{
		x_0_ = q_0_ = f_.vs( tX = tQ = tE );
		set_qTol();
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1()
	{
		x_1_ = f_.df1( tQ );
		set_tE();
		tD = f_.tD( tQ );
		tE < tD ? shift_QSS( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << " [q]" << "   = " << x_0_ << x_1_ << "*t" << " [x]" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
	}

private: // Methods

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE()
	{
		assert( tX <= tQ );
		assert( dt_min <= dt_max );
		Time dt( x_1_ != 0.0 ? qTol / std::abs( x_1_ ) : infinity );
		dt = std::min( std::max( dt, dt_min ), dt_max );
		tE = ( dt != infinity ? tQ + dt : infinity );
		tE_infinity_tQ();
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }; // Continuous rep coefficients
	Real q_0_{ 0.0 }; // Quantized rep coefficients

};

} // cod
} // QSS

#endif
