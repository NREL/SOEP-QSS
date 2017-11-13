// SuperdenseTime Representation
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

#ifndef QSS_SuperdenseTime_hh_INCLUDED
#define QSS_SuperdenseTime_hh_INCLUDED

// C++ Headers
#include <cassert>
#include <cstddef>
#include <iomanip>
#include <limits>
#include <ostream>

namespace QSS {

// SuperdenseTime Representation
class SuperdenseTime final
{

public: // Types

	using Time = double;
	using Index = std::size_t;

public: // Creation

	// Default Constructor
	SuperdenseTime() :
	 t( 0.0 ),
	 i( 0 )
	{}

	// Time Constructor
	SuperdenseTime( Time const t ) :
	 t( t ),
	 i( 0 )
	{}

	// Time + Index Constructor
	SuperdenseTime(
	 Time const t,
	 Index const i
	) :
	 t( t ),
	 i( i )
	{}

public: // Assignment

	// SuperdenseTime = Time
	SuperdenseTime &
	operator =( Time const t_ )
	{
		t = t_;
		i = 0;
		return *this;
	}

	// Time + Index Assignment
	void
	assign( Time const t_, Index const i_ )
	{
	    t = t_;
	    i = i_;
	}

public: // Conversion

	// Time Conversion
	operator Time() const
	{
		return t;
	}

	// Time Conversion
	operator Time &()
	{
		return t;
	}

public: // Properties

	// Next Index
	Index
	next_index() const
	{
		assert( i < std::numeric_limits< Index >::max() );
		return i + 1;
	}

public: // Comparison: SuperdenseTime [?] SuperdenseTime

	// SuperdenseTime == SuperdenseTime
	friend
	bool
	operator ==( SuperdenseTime const & s1, SuperdenseTime const & s2 )
	{
		return ( s1.t == s2.t ) && ( s1.i == s2.i );
	}

	// SuperdenseTime != SuperdenseTime
	friend
	bool
	operator !=( SuperdenseTime const & s1, SuperdenseTime const & s2 )
	{
		return ( s1.t != s2.t ) || ( s1.i != s2.i );
	}

	// SuperdenseTime < SuperdenseTime
	friend
	bool
	operator <( SuperdenseTime const & s1, SuperdenseTime const & s2 )
	{
		return ( s1.t < s2.t ) || ( ( s1.t == s2.t ) && ( s1.i < s2.i ) );
	}

	// SuperdenseTime <= SuperdenseTime
	friend
	bool
	operator <=( SuperdenseTime const & s1, SuperdenseTime const & s2 )
	{
		return ( s1.t < s2.t ) || ( ( s1.t == s2.t ) && ( s1.i <= s2.i ) );
	}

	// SuperdenseTime >= SuperdenseTime
	friend
	bool
	operator >=( SuperdenseTime const & s1, SuperdenseTime const & s2 )
	{
		return ( s1.t > s2.t ) || ( ( s1.t == s2.t ) && ( s1.i >= s2.i ) );
	}

	// SuperdenseTime > SuperdenseTime
	friend
	bool
	operator >( SuperdenseTime const & s1, SuperdenseTime const & s2 )
	{
		return ( s1.t > s2.t ) || ( ( s1.t == s2.t ) && ( s1.i > s2.i ) );
	}

public: // Comparison: SuperdenseTime [?] Time

	// SuperdenseTime == Time
	friend
	bool
	operator ==( SuperdenseTime const & s1, Time const t2 )
	{
		return ( s1.t == t2 );
	}

	// SuperdenseTime != Time
	friend
	bool
	operator !=( SuperdenseTime const & s1, Time const t2 )
	{
		return ( s1.t != t2 );
	}

	// SuperdenseTime < Time
	friend
	bool
	operator <( SuperdenseTime const & s1, Time const t2 )
	{
		return ( s1.t < t2 );
	}

	// SuperdenseTime <= Time
	friend
	bool
	operator <=( SuperdenseTime const & s1, Time const t2 )
	{
		return ( s1.t <= t2 );
	}

	// SuperdenseTime >= Time
	friend
	bool
	operator >=( SuperdenseTime const & s1, Time const t2 )
	{
		return ( s1.t >= t2 );
	}

	// SuperdenseTime > Time
	friend
	bool
	operator >( SuperdenseTime const & s1, Time const t2 )
	{
		return ( s1.t > t2 );
	}

public: // Comparison: Time [?] SuperdenseTime

	// Time == SuperdenseTime
	friend
	bool
	operator ==( Time const s1, SuperdenseTime const & s2 )
	{
		return ( s1 == s2.t );
	}

	// Time != SuperdenseTime
	friend
	bool
	operator !=( Time const s1, SuperdenseTime const & s2 )
	{
		return ( s1 != s2.t );
	}

	// Time < SuperdenseTime
	friend
	bool
	operator <( Time const s1, SuperdenseTime const & s2 )
	{
		return ( s1 < s2.t );
	}

	// Time <= SuperdenseTime
	friend
	bool
	operator <=( Time const s1, SuperdenseTime const & s2 )
	{
		return ( s1 <= s2.t );
	}

	// Time >= SuperdenseTime
	friend
	bool
	operator >=( Time const s1, SuperdenseTime const & s2 )
	{
		return ( s1 >= s2.t );
	}

	// Time > SuperdenseTime
	friend
	bool
	operator >( Time const s1, SuperdenseTime const & s2 )
	{
		return ( s1 > s2.t );
	}

public: // Generator

	// Superdense Time with Index Increment
	SuperdenseTime
	up_indexed( Index const up = 1 )
	{
		assert( up < std::numeric_limits< Index >::max() );
		assert( i < std::numeric_limits< Index >::max() - up );
		return SuperdenseTime( t, i + up );
	}

public: // I/O

	// Stream << SuperdenseTime
	friend
	std::ostream &
	operator <<( std::ostream & stream, SuperdenseTime const & s )
	{
		stream << std::setprecision( 16 ) << '(' << s.t << ',' << s.i << ')';
		return stream;
	}

public: // Data

	Time t{ 0.0 };
	Index i{ 0 };

};

} // QSS

#endif
