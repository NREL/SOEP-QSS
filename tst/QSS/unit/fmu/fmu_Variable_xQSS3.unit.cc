// QSS::fmu::Variable_xQSS3 Unit Tests
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
#include <QSS/fmu/Variable_xQSS3.hh>
#include <QSS/fmu/EventIndicator.hh>

using namespace QSS;
using namespace QSS::fmu;

TEST( fmu_Variable_xQSS3Test, Basic )
{
	FMU_ME fmu;

	Variable_xQSS3 x1( "x1", 1.0e-4, 1.0e-6, 42.0, &fmu );
	EXPECT_EQ( 1.0e-4, x1.rTol );
	EXPECT_EQ( 1.0e-6, x1.aTol );
	EXPECT_EQ( std::max( x1.rTol * 42.0, x1.aTol ), x1.qTol );
	EXPECT_EQ( 0.0, x1.tQ );

	EXPECT_EQ( 42.0, x1.x( 0.0 ) );
	EXPECT_EQ( 42.0, x1.q( 0.0 ) );
	EXPECT_EQ( 0.0, x1.x1( 0.0 ) );
	EXPECT_EQ( 0.0, x1.q1( 0.0 ) );
	EXPECT_EQ( 0.0, x1.x2( 0.0 ) );
	EXPECT_EQ( 0.0, x1.q2( 0.0 ) );
	EXPECT_EQ( 0.0, x1.x3( 0.0 ) );
	EXPECT_EQ( 0.0, x1.q3( 0.0 ) );

	EXPECT_EQ( 42.0, x1.x( 1.0 ) );
	EXPECT_EQ( 42.0, x1.q( 1.0 ) );
	EXPECT_EQ( 0.0, x1.x1( 1.0 ) );
	EXPECT_EQ( 0.0, x1.q1( 1.0 ) );
	EXPECT_EQ( 0.0, x1.x2( 1.0 ) );
	EXPECT_EQ( 0.0, x1.q2( 1.0 ) );
	EXPECT_EQ( 0.0, x1.x3( 1.0 ) );
	EXPECT_EQ( 0.0, x1.q3( 1.0 ) );

	Variable_xQSS3 x2( "x2", 1.0e-4, 1.0e-3, 99.0, &fmu );
	EXPECT_EQ( 1.0e-4, x2.rTol );
	EXPECT_EQ( 1.0e-3, x2.aTol );
	EXPECT_EQ( std::max( x2.rTol * 99.0, x2.aTol ), x2.qTol );
	EXPECT_EQ( 0.0, x2.tQ );

	EXPECT_EQ( 99.0, x2.x( 0.0 ) );
	EXPECT_EQ( 99.0, x2.q( 0.0 ) );
	EXPECT_EQ( 0.0, x2.x1( 0.0 ) );
	EXPECT_EQ( 0.0, x1.q1( 0.0 ) );
	EXPECT_EQ( 0.0, x1.x2( 0.0 ) );
	EXPECT_EQ( 0.0, x1.q2( 0.0 ) );
	EXPECT_EQ( 0.0, x1.x3( 0.0 ) );
	EXPECT_EQ( 0.0, x1.q3( 0.0 ) );

	EXPECT_EQ( 99.0, x2.x( 1.0 ) );
	EXPECT_EQ( 99.0, x2.q( 1.0 ) );
	EXPECT_EQ( 0.0, x2.x1( 1.0 ) );
	EXPECT_EQ( 0.0, x1.q1( 1.0 ) );
	EXPECT_EQ( 0.0, x1.x2( 1.0 ) );
	EXPECT_EQ( 0.0, x1.q2( 1.0 ) );
	EXPECT_EQ( 0.0, x1.x3( 1.0 ) );
	EXPECT_EQ( 0.0, x1.q3( 1.0 ) );
}

TEST( fmu_Variable_xQSS3Test, Achilles )
{
	std::string const model( "Achilles.fmu" );
	if ( ! path::is_file( model ) ) {
		std::cout << ">>>>>>>>>>>> fmu::Variable_xQSS3 Achilles test not run: Achilles.fmu not present" << std::endl;
		return;
	}

	options::qss = options::QSS::xQSS3;
	options::specified::qss = true;
	options::rTol = 100.0;
	options::specified::rTol = true;
	options::aTol = 1.0;
	options::specified::aTol = options::specified::aTolAll = true;
	options::output::x = false;

	std::streambuf * coutBuf( std::cout.rdbuf() ); std::ostringstream strCout; std::cout.rdbuf( strCout.rdbuf() ); // Redirect cout
	allEventIndicators.clear();
	FMU_ME fmu( model );
	fmu.instantiate();
	fmu.pre_simulate();
	fmu.init();
	std::cout.rdbuf( coutBuf ); // Re-redirect cout

	Variable_xQSS3 * x1( dynamic_cast< Variable_xQSS3 * >( fmu.var_named( "x1" ) ) );
	Variable_xQSS3 * x2( dynamic_cast< Variable_xQSS3 * >( fmu.var_named( "x2" ) ) );
	if ( ( x1 == nullptr ) || ( x2 == nullptr ) ) {
		std::cout << ">>>>>>>>>>>> fmu::Variable_xQSS3 Achilles test not run: Variables x1 and/or x2 not found in FMU" << std::endl;
		return;
	}

	if ( dynamic_cast< Variable_xQSS3 * >( fmu.var_named( "time" ) ) == nullptr ) {
		EXPECT_EQ( 2u, fmu.eventq->size() );
	} else {
		EXPECT_EQ( 3u, fmu.eventq->size() );
	}

	EXPECT_EQ( 100.0, x1->rTol );
	EXPECT_EQ( 1.0, x1->aTol );
	EXPECT_EQ( 0.0, x1->tQ );
	EXPECT_EQ( 0.0, x1->tX );
	EXPECT_NEAR( 1.16966478092703, x1->tE, 1e-9 );
	EXPECT_EQ( 0.0, x1->x( 0.0 ) );
	EXPECT_EQ( 0.0, x1->q( 0.0 ) );
	EXPECT_EQ( 3.0, x1->x1( 0.0 ) );
	EXPECT_EQ( 3.0, x1->q1( 0.0 ) );
	EXPECT_NEAR( -1.5, x1->x2( 0.0 ), 1e-9 );

	EXPECT_EQ( 100.0, x2->rTol );
	EXPECT_EQ( 1.0, x2->aTol );
	EXPECT_EQ( 0.0, x2->tQ );
	EXPECT_EQ( 0.0, x2->tX );
	EXPECT_NEAR( 9.2831776682322467, x2->tE, 1e-9 );
	EXPECT_EQ( 2.0, x2->x( 0.0 ) );
	EXPECT_EQ( 2.0, x2->q( 0.0 ) );
	EXPECT_EQ( 0.0, x2->x1( 0.0 ) );
	EXPECT_EQ( 0.0, x2->q1( 0.0 ) );
	EXPECT_EQ( -3.0, x2->x2( 0.0 ) );

	double const x1_tE( x1->tE );
	fmu.set_time( x1_tE );
	x1->advance_QSS();

	EXPECT_EQ( x1_tE, x1->tQ );
	EXPECT_EQ( x1_tE, x1->tX );
	EXPECT_NEAR( 10.435189430494203, x1->tE, 1e-9 );
	EXPECT_NEAR( 1.482907567831858, x1->x( x1->tX ), 1e-9 );
	EXPECT_NEAR( 1.482907567831858, x1->q( x1->tQ ), 1e-9 );
	EXPECT_NEAR( -0.21962532719589811, x1->x1( x1->tX ), 1e-9 );
	EXPECT_NEAR( -0.21962532719589811, x1->q1( x1->tQ ), 1e-9 );
	EXPECT_NEAR( -3.6145486887462219, x1->x2( x1->tX ), 1e-9 );
	EXPECT_NEAR( -3.6145486887462219, x1->q2( x1->tQ ), 1e-9 );

	EXPECT_EQ( 0.0, x2->tQ );
	EXPECT_EQ( x1_tE, x2->tX );
	EXPECT_NEAR( 7.1622759068143846, x2->tE, 1e-6 );
	EXPECT_NEAR( 0.34788563781335391, x2->x( x2->tX ), 1e-9 );
	EXPECT_EQ( 2.0, x2->q( x2->tQ ) );
	EXPECT_NEAR( -1.482907567831858, x2->x1( x2->tX ), 1e-9 );
	EXPECT_EQ( 0.0, x2->q1( x2->tQ ) );
	EXPECT_NEAR( 0.21962713447898352, x2->x2( x2->tX ), 1e-6 );
	EXPECT_EQ( -3.0, x2->q2( x2->tQ ) );
}
