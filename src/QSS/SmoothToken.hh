// SmoothToken Class
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

#ifndef QSS_SmoothToken_hh_INCLUDED
#define QSS_SmoothToken_hh_INCLUDED

// QSS Headers
#include <QSS/math.hh>

// C++ Headers
#include <iosfwd>
#include <string>

namespace QSS {

// SmoothToken Type
struct SmoothToken final
{

public: // Types

	using Real = double;
	using Time = double;

public: // Creation

	// Default Constructor
	SmoothToken() = default;

	// Order-0 Constructor
	explicit
	SmoothToken(
	 Real const x0
	) :
	 order( 0 ),
	 x0( x0 )
	{}

	// Order-1 Constructor
	SmoothToken(
	 Real const x0,
	 Real const x1
	) :
	 order( 1 ),
	 x0( x0 ),
	 x1( x1 )
	{}

	// Order-2 Constructor
	SmoothToken(
	 Real const x0,
	 Real const x1,
	 Real const x2
	) :
	 order( 2 ),
	 x0( x0 ),
	 x1( x1 ),
	 x2( x2 )
	{}

	// Order-3 Constructor
	SmoothToken(
	 Real const x0,
	 Real const x1,
	 Real const x2,
	 Real const x3
	) :
	 order( 3 ),
	 x0( x0 ),
	 x1( x1 ),
	 x2( x2 ),
	 x3( x3 )
	{}

	// Order-3 Constructor with Discrete Event Time
	SmoothToken(
	 Real const x0,
	 Real const x1,
	 Real const x2,
	 Real const x3,
	 Time const tD
	) :
	 order( 3 ),
	 x0( x0 ),
	 x1( x1 ),
	 x2( x2 ),
	 x3( x3 ),
	 tD( tD )
	{}

	// Order-0 Named Constructor
	static
	SmoothToken
	order_0(
	 Real const x0,
	 Time const tD = infinity
	)
	{
		SmoothToken s( x0 );
		s.tD = tD;
		return s;
	}

	// Order-1 Named Constructor
	static
	SmoothToken
	order_1(
	 Real const x0,
	 Real const x1,
	 Time const tD = infinity
	)
	{
		SmoothToken s( x0, x1 );
		s.tD = tD;
		return s;
	}

	// Order-2 Named Constructor
	static
	SmoothToken
	order_2(
	 Real const x0,
	 Real const x1,
	 Real const x2,
	 Time const tD = infinity
	)
	{
		SmoothToken s( x0, x1, x2 );
		s.tD = tD;
		return s;
	}

	// Order-3 Named Constructor
	static
	SmoothToken
	order_3(
	 Real const x0,
	 Real const x1,
	 Real const x2,
	 Real const x3,
	 Time const tD = infinity
	)
	{
		return SmoothToken( x0, x1, x2, x3, tD );
	}

public: // Property

	// Has a Discrete Event?
	bool
	has_discrete() const
	{
		return tD < infinity;
	}

	// String Representation
	std::string
	rep() const;

public: // I/O

	// Stream << SmoothToken (For Plotting so tD Omitted)
	friend
	std::ostream &
	operator <<( std::ostream & stream, SmoothToken const & s );

public: // Data

	int order{ 3 }; // Highest derivative order set
	Real x0{ 0.0 }, x1{ 0.0 }, x2{ 0.0 }, x3{ 0.0 }; // Value and derivatives
	Time tD{ infinity }; // Next discrete event time

}; // SmoothToken

} // QSS

#endif
