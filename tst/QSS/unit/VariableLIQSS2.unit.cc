// QSS::VariableLIQSS2 Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// QSS Headers
#include <QSS/FunctionLTI_LIQSS.hh>
#include <QSS/VariableLIQSS2.hh>

// C++ Headers
#include <algorithm>
#include <cmath>

TEST( VariableLIQSS2Test, Basic )
{
	VariableLIQSS2< FunctionLTI_LIQSS > x1( "x1" );
	x1.init0( 2.5 );
	x1.d().add( 12.0 ).add( 2.0, x1 );
	x1.init1();
	x1.init2_LIQSS();
	x1.init_event();
	EXPECT_EQ( 1.0e-6, x1.aTol );
	EXPECT_EQ( 1.0e-6, x1.rTol );
	EXPECT_DOUBLE_EQ( 2.5e-6, x1.qTol );
	EXPECT_DOUBLE_EQ( 2.5 + 2.5e-6, x1.q() );
	EXPECT_DOUBLE_EQ( 17.000005, x1.q1() );
	EXPECT_DOUBLE_EQ( 2.5, x1.x() );
	EXPECT_DOUBLE_EQ( 17.000005, x1.x1() );
	EXPECT_EQ( 0.0, x1.tQ );
	EXPECT_DOUBLE_EQ( std::sqrt( std::max( x1.aTol, x1.rTol * 2.5 ) / 17.000005 ), x1.tE );
	double const x1_tE( x1.tE );
	x1.advance();
	EXPECT_EQ( x1_tE, x1.tQ );

	VariableLIQSS2< FunctionLTI_LIQSS > x2( "x2", 1.0e-3, 1.0e-4 );
	x2.init0( 2.5 );
	x2.d().add( 12.0 ).add( 2.0, x2 );
	x2.init1();
	x2.init2_LIQSS();
	x2.init_event();
	EXPECT_EQ( 1.0e-3, x2.aTol );
	EXPECT_EQ( 1.0e-4, x2.rTol );
	EXPECT_DOUBLE_EQ( 1.0e-3, x2.qTol );
	EXPECT_DOUBLE_EQ( 2.501, x2.q() );
	EXPECT_EQ( 0.0, x2.tQ );
	EXPECT_DOUBLE_EQ( std::sqrt( std::max( x2.aTol, x2.rTol * 2.5 ) / 17.002 ), x2.tE );
	x2.tE = 2.0; // To allow advance to 1.0
	x2.advance( 1.0 );
	EXPECT_EQ( 1.0, x2.tX );
	EXPECT_DOUBLE_EQ( 36.504, x2.x0() );
	EXPECT_DOUBLE_EQ( 51.006, x2.x1() );
	EXPECT_DOUBLE_EQ( 17.002, x2.x2() );

	EXPECT_EQ( 2U, events.size() );
	events.clear();
}
