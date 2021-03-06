// QSS Variable Abstract Base Class
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

#ifndef QSS_cod_Variable_QSS_hh_INCLUDED
#define QSS_cod_Variable_QSS_hh_INCLUDED

// QSS Headers
#include <QSS/cod/Variable.hh>

namespace QSS {
namespace cod {

// QSS Variable Abstract Base Class
template< template< typename > class F >
class Variable_QSS : public Variable
{

public: // Types

	using Super = Variable;

	using Real = Variable::Real;
	using Time = Variable::Time;

	template< typename V > using Function = F< V >;
	using Derivative = Function< Variable >;

protected: // Creation

	// Copy Constructor
	Variable_QSS( Variable_QSS const & ) = default;

	// Move Constructor
	Variable_QSS( Variable_QSS && ) noexcept = default;

	// Constructor
	Variable_QSS(
	 int const order,
	 std::string const & name,
	 Real const rTol = 1.0e-4,
	 Real const aTol = 1.0e-6,
	 Real const zTol = 1.0e-6,
	 Real const xIni = 0.0
	) :
	 Super( order, name, rTol, aTol, xIni ),
	 zTol( std::max( zTol, 0.0 ) )
	{}

protected: // Assignment

	// Copy Assignment
	Variable_QSS &
	operator =( Variable_QSS const & ) = default;

	// Move Assignment
	Variable_QSS &
	operator =( Variable_QSS && ) noexcept = default;

public: // Predicate

	// QSS Variable?
	bool
	is_QSS() const override
	{
		return true;
	}

public: // Property

	// Derivative Function
	Derivative const &
	d() const
	{
		return d_;
	}

	// Derivative Function
	Derivative &
	d()
	{
		return d_;
	}

public: // Function Methods

	// Add Constant
	Variable_QSS &
	add( Coefficient const c0 )
	{
		d_.add( c0 );
		return *this;
	}

	// Add a Variable
	Variable_QSS &
	add( Variable * v )
	{
		d_.add( v, this );
		observe( v );
		return *this;
	}

	// Add a Coefficient * Variable
	Variable_QSS &
	add(
	 Coefficient const c,
	 Variable * v
	)
	{
		d_.add( c, v, this );
		observe( v );
		return *this;
	}

public: // Data

	Real zTol{ 0.0 }; // Zero-crossing tolerance

protected: // Data

	Derivative d_; // Derivative function

}; // Variable_QSS

} // cod
} // QSS

#endif
