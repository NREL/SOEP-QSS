// QSS3 Input Variable
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2023 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_Variable_Inp3_hh_INCLUDED
#define QSS_Variable_Inp3_hh_INCLUDED

// QSS Headers
#include <QSS/Variable_Inp.hh>

namespace QSS {

// QSS3 Input Variable
class Variable_Inp3 final : public Variable_Inp
{

public: // Types

	using Super = Variable_Inp;

public: // Creation

	// Constructor
	Variable_Inp3(
	 FMU_ME * fmu_me,
	 std::string const & name,
	 Real const rTol_ = options::rTol,
	 Real const aTol_ = options::aTol,
	 Real const xIni_ = 0.0,
	 FMU_Variable const var = FMU_Variable(),
	 Function f = Function()
	) :
	 Super( fmu_me, 3, name, rTol_, aTol_, xIni_, var, f )
	{}

public: // Predicate

	// Real Variable?
	bool
	is_Real() const override
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
		return x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const t ) const override
	{
		return x_1_ + ( two * x_2_ * ( t - tQ ) );
	}

	// Quantized Second Derivative at Time t
	Real
	q2( Time const ) const override
	{
		return two * x_2_;
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
		init_F();
	}

	// Initialization: Stage 0
	void
	init_0() override
	{
		assert( f() );
		assert( observees().empty() );
		s_ = f_( tQ );
		x_0_ = s_.x0;
		fmu_set_real( x_0_ );
	}

	// Initialization: Stage 1
	void
	init_1() override
	{
		x_1_ = s_.x1;
	}

	// Initialization: Stage 2
	void
	init_2() override
	{
		x_2_ = one_half * s_.x2;
		x_3_ = one_sixth * s_.x3;
		tD = s_.tD;
	}

	// Initialization: Stage Final
	void
	init_F() override
	{
		set_qTol();
		set_tE();
		( tE < tD ) ? add_QSS_Inp( tE ) : add_discrete( tD );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [q]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << std::endl;
	}

	// Discrete Advance
	void
	advance_discrete() override
	{
		tS = tD - tQ;
		s_ = f_( tQ = tX = tD );
		x_0_ = s_.x0;
		x_1_ = s_.x1;
		x_2_ = one_half * s_.x2;
		x_3_ = one_sixth * s_.x3;
		tD = s_.tD;
		set_qTol();
		set_tE();
		( tE < tD ) ? shift_QSS_Inp( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "|  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [q]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << std::endl;
		if ( observed() ) advance_observers();
	}

	// Discrete Advance: Simultaneous
	void
	advance_discrete_simultaneous() override
	{
		tS = tD - tQ;
		s_ = f_( tQ = tX = tD );
		x_0_ = s_.x0;
		x_1_ = s_.x1;
		x_2_ = one_half * s_.x2;
		x_3_ = one_sixth * s_.x3;
		tD = s_.tD;
		set_qTol();
		set_tE();
		( tE < tD ) ? shift_QSS_Inp( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "|= " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [q]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << std::endl;
	}

	// QSS Advance
	void
	advance_QSS() override
	{
		s_ = f_( tQ = tX = tE );
		x_0_ = s_.x0;
		x_1_ = s_.x1;
		x_2_ = one_half * s_.x2;
		x_3_ = one_sixth * s_.x3;
		tD = s_.tD;
		set_qTol();
		set_tE();
		( tE < tD ) ? shift_QSS_Inp( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [q]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << std::endl;
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
		assert( tQ == tX );
		assert( dt_min <= dt_max );
		if ( x_3_ != 0.0 ) {
			Real const x_3_inv( one / x_3_ );
			Time dt( std::cbrt( qTol * std::abs( x_3_inv ) ) );
			dt = std::min( std::max( dt_infinity( dt ), dt_min ), dt_max );
			assert( dt != infinity );
			tE = tQ + dt;
			if ( options::inflection && nonzero_and_signs_differ( x_2_, x_3_ ) ) { // Inflection point
				Time const tI( tQ - ( x_2_ * ( one_third * x_3_inv ) ) );
				if ( tQ < tI ) tE = std::min( tE, tI ); // Possible that dtI > 0 but tQ + dtI == tQ
			}
		} else {
			Time const dt( std::min( std::max( dt_infinity_of_infinity(), dt_min ), dt_max ) );
			tE = ( dt != infinity ? tQ + dt : infinity );
		}
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }, x_3_{ 0.0 }; // Coefficients

}; // Variable_Inp3

} // QSS

#endif
