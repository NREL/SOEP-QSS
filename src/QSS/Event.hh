#ifndef QSS_Event_hh_INCLUDED
#define QSS_Event_hh_INCLUDED

// QSS Event Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// C++ Headers
#include <cstddef>

namespace QSS {

// QSS Event Class
template< typename V >
class Event
{

public: // Types

	using Variable = V;
	using Value = double;

	// Event Type
	enum Type : std::size_t {
		ZC,
		QSS,
		Observer,
		Handler
	};

public: // Creation

	// Constructor
	Event(
	 Type const typ,
	 Variable * var = nullptr,
	 Value const val = 0.0
	) :
	 typ_( typ ),
	 var_( var ),
	 val_( val )
	{}

public: // Properties

	// Zero-Crossing Event?
	bool
	is_ZC() const
	{
		return ( typ_ == ZC );
	}

	// QSS Event?
	bool
	is_QSS() const
	{
		return ( typ_ == QSS );
	}

	// Observer Event?
	bool
	is_observer() const
	{
		return ( typ_ == Observer );
	}

	// Handler Event?
	bool
	is_handler() const
	{
		return ( typ_ == Handler );
	}

	// Event Type
	Type
	type() const
	{
		return typ_;
	}

	// Variable
	Variable const *
	var() const
	{
		return var_;
	}

	// Variable
	Variable *
	var()
	{
		return var_;
	}

	// Value
	Value
	val() const
	{
		return val_;
	}

	// Value
	Value &
	val()
	{
		return val_;
	}

public: // Comparison: SuperdenseTime [?] SuperdenseTime

	// Event == Event
	friend
	bool
	operator ==( Event const & e1, Event const & e2 )
	{
		return ( e1.typ_ == e2.typ_ ) && ( e1.var_ == e2.var_ ) && ( ( e1.typ_ != Handler ) || ( e1.val_ == e2.val_ ) );
	}

	// Event != Event
	friend
	bool
	operator !=( Event const & e1, Event const & e2 )
	{
		return ( e1.typ_ != e2.typ_ ) || ( e1.var_ != e2.var_ ) || ( ( e1.typ_ == Handler ) && ( e1.val_ != e2.val_ ) );
	}

private: // Data

	Type typ_;
	Variable * var_{ nullptr };
	Value val_{ 0.0 };

};

} // QSS

#endif
