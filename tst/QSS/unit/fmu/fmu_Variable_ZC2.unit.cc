// QSS::fmu::Variable_ZC2 Unit Tests
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
#include <QSS/fmu/Variable_ZC2.hh>
#include <QSS/fmu/Variable_QSS2.hh>
#include <QSS/fmu/EventIndicator.hh>

using namespace QSS;
using namespace QSS::fmu;

TEST( fmu_Variable_ZC2Test, Basic )
{
	FMU_ME fmu;

	Variable_ZC2 z( "z", 2.0, 2.0, 1.0e-4, &fmu );

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
	EXPECT_EQ( 0.0, z.x2( 0.0 ) );
	EXPECT_EQ( 0.0, z.q2( 0.0 ) );

	EXPECT_EQ( 0.0, z.x( 1.0 ) );
	EXPECT_EQ( 0.0, z.q( 1.0 ) );
	EXPECT_EQ( 0.0, z.x1( 1.0 ) );
	EXPECT_EQ( 0.0, z.q1( 1.0 ) );
	EXPECT_EQ( 0.0, z.x2( 1.0 ) );
	EXPECT_EQ( 0.0, z.q2( 1.0 ) );
}

TEST( fmu_Variable_ZC2Test, BouncingBall )
{
	std::string const model( "BouncingBall.fmu" );
	if ( !path::is_file( model ) ) {
		std::cout << ">>>>>>>>>>>> fmu::Variable_ZC2 BouncingBall test not run: BouncingBall.fmu not present" << std::endl;
		return;
	}

	options::qss = options::QSS::QSS2;
	options::specified::qss = true;
	options::eidd = false;
	options::specified::eidd = true;
	options::rTol = 1.0;
	options::specified::rTol = true;
	options::aTol = 1.0;
	options::specified::aTol = true;
	options::output::X = false;
	options::output::F = false;
	options::output::L = false;
	options::zFac = 2.0; // So h tE is less than z tE when we call advance_QSS
	options::zrFac = options::zaFac = 1.0;
	options::dtZMax = 0.0;

	std::streambuf * coutBuf( std::cout.rdbuf() ); std::ostringstream strCout; std::cout.rdbuf( strCout.rdbuf() ); // Redirect cout
	allEventIndicators.clear();
	FMU_ME fmu( model );
	fmu.instantiate();
	fmu.pre_simulate();
	fmu.init();
	std::cout.rdbuf( coutBuf ); // Re-redirect cout

	Variable_QSS2 * h( dynamic_cast< Variable_QSS2 * >( fmu.var_named( "h" ) ) );
	Variable_QSS2 * v( dynamic_cast< Variable_QSS2 * >( fmu.var_named( "v" ) ) );
	Variable_ZC2 * z( dynamic_cast< Variable_ZC2 * >( fmu.var_named( "_eventIndicator_1" ) ) );
	if ( ( h == nullptr ) || ( v == nullptr ) || ( z == nullptr ) ) {
		std::cout << ">>>>>>>>>>>> fmu::Variable_ZC2 BouncingBall test not run: Variables h and/or v and/or _eventIndicator_1 not found in FMU" << std::endl;
		return;
	}

	EXPECT_EQ( 1.0, h->rTol );
	EXPECT_EQ( 1.0, h->aTol );
	EXPECT_EQ( 1.0, h->qTol );
	EXPECT_EQ( 0.0, h->tQ );
	EXPECT_EQ( 0.0, h->tX );
	EXPECT_DOUBLE_EQ( std::sqrt( 1.0 / ( 0.5 * 9.80665 ) ), h->tE );
	EXPECT_EQ( 1.0, h->x( 0.0 ) );
	EXPECT_EQ( 1.0, h->q( 0.0 ) );
	EXPECT_EQ( 0.0, h->x1( 0.0 ) );
	EXPECT_EQ( 0.0, h->q1( 0.0 ) );
	EXPECT_DOUBLE_EQ( -9.80665, h->x2( 0.0 ) );

	EXPECT_EQ( 1.0, v->rTol );
	EXPECT_EQ( 1.0, v->aTol );
	EXPECT_EQ( 1.0, v->qTol );
	EXPECT_EQ( 0.0, v->tQ );
	EXPECT_EQ( 0.0, v->tX );
	EXPECT_EQ( infinity, v->tE );
	EXPECT_EQ( 0.0, v->x( 0.0 ) );
	EXPECT_EQ( 0.0, v->q( 0.0 ) );
	EXPECT_EQ( -9.80665, v->x1( 0.0 ) );
	EXPECT_EQ( -9.80665, v->q1( 0.0 ) );
	EXPECT_EQ( 0.0, v->x2( 0.0 ) );

	EXPECT_EQ( 2.0, z->rTol ); // zFac modifies this
	EXPECT_EQ( 2.0, z->aTol ); // zFac modifies this
	EXPECT_EQ( 2.0, z->qTol );
	EXPECT_EQ( 0.0, z->tQ );
	EXPECT_EQ( 0.0, z->tX );
	EXPECT_NEAR( std::sqrt( 2.0 / ( 0.5 * 9.80665 ) ), z->tE, 1e-5 );
	EXPECT_EQ( 1.0, z->x( 0.0 ) );
	EXPECT_EQ( 1.0, z->q( 0.0 ) );
	EXPECT_NEAR( 0.0, z->x1( 0.0 ), 1.0e-9 );
	EXPECT_NEAR( 0.0, z->q1( 0.0 ), 1.0e-9 );
	EXPECT_NEAR( -9.80665, z->x2( 0.0 ), 3e-4 );

	double const h_tE( h->tE );
	fmu.set_time( h->tE );
	h->advance_QSS();

	EXPECT_EQ( h_tE, h->tQ );
	EXPECT_EQ( h_tE, h->tX );
	EXPECT_NEAR( 1.0 - ( 0.5 * 9.80665 ) * square( h_tE ), h->x( h->tX ), 1e-12 );
	EXPECT_NEAR( 1.0 - ( 0.5 * 9.80665 ) * square( h_tE ), h->q( h->tQ ), 1e-12 );

	EXPECT_EQ( 0.0, v->tQ );
	EXPECT_EQ( h_tE, v->tX );

	EXPECT_EQ( h_tE, z->tQ );
	EXPECT_EQ( h_tE, z->tX );
	EXPECT_NEAR( 1.0 - ( 0.5 * 9.80665 ) * square( h_tE ), z->x( z->tX ), 1e-12 );
	EXPECT_NEAR( 1.0 - ( 0.5 * 9.80665 ) * square( h_tE ), z->q( z->tQ ), 1e-12 );

	options::zFac = 1.0; // Reset to common unit testing value
}
