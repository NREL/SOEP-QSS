#ifndef QSS_EventQueue_hh_INCLUDED
#define QSS_EventQueue_hh_INCLUDED

// QSS Event Queue Based on std::multimap
//
// This is a simple baseline event queue that is non-optimal for sequential and concurrent access
// Will need to put mutex locks around modifying operations for concurrent use
// Should explore concurrent-friendly priority queues once we have large scale cases to test with

// C++ Headers
#include <cassert>
#include <map>

// QSS Event Queue Based on std::multimap
template< typename V >
class EventQueue
{

public: // Types

	using Time = double;
	using Variable = V;

	using EventMap = std::multimap< Time, Variable * >;
	using size_type = typename EventMap::size_type;
	using const_iterator = typename EventMap::const_iterator;
	using iterator = typename EventMap::iterator;
	using const_pointer = typename EventMap::const_pointer;
	using pointer = typename EventMap::pointer;
	using const_reference = typename EventMap::const_reference;
	using reference = typename EventMap::reference;

public: // Creation

	// Default Constructor
	EventQueue()
	{}

public: // Collection Methods

	// Empty?
	bool
	empty() const
	{
		return m_.empty();
	}

	// Size
	size_type
	size() const
	{
		return m_.size();
	}

	// Top Event Variable
	Variable *
	top()
	{
		assert ( ! m_.empty() );
		return m_.begin()->second;
	}

	// Top Event Time
	Time
	top_time() const
	{
		assert ( ! m_.empty() );
		return m_.begin()->first;
	}

	// Top Event Iterator
	iterator
	top_iterator()
	{
		assert ( ! m_.empty() );
		return m_.begin();
	}

	// Pop and Return Top Event Variable
	Variable *
	pop()
	{
		assert ( ! m_.empty() );
		iterator const begin( m_.begin() );
		Variable * x( begin->second );
		m_.erase( begin );
		return x;
	}

	// Clear
	void
	clear()
	{
		m_.clear();
	}

public: // Time Methods

	// Add an Event
	iterator
	add(
	 Time const t,
	 Variable * x
	)
	{
		return m_.emplace( t, x );
	}

	// Push an Event
	iterator
	push(
	 Time const t,
	 Variable * x
	)
	{
		return m_.emplace( t, x );
	}

	// Shift an Event to a New Time
	iterator
	shift(
	 Time const t,
	 iterator const i
	)
	{
		Variable * x( i->second );
		m_.erase( i );
		return m_.emplace( t, x ); //Do See if faster to insert with position hint of i
	}

	// Has Event at Time t?
	bool
	has( Time const t ) const
	{
		return m_.find( t ) != m_.end();
	}

	// Count of Events at Time t
	size_type
	count( Time const t ) const
	{
		return m_.count( t );
	}

	// Any Event at Time t
	const_iterator
	any( Time const t ) const
	{
		return m_.find( t );
	}

	// Any Event at Time t
	iterator
	any( Time const t )
	{
		return m_.find( t );
	}

	// All Events at Time t
	std::pair< const_iterator, const_iterator >
	all( Time const t ) const
	{
		return m_.equal_range( t );
	}

	// All Events at Time t
	std::pair< iterator, iterator >
	all( Time const t )
	{
		return m_.equal_range( t );
	}

private: // Data

	EventMap m_;

};

#endif
