// Sine Input Function
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

#ifndef QSS_fmu_Function_Inp_sin_hh_INCLUDED
#define QSS_fmu_Function_Inp_sin_hh_INCLUDED

// QSS Headers
#include <QSS/SmoothToken.hh>
#include <QSS/math.hh>

namespace QSS {
namespace fmu {

// Sine Input Function
class Function_Inp_sin final
{

public: // Types

	using Real = double;
	using Time = double;

public: // Creation

	// Constructor
	explicit
	Function_Inp_sin(
	 Real const a = 1.0, // Amplitude
	 Real const b = 1.0, // Time scaling (2*pi/period)
	 Real const c = 0.0 // Shift
	) :
	 a_( a ),
	 b_( b ),
	 c_( c ),
	 a_b_( a * b ),
	 a_b2_( a * b * b ),
	 a_b3_( a * b * b * b )
	{}

public: // Property

	// State at Time t
	SmoothToken
	operator ()( Time const t ) const
	{
		Real const b_t( b_ * t );
		Real const sin_b_t( std::sin( b_t ) );
		Real const cos_b_t( std::cos( b_t ) );
		return SmoothToken(
		 ( a_ * sin_b_t ) + c_,
		 a_b_ * cos_b_t,
		 -a_b2_ * sin_b_t,
		 -a_b3_ * cos_b_t
		);
	}

	// Value at Time t
	Real
	v( Time const t ) const
	{
		return ( a_ * std::sin( b_ * t ) ) + c_;
	}

	// First Derivative at Time t
	Real
	d1( Time const t ) const
	{
		return a_b_ * std::cos( b_ * t );
	}

	// Second Derivative at Time t
	Real
	d2( Time const t ) const
	{
		return -a_b2_ * std::sin( b_ * t );
	}

	// Third Derivative at Time t
	Real
	d3( Time const t ) const
	{
		return -a_b3_ * std::cos( b_ * t );
	}

	// Discrete Event after Time t
	Time
	tD( Time const ) const
	{
		return infinity;
	}

private: // Data

	Real const a_{ 1.0 }; // Amplitude
	Real const b_{ 1.0 }; // Time scaling (2*pi/period)
	Real const c_{ 0.0 }; // Shift
	Real const a_b_{ 1.0 }; // a * b
	Real const a_b2_{ 1.0 }; // a * b^2
	Real const a_b3_{ 1.0 }; // a * b^3

}; // Function_Inp_sin

} // fmu
} // QSS

#endif
