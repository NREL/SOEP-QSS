// FMU-Based QSS Discrete Variable
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

#ifndef QSS_fmu_Variable_D_hh_INCLUDED
#define QSS_fmu_Variable_D_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable.hh>

namespace QSS {
namespace fmu {

// FMU-Based QSS Discrete Variable
class Variable_D final : public Variable
{

public: // Types

	using Super = Variable;

public: // Creation

	// Name + Value Constructor
	Variable_D(
	 std::string const & name,
	 Real const xIni,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable()
	) :
	 Super( 0, name, xIni, fmu_me, var ),
	 x_( xIni )
	{}

	// Name Constructor
	Variable_D(
	 std::string const & name,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable()
	) :
	 Super( 0, name, fmu_me, var ),
	 x_( xIni )
	{}

public: // Predicate

	// Discrete Variable?
	bool
	is_Discrete() const override
	{
		return true;
	}

	// B|I|D|R Variable?
	bool
	is_BIDR() const override
	{
		return true;
	}

public: // Property

	// Boolean Value
	Boolean
	b() const override
	{
		return Boolean( x_ );
	}

	// Boolean Value at Time t
	Boolean
	b( Time const ) const override
	{
		return Boolean( x_ );
	}

	// Integer Value
	Integer
	i() const override
	{
		return Integer( x_ );
	}

	// Integer Value at Time t
	Integer
	i( Time const ) const override
	{
		return Integer( x_ );
	}

	// Real Value
	Real
	r() const override
	{
		return x_;
	}

	// Real Value at Time t
	Real
	r( Time const ) const override
	{
		return x_;
	}

	// Continuous Value at Time t
	Real
	x( Time const ) const override
	{
		return x_;
	}

	// Quantized Value at Time t
	Real
	q( Time const ) const override
	{
		return x_;
	}

public: // Methods

	// Initialization
	void
	init() override
	{
		init_0();
	}

	// Initialization to a Value
	void
	init( Real const x ) override
	{
		init_0( x );
	}

	// Initialization: Stage 0
	void
	init_0() override
	{
		assert( ! observes() );
		init_observers();
		x_ = xIni;
		add_handler();
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << std::endl;
	}

	// Initialization to a Value: Stage 0
	void
	init_0( Real const x ) override
	{
		assert( ! observes() );
		init_observers();
		x_ = x;
		add_handler();
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << std::endl;
	}

	// Handler Advance
	void
	advance_handler( Time const t ) override
	{
		assert( tX <= t );
		tX = tQ = t;
		Real const x_old( x_ );
		x_ = fmu_get_real(); // Assume FMU ran event handler
		shift_handler();
		if ( options::output::d ) std::cout << "*  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << std::endl;
		if ( observed() && ( x_ != x_old ) ) advance_observers();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t ) override
	{
		assert( tX <= t );
		tX = tQ = t;
		x_ = fmu_get_real(); // Assume FMU ran event handler
	}

	// Handler Advance: Stage Final
	void
	advance_handler_F() override
	{
		shift_handler();
		if ( options::output::d ) std::cout << "*= " << name() << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << std::endl;
	}

	// Handler No-Advance
	void
	no_advance_handler() override
	{
		shift_handler();
	}

	// Observer Advance
	void
	advance_observer( Time const t ) override
	{
		assert( tX <= t );
		tX = t;
		x_ = z_0( t );
	}

	// Observer Advance: Stage 1
	void
	advance_observer_1( Time const t, Real const x ) override
	{
		assert( tX <= t );
		tX = t;
		x_ = x;
	}

	// Observer Advance: Stage d
	void
	advance_observer_d() const override
	{
		std::cout << " ^ " << name() << '(' << tX << ')' << " = " << std::showpos << x_ << std::noshowpos << std::endl;
	}

private: // Data

	Real x_{ 0.0 }; // Value

}; // Variable_D

} // fmu
} // QSS

#endif
