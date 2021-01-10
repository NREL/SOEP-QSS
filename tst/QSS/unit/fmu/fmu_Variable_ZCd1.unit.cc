// QSS::fmu::Variable_ZCd1 Unit Tests
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
#include <QSS/fmu/Variable_ZCd1.hh>
#include <QSS/fmu/Variable_QSS1.hh>
#include <QSS/fmu/EventIndicator.hh>

using namespace QSS;
using namespace QSS::fmu;

TEST( fmu_Variable_ZCd1Test, Basic )
{
	FMU_ME fmu;

	Variable_ZCd1 z( "z", 2.0, 2.0, 1.0e-4, &fmu );

	EXPECT_EQ( 2.0, z.rTol );
	EXPECT_EQ( 2.0, z.aTol );
	EXPECT_EQ( 1.0e-4, z.zTol );
	EXPECT_EQ( 0.0, z.tQ );
	EXPECT_EQ( 0.0, z.tX );
	EXPECT_EQ( 0.0, z.tE );
	EXPECT_EQ( infinity, z.tZ );

	EXPECT_EQ( 0.0, z.x( 0.0 ) );
	EXPECT_EQ( 0.0, z.q( 0.0 ) );
	EXPECT_EQ( 0.0, z.x1( 0.0 ) );
	EXPECT_EQ( 0.0, z.q1( 0.0 ) );

	EXPECT_EQ( 0.0, z.x( 1.0 ) );
	EXPECT_EQ( 0.0, z.q( 1.0 ) );
	EXPECT_EQ( 0.0, z.x1( 1.0 ) );
	EXPECT_EQ( 0.0, z.q1( 1.0 ) );
}

TEST( fmu_Variable_ZCd1Test, BouncingBall )
{
	std::string const model( "BouncingBall.fmu" );
	if ( ! path::is_file( model ) ) {
		std::cout << ">>>>>>>>>>>> fmu::Variable_ZCd1 BouncingBall test not run: BouncingBall.fmu not present" << std::endl;
		return;
	}

	options::qss = options::QSS::QSS1;
	options::specified::qss = true;
	options::rTol = 1.0;
	options::specified::rTol = true;
	options::aTol = 1.0;
	options::specified::aTol = true;
	options::output::X = false;
	options::output::F = false;
	options::output::L = false;
	options::zFac = 1.0;

	std::streambuf * coutBuf( std::cout.rdbuf() ); std::ostringstream strCout; std::cout.rdbuf( strCout.rdbuf() ); // Redirect cout
	allEventIndicators.clear();
	FMU_ME fmu( model );
	fmu.instantiate();
	fmu.pre_simulate();
	fmu.init();
	std::cout.rdbuf( coutBuf ); // Re-redirect cout

	Variable_QSS1 * h( dynamic_cast< Variable_QSS1 * >( fmu.var_named( "h" ) ) );
	Variable_QSS1 * v( dynamic_cast< Variable_QSS1 * >( fmu.var_named( "v" ) ) );
	Variable_ZCd1 * z( dynamic_cast< Variable_ZCd1 * >( fmu.var_named( "_eventIndicator_1" ) ) );
	if ( ( h == nullptr ) || ( v == nullptr ) || ( z == nullptr ) ) {
		std::cout << ">>>>>>>>>>>> fmu::Variable_ZCd1 BouncingBall test not run: Variables h and/or v and/or _eventIndicator_1 not found in FMU" << std::endl;
		return;
	}

	EXPECT_EQ( 1.0, h->rTol );
	EXPECT_EQ( 1.0, h->aTol );
	EXPECT_EQ( 1.0, h->qTol );
	EXPECT_EQ( 0.0, h->tQ );
	EXPECT_EQ( 0.0, h->tX );
	EXPECT_EQ( infinity, h->tE );
	EXPECT_EQ( 1.0, h->x( 0.0 ) );
	EXPECT_EQ( 1.0, h->q( 0.0 ) );
	EXPECT_EQ( 0.0, h->x1( 0.0 ) );

	EXPECT_EQ( 1.0, v->rTol );
	EXPECT_EQ( 1.0, v->aTol );
	EXPECT_EQ( 1.0, v->qTol );
	EXPECT_EQ( 0.0, v->tQ );
	EXPECT_EQ( 0.0, v->tX );
	EXPECT_EQ( 1.0 / 9.80665, v->tE );
	EXPECT_EQ( 0.0, v->x( 0.0 ) );
	EXPECT_EQ( 0.0, v->q( 0.0 ) );
	EXPECT_EQ( -9.80665, v->x1( 0.0 ) );

	EXPECT_EQ( 1.0, z->rTol );
	EXPECT_EQ( 1.0, z->aTol );
	EXPECT_EQ( 1.0, z->qTol );
	EXPECT_EQ( 0.0, z->tQ );
	EXPECT_EQ( 0.0, z->tX );
	EXPECT_EQ( infinity, z->tE );
	EXPECT_EQ( 1.0, z->x( 0.0 ) );
	EXPECT_EQ( 1.0, z->q( 0.0 ) );
	EXPECT_EQ( 0.0, z->x1( 0.0 ) );

	double const v_tE( v->tE );
	fmu.set_time( v->tE );
	v->advance_QSS();

	EXPECT_EQ( 0.0, h->tQ );
	EXPECT_EQ( v_tE, h->tX );
	EXPECT_EQ( 1.0, h->x( h->tX ) );
	EXPECT_EQ( -1.0, h->x1( h->tX ) );

	EXPECT_EQ( v_tE, v->tQ );
	EXPECT_EQ( v_tE, v->tX );
	EXPECT_EQ( -1.0, v->x( v->tX ) );
	EXPECT_EQ( -1.0, v->q( v->tX ) );

	EXPECT_EQ( v_tE, z->tQ );
	EXPECT_EQ( v_tE, z->tX );
	EXPECT_EQ( 1.0, z->x( z->tX ) );
	EXPECT_NEAR( -1.0, z->x1( z->tX ), 1e-9 );
}
