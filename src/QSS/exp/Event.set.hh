#ifndef QSS_Event_hh_INCLUDED
#define QSS_Event_hh_INCLUDED

// QSS Event

// C++ Headers
#include <cassert>

// QSS Event
template< typename V >
class Event
{

public: // Types

	using Time = double;

	using Variable = V;

public: // Creation

	// Default Constructor
	Event()
	{}

	// Constructor
	Event(
	 Time const t,
	 Variable const * x
	) :
	 t( t ),
	 x( x )
	{}

	// Constructor
	Event(
	 Time const t,
	 Variable const & x
	) :
	 t( t ),
	 x( &x )
	{}

public: // Operators

	// Event == Event
	friend
	bool
	simultaneous( Event const & e1, Event const & e2 )
	{
		return ( e1.t == e2.t );
	}

	// Event == Event
	friend
	bool
	operator ==( Event const & e1, Event const & e2 )
	{
		return ( ( e1.t == e2.t ) && ( e1.x == e2.x ) );
	}

	// Event != Event
	friend
	bool
	operator !=( Event const & e1, Event const & e2 )
	{
		return ( ( e1.t != e2.t ) || ( e1.x != e2.x ) );
	}

	// Event < Event
	friend
	bool
	operator <( Event const & e1, Event const & e2 )
	{
		return
		 ( e1.t < e2.t ? true :
		 ( e1.t > e2.t ? false :
		 ( e1.x < e2.x ) ) );
	}

public: // Data

	Time t;
	Variable * x;

};

#endif
