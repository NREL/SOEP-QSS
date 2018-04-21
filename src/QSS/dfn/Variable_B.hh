// QSS Boolean Variable
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

#ifndef QSS_dfn_Variable_B_hh_INCLUDED
#define QSS_dfn_Variable_B_hh_INCLUDED

// QSS Headers
#include <QSS/dfn/Variable.hh>

namespace QSS {
namespace dfn {

// QSS Boolean Variable
class Variable_B final : public Variable
{

public: // Types

	using Super = Variable;
	using Integer = std::int64_t;

public: // Creation

	// Constructor
	explicit
	Variable_B(
	 std::string const & name,
	 bool const xIni = false
	) :
	 Super( name, xIni ),
	 x_( xIni )
	{}

public: // Predicate

	// Discrete Variable?
	bool
	is_Discrete() const
	{
		return true;
	}

public: // Properties

	// Order of Method
	int
	order() const
	{
		return 0;
	}

	// Boolean Value
	bool
	b() const
	{
		return x_;
	}

	// Boolean Value at Time t
	bool
	b( Time const ) const
	{
		return x_;
	}

	// Integer Value
	Integer
	i() const
	{
		return x_;
	}

	// Continuous Integer Value at Time t
	Integer
	i( Time const ) const
	{
		return x_;
	}

	// Value
	Value
	x() const
	{
		return x_;
	}

	// Continuous Value at Time t
	Value
	x( Time const ) const
	{
		return x_;
	}

	// Continuous First Derivative at Time t
	Value
	x1( Time const ) const
	{
		return 0.0;
	}

	// Quantized Value
	Value
	q() const
	{
		return x_;
	}

	// Quantized Value at Time t
	Value
	q( Time const ) const
	{
		return x_;
	}

	// Simultaneous Value at Time t
	Value
	s( Time const ) const
	{
		return x_;
	}

	// Simultaneous Numeric Differentiation Value at Time t
	Value
	sn( Time const ) const
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

	// Initialization to a Value
	void
	init( Value const x )
	{
		init_0( x );
	}

	// Initialization: Stage 0
	void
	init_0()
	{
		assert( Super::observees_.empty() );
		shrink_observers();
		x_ = ( xIni != 0 );
		add_handler();
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << std::showpos << " = " << x_ << std::noshowpos << '\n';
	}

	// Initialization to a Value: Stage 0
	void
	init_0( Value const x )
	{
		assert( Super::observees_.empty() );
		shrink_observers();
		x_ = ( x != 0 );
		add_handler();
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << std::showpos << " = " << x_ << std::noshowpos << '\n';
	}

	// Handler Advance
	void
	advance_handler( Time const t, Value const x )
	{
		assert( tX <= t );
		tX = tQ = t;
		bool const x_new( x != 0.0 );
		if ( x_ != x_new ) {
			x_ = x_new;
			advance_observers();
			if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << std::showpos << " = " << x_ << std::noshowpos << '\n';
		} else {
			if ( options::output::d ) std::cout << "# " << name << '(' << tQ << ')' << std::showpos << " = " << x_ << std::noshowpos << '\n';
		}
		shift_handler();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t, Value const x )
	{
		assert( tX <= t );
		tX = tQ = t;
		x_ = ( x != 0.0 );
		shift_handler();
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << std::showpos << " = " << x_ << std::noshowpos << '\n';
	}

private: // Data

	bool x_; // Value

};

} // dfn
} // QSS

#endif
