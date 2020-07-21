// QSS::fmu::Variable_D Unit Tests
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
#include <QSS/fmu/Variable_D.hh>

using namespace QSS;
using namespace QSS::fmu;

TEST( fmu_Variable_DTest, Basic )
{
	FMU_ME fmu;

	Variable_D d( "d", &fmu );
	d.init( 42.0 );

	EXPECT_TRUE( d.is_Discrete() );
	EXPECT_EQ( 0, d.order() );
	EXPECT_EQ( 1.0e-4, d.rTol );
	EXPECT_EQ( 1.0e-4, d.aTol );
	EXPECT_EQ( 0.0, d.tX );
	EXPECT_EQ( 0.0, d.tQ );
	EXPECT_EQ( 42.0, d.r() );
	EXPECT_EQ( 42, d.i() );
	EXPECT_EQ( true, d.b() );

	EXPECT_EQ( 42.0, d.x( 0.0 ) );
	EXPECT_EQ( 42.0, d.q( 0.0 ) );
	EXPECT_EQ( 0.0, d.x1( 0.0 ) );
	EXPECT_EQ( 0.0, d.q1( 0.0 ) );
	EXPECT_EQ( 0.0, d.x2( 0.0 ) );
	EXPECT_EQ( 0.0, d.q2( 0.0 ) );

	EXPECT_EQ( 42.0, d.x( 1.0 ) );
	EXPECT_EQ( 42.0, d.q( 1.0 ) );
	EXPECT_EQ( 0.0, d.x1( 1.0 ) );
	EXPECT_EQ( 0.0, d.q1( 1.0 ) );
	EXPECT_EQ( 0.0, d.x2( 1.0 ) );
	EXPECT_EQ( 0.0, d.q2( 1.0 ) );

//	d.advance_handler( 2.0 );
//	EXPECT_EQ( 2.0, d.tX );
//	EXPECT_EQ( 2.0, d.tQ );
//	EXPECT_EQ( 99.0, d.r() );
//	EXPECT_EQ( 99.0, d.x( 2.0 ) );
//	EXPECT_EQ( 99.0, d.q( 2.0 ) );
}
