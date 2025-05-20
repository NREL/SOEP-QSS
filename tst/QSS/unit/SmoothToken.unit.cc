// QSS::SmoothToken Unit Tests
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2025 Objexx Engineering, Inc. All rights reserved.
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
#include <QSS/SmoothToken.hh>

using namespace QSS;

TEST( SmoothTokenTest, Default )
{
	SmoothToken st;

	EXPECT_EQ( 3, st.order );
	EXPECT_EQ( 0.0, st.x0 );
	EXPECT_EQ( 0.0, st.x1 );
	EXPECT_EQ( 0.0, st.x2 );
	EXPECT_EQ( 0.0, st.x3 );
	EXPECT_FALSE( st.has_discrete() );
	EXPECT_EQ( "0 0 0 0", st.rep() );
}

TEST( SmoothTokenTest, Order0 )
{
	SmoothToken st( 5.5 );

	EXPECT_EQ( 0, st.order );
	EXPECT_EQ( 5.5, st.x0 );
	EXPECT_EQ( 0.0, st.x1 );
	EXPECT_EQ( 0.0, st.x2 );
	EXPECT_EQ( 0.0, st.x3 );
	EXPECT_FALSE( st.has_discrete() );
	EXPECT_EQ( "5.5", st.rep() );
}

TEST( SmoothTokenTest, Order1 )
{
	SmoothToken st( 5.5, 3.0 );

	EXPECT_EQ( 1, st.order );
	EXPECT_EQ( 5.5, st.x0 );
	EXPECT_EQ( 3.0, st.x1 );
	EXPECT_EQ( 0.0, st.x2 );
	EXPECT_EQ( 0.0, st.x3 );
	EXPECT_FALSE( st.has_discrete() );
	EXPECT_EQ( "5.5 3", st.rep() );
}

TEST( SmoothTokenTest, Order2 )
{
	SmoothToken st( 5.5, 3.0, 42.0 );

	EXPECT_EQ( 2, st.order );
	EXPECT_EQ( 5.5, st.x0 );
	EXPECT_EQ( 3.0, st.x1 );
	EXPECT_EQ( 42.0, st.x2 );
	EXPECT_EQ( 0.0, st.x3 );
	EXPECT_FALSE( st.has_discrete() );
	EXPECT_EQ( "5.5 3 42", st.rep() );
}

TEST( SmoothTokenTest, Order3 )
{
	SmoothToken st( 5.5, 3.0, 42.0, 8.75 );

	EXPECT_EQ( 3, st.order );
	EXPECT_EQ( 5.5, st.x0 );
	EXPECT_EQ( 3.0, st.x1 );
	EXPECT_EQ( 42.0, st.x2 );
	EXPECT_EQ( 8.75, st.x3 );
	EXPECT_FALSE( st.has_discrete() );
	EXPECT_EQ( "5.5 3 42 8.75", st.rep() );
}

TEST( SmoothTokenTest, Order3D )
{
	SmoothToken st( 5.5, 3.0, 42.0, 8.75, 25.0 );

	EXPECT_EQ( 3, st.order );
	EXPECT_EQ( 5.5, st.x0 );
	EXPECT_EQ( 3.0, st.x1 );
	EXPECT_EQ( 42.0, st.x2 );
	EXPECT_EQ( 8.75, st.x3 );
	EXPECT_EQ( 25.0, st.tD );
	EXPECT_TRUE( st.has_discrete() );
	EXPECT_EQ( "5.5 3 42 8.75 ->| 25 s", st.rep() );
}

TEST( SmoothTokenTest, Named0 )
{
	SmoothToken st( SmoothToken::order_0( 5.5, 25.0 ) );

	EXPECT_EQ( 0, st.order );
	EXPECT_EQ( 5.5, st.x0 );
	EXPECT_EQ( 0.0, st.x1 );
	EXPECT_EQ( 0.0, st.x2 );
	EXPECT_EQ( 0.0, st.x3 );
	EXPECT_EQ( 25.0, st.tD );
	EXPECT_TRUE( st.has_discrete() );
	EXPECT_EQ( "5.5 ->| 25 s", st.rep() );
}

TEST( SmoothTokenTest, Named1 )
{
	SmoothToken st( SmoothToken::order_1( 5.5, 3.0, 25.0 ) );

	EXPECT_EQ( 1, st.order );
	EXPECT_EQ( 5.5, st.x0 );
	EXPECT_EQ( 3.0, st.x1 );
	EXPECT_EQ( 0.0, st.x2 );
	EXPECT_EQ( 0.0, st.x3 );
	EXPECT_EQ( 25.0, st.tD );
	EXPECT_TRUE( st.has_discrete() );
	EXPECT_EQ( "5.5 3 ->| 25 s", st.rep() );
}

TEST( SmoothTokenTest, Named2 )
{
	SmoothToken st( SmoothToken::order_2( 5.5, 3.0, 42.0 ) );

	EXPECT_EQ( 2, st.order );
	EXPECT_EQ( 5.5, st.x0 );
	EXPECT_EQ( 3.0, st.x1 );
	EXPECT_EQ( 42.0, st.x2 );
	EXPECT_EQ( 0.0, st.x3 );
	EXPECT_EQ( infinity, st.tD );
	EXPECT_FALSE( st.has_discrete() );
	EXPECT_EQ( "5.5 3 42", st.rep() );
}

TEST( SmoothTokenTest, Named3 )
{
	SmoothToken st( SmoothToken::order_3( 5.5, 3.0, 42.0, 8.75, 25.0 ) );

	EXPECT_EQ( 3, st.order );
	EXPECT_EQ( 5.5, st.x0 );
	EXPECT_EQ( 3.0, st.x1 );
	EXPECT_EQ( 42.0, st.x2 );
	EXPECT_EQ( 8.75, st.x3 );
	EXPECT_EQ( 25.0, st.tD );
	EXPECT_TRUE( st.has_discrete() );
	EXPECT_EQ( "5.5 3 42 8.75 ->| 25 s", st.rep() );
}
