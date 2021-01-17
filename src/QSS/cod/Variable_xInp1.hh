// QSS1 Input Variable
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

#ifndef QSS_cod_Variable_xInp1_hh_INCLUDED
#define QSS_cod_Variable_xInp1_hh_INCLUDED

// QSS Headers
#include <QSS/cod/Variable_Inp.hh>

namespace QSS {
namespace cod {

// QSS1 Input Variable
template< class F >
class Variable_xInp1 final : public Variable_Inp< F >
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
	using Super::observed;
	using Super::observes;

	using Super::add_discrete;
	using Super::add_QSS_Inp;
	using Super::advance_observers;
	using Super::dt_infinity;
	using Super::init_observers;
	using Super::shift_discrete;
	using Super::shift_QSS_Inp;

private: // Types

	using Super::f_;

public: // Creation

	// Constructor
	explicit
	Variable_xInp1(
	 std::string const & name,
	 Real const rTol = 1.0e-4,
	 Real const aTol = 1.0e-6
	) :
	 Super( 1, name, rTol, aTol )
	{}

public: // Property

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
	q( Time const t ) const
	{
		return x_0_ + ( x_1_ * ( t - tQ ) );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const ) const
	{
		return x_1_;
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
		assert( ! observes() );
		init_observers();
		x_0_ = f_.vs( tQ );
	}

	// Initialization: Stage 1
	void
	init_1()
	{
		x_1_ = f_.df1( tQ );
		tD = f_.tD( tQ );
		set_qTol();
		set_tE();
		( tE < tD ) ? add_QSS_Inp( tE ) : add_discrete( tD );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
	}

	// Discrete Advance
	void
	advance_discrete()
	{
		x_0_ = f_.vs( tX = tQ = tD );
		x_1_ = f_.df1( tD );
		tD = f_.tD( tD );
		set_qTol();
		set_tE();
		( tE < tD ) ? shift_QSS_Inp( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "↕  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
		if ( observed() ) advance_observers();
	}

	// Discrete Advance: Simultaneous
	void
	advance_discrete_s()
	{
		x_0_ = f_.vs( tX = tQ = tD );
		x_1_ = f_.df1( tD );
		tD = f_.tD( tD );
		set_qTol();
		set_tE();
		( tE < tD ) ? shift_QSS_Inp( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "↕= " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
	}

	// QSS Advance
	void
	advance_QSS()
	{
		x_0_ = f_.vs( tX = tQ = tE );
		x_1_ = f_.df1( tQ );
		tD = f_.tD( tQ );
		set_qTol();
		set_tE();
		( tE < tD ) ? shift_QSS_Inp( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
		if ( observed() ) advance_observers();
	}

private: // Methods

	// Set QSS Tolerance
	void
	set_qTol()
	{
		qTol = std::max( rTol * std::abs( x_0_ ), aTol );
		assert( qTol > 0.0 );
	}

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE()
	{
		assert( tX <= tQ );
		assert( dt_min <= dt_max );
		Time dt( x_1_ != 0.0 ? qTol / std::abs( x_1_ ) : infinity );
		dt = std::min( std::max( dt_infinity( dt ), dt_min ), dt_max );
		tE = ( dt != infinity ? tQ + dt : infinity );
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }; // Coefficients

}; // Variable_xInp1

} // cod
} // QSS

#endif
