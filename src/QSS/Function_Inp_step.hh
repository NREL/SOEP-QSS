// Step Input Function
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2025 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_Function_Inp_step_hh_INCLUDED
#define QSS_Function_Inp_step_hh_INCLUDED

// QSS Headers
#include <QSS/SmoothToken.hh>

// C++ Headers
#include <cassert>

namespace QSS {

// Step Input Function
class Function_Inp_step final
{

public: // Types

	using Real = double;
	using Time = double;

public: // Creation

	// Constructor
	explicit
	Function_Inp_step(
	 Real const h_0 = 0.0,
	 Real const h = 1.0,
	 Real const d = 1.0
	) :
	 h_0_( h_0 ),
	 h_( h ),
	 d_( d )
	{
		assert( d_ > 0.0 );
	}

public: // Property

	// State at Time t
	SmoothToken
	operator ()( Time const t ) const
	{
		return SmoothToken::order_0( v( t ), tD( t ) );
	}

	// Value at Time t
	Real
	v( Time const t ) const
	{
		return h_0_ + ( h_ * step_number( t ) );
	}

	// First Derivative at Time t
	Real
	d1( Time const ) const
	{
		return 0.0;
	}

	// Second Derivative at Time t
	Real
	d2( Time const ) const
	{
		return 0.0;
	}

	// Third Derivative at Time t
	Real
	d3( Time const ) const
	{
		return 0.0;
	}

	// Discrete Event after Time t
	Time
	tD( Time const t ) const
	{
		Real const n_next( std::floor( t / d_ ) + 1.0 );
		Real const t_next( d_ * n_next );
		return t_next > t ? t_next : d_ * ( n_next + 1.0 );
	}

private: // Methods

	// Step Number at Time t
	Real
	step_number( Time const t ) const
	{
		Real const ftd( std::floor( t / d_ ) );
		return d_ * ( ftd + 1.0 ) > t ? ftd : ftd + 1.0;
	}

private: // Data

	Real const h_0_{ 0.0 }; // Initial height
	Real const h_{ 1.0 }; // Step height
	Real const d_{ 1.0 }; // Step time delta

}; // Function_Inp_step

} // QSS

#endif
