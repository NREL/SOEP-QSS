// QSS::fmu::Variable_Inp2 Unit Tests
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
#include <QSS/fmu/Variable_Inp2.hh>
#include <QSS/fmu/Variable_QSS2.hh>
#include <QSS/fmu/EventIndicator.hh>

using namespace QSS;
using namespace QSS::fmu;

TEST( fmu_Variable_Inp2Test, Basic )
{
	FMU_ME fmu;

	Variable_Inp2 u( "u", 1.0e-4, 1.0e-6, &fmu );

	EXPECT_EQ( 1.0e-4, u.rTol );
	EXPECT_EQ( 1.0e-6, u.aTol );
	EXPECT_EQ( 0.0, u.tQ );
	EXPECT_EQ( 0.0, u.tX );
	EXPECT_EQ( 0.0, u.tE );

	EXPECT_EQ( 0.0, u.x( 0.0 ) );
	EXPECT_EQ( 0.0, u.q( 0.0 ) );
	EXPECT_EQ( 0.0, u.x1( 0.0 ) );
	EXPECT_EQ( 0.0, u.q1( 0.0 ) );
	EXPECT_EQ( 0.0, u.x2( 0.0 ) );
	EXPECT_EQ( 0.0, u.q2( 0.0 ) );

	EXPECT_EQ( 0.0, u.x( 1.0 ) );
	EXPECT_EQ( 0.0, u.q( 1.0 ) );
	EXPECT_EQ( 0.0, u.x1( 1.0 ) );
	EXPECT_EQ( 0.0, u.q1( 1.0 ) );
	EXPECT_EQ( 0.0, u.x2( 1.0 ) );
	EXPECT_EQ( 0.0, u.q2( 1.0 ) );
}

TEST( fmu_Variable_Inp2Test, InputFunction )
{
	std::string const model( "InputFunction.fmu" );
	if ( ! path::is_file( model ) ) {
		std::cout << ">>>>>>>>>>>> fmu::Variable_Inp2 InputFunction test not run: InputFunction.fmu not present" << std::endl;
		return;
	}

	options::qss = options::QSS::QSS2;
	options::specified::qss = true;
	options::eidd = false;
	options::specified::eidd = true;
	options::rTol = 100.0;
	options::specified::rTol = true;
	options::aTol = 1.0;
	options::specified::aTol = true;
	options::output::X = false;
	options::fxn[ "u" ] = "sin[1,1,1]";

	std::streambuf * coutBuf( std::cout.rdbuf() ); std::ostringstream strCout; std::cout.rdbuf( strCout.rdbuf() ); // Redirect cout
	allEventIndicators.clear();
	FMU_ME fmu( model );
	fmu.instantiate();
	fmu.pre_simulate();
	fmu.init();
	std::cout.rdbuf( coutBuf ); // Re-redirect cout

	Variable_QSS2 * x( dynamic_cast< Variable_QSS2 * >( fmu.var_named( "x" ) ) );
	Variable_Inp2 * u( dynamic_cast< Variable_Inp2 * >( fmu.var_named( "u" ) ) );
	if ( ( x == nullptr ) || ( u == nullptr ) ) {
		std::cout << ">>>>>>>>>>>> fmu::Variable_Inp2 InputFunction test not run: Variables x and/or u not found in FMU" << std::endl;
		return;
	}

	if ( dynamic_cast< Variable_QSS2 * >( fmu.var_named( "time" ) ) == nullptr ) {
		EXPECT_EQ( 2u, fmu.eventq->size() );
	} else {
		EXPECT_EQ( 3u, fmu.eventq->size() );
	}

	EXPECT_EQ( 100.0, x->rTol );
	EXPECT_EQ( 1.0, x->aTol );
	EXPECT_EQ( 1.0, x->qTol );
	EXPECT_EQ( 0.0, x->tQ );
	EXPECT_EQ( 0.0, x->tX );
	EXPECT_NEAR( std::sqrt( 2.0 ), x->tE, 1e-9 );
	EXPECT_EQ( 0.0, x->x( 0.0 ) );
	EXPECT_EQ( 0.0, x->q( 0.0 ) );
	EXPECT_EQ( 1.0, x->x1( 0.0 ) );
	EXPECT_EQ( 1.0, x->q1( 0.0 ) );
	EXPECT_NEAR( 1.0, x->x2( 0.0 ), 1e-9 );
	EXPECT_EQ( 0.0, x->q2( 0.0 ) );

	EXPECT_EQ( 100.0, u->rTol );
	EXPECT_EQ( 1.0, u->aTol );
	EXPECT_EQ( 100.0, u->qTol );
	EXPECT_EQ( 0.0, u->tQ );
	EXPECT_EQ( 0.0, u->tX );
	EXPECT_EQ( infinity, u->tE );
	EXPECT_EQ( 1.0, u->x( 0.0 ) );
	EXPECT_EQ( 1.0, u->q( 0.0 ) );
	EXPECT_EQ( 1.0, u->x1( 0.0 ) );
	EXPECT_EQ( 1.0, u->q1( 0.0 ) );
	EXPECT_EQ( 0.0, u->x2( 0.0 ) );
	EXPECT_EQ( 0.0, u->q2( 0.0 ) );

	fmu.set_time( x->tE );
	x->advance_QSS();

	EXPECT_NEAR( std::sqrt( 2.0 ), x->tQ, 1e-9 );
	EXPECT_NEAR( std::sqrt( 2.0 ), x->tX, 1e-9 );
	EXPECT_NEAR( 100.0 * ( std::sqrt( 2.0 ) + 1.0 ), x->qTol, 1e-8 );
	EXPECT_NEAR( std::sqrt( 2.0 ) + std::sqrt( x->qTol / 0.5 ), x->tE, 1e-8 );
	EXPECT_NEAR( std::sqrt( 2.0 ) + 1.0, x->x( x->tX ), 1e-9 );
	EXPECT_NEAR( std::sqrt( 2.0 ) + 1.0, x->q( x->tQ ), 1e-9 );
	EXPECT_NEAR( std::sqrt( 2.0 ) + 1.0, x->x1( x->tX ), 1e-9 );
	EXPECT_NEAR( std::sqrt( 2.0 ) + 1.0, x->q1( x->tX ), 1e-9 );
	EXPECT_NEAR( 1.0, x->x2( x->tX ), 1e-9 );
	EXPECT_EQ( 0.0, x->q2( x->tX ) );

	EXPECT_EQ( 0.0, u->tQ );
	EXPECT_EQ( 0.0, u->tX );
	EXPECT_EQ( infinity, u->tE );
	EXPECT_EQ( 1.0, u->x( u->tX ) );
	EXPECT_EQ( 1.0, u->q( u->tQ ) );
	EXPECT_EQ( 1.0, u->x1( 0.0 ) );
	EXPECT_EQ( 1.0, u->q1( 0.0 ) );
	EXPECT_EQ( 0.0, u->x2( 0.0 ) );
	EXPECT_EQ( 0.0, u->q2( 0.0 ) );
}
