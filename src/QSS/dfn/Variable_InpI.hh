// Integer Input Variable
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

#ifndef QSS_dfn_Variable_InpI_hh_INCLUDED
#define QSS_dfn_Variable_InpI_hh_INCLUDED

// QSS Headers
#include <QSS/dfn/Variable_Inp.hh>

namespace QSS {
namespace dfn {

// Integer Input Variable
template< class F >
class Variable_InpI final : public Variable_Inp< F >
{

public: // Types

	using Super = Variable_Inp< F >;
	using Time = Variable::Time;
	using Value = Variable::Value;
	using Integer = std::int64_t;

	using Super::name;
	using Super::tQ;
	using Super::tX;
	using Super::tD;

	using Super::add_discrete;
	using Super::advance_observers;
	using Super::event;
	using Super::shift_discrete;
	using Super::shrink_observers;

private: // Types

	using Super::f_;

public: // Creation

	// Constructor
	explicit
	Variable_InpI( std::string const & name ) :
	 Super( name )
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

	// Value
	Integer
	i() const
	{
		return x_;
	}

	// Continuous Value at Time t
	Integer
	i( Time const ) const
	{
		return x_;
	}

	// Value
	Value
	x() const
	{
		return Value( x_ );
	}

	// Continuous Value at Time t
	Value
	x( Time const ) const
	{
		return Value( x_ );
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
		return Value( x_ );
	}

	// Quantized Value at Time t
	Value
	q( Time const ) const
	{
		return Value( x_ );
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
		return Value( x_ );
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
		assert( Super::observees_.empty() );
		shrink_observers();
		x_ = static_cast< Integer >( f_.vs( tQ ) );
		tD = f_.tD( tQ );
		add_discrete( tD );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << "   tD=" << tD << '\n';
	}

	// Discrete Advance
	void
	advance_discrete()
	{
		Integer const x_new( static_cast< Integer >( f_.vs( tX = tQ = tD ) ) );
		tD = f_.tD( tQ );
		shift_discrete( tD );
		bool const chg( x_ != x_new );
		if ( chg ) x_ = x_new;
		if ( options::output::d ) std::cout << ( chg ? '*' : '#' ) << ' ' << name << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << "   tD=" << tD << '\n';
		if ( chg ) advance_observers();
	}

	// Discrete Advance: Stages 0 and 1
	void
	advance_discrete_0_1()
	{
		Integer const x_new( static_cast< Integer >( f_.vs( tX = tQ = tD ) ) );
		tD = f_.tD( tD );
		shift_discrete( tD );
		bool const chg( x_ != x_new );
		if ( chg ) x_ = x_new;
		if ( options::output::d ) std::cout << ( chg ? '*' : '#' ) << ' ' << name << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << "   tD=" << tD << '\n';
	}

private: // Data

	Integer x_; // Value

};

} // dfn
} // QSS

#endif
