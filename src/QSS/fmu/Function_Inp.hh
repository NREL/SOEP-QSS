// Input Function Mockup
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

#ifndef QSS_fmu_Function_Inp_hh_INCLUDED
#define QSS_fmu_Function_Inp_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/SmoothToken.hh>

namespace QSS {
namespace fmu {

// Input Function Mockup
class Function_Inp final
{

public: // Types

	using Time = double;
	using Value = double;

public: // Creation

	// Default Constructor
	Function_Inp(
	 Value const x_0 = 0.0,
	 Value const x_1 = 0.0,
	 Value const x_2 = 0.0,
	 Value const x_3 = 0.0,
	) :
	 s_( 3, x_0, x_1, x_2, x_3 )
	{}

public: // Properties

	// State at Time t
	SmoothToken const &
	operator ()( Time const t ) const
	{
		if ( t != s_.t ) { // Reevaluate state
			s_.t = t;
			s_.tD = tD( t );
			s_.x_0 = v( t );
			s_.x_1 = d1( t );
			s_.x_2 = d2( t );
			s_.x_3 = d3( t );
		}
		return s_;
	}

	// State at Time t (Reevaluated)
	SmoothToken const &
	smooth_token( Time const t ) const
	{
		s_.t = t;
		s_.tD = tD( t );
		s_.x_0 = v( t );
		s_.x_1 = d1( t );
		s_.x_2 = d2( t );
		s_.x_3 = d3( t );
		return s_;
	}

	// Value at Time t
	Value
	v( Time const t ) const
	{
		return ?;
	}

	// First Derivative at Time t
	Value
	d1( Time const t ) const
	{
		return ?;
	}

	// Second Derivative at Time t
	Value
	d2( Time const t ) const
	{
		return ?;
	}

	// Third Derivative at Time t
	Value
	d3( Time const t ) const
	{
		return ?;
	}

	// Discrete Event after Time t
	Time
	tD( Time const t ) const
	{
		return ?;
	}

private: // Data

	mutable SmoothToken s_; // Cached state

};

} // fmu
} // QSS

#endif
