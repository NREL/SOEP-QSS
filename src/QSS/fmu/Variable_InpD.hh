// FMU-Based Discrete Input Variable
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

#ifndef QSS_fmu_Variable_InpD_hh_INCLUDED
#define QSS_fmu_Variable_InpD_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable_Inp.hh>

namespace QSS {
namespace fmu {

// FMU-Based Discrete Input Variable
class Variable_InpD final : public Variable_Inp
{

public: // Types

	using Super = Variable_Inp;

public: // Creation

	// Constructor
	Variable_InpD(
	 std::string const & name,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable(),
	 Function f = Function()
	) :
	 Super( 0, name, fmu_me, var, f )
	{}

public: // Predicate

	// Discrete Variable?
	bool
	is_Discrete() const
	{
		return true;
	}

public: // Property

	// Real Value
	Real
	r() const
	{
		return x_;
	}

	// Real Value at Time t
	Real
	r( Time const ) const
	{
		return x_;
	}

	// Continuous Value at Time t
	Real
	x( Time const ) const
	{
		return x_;
	}

	// Quantized Value at Time t
	Real
	q( Time const ) const
	{
		return x_;
	}

public: // Methods

	// Initialization
	void
	init()
	{
		init_0();
	}

	// Initialization: Stage 0
	void
	init_0()
	{
		assert( f() );
		assert( ! observes() );
		init_observers();
		s_ = f_( tQ );
		x_ = s_.x0;
		tD = s_.tD;
		add_discrete( tD );
		if ( options::output::d ) std::cout << "! " << name() << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << "   tD=" << tD << '\n';
	}

	// Discrete Advance
	void
	advance_discrete()
	{
		s_ = f_( tX = tQ = tD );
		Real const x_new( s_.x0 );
		tD = s_.tD;
		shift_discrete( tD );
		bool const chg( x_ != x_new );
		x_ = x_new;
		if ( options::output::d ) std::cout << "* " << name() << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << "   tD=" << tD << '\n';
		if ( chg && observed() ) advance_observers();
	}

	// Discrete Advance: Simultaneous
	void
	advance_discrete_s()
	{
		s_ = f_( tX = tQ = tD );
		Real const x_new( s_.x0 );
		tD = s_.tD;
		shift_discrete( tD );
		x_ = x_new;
		if ( options::output::d ) std::cout << "* " << name() << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << "   tD=" << tD << '\n';
	}

private: // Data

	Real x_{ 0.0 }; // Value

}; // Variable_InpD

} // fmu
} // QSS

#endif
