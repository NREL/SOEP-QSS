// Sine Input Function
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

#ifndef QSS_cod_mdl_Function_Inp_sin_hh_INCLUDED
#define QSS_cod_mdl_Function_Inp_sin_hh_INCLUDED

// QSS Headers
#include <QSS/math.hh>

namespace QSS {
namespace cod {
namespace mdl {

// Sine Function
class Function_Inp_sin final
{

public: // Types

	using Time = double;
	using Real = double;
	using Coefficient = double;

public: // Creation

	// Constructor
	explicit
	Function_Inp_sin(
	 Coefficient const c = 1.0,
	 Coefficient const s = 1.0
	) :
	 c_( c ),
	 s_( s )
	{}

public: // Property

	// Value Scaling
	Coefficient
	c() const
	{
		return c_;
	}

	// Time Scaling
	Coefficient
	s() const
	{
		return s_;
	}

	// Value at Time t
	Real
	operator ()( Time const t ) const
	{
		return c_ * std::sin( s_ * t );
	}

	// Value at Time t
	Real
	v( Time const t ) const
	{
		return c_ * std::sin( s_ * t );
	}

	// First Derivative at Time t
	Real
	d1( Time const t ) const
	{
		return c_ * s_ * std::cos( s_ * t );
	}

	// Second Derivative at Time t
	Real
	d2( Time const t ) const
	{
		return -c_ * ( s_ * s_ ) * std::sin( s_ * t );
	}

	// Third Derivative at Time t
	Real
	d3( Time const t ) const
	{
		return -c_ * ( s_ * s_ * s_ ) * std::cos( s_ * t );
	}

	// Sequential Value at Time t
	Real
	vs( Time const t ) const
	{
		return v( t );
	}

	// Forward-Difference Sequential First Derivative at Time t
	Real
	df1( Time const t ) const
	{
		return d1( t );
	}

	// Centered-Difference Sequential First Derivative at Time t
	Real
	dc1( Time const t ) const
	{
		return d1( t );
	}

	// Centered-Difference Sequential Second Derivative at Time t
	Real
	dc2( Time const t ) const
	{
		return d2( t );
	}

	// Centered-Difference Sequential Third Derivative at Time t
	Real
	dc3( Time const t ) const
	{
		return d3( t );
	}

	// Discrete Event after Time t
	Time
	tD( Time const ) const
	{
		return infinity;
	}

public: // Methods

	// Set Value Scaling
	Function_Inp_sin &
	c( Coefficient const c )
	{
		c_ = c;
		return *this;
	}

	// Set Time Scaling
	Function_Inp_sin &
	s( Coefficient const s )
	{
		s_ = s;
		return *this;
	}

private: // Data

	Coefficient c_{ 1.0 }; // Value scaling
	Coefficient s_{ 1.0 }; // Time scaling

};

} // mdl
} // cod
} // QSS

#endif
