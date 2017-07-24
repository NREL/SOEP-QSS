// QSS::EventQueue Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// QSS Headers
#include <QSS/EventQueue.hh>

// C++ Headers
#include <vector>

using namespace QSS;

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
		events.add_QSS( Time( i ), &vars[ i ] );
	}

	EXPECT_FALSE( events.empty() );
	EXPECT_EQ( 10u, events.size() );
	EXPECT_EQ( &vars[ 0 ], events.top_var() );
	EXPECT_EQ( Time( 0.0 ), events.top_time() );
	for ( Variables::size_type i = 0; i < 10; ++i ) {
		SuperdenseTime const s( Time( i ), EventQ::Off::QSS );
		EXPECT_TRUE( events.has( s ) );
		EXPECT_EQ( 1u, events.count( s ) );
		EXPECT_EQ( Time( i ), events.find( s )->first.t );
		EXPECT_EQ( &vars[ i ], events.find( s )->second.var() );
	}

	events.set_active_time();
	events.shift_QSS( Time( 2.0 ), events.begin() );
	SuperdenseTime const s( Time( 2.0 ), EventQ::Off::QSS );
	EXPECT_EQ( &vars[ 1 ], events.top_var() );
	EXPECT_EQ( Time( 1.0 ), events.top_time() );
	EXPECT_EQ( SuperdenseTime( Time( 1.0 ), EventQ::Off::QSS ), events.top_superdense_time() );
	EXPECT_EQ( 2u, events.count( s ) );
	auto all( events.equal_range( s ) );
	EXPECT_EQ( 2u, std::distance( all.first, all.second ) );
	for ( auto i = all.first; i != all.second; ++i ) {
		EXPECT_EQ( Time( 2.0 ), i->first.t );
	}

	events.clear();
	EXPECT_TRUE( events.empty() );
}
