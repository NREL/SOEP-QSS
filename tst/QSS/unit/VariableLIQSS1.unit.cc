// QSS::VariableLIQSS1 Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// QSS Headers
#include <QSS/FunctionLIQSSLTI.hh>
#include <QSS/VariableLIQSS1.hh>

// C++ Headers
#include <algorithm>

TEST( VariableLIQSS1Test, Basic )
{
	VariableLIQSS1< FunctionLIQSSLTI > x1( "x1" );
	x1.init0( 2.5 );
	x1.d().add( 12.0 ).add( 2.0, x1 );
	x1.init1();
	x1.init_event();
	EXPECT_EQ( 1.0e-6, x1.aTol );
	EXPECT_EQ( 1.0e-6, x1.rTol );
	EXPECT_DOUBLE_EQ( 2.5 + 2.5e-6, x1.q( 1.0e-7 ) );
	EXPECT_EQ( 0.0, x1.q1( 1.0e-7 ) );
	EXPECT_DOUBLE_EQ( 2.5 + ( ( 17.0 + 5.0e-6 ) * 1.0e-7 ), x1.x( 1.0e-7 ) );
	EXPECT_EQ( 0.0, x1.tQ );
	EXPECT_DOUBLE_EQ( std::max( x1.aTol, x1.rTol * 2.5 ) / ( 17.0 + 5.0e-6 ), x1.tE );
	double const x1_tEnd( x1.tE );
	x1.advance();
	EXPECT_EQ( x1_tEnd, x1.tQ );

	VariableLIQSS1< FunctionLIQSSLTI > x2( "x2", 1.0e-3, 1.0e-4 );
	x2.init0( 2.5 );
	x2.d().add( 12.0 ).add( 2.0, x2 ).finalize( x2 );
	x2.init1();
	x2.init_event();
	EXPECT_EQ( 1.0e-3, x2.aTol );
	EXPECT_EQ( 1.0e-4, x2.rTol );
	EXPECT_EQ( 0.0, x2.tQ );
	EXPECT_DOUBLE_EQ( std::max( x2.aTol, x2.rTol * 2.5 ) / 17.002, x2.tE );

	EXPECT_EQ( 2U, events.size() );
	events.clear();
}
