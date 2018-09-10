// FMU-Based QSS2 Input Variable
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

#ifndef QSS_fmu_Variable_xInp2_hh_INCLUDED
#define QSS_fmu_Variable_xInp2_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable_Inp.hh>

namespace QSS {
namespace fmu {

// FMU-Based QSS2 Input Variable
class Variable_xInp2 final : public Variable_Inp
{

public: // Types

	using Super = Variable_Inp;

public: // Creation

	// Constructor
	explicit
	Variable_xInp2(
	 std::string const & name,
	 Real const rTol = 1.0e-4,
	 Real const aTol = 1.0e-6,
	 FMU_Variable const var = FMU_Variable(),
	 Function f = Function()
	) :
	 Super( name, rTol, aTol, var, f )
	{}

public: // Properties

	// Order of Method
	int
	order() const
	{
		return 2;
	}

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
		Time const tDel( t - tQ );
		return x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const t ) const
	{
		return x_1_ + ( two * x_2_ * ( t - tQ ) );
	}

	// Quantized Second Derivative at Time t
	Real
	q2( Time const ) const
	{
		return two * x_2_;
	}

	// Simultaneous Value at Time t
	Real
	s( Time const t ) const
	{
		Time const tDel( t - tQ );
		return x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// Simultaneous Numeric Differentiation Value at Time t
	Real
	sn( Time const t ) const
	{
		Time const tDel( t - tQ );
		return x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// Simultaneous First Derivative at Time t
	Real
	s1( Time const t ) const
	{
		return x_1_ + ( two * x_2_ * ( t - tQ ) );
	}

	// Simultaneous Second Derivative at Time t
	Real
	s2( Time const ) const
	{
		return two * x_2_;
	}

public: // Methods

	// Initialization
	void
	init()
	{
		init_0();
		init_1();
		init_2();
	}

	// Initialization: Stage 0
	void
	init_0()
	{
		assert( observees_.empty() );
		init_observers();
		x_0_ = f_( tQ ).x_0;
		set_qTol();
	}

	// Initialization: Stage 1
	void
	init_1()
	{
		x_1_ = f_( tQ ).x_1;
	}

	// Initialization: Stage 2
	void
	init_2()
	{
		x_2_ = one_half * f_( tQ ).x_2;
		set_tE();
		tD = f_( tQ ).tD;
		tE < tD ? add_QSS( tE ) : add_discrete( tD );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
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
		x_0_ = f_( tX = tQ = tD ).x_0;
		set_qTol();
		x_1_ = f_( tD ).x_1;
		x_2_ = one_half * f_( tD ).x_2;
		set_tE();
		tD = f_( tD ).tD;
		tE < tD ? shift_QSS( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
		if ( have_observers_ ) advance_observers();
	}

	// Discrete Advance: Stage 0
	void
	advance_discrete_0()
	{
		x_0_ = f_( tX = tQ = tD ).x_0;
		set_qTol();
	}

	// Discrete Advance: Stage 1
	void
	advance_discrete_1()
	{
		x_1_ = f_( tD ).x_1;
	}

	// Discrete Advance: Stage 2
	void
	advance_discrete_2()
	{
		x_2_ = one_half * f_( tD ).x_2;
		set_tE();
		tD = f_( tD ).tD;
		tE < tD ? shift_QSS( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
	}

	// QSS Advance
	void
	advance_QSS()
	{
		x_0_ = f_( tX = tQ = tE ).x_0;
		set_qTol();
		x_1_ = f_( tQ ).x_1;
		x_2_ = one_half * f_( tQ ).x_2;
		set_tE();
		tD = f_( tQ ).tD;
		tE < tD ? shift_QSS( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
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
		x_1_ = f_( tQ ).x_1;
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2()
	{
		x_2_ = one_half * f_( tQ ).x_2;
		set_tE();
		tD = f_( tQ ).tD;
		tE < tD ? shift_QSS( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
	}

private: // Methods

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE()
	{
		assert( tX <= tQ );
		assert( dt_min <= dt_max );
		Time dt( x_2_ != 0.0 ? std::sqrt( qTol / std::abs( x_2_ ) ) : infinity );
		dt = std::min( std::max( dt, dt_min ), dt_max );
		tE = ( dt != infinity ? tQ + dt : infinity );
		if ( ( options::inflection ) && ( x_2_ != 0.0 ) && ( signum( x_1_ ) != signum( x_2_ ) ) ) {
			Time const tI( tX - ( x_1_ / ( two * x_2_ ) ) );
			if ( tQ < tI ) tE = std::min( tE, tI );
		}
		tE_infinity_tQ();
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }; // Continuous rep coefficients

};

} // fmu
} // QSS

#endif