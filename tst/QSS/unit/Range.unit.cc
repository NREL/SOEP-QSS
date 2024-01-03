// QSS::Range Unit Tests
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2024 Objexx Engineering, Inc. All rights reserved.
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
#include <QSS/Range.hh>

using namespace QSS;

TEST( RangeTest, Basic )
{
	Range range;

	EXPECT_TRUE( range.empty() );
	EXPECT_FALSE( range.have() );
	EXPECT_FALSE( range.began() );
	EXPECT_EQ( 0u, range.size() );
	EXPECT_EQ( 0u, range.n() );

	range.assign( 3u, 8u );

	EXPECT_FALSE( range.empty() );
	EXPECT_TRUE( range.have() );
	EXPECT_TRUE( range.began() );
	EXPECT_EQ( 5u, range.size() );
	EXPECT_EQ( 3u, range.b() );
	EXPECT_EQ( 8u, range.e() );
	EXPECT_EQ( 5u, range.n() );

	Range range2( 12, 22 );

	EXPECT_FALSE( range2.empty() );
	EXPECT_TRUE( range2.have() );
	EXPECT_TRUE( range2.began() );
	EXPECT_EQ( 10u, range2.size() );
	EXPECT_EQ( 12u, range2.b() );
	EXPECT_EQ( 22u, range2.e() );
	EXPECT_EQ( 10u, range2.n() );

	swap( range, range2 );

	EXPECT_FALSE( range.empty() );
	EXPECT_TRUE( range.have() );
	EXPECT_TRUE( range.began() );
	EXPECT_EQ( 10u, range.size() );
	EXPECT_EQ( 12u, range.b() );
	EXPECT_EQ( 22u, range.e() );
	EXPECT_EQ( 10u, range.n() );
	EXPECT_FALSE( range2.empty() );
	EXPECT_TRUE( range2.have() );
	EXPECT_TRUE( range2.began() );
	EXPECT_EQ( 5u, range2.size() );
	EXPECT_EQ( 3u, range2.b() );
	EXPECT_EQ( 8u, range2.e() );
	EXPECT_EQ( 5u, range2.n() );

	range.reset();

	EXPECT_TRUE( range.empty() );
	EXPECT_FALSE( range.have() );
	EXPECT_FALSE( range.began() );
	EXPECT_EQ( 0u, range.size() );
	EXPECT_EQ( 0u, range.n() );
}
