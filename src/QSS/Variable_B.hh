// QSS Boolean Variable
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

#ifndef QSS_Variable_B_hh_INCLUDED
#define QSS_Variable_B_hh_INCLUDED

// QSS Headers
#include <QSS/Variable.hh>

namespace QSS {

// QSS Boolean Variable
class Variable_B final : public Variable
{

public: // Types

	using Super = Variable;

public: // Creation

	// Name + Value Constructor
	Variable_B(
	 FMU_ME * fmu_me,
	 std::string const & name,
	 Start_Boolean const & xIni_ = Start_Boolean(),
	 FMU_Variable const var = FMU_Variable()
	) :
	 Super( fmu_me, 0, name, xIni_, var ),
	 x_( xIni_ )
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
		return x_;
	}

	// Boolean Value at Time t
	Boolean
	b( Time const ) const override
	{
		return x_;
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
		return Real( x_ );
	}

	// Real Value at Time t
	Real
	r( Time const ) const override
	{
		return Real( x_ );
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
		init_F();
	}

	// Initialization to a Value
	void
	init( Real const x ) override
	{
		init_0( x );
		init_F();
	}

	// Initialization: Stage 0
	void
	init_0() override
	{
		init_observers();
		init_observees();
		fmu_set_boolean( x_ = Boolean( xIni ) );
	}

	// Initialization to a Value: Stage 0
	void
	init_0( Real const x ) override
	{
		init_observers();
		init_observees();
		fmu_set_boolean( x_ = ( x != 0.0 ) );
	}

	// Initialization: Stage Final
	void
	init_F() override
	{
		add_handler();
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << x_ << std::endl;
	}

	// Handler Advance
	void
	advance_handler( Time const t ) override
	{
		assert( tX <= t );
		tX = tQ = t;
		x_new_ = b_0();
		shift_handler();
		if ( x_ != x_new_ ) {
			x_ = x_new_;
			if ( options::output::d ) std::cout << "*  " << name() << '(' << tX << ')' << " = " << x_ << std::endl;
			if ( observed() ) advance_observers();
			if ( connected() ) advance_connections();
		}
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t ) override
	{
		assert( tX <= t );
		tX = tQ = t;
		x_new_ = b_0();
	}

	// Handler Advance: Stage Final
	void
	advance_handler_F() override
	{
		shift_handler();
		if ( x_ != x_new_ ) {
			x_ = x_new_;
			if ( options::output::d ) std::cout << "*= " << name() << '(' << tX << ')' << " = " << x_ << std::endl;
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
		tX = tQ = t;
		x_new_ = b_0();
		if ( x_ != x_new_ ) {
			x_ = x_new_;
			if ( observed() ) advance_observer_observers();
			if ( connected() ) advance_connections_observer();
		}
	}

	// Observer Non-State Advance
	void
	advance_observer_ns( Time const t ) override
	{
		assert( tX <= t );
		tX = tQ = t;
		x_new_ = b_0();
		if ( x_ != x_new_ ) {
			x_ = x_new_;
			if ( observed_ns() ) advance_observer_ns_observers();
			if ( connected() ) advance_connections_observer();
		}
	}

	// Observer Advance: Stage d
	void
	advance_observer_d() const override
	{
		std::cout << " ^ " << name() << '(' << tX << ')' << " = " << x_ << std::endl;
	}

public: // Methods: FMU

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
	Boolean x_new_{ false }; // New value

}; // Variable_B

} // QSS

#endif
