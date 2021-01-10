// QSS::SuperdenseTime Unit Tests
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

// Google Test Headers
#include <gtest/gtest.h>

// QSS Headers
#include <QSS/SuperdenseTime.hh>

using namespace QSS;

TEST( SuperdenseTimeTest, Default )
{
	SuperdenseTime st;

	EXPECT_EQ( 0.0, st.t );
	EXPECT_EQ( 0u, st.i );
	EXPECT_EQ( 0u, st.o );

	SuperdenseTime st2;

	EXPECT_TRUE( st == st2 );
	EXPECT_FALSE( st != st2 );
	EXPECT_FALSE( st < st2 );
	EXPECT_TRUE( st <= st2 );
	EXPECT_TRUE( st >= st2 );
	EXPECT_FALSE( st > st2 );
	EXPECT_TRUE( same_time( st, st2 ) );
	EXPECT_TRUE( same_pass( st, st2 ) );
	EXPECT_TRUE( same_type( st, st2 ) );

	SuperdenseTime::Time t( 0.0 );

	EXPECT_TRUE( st == t );
	EXPECT_FALSE( st != t );
	EXPECT_FALSE( st < t );
	EXPECT_TRUE( st <= t );
	EXPECT_TRUE( st >= t );
	EXPECT_FALSE( st > t );

	EXPECT_TRUE( t == st );
	EXPECT_FALSE( t != st );
	EXPECT_FALSE( t < st );
	EXPECT_TRUE( t <= st );
	EXPECT_TRUE( t >= st );
	EXPECT_FALSE( t > st );
}

TEST( SuperdenseTimeTest, Time )
{
	SuperdenseTime st( 12.0 );

	EXPECT_EQ( 12.0, st.t );
	EXPECT_EQ( 0u, st.i );
	EXPECT_EQ( 0u, st.o );

	SuperdenseTime st2( 22.0 );

	EXPECT_FALSE( st == st2 );
	EXPECT_TRUE( st != st2 );
	EXPECT_TRUE( st < st2 );
	EXPECT_TRUE( st <= st2 );
	EXPECT_FALSE( st >= st2 );
	EXPECT_FALSE( st > st2 );
	EXPECT_FALSE( same_time( st, st2 ) );
	EXPECT_FALSE( same_pass( st, st2 ) );
	EXPECT_TRUE( same_type( st, st2 ) );

	SuperdenseTime::Time t( 22.0 );

	EXPECT_FALSE( st == t );
	EXPECT_TRUE( st != t );
	EXPECT_TRUE( st < t );
	EXPECT_TRUE( st <= t );
	EXPECT_FALSE( st >= t );
	EXPECT_FALSE( st > t );

	EXPECT_FALSE( t == st );
	EXPECT_TRUE( t != st );
	EXPECT_FALSE( t < st );
	EXPECT_FALSE( t <= st );
	EXPECT_TRUE( t >= st );
	EXPECT_TRUE( t > st );
}

TEST( SuperdenseTimeTest, TimeIndexOffset )
{
	SuperdenseTime st( 12.0, 5u, 3u );

	EXPECT_EQ( 12.0, st.t );
	EXPECT_EQ( 5u, st.i );
	EXPECT_EQ( 3u, st.o );

	SuperdenseTime st2( 12.0, 6u, 3u );

	EXPECT_FALSE( st == st2 );
	EXPECT_TRUE( st != st2 );
	EXPECT_TRUE( st < st2 );
	EXPECT_TRUE( st <= st2 );
	EXPECT_FALSE( st >= st2 );
	EXPECT_FALSE( st > st2 );
	EXPECT_TRUE( same_time( st, st2 ) );
	EXPECT_FALSE( same_pass( st, st2 ) );
	EXPECT_TRUE( same_type( st, st2 ) );
}
