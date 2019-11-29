// QSS Discrete Variable
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

#ifndef QSS_cod_Variable_D_hh_INCLUDED
#define QSS_cod_Variable_D_hh_INCLUDED

// QSS Headers
#include <QSS/cod/Variable.hh>

namespace QSS {
namespace cod {

// QSS Discrete Variable
class Variable_D final : public Variable
{

public: // Types

	using Super = Variable;

public: // Creation

	// Constructor
	explicit
	Variable_D(
	 std::string const & name,
	 Real const xIni = 0.0
	) :
	 Super( 0, name, xIni ),
	 x_( xIni )
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
		return Boolean( x_ );
	}

	// Boolean Value at Time t
	Boolean
	b( Time const ) const
	{
		return Boolean( x_ );
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
		assert( observees_.empty() );
		init_observers();
		x_ = xIni;
		add_handler();
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << '\n';
	}

	// Initialization to a Value: Stage 0
	void
	init_0( Real const x )
	{
		assert( observees_.empty() );
		init_observers();
		x_ = x;
		add_handler();
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << '\n';
	}

	// Handler Advance
	void
	advance_handler( Time const t, Real const x )
	{
		assert( tX <= t );
		tX = tQ = t;
		shift_handler();
		bool const chg( x_ != x );
		if ( chg ) x_ = x;
		if ( options::output::d ) std::cout << ( chg ? '*' : '#' ) << ' ' << name << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << '\n';
		if ( chg && have_observers_ ) advance_observers();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t, Real const x )
	{
		assert( tX <= t );
		tX = tQ = t;
		shift_handler();
		bool const chg( x_ != x );
		if ( chg ) x_ = x;
		if ( options::output::d ) std::cout << ( chg ? '*' : '#' ) << ' ' << name << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << '\n';
	}

private: // Data

	Real x_{ 0.0 }; // Value

};

} // cod
} // QSS

#endif
