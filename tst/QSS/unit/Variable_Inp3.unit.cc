// QSS::Variable_Inp3 Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// QSS Headers
#include <QSS/Function_sin.hh>
#include <QSS/Variable_Inp3.hh>

// C++ Headers
#include <algorithm>
#include <cmath>

TEST( Variable_Inp3Test, Basic )
{
	Variable_Inp3< Function_sin > u1( "u1" );
	u1.set_dt_max( 1.0 );
	u1.f().c( 0.05 ).s( 0.5 );
	u1.init();
	EXPECT_EQ( 1.0e-4, u1.rTol );
	EXPECT_EQ( 1.0e-6, u1.aTol );
	EXPECT_EQ( 0.0, u1.f()( 0.0 ) );
	EXPECT_DOUBLE_EQ( 0.025, u1.f().d1( 0.0 ) );
	EXPECT_EQ( 0.0, u1.f().d2( 0.0 ) );
	EXPECT_DOUBLE_EQ( -0.00625, u1.f().d3( 0.0 ) );
	EXPECT_DOUBLE_EQ( 0.0, u1.x( 0.0 ) );
	EXPECT_DOUBLE_EQ( 0.025 - ( 0.00625 / 6.0 ), u1.xn( 1.0 ) );
	EXPECT_DOUBLE_EQ( 0.025, u1.x1( 0.0 ) );
	EXPECT_DOUBLE_EQ( 0.0, u1.x2( 0.0 ) );
	EXPECT_DOUBLE_EQ( -0.00625, u1.x3( 0.0 ) );
	EXPECT_DOUBLE_EQ( 0.0, u1.q( 0.0 ) );
	EXPECT_DOUBLE_EQ( 0.025, u1.qn( 1.0 ) );
	EXPECT_DOUBLE_EQ( 0.025, u1.q1( 0.0 ) );
	EXPECT_EQ( 0.0, u1.tQ );
	EXPECT_DOUBLE_EQ( std::cbrt( 6.0e-6 / 0.00625 ), u1.tE );
	double const u1_tE( u1.tE );
	u1.advance();
	EXPECT_EQ( u1_tE, u1.tQ );
	EXPECT_EQ( 1U, events.size() );
	events.clear();
}
