#ifndef QSS_EventQueue_hh_INCLUDED
#define QSS_EventQueue_hh_INCLUDED

// QSS Event Queue
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// This is a simple event queue based on std::multimap that is non-optimal for sequential and concurrent access
// Will need to put mutex locks around modifying operations for concurrent use
// Should explore concurrent-friendly priority queues once we have large scale test cases

// QSS Headers
#include <QSS/Event.hh>
#include <QSS/math.hh>
#include <QSS/SuperdenseTime.hh>

// C++ Headers
#include <cassert>
#include <cstddef>
#include <iostream>
#include <map>
#include <vector>

namespace QSS {

// QSS Event Queue
template< typename V >
class EventQueue
{

public: // Types

	using Type = typename Event< V >::Type;
	using Value = typename Event< V >::Value;
	using Time = SuperdenseTime::Time;
	using Index = SuperdenseTime::Index;
	using Var = V;
	using Variables = std::vector< V * >;
	using Events = std::vector< Event< V > >;

	using EventMap = std::multimap< SuperdenseTime, Event< V > >;
	using size_type = typename EventMap::size_type;
	using const_iterator = typename EventMap::const_iterator;
	using iterator = typename EventMap::iterator;
	using const_pointer = typename EventMap::const_pointer;
	using pointer = typename EventMap::pointer;
	using const_reference = typename EventMap::const_reference;
	using reference = typename EventMap::reference;

	// SuperdenseTime Index Offsets
	struct Off {
		static SuperdenseTime::Index const ZC{ 0 };
		static SuperdenseTime::Index const QSS{ 2 }; // So QSS observer updates occur after observer ZC -> Handler events
		static SuperdenseTime::Index const Observer{ 0 };
		static SuperdenseTime::Index const Handler{ 0 };
	};

public: // Predicates

	// Empty?
	bool
	empty() const
	{
		return m_.empty();
	}

	// Has Event at SuperdenseTime s?
	bool
	has( SuperdenseTime const & s ) const
	{
		return ( m_.find( s ) != m_.end() );
	}

	// Top Event is ZC?
	bool
	top_is_ZC() const
	{
		assert( ! m_.empty() );
		return m_.begin()->second.is_ZC();
	}

	// Top Event is QSS?
	bool
	top_is_QSS() const
	{
		assert( ! m_.empty() );
		return m_.begin()->second.is_QSS();
	}

	// Top Event is Observer?
	bool
	top_is_observer() const
	{
		assert( ! m_.empty() );
		return m_.begin()->second.is_observer();
	}

	// Top Event is Handler?
	bool
	top_is_handler() const
	{
		assert( ! m_.empty() );
		return m_.begin()->second.is_handler();
	}

	// Single Trigger Variable at Front of Queue?
	bool
	single() const
	{
		if ( m_.size() >= 2u ) {
			const_iterator i( m_.begin() );
			const_iterator const event1( i );
			const_iterator const event2( ++i );
			return ( event1->first != event2->first );
		} else {
			return ( m_.size() == 1u );
		}
	}

	// Simultaneous Trigger Variables at Front of Queue?
	bool
	simultaneous() const
	{
		if ( m_.size() >= 2u ) {
			const_iterator i( m_.begin() );
			const_iterator const event1( i );
			const_iterator const event2( ++i );
			return ( event1->first == event2->first );
		} else {
			return false;
		}
	}

public: // Properties

	// Size
	size_type
	size() const
	{
		return m_.size();
	}

	// Count of Events at SuperdenseTime s
	size_type
	count( SuperdenseTime const & s ) const
	{
		return m_.count( s );
	}

	// Any Event at SuperdenseTime s
	const_iterator
	find( SuperdenseTime const & s ) const
	{
		return m_.find( s );
	}

	// Any Event at SuperdenseTime s
	iterator
	find( SuperdenseTime const & s )
	{
		return m_.find( s );
	}

	// All Events at SuperdenseTime s
	std::pair< const_iterator, const_iterator >
	equal_range( SuperdenseTime const & s ) const
	{
		return m_.equal_range( s );
	}

	// All Events at SuperdenseTime s
	std::pair< iterator, iterator >
	equal_range( SuperdenseTime const & s )
	{
		return m_.equal_range( s );
	}

	// All Events at Top Events SuperdenseTime
	std::pair< const_iterator, const_iterator >
	tops() const
	{
		return m_.equal_range( top_stime() );
	}

	// All Events at Top Events SuperdenseTime
	std::pair< iterator, iterator >
	tops()
	{
		return m_.equal_range( top_stime() );
	}

	// Top Event Type
	typename Event< V >::Type
	top_Event_Type() const
	{
		assert( ! m_.empty() );
		return m_.begin()->second.type();
	}

	// Top Event
	Event< V > const &
	top() const
	{
		assert( ! m_.empty() );
		return m_.begin()->second;
	}

	// Top Event
	Event< V > &
	top()
	{
		assert( ! m_.empty() );
		return m_.begin()->second;
	}

	// Top Event Variable
	Var const *
	top_var() const
	{
		assert( ! m_.empty() );
		return m_.begin()->second.var();
	}

	// Top Event Variable
	Var *
	top_var()
	{
		assert( ! m_.empty() );
		return m_.begin()->second.var();
	}

	// Top Event Time
	Time
	top_time() const
	{
		assert( ! m_.empty() );
		return m_.begin()->first.t;
	}

	// Top Event SuperdenseTime
	SuperdenseTime const &
	top_stime() const
	{
		assert( ! m_.empty() );
		SuperdenseTime const & s( m_.begin()->first );
		t_ = s.t; // Set active time
		return s;
	}

	// Top Event Index
	Index
	top_index() const
	{
		assert( ! m_.empty() );
		return m_.begin()->first.i;
	}

	// Next Event Index
	Index
	next_index() const
	{
		assert( ! m_.empty() );
		return m_.begin()->first.i + Index( 1u );
	}

public: // Iterators

	// Begin Iterator
	const_iterator
	begin() const
	{
		return m_.begin();
	}

	// Begin Iterator
	iterator
	begin()
	{
		return m_.begin();
	}

	// End Iterator
	const_iterator
	end() const
	{
		return m_.end();
	}

	// End Iterator
	iterator
	end()
	{
		return m_.end();
	}

public: // Methods

	// Simultaneous Trigger Variables at Front of Queue
	Variables
	top_vars()
	{
		Variables vars;
		if ( ! m_.empty() ) {
			iterator i( m_.begin() );
			iterator e( m_.end() );
			SuperdenseTime const & s( i->first );
			while ( ( i != e ) && ( i->first == s ) ) {
				vars.push_back( i->second.var() );
				++i;
			}
		}
		return vars;
	}

	// Simultaneous Events at Front of Queue
	Events
	top_events()
	{
		Events tops;
		if ( ! m_.empty() ) {
			iterator i( m_.begin() );
			iterator e( m_.end() );
			SuperdenseTime const & s( i->first );
			while ( ( i != e ) && ( i->first == s ) ) {
				tops.push_back( i->second );
				++i;
			}
		}
		return tops;
	}

	// Set Active Time
	void
	set_active_time() const
	{
		assert( ! m_.empty() );
		t_ = m_.begin()->first.t;
	}

	// Clear
	void
	clear()
	{
		m_.clear();
	}

public: // Zero-Crossing Event Methods

	// Add Zero-Crossing Event
	iterator
	add_ZC(
	 Time const t,
	 Var * var
	)
	{
		return m_.emplace( SuperdenseTime( t, Off::ZC ), Event< V >( Event< V >::ZC, var ) );
	}

	// Shift Zero-Crossing Event
	iterator
	shift_ZC(
	 Time const t,
	 iterator const i
	)
	{
		Index const idx( t == t_ ? next_index() + Off::ZC : Off::ZC );
		Var * var( i->second.var() );
		m_.erase( i );
		return m_.emplace( SuperdenseTime( t, idx ), Event< V >( Event< V >::ZC, var ) );
	}

public: // QSS Event Methods

	// Add QSS Event
	iterator
	add_QSS(
	 Time const t,
	 Var * var
	)
	{
		return m_.emplace( SuperdenseTime( t, Off::QSS ), Event< V >( Event< V >::QSS, var ) );
	}

	// Shift QSS Event
	iterator
	shift_QSS(
	 Time const t,
	 iterator const i
	)
	{
		Index const idx( t == t_ ? next_index() + Off::QSS : Off::QSS );
		Var * var( i->second.var() );
		m_.erase( i );
		return m_.emplace( SuperdenseTime( t, idx ), Event< V >( Event< V >::QSS, var ) );
	}

public: // Observer Event Methods

	// Shift Observer Event
	iterator
	shift_observer(
	 Time const t,
	 iterator const i
	)
	{
		Index const idx( next_index() + Off::Observer );
		Var * var( i->second.var() );
		m_.erase( i );
		return m_.emplace( SuperdenseTime( t, idx ), Event< V >( Event< V >::Observer, var ) );
	}

public: // Handler Event Methods

	// Add Handler Event
	iterator
	add_handler(
	 Time const t,
	 Var * var
	)
	{
		return m_.emplace( SuperdenseTime( t, Off::Handler ), Event< V >( Event< V >::Handler, var ) );
	}

	// Add Handler Event at Time Infinity
	iterator
	add_handler( Var * var )
	{
		return m_.emplace( SuperdenseTime( infinity, Off::Handler ), Event< V >( Event< V >::Handler, var ) );
	}

	// Shift Handler Event
	iterator
	shift_handler(
	 Time const t,
	 Value const val,
	 iterator const i
	)
	{
		Index const idx( next_index() + Off::Handler );
		Var * var( i->second.var() );
		SuperdenseTime const & s( i->first );
		if ( ( s.t == t ) && ( s.i == idx ) ) { // Variable already has event at same superdense time
			Event< V > const & e( i->second );
			if ( ( e.is_handler() ) && ( e.val() != val ) ) std::cerr << "Zero-crossing handler events at the same time but with different values occurred for: " << var->name << std::endl;
		}
		m_.erase( i );
		return m_.emplace( SuperdenseTime( t, idx ), Event< V >( Event< V >::Handler, var, val ) );
	}

	// Shift Handler Event: FMU Sets Value
	iterator
	shift_handler(
	 Time const t,
	 iterator const i
	)
	{
		Index const idx( next_index() + Off::Handler );
		Var * var( i->second.var() );
		SuperdenseTime const & s( i->first );
		m_.erase( i );
		return m_.emplace( SuperdenseTime( t, idx ), Event< V >( Event< V >::Handler, var ) );
	}

	// Shift Handler Event to Time Infinity
	iterator
	shift_handler( iterator const i )
	{
		Var * var( i->second.var() );
		m_.erase( i );
		return m_.emplace( SuperdenseTime( infinity, Off::Handler ), Event< V >( Event< V >::Handler, var ) );
	}

private: // Data

	EventMap m_;
	mutable Time t_{ 0.0 }; // Active event time

};

} // QSS

#endif
