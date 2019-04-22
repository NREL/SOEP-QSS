// FMU-Based QSS1 Input Variable
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2018 Objexx Engineerinc, Inc. All rights reserved.
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

#ifndef QSS_fmu_Variable_xInp1_hh_INCLUDED
#define QSS_fmu_Variable_xInp1_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable_Inp.hh>

namespace QSS {
namespace fmu {

// FMU-Based QSS1 Input Variable
class Variable_xInp1 final : public Variable_Inp
{

public: // Types

	using Super = Variable_Inp;

public: // Creation

	// Constructor
	Variable_xInp1(
	 std::string const & name,
	 Real const rTol,
	 Real const aTol,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable(),
	 Function f = Function()
	) :
	 Super( 1, name, rTol, aTol, fmu_me, var, f )
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

	// Simultaneous Value at Time t
	Real
	s( Time const t ) const
	{
		return x_0_ + ( x_1_ * ( t - tQ ) );
	}

	// Simultaneous Numeric Differentiation Value at Time t
	Real
	sn( Time const t ) const
	{
		return x_0_ + ( x_1_ * ( t - tQ ) );
	}

	// Simultaneous First Derivative at Time t
	Real
	s1( Time const ) const
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
		assert( f() );
		assert( observees_.empty() );
		init_observers();
		fmu_set_real( x_0_ = f_( tQ ).x_0 );
		set_qTol();
	}

	// Initialization: Stage 1
	void
	init_1()
	{
		SmoothToken const s( f_( tQ ) );
		x_1_ = s.x_1;
		set_tE();
		tD = s.tD;
		tE < tD ? add_QSS( tE ) : add_discrete( tD );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
	}

	// Set Current Tolerance
	void
	set_qTol()
	{
		qTol = std::max( rTol * std::abs( x_0_ ), aTol );
		assert( qTol > 0.0 );
	}

	// Discrete Advance
	void
	advance_discrete()
	{
		SmoothToken const s( f_( tX = tQ = tD ) );
		x_0_ = s.x_0;
		set_qTol();
		x_1_ = s.x_1;
		set_tE();
		tD = s.tD;
		tE < tD ? shift_QSS( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
		if ( have_observers_ ) advance_observers();
	}

	// Discrete Advance Simultaneous
	void
	advance_discrete_simultaneous()
	{
		SmoothToken const s( f_( tX = tQ = tD ) );
		x_0_ = s.x_0;
		set_qTol();
		x_1_ = s.x_1;
		set_tE();
		tD = s.tD;
		tE < tD ? shift_QSS( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
	}

	// QSS Advance
	void
	advance_QSS()
	{
		SmoothToken const s( f_( tX = tQ = tE ) );
		x_0_ = s.x_0;
		set_qTol();
		x_1_ = s.x_1;
		set_tE();
		tD = s.tD;
		tE < tD ? shift_QSS( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
		if ( have_observers_ ) advance_observers();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0()
	{
		x_0_ = f_( tX = tQ = tE ).x_0;
		set_qTol();
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1()
	{
		SmoothToken const s( f_( tQ ) );
		x_1_ = s.x_1;
		set_tE();
		tD = s.tD;
		tE < tD ? shift_QSS( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
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

};

} // fmu
} // QSS

#endif
