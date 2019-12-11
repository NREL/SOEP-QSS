// QSS3 Input Variable
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

#ifndef QSS_cod_Variable_Inp3_hh_INCLUDED
#define QSS_cod_Variable_Inp3_hh_INCLUDED

// QSS Headers
#include <QSS/cod/Variable_Inp.hh>

namespace QSS {
namespace cod {

// QSS3 Input Variable
template< class F >
class Variable_Inp3 final : public Variable_Inp< F >
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
	using Super::observed_;
	using Super::observees_;

	using Super::add_discrete;
	using Super::add_QSS;
	using Super::advance_observers;
	using Super::event;
	using Super::shift_discrete;
	using Super::shift_QSS;
	using Super::init_observees;
	using Super::init_observers;
	using Super::tE_infinity_tQ;

private: // Types

	using Super::f_;

public: // Creation

	// Constructor
	explicit
	Variable_Inp3(
	 std::string const & name,
	 Real const rTol = 1.0e-4,
	 Real const aTol = 1.0e-6
	) :
	 Super( 3, name, rTol, aTol )
	{}

public: // Property

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

	// Initialization: Stage 0
	void
	init_0()
	{
		assert( observees_.empty() );
		init_observers();
		x_0_ = f_.vs( tQ );
	}

	// Initialization: Stage 1
	void
	init_1()
	{
		x_1_ = f_.dc1( tQ );
	}

	// Initialization: Stage 2
	void
	init_2()
	{
		x_2_ = one_half * f_.dc2( tQ );
	}

	// Initialization: Stage 3
	void
	init_3()
	{
		x_3_ = one_sixth * f_.dc3( tQ );
		tD = f_.tD( tQ );
		set_qTol();
		set_tE();
		( tE < tD ) ? add_QSS( tE ) : add_discrete( tD );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
	}

	// Discrete Advance
	void
	advance_discrete()
	{
		x_0_ = f_.vs( tX = tQ = tD );
		x_1_ = f_.dc1( tD );
		x_2_ = one_half * f_.dc2( tD );
		x_3_ = one_sixth * f_.dc3( tD );
		tD = f_.tD( tD );
		set_qTol();
		set_tE();
		( tE < tD ) ? shift_QSS( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
		if ( observed_ ) advance_observers();
	}

	// Discrete Advance: Simultaneous
	void
	advance_discrete_s()
	{
		x_0_ = f_.vs( tX = tQ = tD );
		x_1_ = f_.dc1( tD );
		x_2_ = one_half * f_.dc2( tD );
		x_3_ = one_sixth * f_.dc3( tD );
		tD = f_.tD( tD );
		set_qTol();
		set_tE();
		( tE < tD ) ? shift_QSS( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
	}

	// QSS Advance
	void
	advance_QSS()
	{
		x_0_ = f_.vs( tX = tQ = tE );
		x_1_ = f_.dc1( tQ );
		x_2_ = one_half * f_.dc2( tQ );
		x_3_ = one_sixth * f_.dc3( tQ );
		tD = f_.tD( tQ );
		set_qTol();
		set_tE();
		( tE < tD ) ? shift_QSS( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
		if ( observed_ ) advance_observers();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0()
	{
		x_0_ = f_.vs( tX = tQ = tE );
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1()
	{
		x_1_ = f_.dc1( tQ );
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2()
	{
		x_2_ = one_half * f_.dc2( tQ );
	}

	// QSS Advance: Stage 3
	void
	advance_QSS_3()
	{
		x_3_ = one_sixth * f_.dc3( tQ );
		tD = f_.tD( tQ );
		set_qTol();
		set_tE();
		( tE < tD ) ? shift_QSS( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << '\n';
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
		Time dt( x_3_ != 0.0 ? std::cbrt( qTol / std::abs( x_3_ ) ) : infinity );
		dt = std::min( std::max( dt, dt_min ), dt_max );
		tE = ( dt != infinity ? tQ + dt : infinity );
		if ( ( options::inflection ) && ( x_3_ != 0.0 ) && ( signum( x_2_ ) != signum( x_3_ ) ) ) {
			Time const tI( tX - ( x_2_ / ( three * x_3_ ) ) );
			if ( tQ < tI ) tE = std::min( tE, tI );
		}
		tE_infinity_tQ();
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }, x_3_{ 0.0 }; // Coefficients

};

} // cod
} // QSS

#endif
