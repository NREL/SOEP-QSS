// QSS::Variable_nLIQSS3 Unit Tests
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2025 Objexx Engineering, Inc. All rights reserved.
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
#include <QSS/Variable_nLIQSS3.hh>
#include <QSS/EventIndicators.hh>

using namespace QSS;

TEST( Variable_nLIQSS3Test, Basic )
{
	FMU_ME fmu;

	Variable_nLIQSS3 x1( &fmu, "x1", 1.0e-4, 1.0e-6, 0.0, 42.0 );
	EXPECT_EQ( 1.0e-4, x1.rTol );
	EXPECT_EQ( 1.0e-6, x1.aTol );
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

	Variable_nLIQSS3 x2( &fmu, "x2", 1.0e-4, 1.0e-3, 0.0, 99.0 );
	EXPECT_EQ( 1.0e-4, x2.rTol );
	EXPECT_EQ( 1.0e-3, x2.aTol );
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

TEST( Variable_nLIQSS3Test, Achilles )
{
	std::string const model( "Achilles.fmu" );
	if ( !path::is_file( model ) ) {
		std::cout << ">>>>>>>>>>>> Variable_nLIQSS3 Achilles test not run: Achilles.fmu not present" << std::endl;
		return;
	}

	options::qss = options::QSS::nLIQSS3;
	options::specified::qss = true;
	options::rTol = 100.0;
	options::specified::rTol = true;
	options::aTol = 1.0;
	options::specified::aTol = true;
	options::output::X = false;

	std::streambuf * coutBuf( std::cout.rdbuf() ); std::ostringstream strCout; std::cout.rdbuf( strCout.rdbuf() ); // Redirect cout
	all_eventindicators.clear();
	all_dependencies.clear();
	FMU_ME fmu( model );
	fmu.instantiate();
	fmu.pre_simulate();
	fmu.init();
	std::cout.rdbuf( coutBuf ); // Re-redirect cout

	Variable_nLIQSS3 * x1( dynamic_cast< Variable_nLIQSS3 * >( fmu.var_named( "x1" ) ) );
	Variable_nLIQSS3 * x2( dynamic_cast< Variable_nLIQSS3 * >( fmu.var_named( "x2" ) ) );
	if ( ( x1 == nullptr ) || ( x2 == nullptr ) ) {
		std::cout << ">>>>>>>>>>>> Variable_nLIQSS3 Achilles test not run: Variables x1 and/or x2 not found in FMU" << std::endl;
		return;
	}

	EXPECT_EQ( 3u, fmu.eventq->size() );

	EXPECT_EQ( 100.0, x1->rTol );
	EXPECT_EQ( 1.0, x1->aTol );
	EXPECT_EQ( 0.0, x1->tQ );
	EXPECT_EQ( 0.0, x1->tX );
	EXPECT_NEAR( 1.19683211408102, x1->tE, 1e-9 );
	EXPECT_EQ( -1.0, x1->q( 0.0 ) );
	EXPECT_EQ( 0.0, x1->x( 0.0 ) );
	EXPECT_EQ( 3.5, x1->q1( 0.0 ) );
	EXPECT_EQ( 3.5, x1->x1( 0.0 ) );
	EXPECT_NEAR( 2 * -0.8750009374836054, x1->q2( 0.0 ), 1e-9 );
	EXPECT_NEAR( 2 * -0.8750009374836054, x1->x2( 0.0 ), 1e-9 );
	EXPECT_NEAR( 6 * -0.5833111771380572, x1->x3( 0.0 ), 1e-9 );

	EXPECT_EQ( 100.0, x2->rTol );
	EXPECT_EQ( 1.0, x2->aTol );
	EXPECT_EQ( 0.0, x2->tQ );
	EXPECT_EQ( 0.0, x2->tX );
	EXPECT_NEAR( 0.001062658569183231, x2->tE, 1e-9 );
	EXPECT_EQ( -198.0, x2->q( 0.0 ) );
	EXPECT_EQ( 2.0, x2->x( 0.0 ) );
	EXPECT_EQ( 0.0, x2->q1( 0.0 ) );
	EXPECT_EQ( 0.0, x2->x1( 0.0 ) );
	EXPECT_NEAR( 2 * -1.499999625, x2->q2( 0.0 ), 1e-9 );
	EXPECT_NEAR( 2 * -1.499999625, x2->x2( 0.0 ), 1e-9 );
	EXPECT_NEAR( 6 * -166666666666.375, x2->x3( 0.0 ), 1e-9 );

	double const tE( x2->tE );
	fmu.set_time( tE );
	x2->advance_QSS();

	EXPECT_EQ( 0.0, x1->tQ );
	EXPECT_EQ( tE, x1->tX );
	EXPECT_NEAR( 0.001096672174328209, x1->tE, 1e-6 );
	EXPECT_EQ( -1.0, x1->q( x1->tQ ) );
	EXPECT_NEAR( 0.003718316203278922, x1->x( x1->tX ), 1e-9 );
	EXPECT_EQ( 3.5, x1->q1( x1->tQ ) );
	EXPECT_NEAR( 29403.4983923804, x1->x1( x1->tX ), 1e-9 );
	EXPECT_NEAR( 2 * -0.8750009374836054, x1->q2( x1->tQ ), 1e-7 );
	EXPECT_NEAR( 2 * -0.1273238012799993, x1->x2( x1->tX ), 1e-7 );
	EXPECT_EQ( 6 * 0.0, x1->x3( x1->tX ) );

	EXPECT_EQ( tE, x2->tQ );
	EXPECT_EQ( tE, x2->tX );
	EXPECT_NEAR( 40.79525104547013, x2->tE, 1e-9 );
	EXPECT_NEAR( 19602.00016769257, x2->q( x2->tQ ), 1e-9 );
	EXPECT_NEAR( -198.0000016938644, x2->x( x2->tX ), 1e-9 );
	EXPECT_NEAR( 0.9962816830967477, x2->q1( x2->tQ ), 1e-9 );
	EXPECT_NEAR( 0.9962816830967477, x2->x1( x2->tX ), 1e-9 );
	EXPECT_NEAR( 2 * -1.749070172751832, x2->q2( x2->tQ ), 1e-7 );
	EXPECT_NEAR( 2 * -1.749070172751832, x2->x2( x2->tX ), 1e-7 );
	EXPECT_NEAR( 6 * 0.2916555885690286, x2->x3( x2->tX ), 1e-7 );
}
