// QSS::Variable_LIQSS2 Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// QSS Headers
#include <QSS/Function_LTI.hh>
#include <QSS/Variable_LIQSS2.hh>

// C++ Headers
#include <algorithm>
#include <cmath>

TEST( Variable_LIQSS2Test, Basic )
{
	Variable_LIQSS2< Function_LTI > x1( "x1" );
	x1.d().add( 12.0 ).add( 2.0, x1 );
	x1.init( 2.5 );
	EXPECT_EQ( 1.0e-4, x1.rTol );
	EXPECT_EQ( 1.0e-6, x1.aTol );
	EXPECT_DOUBLE_EQ( 2.5e-4, x1.qTol );
	EXPECT_DOUBLE_EQ( 2.5 + 2.5e-4, x1.q( 0.0 ) );
	EXPECT_DOUBLE_EQ( 17.0005, x1.q1( 0.0 ) );
	EXPECT_DOUBLE_EQ( 2.5, x1.x( 0.0 ) );
	EXPECT_DOUBLE_EQ( 17.0005, x1.x1( 0.0 ) );
	EXPECT_EQ( 0.0, x1.tQ );
	EXPECT_DOUBLE_EQ( std::sqrt( std::max( x1.rTol * 2.5, x1.aTol ) / 17.0005 ), x1.tE );
	double const x1_tE( x1.tE );
	x1.advance();
	EXPECT_EQ( x1_tE, x1.tQ );

	Variable_LIQSS2< Function_LTI > x2( "x2", 1.0e-4, 1.0e-3 );
	x2.d().add( 12.0 ).add( 2.0, x2 );
	x2.init( 2.5 );
	EXPECT_EQ( 1.0e-4, x2.rTol );
	EXPECT_EQ( 1.0e-3, x2.aTol );
	EXPECT_DOUBLE_EQ( 1.0e-3, x2.qTol );
	EXPECT_DOUBLE_EQ( 2.501, x2.q( 0.0 ) );
	EXPECT_EQ( 0.0, x2.tQ );
	EXPECT_DOUBLE_EQ( std::sqrt( std::max( x2.rTol * 2.5, x2.aTol ) / 17.002 ), x2.tE );
	x2.tE = 2.0; // To allow advance to 1.0
	x2.advance( 1.0 );
	EXPECT_EQ( 1.0, x2.tX );
	EXPECT_DOUBLE_EQ( 36.504, x2.x( x2.tX ) );
	EXPECT_DOUBLE_EQ( 51.006, x2.x1( x2.tX ) );
	EXPECT_DOUBLE_EQ( 34.004, x2.x2( x2.tX ) );

	EXPECT_EQ( 2U, events.size() );
	events.clear();
}
