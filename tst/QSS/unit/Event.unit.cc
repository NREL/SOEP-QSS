// QSS::Event Unit Tests
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2023 Objexx Engineering, Inc. All rights reserved.
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
#include <QSS/Event.hh>

using namespace QSS;

// Variable Mock
class V final {};

// Types
using EventV = Event< V >;
using Variables = std::vector< V >;
using Time = double;

TEST( EventTest, QSS )
{
	EventV event( EventV::Type::QSS );

	EXPECT_FALSE( event.is_discrete() );
	EXPECT_FALSE( event.is_ZC() );
	EXPECT_FALSE( event.is_conditional() );
	EXPECT_FALSE( event.is_handler() );
	EXPECT_TRUE( event.is_QSS() );
	EXPECT_FALSE( event.is_QSS_ZC() );
	EXPECT_FALSE( event.is_QSS_Inp() );
	EXPECT_EQ( EventV::Type::QSS, event.type() );
	EXPECT_EQ( nullptr, event.tar() );
	EXPECT_EQ( nullptr, event.target() );
	EXPECT_EQ( 0.0, event.val() );
	EXPECT_EQ( 0.0, event.value() );

	event.val() = 42.0;

	EXPECT_EQ( 42.0, event.val() );
	EXPECT_EQ( 42.0, event.value() );

	EventV event2( EventV::Type::QSS, nullptr, 88.0 );

	EXPECT_EQ( nullptr, event2.tar() );
	EXPECT_EQ( nullptr, event2.target() );
	EXPECT_EQ( 88.0, event2.val() );
	EXPECT_EQ( 88.0, event2.value() );
	EXPECT_TRUE( event == event2 ); // Value doesn't matter if not a Handler event
	EXPECT_FALSE( event != event2 );
}

TEST( EventTest, Handler )
{
	EventV event( EventV::Type::Handler );

	EXPECT_FALSE( event.is_discrete() );
	EXPECT_FALSE( event.is_ZC() );
	EXPECT_FALSE( event.is_conditional() );
	EXPECT_TRUE( event.is_handler() );
	EXPECT_FALSE( event.is_QSS() );
	EXPECT_FALSE( event.is_QSS_ZC() );
	EXPECT_FALSE( event.is_QSS_Inp() );
	EXPECT_EQ( EventV::Type::Handler, event.type() );
	EXPECT_EQ( nullptr, event.tar() );
	EXPECT_EQ( nullptr, event.target() );
	EXPECT_EQ( 0.0, event.val() );
	EXPECT_EQ( 0.0, event.value() );

	event.val() = 42.0;

	EXPECT_EQ( 42.0, event.val() );
	EXPECT_EQ( 42.0, event.value() );

	EventV event2( EventV::Type::Handler, nullptr, 88.0 );

	EXPECT_EQ( nullptr, event2.tar() );
	EXPECT_EQ( nullptr, event2.target() );
	EXPECT_EQ( 88.0, event2.val() );
	EXPECT_EQ( 88.0, event2.value() );
	EXPECT_FALSE( event == event2 );
	EXPECT_TRUE( event != event2 );
}
