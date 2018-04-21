// QSS::EventQueue Unit Tests
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2018 Objexx Engineerinc, Inc. All rights reserved.
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

// Google Test Headers
#include <gtest/gtest.h>

// QSS Headers
#include <QSS/EventQueue.hh>

// C++ Headers
#include <iterator>
#include <vector>

using namespace QSS;

// Variable Mock
class V final {};

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
	EXPECT_EQ( &vars[ 0 ], events.top_target() );
	EXPECT_EQ( Time( 0.0 ), events.top_time() );
	for ( Variables::size_type i = 0; i < 10; ++i ) {
		SuperdenseTime const s( Time( i ), 0, EventQ::Off::QSS );
		EXPECT_TRUE( events.has( s ) );
		EXPECT_EQ( 1u, events.count( s ) );
		EXPECT_EQ( Time( i ), events.find( s )->first.t );
		EXPECT_EQ( &vars[ i ], events.find( s )->second.tar() );
	}

	events.set_active_time();
	events.shift_QSS( Time( 2.0 ), events.begin() );
	SuperdenseTime const s( Time( 2.0 ), 0, EventQ::Off::QSS );
	EXPECT_EQ( &vars[ 1 ], events.top_target() );
	EXPECT_EQ( Time( 1.0 ), events.top_time() );
	EXPECT_EQ( SuperdenseTime( Time( 1.0 ), 0, EventQ::Off::QSS ), events.top_superdense_time() );
	EXPECT_EQ( 2u, events.count( s ) );
	auto all( events.equal_range( s ) );
	EXPECT_EQ( 2u, std::distance( all.first, all.second ) );
	for ( auto i = all.first; i != all.second; ++i ) {
		EXPECT_EQ( Time( 2.0 ), i->first.t );
	}

	events.clear();
	EXPECT_TRUE( events.empty() );
}
