// QSS1 Input Variable
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2025 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_Variable_Inp1_hh_INCLUDED
#define QSS_Variable_Inp1_hh_INCLUDED

// QSS Headers
#include <QSS/Variable_Inp.hh>

namespace QSS {

// QSS1 Input Variable
class Variable_Inp1 final : public Variable_Inp
{

public: // Types

	using Super = Variable_Inp;

public: // Creation

	// Constructor
	Variable_Inp1(
	 FMU_ME * fmu_me,
	 std::string const & name,
	 Real const rTol_ = options::rTol,
	 Real const aTol_ = options::aTol,
	 Real const xIni_ = 0.0,
	 FMU_Variable const & var = FMU_Variable(),
	 Function f = Function()
	) :
	 Super( fmu_me, 1, name, rTol_, aTol_, xIni_, var, f )
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
		return x_0_;
	}

public: // Methods

	// Initialization
	void
	init() override
	{
		init_0();
		init_observers();
		init_1();
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
		tD = s_.tD;
	}

	// Initialization: Stage Final
	void
	init_F() override
	{
		set_qTol();
		set_tE();
		( tE < tD ) ? add_QSS_Inp( tE ) : add_discrete( tD );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << " [q]   = " << x_0_ << x_1_ << x_delta << " [x]" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << std::endl;
	}

	// Discrete Advance
	void
	advance_discrete() override
	{
		tS = tD - tQ;
		s_ = f_( tQ = tX = tD );
		x_0_ = s_.x0;
		x_1_ = s_.x1;
		tD = s_.tD;
		set_qTol();
		set_tE();
		( tE < tD ) ? shift_QSS_Inp( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "|  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << " [q]   = " << x_0_ << x_1_ << x_delta << " [x]" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << std::endl;
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
		tD = s_.tD;
		set_qTol();
		set_tE();
		( tE < tD ) ? shift_QSS_Inp( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "|= " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << " [q]   = " << x_0_ << x_1_ << x_delta << " [x]" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << std::endl;
	}

	// QSS Advance
	void
	advance_QSS() override
	{
		s_ = f_( tQ = tX = tE );
		x_0_ = s_.x0;
		x_1_ = s_.x1;
		tD = s_.tD;
		set_qTol();
		set_tE();
		( tE < tD ) ? shift_QSS_Inp( tE ) : shift_discrete( tD );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << " [q]   = " << x_0_ << x_1_ << x_delta << " [x]" << std::noshowpos << "   tE=" << tE << "   tD=" << tD << std::endl;
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
		clip();
		Time dt( x_1_ != 0.0 ? qTol / std::abs( x_1_ ) : infinity );
		dt = std::min( std::max( dt_infinity( dt ), dt_min ), dt_max );
		tE = dt != infinity ? tQ + dt : infinity;
	}

	// Clip Small Trajectory Coefficients
	void
	clip()
	{
		if ( options::clipping ) {
			if ( std::abs( x_0_ ) <= options::clip ) x_0_ = 0.0;
			if ( std::abs( x_1_ ) <= options::clip ) x_1_ = 0.0;
		}
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }; // Coefficients

}; // Variable_Inp1

} // QSS

#endif
