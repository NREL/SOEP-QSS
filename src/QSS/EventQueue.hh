// QSS Event Queue
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2021 Objexx Engineering, Inc. All rights reserved.
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

// This is a simple event queue based on std::multimap
// It is non-optimal for sequential and concurrent access
// Will need to put mutex locks around modifying operations for concurrent use
// Will explore concurrent-friendly priority queues once we have large scale test cases

#ifndef QSS_EventQueue_hh_INCLUDED
#define QSS_EventQueue_hh_INCLUDED

// QSS Headers
#include <QSS/Event.hh>
#include <QSS/math.hh>
#include <QSS/SuperdenseTime.hh>

// Allocator Headers
//#include <Allocator/Allocator.h>

// C++ Headers
#include <cassert>
#include <cstddef>
//#include <functional>
#include <iostream>
#include <map>
#include <vector>

namespace QSS {

// QSS Event Queue
template< typename T >
class EventQueue final
{

public: // Types

	using EventT = Event< T >;
	using Type = typename EventT::Type;
	using Real = typename EventT::Real;
	using Time = SuperdenseTime::Time;
	using Index = SuperdenseTime::Index;
	using Offset = SuperdenseTime::Offset;
	using Target = T;
	using Targets = std::vector< T * >;
	using Events = std::vector< EventT >;

	using EventMap = std::multimap< SuperdenseTime, EventT >; // C++ allocator
//	using EventMap = std::multimap< SuperdenseTime, EventT, std::less< SuperdenseTime >, Moya::Allocator< std::pair< SuperdenseTime const, EventT > > >; // Moya allocator // Default 1024 grow size was best // Fastest but gives access violation in unit tests
	using value_type = typename EventMap::value_type;
	using size_type = typename EventMap::size_type;
	using const_iterator = typename EventMap::const_iterator;
	using iterator = typename EventMap::iterator;
	using const_pointer = typename EventMap::const_pointer;
	using pointer = typename EventMap::pointer;
	using const_reference = typename EventMap::const_reference;
	using reference = typename EventMap::reference;

	// Event Type SuperdenseTime Offsets
	struct Off final {
		static SuperdenseTime::Offset const Discrete{ 0 };
		static SuperdenseTime::Offset const ZC{ 1 };
		static SuperdenseTime::Offset const Conditional{ 2 };
		static SuperdenseTime::Offset const Handler{ 3 };
		static SuperdenseTime::Offset const QSS{ 4 };
		static SuperdenseTime::Offset const QSS_ZC{ 5 };
		static SuperdenseTime::Offset const QSS_Inp{ 6 };
	};

public: // Predicate

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

	// Single Trigger Target at Front of Queue?
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

	// Simultaneous Trigger Targets at Front of Queue?
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

public: // Property

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
		return m_.equal_range( top_superdense_time() );
	}

	// All Events at Top Events SuperdenseTime
	std::pair< iterator, iterator >
	tops()
	{
		return m_.equal_range( top_superdense_time() );
	}

	// Top Event Type
	Type
	top_Event_Type() const
	{
		assert( ! m_.empty() );
		return m_.begin()->second.type();
	}

	// Top Event
	EventT const &
	top() const
	{
		assert( ! m_.empty() );
		return m_.begin()->second;
	}

	// Top Event
	EventT &
	top()
	{
		assert( ! m_.empty() );
		return m_.begin()->second;
	}

	// Top Event Target
	Target const *
	top_target() const
	{
		assert( ! m_.empty() );
		return m_.begin()->second.tar();
	}

	// Top Event Target
	Target *
	top_target()
	{
		assert( ! m_.empty() );
		return m_.begin()->second.tar();
	}

	// Top Event Target Subtype
	template< typename S >
	S const *
	top_sub() const
	{
		assert( ! m_.empty() );
		return m_.begin()->second.template sub< S >();
	}

	// Top Event Target
	template< typename S >
	S *
	top_sub()
	{
		assert( ! m_.empty() );
		return m_.begin()->second.template sub< S >();
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
	top_superdense_time() const
	{
		assert( ! m_.empty() );
		return m_.begin()->first;
	}

	// Active Event Time
	Time
	active_time() const
	{
		return t_;
	}

	// Active Event SuperdenseTime
	SuperdenseTime const &
	active_superdense_time() const
	{
		return s_;
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

public: // Iterator

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

	// Insert
	iterator
	insert( value_type const & value )
	{
		return m_.insert( value );
	}

	// Insert
	iterator
	insert( value_type && value )
	{
		return m_.insert( value );
	}

	// Clear
	void
	clear()
	{
		m_.clear();
	}

	// Simultaneous Events at Front of Queue
	Events
	top_events() const
	{
		Events tops;
		if ( ! m_.empty() ) {
			const_iterator i( m_.begin() );
			const_iterator const e( m_.end() );
			SuperdenseTime const & s( i->first );
			while ( ( i != e ) && ( i->first == s ) ) {
				tops.push_back( i->second );
				++i;
			}
		}
		return tops;
	}

	// Simultaneous Events at Front of Queue
	void
	top_events( Events & tops ) const
	{
		tops.clear();
		if ( ! m_.empty() ) {
			const_iterator i( m_.begin() );
			const_iterator const e( m_.end() );
			SuperdenseTime const & s( i->first );
			while ( ( i != e ) && ( i->first == s ) ) {
				tops.push_back( i->second );
				++i;
			}
		}
	}

	// Simultaneous Trigger Targets at Front of Queue
	Targets
	top_targets() const
	{
		Targets targets;
		if ( ! m_.empty() ) {
			const_iterator i( m_.begin() );
			const_iterator const e( m_.end() );
			SuperdenseTime const & s( i->first );
			while ( ( i != e ) && ( i->first == s ) ) {
				targets.push_back( i->second.tar() );
				++i;
			}
		}
		return targets;
	}

	// Simultaneous Trigger Targets at Front of Queue
	void
	top_targets( Targets & targets ) const
	{
		targets.clear();
		if ( ! m_.empty() ) {
			const_iterator i( m_.begin() );
			const_iterator const e( m_.end() );
			SuperdenseTime const & s( i->first );
			while ( ( i != e ) && ( i->first == s ) ) {
				targets.push_back( i->second.tar() );
				++i;
			}
		}
	}

	// Simultaneous Trigger Target Subtypes at Front of Queue
	template< typename S >
	std::vector< S * >
	top_subs()
	{
		std::vector< S * > subs;
		if ( ! m_.empty() ) {
			iterator i( m_.begin() );
			iterator const e( m_.end() );
			SuperdenseTime const & s( i->first );
			while ( ( i != e ) && ( i->first == s ) ) {
				subs.push_back( i->second.template sub< S >() );
				++i;
			}
		}
		return subs;
	}

	// Simultaneous Trigger Target Subtypes at Front of Queue
	template< typename S >
	void
	top_subs( std::vector< S * > & subs )
	{
		subs.clear();
		if ( ! m_.empty() ) {
			iterator i( m_.begin() );
			iterator const e( m_.end() );
			SuperdenseTime const & s( i->first );
			while ( ( i != e ) && ( i->first == s ) ) {
				subs.push_back( i->second.template sub< S >() );
				++i;
			}
		}
	}

	// QSS Requantization Bin Subtypes at Front of Queue
	template< typename S >
	std::vector< S * >
	bin_QSS( size_type const bin_size, double const bin_frac )
	{
		std::vector< S * > subs;
		if ( ! m_.empty() ) {
			iterator i( m_.begin() );
			iterator const e( m_.end() );
			SuperdenseTime const & s( i->first );
			while ( ( i != e ) && ( i->first == s ) ) { // First get the simultaneous events
				subs.push_back( i->second.template sub< S >() );
				++i;
			}
			Time const t_top( s.t );
			size_type j( 0u ); // Loop counter (non-simultaneous events)
			while ( ( i != e ) && ( ++j < 5 * bin_size ) && ( subs.size() < bin_size ) ) { // Bin events
				if ( i->second.is_QSS() ) { // QSS requantization event
					S * sub( i->second.template sub< S >() );
					double const sub_frac( ( t_top - sub->tQ ) / ( sub->tE - sub->tQ ) );
					if ( sub_frac >= bin_frac ) { // Time step fraction is acceptable
						subs.push_back( sub );
					}
				}
				++i;
			}
		}
		return subs;
	}

	// QSS Requantization Bin Subtypes at Front of Queue
	template< typename S >
	void
	bin_QSS( size_type const bin_size, double const bin_frac, std::vector< S * > & subs )
	{
		subs.clear();
		if ( ! m_.empty() ) {
			iterator i( m_.begin() );
			iterator const e( m_.end() );
			SuperdenseTime const & s( i->first );
			while ( ( i != e ) && ( i->first == s ) ) { // First get the simultaneous events
				subs.push_back( i->second.template sub< S >() );
				++i;
			}
			Time const t_top( s.t );
			size_type j( 0u ); // Loop counter (non-simultaneous events)
			while ( ( i != e ) && ( ++j < 5 * bin_size ) && ( subs.size() < bin_size ) ) { // Bin events
				if ( i->second.is_QSS() ) { // QSS requantization event
					S * sub( i->second.template sub< S >() );
					double const sub_frac( ( t_top - sub->tQ ) / ( sub->tE - sub->tQ ) );
					if ( sub_frac >= bin_frac ) { // Time step fraction is acceptable
						subs.push_back( sub );
					}
				}
				++i;
			}
		}
	}

	// QSS ZC Requantization Bin Subtypes at Front of Queue
	template< typename S >
	void
	bin_QSS_ZC( size_type const bin_size, double const bin_frac, std::vector< S * > & subs )
	{
		subs.clear();
		if ( ! m_.empty() ) {
			iterator i( m_.begin() );
			iterator const e( m_.end() );
			SuperdenseTime const & s( i->first );
			while ( ( i != e ) && ( i->first == s ) ) { // First get the simultaneous events
				subs.push_back( i->second.template sub< S >() );
				++i;
			}
			Time const t_top( s.t );
			size_type j( 0u ); // Loop counter (non-simultaneous events)
			while ( ( i != e ) && ( ++j < 5 * bin_size ) && ( subs.size() < bin_size ) ) { // Bin events
				if ( i->second.is_QSS_ZC() ) { // QSS ZC requantization event
					S * sub( i->second.template sub< S >() );
					double const sub_frac( ( t_top - sub->tQ ) / ( sub->tE - sub->tQ ) );
					if ( sub_frac >= bin_frac ) { // Time step fraction is acceptable
						subs.push_back( sub );
					}
				}
				++i;
			}
		}
	}

	// Set Active Time
	void
	set_active_time()
	{
		s_ = ( ! m_.empty() ? m_.begin()->first : sZero_ );
		t_ = s_.t;
	}

public: // Discrete Event Methods

	// Add Discrete Event
	iterator
	add_discrete(
	 Time const t,
	 Target * tar
	)
	{
		return m_.emplace( SuperdenseTime( t, 0, Off::Discrete ), EventT( Type::Discrete, tar ) );
	}

	// Shift Discrete Event
	iterator
	shift_discrete(
	 Time const t,
	 iterator const i
	)
	{
		assert( t_ == s_.t );
		assert( t >= t_ );
		Index const idx( t == t_ ? ( s_.o < Off::Discrete ? s_.i : s_.i + 1u ) : Index( 0 ) );
		Target * tar( i->second.tar() );
		m_.erase( i );
		return m_.emplace( SuperdenseTime( t, idx, Off::Discrete ), EventT( Type::Discrete, tar ) );
	}

public: // Zero-Crossing Event Methods

	// Add Zero-Crossing Event
	iterator
	add_ZC(
	 Time const t,
	 Target * tar
	)
	{
		return m_.emplace( SuperdenseTime( t, 0, Off::ZC ), EventT( Type::ZC, tar ) );
	}

	// Shift Zero-Crossing Event
	iterator
	shift_ZC(
	 Time const t,
	 iterator const i
	)
	{
		assert( t_ == s_.t );
		assert( t >= t_ );
		Index const idx( t == t_ ? ( s_.o < Off::ZC ? s_.i : s_.i + 1u ) : Index( 0 ) );
		Target * tar( i->second.tar() );
		m_.erase( i );
		return m_.emplace( SuperdenseTime( t, idx, Off::ZC ), EventT( Type::ZC, tar ) );
	}

public: // Conditional Event Methods

	// Add Conditional Event at Time Infinity
	iterator
	add_conditional( Target * tar )
	{
		return m_.emplace( SuperdenseTime( infinity, 0, Off::Conditional ), EventT( Type::Conditional, tar ) );
	}

	// Shift Conditional Event
	iterator
	shift_conditional(
	 Time const t,
	 iterator const i
	)
	{
		assert( t_ == s_.t );
		assert( t == t_ );
		Index const idx( s_.o < Off::Conditional ? s_.i : s_.i + 1u );
		Target * tar( i->second.tar() );
		m_.erase( i );
		return m_.emplace( SuperdenseTime( t, idx, Off::Conditional ), EventT( Type::Conditional, tar ) );
	}

	// Shift Conditional Event to Time Infinity
	iterator
	shift_conditional( iterator const i )
	{
		Target * tar( i->second.tar() );
		m_.erase( i );
		return m_.emplace( SuperdenseTime( infinity, 0, Off::Conditional ), EventT( Type::Conditional, tar ) );
	}

public: // Handler Event Methods

	// Add Handler Event at Time Infinity
	iterator
	add_handler( Target * tar )
	{
		return m_.emplace( SuperdenseTime( infinity, 0, Off::Handler ), EventT( Type::Handler, tar ) );
	}

	// Shift Handler Event
	iterator
	shift_handler(
	 Time const t,
	 Real const val,
	 iterator const i
	)
	{
		assert( t_ == s_.t );
		assert( t == t_ );
		Index const idx( s_.o < Off::Handler ? s_.i : s_.i + 1u );
		Target * tar( i->second.tar() );
		SuperdenseTime const & s( i->first );
		if ( ( s.t == t ) && ( s.i == idx ) && ( s.o == Off::Handler ) ) { // Target already has event in same pass
			EventT const & e( i->second );
			assert( e.is_handler() );
			if ( e.val() != val ) std::cerr << "Error: Conditional handler events in the same pass but with different values occurred for: " << tar->name() << std::endl;
		}
		m_.erase( i );
		return m_.emplace( SuperdenseTime( t, idx, Off::Handler ), EventT( Type::Handler, tar, val ) );
	}

	// Shift Handler Event
	iterator
	shift_handler(
	 Time const t,
	 iterator const i
	)
	{
		assert( t_ == s_.t );
		assert( t == t_ );
		Index const idx( s_.o < Off::Handler ? s_.i : s_.i + 1u );
		Target * tar( i->second.tar() );
		m_.erase( i );
		return m_.emplace( SuperdenseTime( t, idx, Off::Handler ), EventT( Type::Handler, tar ) );
	}

	// Shift Handler Event to Time Infinity
	iterator
	shift_handler( iterator const i )
	{
		Target * tar( i->second.tar() );
		m_.erase( i );
		return m_.emplace( SuperdenseTime( infinity, 0, Off::Handler ), EventT( Type::Handler, tar ) );
	}

public: // QSS Event Methods

	// Add QSS Event
	iterator
	add_QSS(
	 Time const t,
	 Target * tar
	)
	{
		return m_.emplace( SuperdenseTime( t, 0, Off::QSS ), EventT( Type::QSS, tar ) );
	}

	// Shift QSS Event
	iterator
	shift_QSS(
	 Time const t,
	 iterator const i
	)
	{
		assert( t_ == s_.t );
		assert( t >= t_ );
		Index const idx( t == t_ ? ( s_.o < Off::QSS ? s_.i : s_.i + 1u ) : Index( 0 ) );
		Target * tar( i->second.tar() );
		m_.erase( i );
		return m_.emplace( SuperdenseTime( t, idx, Off::QSS ), EventT( Type::QSS, tar ) );
	}

public: // QSS ZC Event Methods

	// Add QSS ZC Event
	iterator
	add_QSS_ZC(
	 Time const t,
	 Target * tar
	)
	{
		return m_.emplace( SuperdenseTime( t, 0, Off::QSS_ZC ), EventT( Type::QSS_ZC, tar ) );
	}

	// Shift QSS ZC Event
	iterator
	shift_QSS_ZC(
	 Time const t,
	 iterator const i
	)
	{
		assert( t_ == s_.t );
		assert( t >= t_ );
		Index const idx( t == t_ ? ( s_.o < Off::QSS_ZC ? s_.i : s_.i + 1u ) : Index( 0 ) );
		Target * tar( i->second.tar() );
		m_.erase( i );
		return m_.emplace( SuperdenseTime( t, idx, Off::QSS_ZC ), EventT( Type::QSS_ZC, tar ) );
	}

public: // QSS Input Event Methods

	// Add QSS Input Event
	iterator
	add_QSS_Inp(
	 Time const t,
	 Target * tar
	)
	{
		return m_.emplace( SuperdenseTime( t, 0, Off::QSS_Inp ), EventT( Type::QSS_Inp, tar ) );
	}

	// Shift QSS Input Event
	iterator
	shift_QSS_Inp(
	 Time const t,
	 iterator const i
	)
	{
		assert( t_ == s_.t );
		assert( t >= t_ );
		Index const idx( t == t_ ? ( s_.o < Off::QSS_Inp ? s_.i : s_.i + 1u ) : Index( 0 ) );
		Target * tar( i->second.tar() );
		m_.erase( i );
		return m_.emplace( SuperdenseTime( t, idx, Off::QSS_Inp ), EventT( Type::QSS_Inp, tar ) );
	}

private: // Static Data

	static SuperdenseTime const sZero_; // Zero superdense time

private: // Data

	EventMap m_;
	SuperdenseTime s_; // Active event superdense time
	Time t_{ 0.0 }; // Active event time

}; // EventQueue

	// Static Data Member Template Definitions
	template< typename T > SuperdenseTime const EventQueue< T >::sZero_ = SuperdenseTime();

} // QSS

#endif
