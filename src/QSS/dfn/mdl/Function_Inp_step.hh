// Step Input Function
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017 Objexx Engineerinc, Inc. All rights reserved.
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

#ifndef QSS_dfn_mdl_Function_Inp_step_hh_INCLUDED
#define QSS_dfn_mdl_Function_Inp_step_hh_INCLUDED

// QSS Headers
#include <QSS/math.hh>

// C++ Headers
#include <cassert>
#include <cmath>

namespace QSS {
namespace dfn {
namespace mdl {

// Step Input Function
class Function_Inp_step final
{

public: // Types

	using Time = double;
	using Value = double;
	using Coefficient = double;

public: // Creation

	// Constructor
	explicit
	Function_Inp_step(
	 Coefficient const h_0 = 0.0,
	 Coefficient const h = 1.0,
	 Coefficient const d = 1.0
	) :
	 h_0_( h_0 ),
	 h_( h ),
	 d_( d )
	{
		assert( d_ > 0.0 );
	}

public: // Properties

	// Initial Height
	Coefficient
	h_0() const
	{
		return h_0_;
	}

	// Height
	Coefficient
	h() const
	{
		return h_;
	}

	// Step Time Delta
	Coefficient
	d() const
	{
		return d_;
	}

	// Value at Time t
	Value
	operator ()( Time const t ) const
	{
		return h_0_ + ( h_ * step_number( t ) );
	}

	// Value at Time t
	Value
	v( Time const t ) const
	{
		return h_0_ + ( h_ * step_number( t ) );
	}

	// First Derivative at Time t
	Value
	d1( Time const ) const
	{
		return 0.0;
	}

	// Second Derivative at Time t
	Value
	d2( Time const ) const
	{
		return 0.0;
	}

	// Third Derivative at Time t
	Value
	d3( Time const ) const
	{
		return 0.0;
	}

	// Sequential Value at Time t
	Value
	vs( Time const t ) const
	{
		return h_0_ + ( h_ * step_number( t ) );
	}

	// Forward-Difference Sequential First Derivative at Time t
	Value
	df1( Time const ) const
	{
		return 0.0;
	}

	// Centered-Difference Sequential First Derivative at Time t
	Value
	dc1( Time const ) const
	{
		return 0.0;
	}

	// Centered-Difference Sequential Second Derivative at Time t
	Value
	dc2( Time const ) const
	{
		return 0.0;
	}

	// Centered-Difference Sequential Third Derivative at Time t
	Value
	dc3( Time const ) const
	{
		return 0.0;
	}

	// Discrete Event after Time t
	Time
	tD( Time const t ) const
	{
		Coefficient const n_next( std::floor( t / d_ ) + 1.0 );
		Coefficient const t_next( d_ * n_next );
		return ( t_next > t ? t_next : d_ * ( n_next + 1.0 ) );
	}

public: // Methods

	// Set Initial Height
	Function_Inp_step &
	h_0( Coefficient const & h_0 )
	{
		h_0_ = h_0;
		return *this;
	}

	// Set Height
	Function_Inp_step &
	h( Coefficient const & h )
	{
		h_ = h;
		return *this;
	}

	// Set Step Time Delta
	Function_Inp_step &
	d( Coefficient const & d )
	{
		d_ = d;
		assert( d_ > 0.0 );
		return *this;
	}

private: // Methods

	// Step Number at Time t
	Value
	step_number( Time const t ) const
	{
		Value const ftd( std::floor( t / d_ ) );
		return ( d_ * ( ftd + 1.0 ) > t ? ftd : ftd + 1.0 );
	}

private: // Data

	Coefficient h_0_{ 0.0 }; // Initial height
	Coefficient h_{ 1.0 }; // Step height
	Coefficient d_{ 1.0 }; // Step time delta

};

} // mdl
} // dfn
} // QSS

#endif
