// Integer Input Variable
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

#ifndef QSS_cod_Variable_InpI_hh_INCLUDED
#define QSS_cod_Variable_InpI_hh_INCLUDED

// QSS Headers
#include <QSS/cod/Variable_Inp.hh>

namespace QSS {
namespace cod {

// Integer Input Variable
template< class F >
class Variable_InpI final : public Variable_Inp< F >
{

public: // Types

	using Super = Variable_Inp< F >;

	using Boolean = Variable::Boolean;
	using Integer = Variable::Integer;
	using Real = Variable::Real;
	using Time = Variable::Time;

	using Super::name;
	using Super::tQ;
	using Super::tX;
	using Super::tD;
	using Super::observed;
	using Super::observes;

	using Super::add_discrete;
	using Super::advance_observers;
	using Super::shift_discrete;
	using Super::init_observers;

private: // Types

	using Super::f_;

public: // Creation

	// Constructor
	explicit
	Variable_InpI( std::string const & name ) :
	 Super( 0, name )
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

	// Value
	Integer
	i() const
	{
		return x_;
	}

	// Integer Value at Time t
	Integer
	i( Time const ) const
	{
		return x_;
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

	// Initialization: Stage 0
	void
	init_0()
	{
		assert( ! observes() );
		init_observers();
		x_ = static_cast< Integer >( f_.vs( tQ ) );
		tD = f_.tD( tQ );
		add_discrete( tD );
		if ( options::output::d ) std::cout << "! " << name() << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << "   tD=" << tD << '\n';
	}

	// Discrete Advance
	void
	advance_discrete()
	{
		Integer const x_new( static_cast< Integer >( f_.vs( tX = tQ = tD ) ) );
		tD = f_.tD( tQ );
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
		Integer const x_new( static_cast< Integer >( f_.vs( tX = tQ = tD ) ) );
		tD = f_.tD( tQ );
		shift_discrete( tD );
		x_ = x_new;
		if ( options::output::d ) std::cout << "* " << name() << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << "   tD=" << tD << '\n';
	}

private: // Data

	Integer x_{ 0 }; // Value

}; // Variable_InpI

} // cod
} // QSS

#endif
