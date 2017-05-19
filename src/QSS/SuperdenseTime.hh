#ifndef QSS_SuperdenseTime_hh_INCLUDED
#define QSS_SuperdenseTime_hh_INCLUDED

// SuperdenseTime Representation
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// C++ Headers
#include <cassert>
#include <cstddef>
#include <iomanip>
#include <limits>
#include <ostream>

namespace QSS {

// SuperdenseTime Representation
class SuperdenseTime
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
