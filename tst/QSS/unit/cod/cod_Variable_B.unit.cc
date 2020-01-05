// QSS::cod::Variable_B Unit Tests
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2020 Objexx Engineering, Inc. All rights reserved.
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
#include <QSS/cod/Variable_B.hh>

using namespace QSS;
using namespace QSS::cod;

TEST( cod_Variable_BTest, Basic )
{
	Variable_B b( "b" );
	b.init( true );

	EXPECT_TRUE( b.is_Discrete() );
	EXPECT_EQ( 0, b.order() );
	EXPECT_EQ( 0.0, b.tX );
	EXPECT_EQ( 0.0, b.tQ );
	EXPECT_EQ( true, b.b() );
	EXPECT_EQ( 1, b.i() );
	EXPECT_EQ( 1.0, b.r() );

	EXPECT_EQ( 1.0, b.x( 0.0 ) );
	EXPECT_EQ( 1.0, b.q( 0.0 ) );
	EXPECT_EQ( 0.0, b.x1( 0.0 ) );
	EXPECT_EQ( 0.0, b.q1( 0.0 ) );
	EXPECT_EQ( 0.0, b.x2( 0.0 ) );
	EXPECT_EQ( 0.0, b.q2( 0.0 ) );
	EXPECT_EQ( 0.0, b.x3( 0.0 ) );
	EXPECT_EQ( 0.0, b.q3( 0.0 ) );

	EXPECT_EQ( 1.0, b.x( 1.0 ) );
	EXPECT_EQ( 1.0, b.q( 1.0 ) );
	EXPECT_EQ( 0.0, b.x1( 1.0 ) );
	EXPECT_EQ( 0.0, b.q1( 1.0 ) );
	EXPECT_EQ( 0.0, b.x2( 1.0 ) );
	EXPECT_EQ( 0.0, b.q2( 1.0 ) );
	EXPECT_EQ( 0.0, b.x3( 1.0 ) );
	EXPECT_EQ( 0.0, b.q3( 1.0 ) );

	b.advance_handler( 2.0, 0.0 );
	EXPECT_EQ( 2.0, b.tX );
	EXPECT_EQ( 2.0, b.tQ );

	EXPECT_EQ( false, b.b() );
	EXPECT_EQ( 0, b.i() );
	EXPECT_EQ( 0.0, b.r() );
	EXPECT_EQ( 0.0, b.x( 2.0 ) );
	EXPECT_EQ( 0.0, b.q( 2.0 ) );

	events.clear();
}
