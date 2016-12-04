// QSS::EventQueue Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// QSS Headers
#include <QSS/EventQueue.hh>

// C++ Headers
#include <vector>

// Variable Mock
class V {};

// Types
using EventQ = EventQueue< V >;
using Variables = std::vector< V >;
using Time = double;

TEST( EventQueueTest, Basic )
{
	Variables vars;
	vars.reserve( 10 ); // Prevent reallocation
	EventQ events;
	Time t( 0.0 );
	Time const tE( 10.0 );
	for ( Variables::size_type i = 0; i < 10; ++i ) {
		vars.emplace_back( V() );
		events.add( Time( i ), &vars[ i ] );
	}

	EXPECT_FALSE( events.empty() );
	EXPECT_EQ( 10u, events.size() );
	EXPECT_EQ( &vars[ 0 ], events.top() );
	EXPECT_EQ( Time( 0.0 ), events.top_time() );
	for ( Variables::size_type i = 0; i < 10; ++i ) {
		EXPECT_TRUE( events.has( Time( i ) ) );
		EXPECT_EQ( 1u, events.count( Time( i ) ) );
		EXPECT_EQ( Time( i ), events.any( Time( i ) )->first );
		EXPECT_EQ( &vars[ i ], events.any( Time( i ) )->second );
	}

	events.shift( 2.0, events.top_iterator() );
	EXPECT_EQ( &vars[ 1 ], events.top() );
	EXPECT_EQ( Time( 1.0 ), events.top_time() );
	EXPECT_EQ( 2u, events.count( 2.0 ) );
	auto all( events.all( 2.0 ) );
	EXPECT_EQ( 2u, std::distance( all.first, all.second ) );
	for ( auto i = all.first; i != all.second; ++i ) {
		EXPECT_EQ( Time( 2.0 ), i->first );
	}

	events.clear();
	EXPECT_TRUE( events.empty() );
}
