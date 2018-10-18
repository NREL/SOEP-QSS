// FMU-Based QSS0 Connection Variable
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

#ifndef QSS_fmu_Variable_Con0_hh_INCLUDED
#define QSS_fmu_Variable_Con0_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable_Con.hh>

namespace QSS {
namespace fmu {

// FMU-Based QSS0 Connection Variable
class Variable_Con0 final : public Variable_Con
{

public: // Types

	using Super = Variable_Con;

public: // Creation

	// Name Constructor
	Variable_Con0(
	 std::string const & name,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable()
	) :
	 Super( 0, name, fmu_me, var )
	{}

	// Name + Initial Value Constructor
	Variable_Con0(
	 std::string const & name,
	 Real const xIni,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable()
	) :
	 Super( 0, name, xIni, fmu_me, var ),
	 x_0_( xIni )
	{
		fmu_set_real( xIni );
	}

public: // Properties

	// Continuous Value at Time t
	Real
	x( Time const ) const
	{
		return x_0_ = fmu_get_real();
	}

	// Quantized Value at Time t
	Real
	q( Time const ) const
	{
		return x_0_ = fmu_get_real();
	}

	// Simultaneous Value at Time t
	Real
	s( Time const ) const
	{
		return x_0_ = fmu_get_real();
	}

	// Simultaneous Numeric Differentiation Value at Time t
	Real
	sn( Time const ) const
	{
		return x_0_ = fmu_get_real();
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
		assert( observees_.empty() );
		init_observers();
		x_0_ = fmu_get_real();
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << std::noshowpos << '\n';
	}

	// Discrete Advance
	void
	advance_discrete()
	{
		x_0_ = fmu_get_real();
		tX = tQ = tD;
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << std::noshowpos << '\n';
		if ( have_observers_ ) advance_observers();
	}

	// Discrete Advance: Stage 0
	void
	advance_discrete_0()
	{
		x_0_ = fmu_get_real();
		tX = tQ = tD;
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << std::noshowpos << '\n';
	}

	// QSS Advance
	void
	advance_QSS()
	{
		x_0_ = fmu_get_real();
		tX = tQ = tE;
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << std::noshowpos << '\n';
		if ( have_observers_ ) advance_observers();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0()
	{
		x_0_ = fmu_get_real();
		tX = tQ = tE;
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << std::noshowpos << '\n';
	}

private: // Data

	mutable Real x_0_{ 0.0 }; // Trajecotry coefficients

};

} // fmu
} // QSS

#endif
