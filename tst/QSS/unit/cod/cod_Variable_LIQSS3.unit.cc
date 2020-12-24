// QSS::cod::Variable_LIQSS3 Unit Tests
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
#include <QSS/cod/Variable_LIQSS3.hh>
#include <QSS/cod/mdl/Function_LTI.hh>

using namespace QSS;
using namespace QSS::cod;
using namespace QSS::cod::mdl;

TEST( cod_Variable_LIQSS3Test, Basic )
{
	Variable_LIQSS3< Function_LTI > x1( "x1", 1.0e-4, 1.0e-4 );
	Variable_LIQSS3< Function_LTI > x2( "x2", 1.0e-4, 1.0e-3 );

	x1.add( 12.0 ).add( 2.0, &x1 );
	x2.add( 12.0 ).add( &x2 ).add( &x1 );

	x1.init( 2.5 );
	x2.init( 2.5 );

	EXPECT_EQ( 1.0e-4, x1.rTol );
	EXPECT_EQ( 1.0e-4, x1.aTol );
	EXPECT_DOUBLE_EQ( 2.5e-4, x1.qTol );
	EXPECT_EQ( 0.0, x1.tQ );
	EXPECT_DOUBLE_EQ( std::cbrt( x1.qTol / ( 68.002 / 6.0 ) ), x1.tE );

	EXPECT_EQ( 2.5, x1.x( 0.0 ) );
	EXPECT_DOUBLE_EQ( 2.5 + 2.5e-4, x1.q( 0.0 ) );
	EXPECT_DOUBLE_EQ( 17.0005, x1.x1( 0.0 ) );
	EXPECT_DOUBLE_EQ( 17.0005, x1.q1( 0.0 ) );
	EXPECT_DOUBLE_EQ( 34.001, x1.x2( 0.0 ) );
	EXPECT_DOUBLE_EQ( 34.001, x1.q2( 0.0 ) );
	EXPECT_DOUBLE_EQ( 68.002, x1.x3( 0.0 ) );
	EXPECT_EQ( 0.0, x1.q3( 0.0 ) );

	EXPECT_EQ( 1.0e-4, x2.rTol );
	EXPECT_EQ( 1.0e-3, x2.aTol );
	EXPECT_DOUBLE_EQ( 1.0e-3, x2.qTol );
	EXPECT_EQ( 0.0, x2.tQ );
	EXPECT_DOUBLE_EQ( std::cbrt( x2.qTol / ( ( 68.0 + x2.qTol + ( 7.0 * x1.qTol ) ) / 6.0 ) ), x2.tE );

	EXPECT_EQ( 2.5, x2.x( 0.0 ) );
	EXPECT_DOUBLE_EQ( 2.5 + x2.aTol, x2.q( 0.0 ) );
	EXPECT_DOUBLE_EQ( 17.0 + x2.qTol + x1.qTol, x2.x1( 0.0 ) );
	EXPECT_DOUBLE_EQ( 17.0 + x2.qTol + x1.qTol, x2.q1( 0.0 ) );
 	EXPECT_DOUBLE_EQ( 34.0 + x2.qTol + ( 3.0 * x1.qTol ), x2.x2( 0.0 ) );
 	EXPECT_DOUBLE_EQ( 34.0 + x2.qTol + ( 3.0 * x1.qTol ), x2.q2( 0.0 ) );
	EXPECT_DOUBLE_EQ( 68.0 + x2.qTol + ( 7.0 * x1.qTol ), x2.x3( 0.0 ) );
	EXPECT_EQ( 0.0, x2.q3( 0.0 ) );

	double const x1_tE( x1.tE );
	double const x2_x_1( x2.x1( 0.0 ) );
	double const x2_x_2( 0.5 * x2.x2( 0.0 ) );
	double const x2_x_3( x2.x3( 0.0 ) / 6.0 );
	x1.advance_QSS();
	EXPECT_EQ( x1_tE, x1.tQ );
	EXPECT_EQ( x1_tE, x2.tX );
	EXPECT_DOUBLE_EQ( 2.5 + ( x2_x_1 * x2.tX ) + ( x2_x_2 * square( x2.tX ) ) + ( x2_x_3 * cube( x2.tX ) ), x2.x( x2.tX ) );

	x2.tE = 2.0; // To allow advance to 1.0
	x2.advance_observer( 1.0 );
	EXPECT_EQ( 1.0, x2.tX );

	EXPECT_EQ( 2u, events.size() );
	events.clear();
}
