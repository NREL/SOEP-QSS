// FMU-Based QSS Boolean Variable
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2020 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_fmu_Variable_B_hh_INCLUDED
#define QSS_fmu_Variable_B_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable.hh>

namespace QSS {
namespace fmu {

// FMU-Based QSS Boolean Variable
class Variable_B final : public Variable
{

public: // Types

	using Super = Variable;

public: // Creation

	// Name + Value Constructor
	Variable_B(
	 std::string const & name,
	 Boolean const xIni,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable()
	) :
	 Super( 0, name, xIni, fmu_me, var ),
	 x_( xIni )
	{}

	// Name Constructor
	Variable_B(
	 std::string const & name,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable()
	) :
	 Super( 0, name, fmu_me, var ),
	 x_( xIni != 0 )
	{}

public: // Predicate

	// Discrete Variable?
	bool
	is_Discrete() const
	{
		return true;
	}

public: // Property

	// Boolean Value
	Boolean
	b() const
	{
		return x_;
	}

	// Boolean Value at Time t
	Boolean
	b( Time const ) const
	{
		return x_;
	}

	// Integer Value
	Integer
	i() const
	{
		return Integer( x_ );
	}

	// Integer Value at Time t
	Integer
	i( Time const ) const
	{
		return Integer( x_ );
	}

	// Real Value
	Real
	r() const
	{
		return Real( x_ );
	}

	// Real Value at Time t
	Real
	r( Time const ) const
	{
		return Real( x_ );
	}

	// Continuous Value at Time t
	Real
	x( Time const ) const
	{
		return Real( x_ );
	}

	// Quantized Value at Time t
	Real
	q( Time const ) const
	{
		return Real( x_ );
	}

public: // Methods

	// Initialization
	void
	init()
	{
		init_0();
	}

	// Initialization to a Value
	void
	init( Real const x )
	{
		init_0( x );
	}

	// Initialization: Stage 0
	void
	init_0()
	{
		assert( ! observes() );
		init_observers();
		x_ = ( xIni != 0 );
		add_handler();
		if ( options::output::d ) std::cout << "! " << name() << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << '\n';
	}

	// Initialization to a Value: Stage 0
	void
	init_0( Real const x )
	{
		assert( ! observes() );
		init_observers();
		x_ = ( x != 0 );
		add_handler();
		if ( options::output::d ) std::cout << "! " << name() << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << '\n';
	}

	// Handler Advance
	void
	advance_handler( Time const t )
	{
		assert( tX <= t );
		tX = tQ = t;
		Boolean const x_old( x_ );
		x_ = fmu_get_boolean(); // Assume FMU ran event handler
		shift_handler();
		if ( options::output::d ) std::cout << "* " << name() << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << '\n';
		if ( observed() && ( x_ != x_old ) ) advance_observers();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t )
	{
		assert( tX <= t );
		tX = tQ = t;
		x_ = fmu_get_boolean(); // Assume FMU ran event handler
	}

	// Handler Advance: Stage Final
	void
	advance_handler_F()
	{
		shift_handler();
		if ( options::output::d ) std::cout << "* " << name() << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << '\n';
	}

	// Handler No-Advance
	void
	no_advance_handler()
	{
		shift_handler();
	}

	// Set FMU Variable to Continuous Value at Time t
	void
	fmu_set_x( Time const ) const
	{
		fmu_set_boolean( x_ );
	}

	// Set FMU Variable to Quantized Value at Time t
	void
	fmu_set_q( Time const ) const
	{
		fmu_set_boolean( x_ );
	}

private: // Data

	Boolean x_{ false }; // Value

}; // Variable_B

} // fmu
} // QSS

#endif
