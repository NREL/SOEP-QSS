// QSS::cod::Variable_ZC3 Unit Tests
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

#define _USE_MATH_DEFINES // For M_E

// Google Test Headers
#include <gtest/gtest.h>

// QSS Headers
#include <QSS/cod/Variable_ZC3.hh>
#include <QSS/cod/Variable_QSS3.hh>
#include <QSS/cod/mdl/Function_LTI.hh>

using namespace QSS;
using namespace QSS::cod;
using namespace QSS::cod::mdl;

TEST( cod_Variable_ZC3Test, Basic )
{
	options::zFac = options::zrFac = options::zaFac = 1.0;
	options::dtZMax = 0.0;

	Variable_QSS3< Function_LTI > x( "x", 1.0e-4, 1.0e-4 );
	x.add( -1.0 );
	x.init( 1.0 );
	EXPECT_EQ( 1.0e-4, x.rTol );
	EXPECT_EQ( 1.0e-4, x.aTol );
	EXPECT_EQ( infinity, x.tE );

	EXPECT_EQ( 1.0, x.x( 0.0 ) );
	EXPECT_EQ( 1.0, x.q( 0.0 ) );

	EXPECT_DOUBLE_EQ( 1.0 - 1.0e-7, x.x( 1.0e-7 ) );
	EXPECT_DOUBLE_EQ( 1.0 - 1.0e-7, x.q( 1.0e-7 ) );

	Variable_ZC3< Function_LTI > z( "z", 1.0e-4, 1.0e-4 );
	z.add_crossings_Dn();
	z.add( &x );
	z.init();
	EXPECT_EQ( 1.0e-4, z.rTol );
	EXPECT_EQ( 1.0e-4, z.aTol );
	EXPECT_EQ( infinity, z.tE );
	EXPECT_DOUBLE_EQ( 1.0, z.tZ );
	EXPECT_EQ( Variable::Crossing::DnPN, z.crossing );

	EXPECT_EQ( 1.0, z.x( 0.0 ) );
	EXPECT_EQ( 1.0, z.q( 0.0 ) );

	EXPECT_DOUBLE_EQ( 1.0 - 1.0e-7, z.x( 1.0e-7 ) );
	EXPECT_DOUBLE_EQ( 1.0 - 1.0e-7, z.q( 1.0e-7 ) );

	EXPECT_EQ( 0.0, x.x( 1.0 ) );
	EXPECT_EQ( 0.0, x.q( 1.0 ) );

	events.clear();
}

TEST( cod_Variable_ZC3Test, Roots )
{
	options::zFac = options::zrFac = options::zaFac = 1.0;
	options::dtZMax = 0.0;

	Variable_QSS3< Function_LTI > x( "x" );
	x.add( &x ).add( -2.0 * M_E );
	x.init( 2.0 * ( M_E - 1.0 ) );
	// x' = x - 2 e, x(0) = 2(e-1) => x = -2 e^t + 2 e with a downward zero crossing at t=1
	// At t=0 x rep is: x_0 = q_0 = 2(e-1), x_1 = q_1 = -2, x_2 = -1, x_3 = -1/3
	EXPECT_DOUBLE_EQ( 2.0 * ( M_E - 1.0 ), x.x( 0.0 ) );
	EXPECT_EQ( -2.0, x.x1( 0.0 ) );
	EXPECT_EQ( -2.0, x.x2( 0.0 ) );
	EXPECT_EQ( -2.0, x.x3( 0.0 ) );

	Variable_ZC3< Function_LTI > z( "z" );
	z.add_crossings_Dn();
	z.add( &x );
	z.init();
	EXPECT_DOUBLE_EQ( 1.0204777568328333243, z.tZ ); // Positive root of t^3 + 3 t^2 + 6 t + 6(1-e) ~ 1.0204777568328333243

	events.clear();
}
