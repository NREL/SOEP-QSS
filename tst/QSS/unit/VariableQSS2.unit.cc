// QSS::VariableQSS2 Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// QSS Headers
#include <QSS/FunctionLTI.hh>
#include <QSS/VariableQSS2.hh>

// C++ Headers
#include <algorithm>
#include <cmath>

TEST( VariableQSS2Test, Basic )
{
	VariableQSS2< FunctionLTI > x1( "x1" );
	x1.d().add( 12.0 ).add( 2.0, x1 );
	x1.init( 2.5 );
	EXPECT_EQ( 1.0e-6, x1.aTol );
	EXPECT_EQ( 1.0e-6, x1.rTol );
	EXPECT_DOUBLE_EQ( 2.5 + 17.0e-6, x1.q( 1.0e-6 ) );
	EXPECT_EQ( 17.0, x1.q1( 1.0e-6 ) );
	EXPECT_DOUBLE_EQ( 2.5 + 17.0e-6 + 17.0e-12, x1.x( 1.0e-6 ) );
	EXPECT_EQ( 0.0, x1.tQ );
	EXPECT_DOUBLE_EQ( std::sqrt( std::max( x1.aTol, x1.rTol * 2.5 ) / 17.0 ), x1.tE );
	double const x1_tE( x1.tE );
	x1.advance();
	EXPECT_EQ( x1_tE, x1.tQ );

	VariableQSS2< FunctionLTI > x2( "x2", 1.0e-3, 1.0e-4 );
	x2.d().add( 12.0 ).add( 2.0, x2 );
	x2.init( 2.5 );
	EXPECT_EQ( 1.0e-3, x2.aTol );
	EXPECT_EQ( 1.0e-4, x2.rTol );
	EXPECT_EQ( 0.0, x2.tQ );
	EXPECT_DOUBLE_EQ( std::sqrt( std::max( x2.aTol, x2.rTol * 2.5 ) / 17.0 ), x2.tE );

	EXPECT_EQ( 2U, events.size() );
	events.clear();
}
