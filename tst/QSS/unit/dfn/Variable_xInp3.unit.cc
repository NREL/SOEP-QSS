// QSS::dfn::Variable_xInp3 Unit Tests
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
#include <QSS/dfn/mdl/Function_Inp_sin.hh>
#include <QSS/dfn/Variable_xInp3.hh>

using namespace QSS;
using namespace QSS::dfn;
using namespace QSS::dfn::mdl;

TEST( Variable_xInp3Test, Basic )
{
	Variable_xInp3< Function_Inp_sin > u( "u" );
	u.set_dt_max( 1.0 );
	u.f().c( 0.05 ).s( 0.5 );
	u.init();
	EXPECT_EQ( 1.0e-4, u.rTol );
	EXPECT_EQ( 1.0e-6, u.aTol );
	EXPECT_EQ( 0.0, u.f()( 0.0 ) );
	EXPECT_DOUBLE_EQ( 0.025, u.f().d1( 0.0 ) );
	EXPECT_EQ( 0.0, u.f().d2( 0.0 ) );
	EXPECT_DOUBLE_EQ( -0.00625, u.f().d3( 0.0 ) );
	EXPECT_DOUBLE_EQ( 0.0, u.x( 0.0 ) );
	EXPECT_DOUBLE_EQ( 0.025 - ( 0.00625 / 6.0 ), u.x( 1.0 ) );
	EXPECT_DOUBLE_EQ( 0.025, u.x1( 0.0 ) );
	EXPECT_DOUBLE_EQ( 0.0, u.x2( 0.0 ) );
	EXPECT_DOUBLE_EQ( -0.00625, u.x3( 0.0 ) );
	EXPECT_DOUBLE_EQ( 0.0, u.q( 0.0 ) );
	EXPECT_DOUBLE_EQ( 0.025 - ( 0.00625 / 6.0 ), u.q( 1.0 ) );
	EXPECT_DOUBLE_EQ( 0.025, u.q1( 0.0 ) );
	EXPECT_DOUBLE_EQ( 0.0, u.q2( 0.0 ) );
	EXPECT_DOUBLE_EQ( -0.00625, u.q3( 0.0 ) );
	EXPECT_EQ( 0.0, u.tQ );
	EXPECT_DOUBLE_EQ( std::cbrt( 6.0e-6 / 0.00625 ), u.tE );
	double const u_tE( u.tE );
	u.advance_QSS();
	EXPECT_EQ( u_tE, u.tQ );
	EXPECT_EQ( 1U, events.size() );
	events.clear();
}
