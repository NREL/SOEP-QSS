#ifndef QSS_Event_hh_INCLUDED
#define QSS_Event_hh_INCLUDED

// QSS Event

// QSS Event
class Event
{

public: // Types

	using Time = double;

public: // Creation

	// Default Constructor
	Event()
	{}

	// Constructor
	Event( Time const t ) :
	 t( t )
	{}

public: // Operators

	// Event == Event
	friend
	bool
	operator ==( Event const & e1, Event const & e2 )
	{
		return ( e1.t == e2.t );
	}

	// Event != Event
	friend
	bool
	operator !=( Event const & e1, Event const & e2 )
	{
		return ( e1.t != e2.t );
	}

	// Event < Event
	friend
	bool
	operator <( Event const & e1, Event const & e2 )
	{
		return ( e1.t < e2.t );
	}

	// Event <= Event
	friend
	bool
	operator <=( Event const & e1, Event const & e2 )
	{
		return ( e1.t <= e2.t );
	}

	// Event > Event
	friend
	bool
	operator >( Event const & e1, Event const & e2 )
	{
		return ( e1.t > e2.t );
	}

	// Event >= Event
	friend
	bool
	operator >=( Event const & e1, Event const & e2 )
	{
		return ( e1.t >= e2.t );
	}

public: // Data

	Time t;

};

#endif
