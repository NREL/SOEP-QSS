// QSS::cod::Variable_ZC1 Unit Tests
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
#include <QSS/cod/mdl/Function_LTI.hh>
#include <QSS/cod/Variable_ZC1.hh>
#include <QSS/cod/Variable_QSS2.hh>

// C++ Headers
#include <algorithm>

using namespace QSS;
using namespace QSS::cod;
using namespace QSS::cod::mdl;

TEST( Variable_ZC1Test, Basic )
{
	Variable_QSS2< Function_LTI > x( "x" );
	x.add( -1.0 );
	x.init( 1.0 );
	EXPECT_EQ( 1.0e-4, x.rTol );
	EXPECT_EQ( 1.0e-6, x.aTol );
	EXPECT_EQ( 1.0, x.x( 0.0 ) );
	EXPECT_EQ( 1.0, x.q( 0.0 ) );
	EXPECT_DOUBLE_EQ( 1.0 - 1.0e-7, x.x( 1.0e-7 ) );
	EXPECT_DOUBLE_EQ( 1.0 - 1.0e-7, x.q( 1.0e-7 ) );
	EXPECT_EQ( infinity, x.tE );

	Variable_ZC1< Function_LTI > z( "z", 2.0, 2.0 ); // Big tolerances to make tE > tZ
	z.add_crossings_Dn();
	z.add( &x );
	z.init();
	EXPECT_EQ( 2.0, z.rTol );
	EXPECT_EQ( 2.0, z.aTol );
	EXPECT_EQ( 1.0, z.x( 0.0 ) );
	EXPECT_EQ( 1.0, z.q( 0.0 ) );
	EXPECT_DOUBLE_EQ( 1.0 - 1.0e-7, z.x( 1.0e-7 ) );
	EXPECT_DOUBLE_EQ( 1.0, z.q( 1.0e-7 ) );
	EXPECT_DOUBLE_EQ( 0.0 + std::max( z.rTol * 1.0, z.aTol ) / 1.0, z.tE );
	EXPECT_DOUBLE_EQ( 1.0, z.tZ );
	EXPECT_EQ( Variable::Crossing::DnPN, z.crossing );
	EXPECT_DOUBLE_EQ( 0.0, x.x( 1.0 ) );
	EXPECT_DOUBLE_EQ( 0.0, x.q( 1.0 ) );

	events.clear();
}