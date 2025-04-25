// Boolean Input Variable
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

#ifndef QSS_Variable_InpB_hh_INCLUDED
#define QSS_Variable_InpB_hh_INCLUDED

// QSS Headers
#include <QSS/Variable_Inp.hh>

namespace QSS {

// Boolean Input Variable
class Variable_InpB final : public Variable_Inp
{

public: // Types

	using Super = Variable_Inp;

public: // Creation

	// Constructor
	Variable_InpB(
	 FMU_ME * fmu_me,
	 std::string const & name,
	 Boolean const xIni_ = false,
	 FMU_Variable const & var = FMU_Variable(),
	 Function f = Function()
	) :
	 Super( fmu_me, 0, name, xIni_, var, f )
	{}

public: // Predicate

	// Boolean Variable?
	bool
	is_Boolean() const override
	{
		return true;
	}

	// Discrete Variable?
	bool
	is_Discrete() const override
	{
		return true;
	}

public: // Property

	// Boolean Value
	Boolean
	b() const override
	{
		return x_;
	}

	// Boolean Value at Time t
	Boolean
	b( Time const ) const override
	{
		return x_;
	}

	// Continuous Value at Time t
	Real
	x( Time const ) const override
	{
		return Real( x_ );
	}

	// Quantized Value at Time t
	Real
	q( Time const ) const override
	{
		return Real( x_ );
	}

public: // Methods

	// Initialization
	void
	init() override
	{
		init_0();
		init_observers();
		init_F();
	}

	// Initialization: Stage 0
	void
	init_0() override
	{
		assert( f() );
		assert( observees().empty() );
		s_ = f_( tQ );
		fmu_set_boolean( x_ = ( s_.x0 != 0.0 ) );
		tD = s_.tD;
	}

	// Initialization: Stage Final
	void
	init_F() override
	{
		add_discrete( tD );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << x_ << "   tD=" << tD << std::endl;
	}

	// Discrete Advance
	void
	advance_discrete() override
	{
		tS = tD - tQ;
		s_ = f_( tQ = tX = tD );
		Boolean const x_new( s_.x0 != 0.0 );
		tD = s_.tD;
		shift_discrete( tD );
		bool const chg( x_ != x_new );
		x_ = x_new;
		if ( options::output::d ) std::cout << "|  " << name() << '(' << tQ << ')' << " = " << x_ << "   tD=" << tD << std::endl;
		if ( chg && observed() ) advance_observers();
	}

	// Discrete Advance: Simultaneous
	void
	advance_discrete_simultaneous() override
	{
		tS = tD - tQ;
		s_ = f_( tQ = tX = tD );
		Boolean const x_new( s_.x0 != 0.0 );
		tD = s_.tD;
		shift_discrete( tD );
		x_ = x_new;
		if ( options::output::d ) std::cout << "|= " << name() << '(' << tQ << ')' << " = " << x_ << "   tD=" << tD << std::endl;
	}

	// Set FMU Variable to Continuous Value at Time t
	void
	fmu_set_x( Time const ) const override
	{
		fmu_set_boolean( x_ );
	}

	// Set FMU Variable to Quantized Value at Time t
	void
	fmu_set_q( Time const ) const override
	{
		fmu_set_boolean( x_ );
	}

	// Set FMU Variable to Appropriate Value at Time t
	void
	fmu_set_s( Time const ) const override
	{
		fmu_set_boolean( x_ );
	}

private: // Data

	Boolean x_{ false }; // Value

}; // Variable_InpB

} // QSS

#endif
