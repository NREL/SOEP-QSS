// Constant Input Function
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2024 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_Function_Inp_constant_hh_INCLUDED
#define QSS_Function_Inp_constant_hh_INCLUDED

// QSS Headers
#include <QSS/SmoothToken.hh>

namespace QSS {

// Constant Input Function
class Function_Inp_constant final
{

public: // Types

	using Real = double;
	using Time = double;

public: // Creation

	// Constructor
	explicit
	Function_Inp_constant( Real const x_0 = 0.0 ) :
	 x_0_( x_0 )
	{}

public: // Property

	// State at Time t
	SmoothToken
	operator ()( Time const ) const
	{
		return SmoothToken( x_0_ );
	}

	// Value at Time t
	Real
	v( Time const ) const
	{
		return x_0_;
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
	tD( Time const ) const
	{
		return infinity;
	}

private: // Data

	Real const x_0_{ 0.0 }; // Constant value

}; // Function_Inp_constant

} // QSS

#endif
