// QSS Integer Variable
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2022 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_Variable_I_hh_INCLUDED
#define QSS_Variable_I_hh_INCLUDED

// QSS Headers
#include <QSS/Variable.hh>

namespace QSS {

// QSS Integer Variable
class Variable_I final : public Variable
{

public: // Types

	using Super = Variable;

public: // Creation

	// Name + Value Constructor
	Variable_I(
	 FMU_ME * fmu_me,
	 std::string const & name,
	 Integer const xIni_ = 0,
	 FMU_Variable const var = FMU_Variable()
	) :
	 Super( fmu_me, 0, name, xIni_, var ),
	 x_( xIni_ )
	{}

public: // Predicate

	// Integer Variable?
	bool
	is_Integer() const override
	{
		return true;
	}

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

	// Integer Value
	Integer
	i() const override
	{
		return x_;
	}

	// Integer Value at Time t
	Integer
	i( Time const ) const override
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
		init_observees();
		x_ = Integer( xIni );
		assert( fmu_get_integer() == x_ );
	}

	// Initialization: Stage Final
	void
	init_F() override
	{
		add_handler();
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << std::endl;
	}

	// Handler Advance
	void
	advance_handler( Time const t ) override
	{
		assert( tX <= t );
		tQ = tX = t;
		d_ = i_f();
		shift_handler();
		if ( x_ != d_ ) {
			x_ = d_;
			if ( options::output::d ) std::cout << "*  " << name() << '(' << tX << ')' << " = " << std::showpos << x_ << std::noshowpos << std::endl;
			if ( observed() ) advance_observers();
			if ( connected() ) advance_connections();
		}
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t, Real const x_0 ) override
	{
		assert( tX <= t );
		d_ = Integer( x_0 );
	}

	// Handler Advance: Stage Final
	void
	advance_handler_F( Time const t ) override
	{
		tQ = tX = t;
		shift_handler();
		if ( x_ != d_ ) {
			x_ = d_;
			if ( options::output::d ) std::cout << "*= " << name() << '(' << tX << ')' << " = " << std::showpos << x_ << std::noshowpos << std::endl;
			if ( connected() ) advance_connections();
		}
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
		tQ = tX = t;
		d_ = i_0();
		if ( x_ != d_ ) {
			x_ = d_;
			if ( connected() ) advance_connections_observer();
		}
	}

	// Observer Advance: Stage 1
	void
	advance_observer_1( Time const t ) override
	{
		assert( tX <= t );
		d_ = i_0( t );
	}

	// Observer Advance: Stage Final
	void
	advance_observer_F( Time const t ) override
	{
		tQ = tX = t;
		if ( x_ != d_ ) {
			x_ = d_;
			if ( connected() ) advance_connections_observer();
		}
	}

	// Observer Advance: Stage d
	void
	advance_observer_d() const override
	{
		std::cout << " ^ " << name() << '(' << tX << ')' << " = " << std::showpos << x_ << std::noshowpos << std::endl;
	}

private: // Data

	Integer x_{ 0 }; // Value
	Integer d_{ 0 }; // Deferred value

}; // Variable_I

} // QSS

#endif