// SuperdenseTime Representation
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2023 Objexx Engineering, Inc. All rights reserved.
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
//
// Model variable dependencies can create a cascade of events at the same clock time.
// Rather than introduce artificial delays this implementation uses the notion of
// superdense time to handle events at the same clock time by using a secondary time
// index to group passes of simultaneous events.
//
// QSS simulations can have a number of different types of events: discrete,
// requantization, zero-crossing, and conditional block handler. These can also
// have interdependencies. To ensure a deterministic simulation these are handled
// in sub-passes for each event type. Some computations, such as conditional block
// evaluation, need to process all events from the same pass. To clearly and
// efficiently order and process these sub-passes a second integer-valued offset
// number is associated with each superdense time.
//
// Poorly defined models can create an infinite loop of simultaneous events. This
// implementation will seek to detect such situations and terminate with an error.
struct SuperdenseTime final
{

public: // Types

	using Time = double;
	using Index = std::size_t;
	using Offset = std::size_t;

public: // Creation

	// Default Constructor
	SuperdenseTime() = default;

	// Time Constructor
	explicit
	SuperdenseTime( Time const t ) :
	 t( t )
	{}

	// Time + Index + Offset Constructor
	SuperdenseTime(
	 Time const t,
	 Index const i,
	 Offset const o
	) :
	 t( t ),
	 i( i ),
	 o( o )
	{}

public: // Assignment

	// SuperdenseTime = Time
	SuperdenseTime &
	operator =( Time const time )
	{
		t = time;
		i = 0;
		o = 0;
		return *this;
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

public: // Property

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
		return ( s1.t == s2.t ) && ( s1.i == s2.i ) && ( s1.o == s2.o );
	}

	// SuperdenseTime != SuperdenseTime
	friend
	bool
	operator !=( SuperdenseTime const & s1, SuperdenseTime const & s2 )
	{
		return ( s1.t != s2.t ) || ( s1.i != s2.i ) || ( s1.o != s2.o );
	}

	// SuperdenseTime < SuperdenseTime
	friend
	bool
	operator <( SuperdenseTime const & s1, SuperdenseTime const & s2 )
	{
		return ( s1.t < s2.t ) || ( ( s1.t == s2.t ) && ( ( s1.i < s2.i ) || ( ( s1.i == s2.i ) && ( s1.o < s2.o ) ) ) );
	}

	// SuperdenseTime <= SuperdenseTime
	friend
	bool
	operator <=( SuperdenseTime const & s1, SuperdenseTime const & s2 )
	{
		return ( s1.t < s2.t ) || ( ( s1.t == s2.t ) && ( ( s1.i < s2.i ) || ( ( s1.i == s2.i ) && ( s1.o <= s2.o ) ) ) );
	}

	// SuperdenseTime >= SuperdenseTime
	friend
	bool
	operator >=( SuperdenseTime const & s1, SuperdenseTime const & s2 )
	{
		return ( s1.t > s2.t ) || ( ( s1.t == s2.t ) && ( ( s1.i > s2.i ) || ( ( s1.i == s2.i ) && ( s1.o >= s2.o ) ) ) );
	}

	// SuperdenseTime > SuperdenseTime
	friend
	bool
	operator >( SuperdenseTime const & s1, SuperdenseTime const & s2 )
	{
		return ( s1.t > s2.t ) || ( ( s1.t == s2.t ) && ( ( s1.i > s2.i ) || ( ( s1.i == s2.i ) && ( s1.o > s2.o ) ) ) );
	}

	// Same Time?
	friend
	bool
	same_time( SuperdenseTime const & s1, SuperdenseTime const & s2 )
	{
		return ( s1.t == s2.t );
	}

	// Same Pass?
	friend
	bool
	same_pass( SuperdenseTime const & s1, SuperdenseTime const & s2 )
	{
		return ( s1.t == s2.t ) && ( s1.i == s2.i );
	}

	// Same Type?
	friend
	bool
	same_type( SuperdenseTime const & s1, SuperdenseTime const & s2 )
	{
		return ( s1.o == s2.o );
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

public: // I/O

	// Stream << SuperdenseTime
	friend
	std::ostream &
	operator <<( std::ostream & stream, SuperdenseTime const & s )
	{
		return stream << std::setprecision( 16 ) << '(' << s.t << ',' << s.i << ',' << s.o << ')';
	}

public: // Data

	Time t{ 0.0 }; // Time
	Index i{ 0 }; // Pass
	Offset o{ 0 }; // Offset

}; // SuperdenseTime

} // QSS

#endif
