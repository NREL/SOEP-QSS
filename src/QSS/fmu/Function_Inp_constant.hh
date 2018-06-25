// Constant Input Function
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

#ifndef QSS_fmu_Function_Inp_constant_hh_INCLUDED
#define QSS_fmu_Function_Inp_constant_hh_INCLUDED

// QSS Headers
#include <QSS/SmoothToken.hh>

namespace QSS {
namespace fmu {

// Constant Input Function
class Function_Inp_constant final
{

public: // Types

	using Time = double;
	using Value = double;

public: // Creation

	// Constructor
	Function_Inp_constant( Value const x_0 ) :
	 s_( 0, x_0 )
	{}

public: // Properties

	// State at Time t
	SmoothToken const &
	operator ()( Time const t ) const
	{
		s_.t = t;
		return s_;
	}

	// State at Time t (Reevaluated)
	SmoothToken const &
	smooth_token( Time const t ) const
	{
		s_.t = t;
		return s_;
	}

	// Value at Time t
	Value
	v( Time const ) const
	{
		return s_.x_0;
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

	// Discrete Event after Time t
	Time
	tD( Time const ) const
	{
		return infinity;
	}

private: // Data

	mutable SmoothToken s_; // Cached state

};

} // fmu
} // QSS

#endif
