// QSS::dfn::Variable_D Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// QSS Headers
#include <QSS/dfn/Variable_D.hh>

// C++ Headers
#include <algorithm>

using namespace QSS;
using namespace QSS::dfn;

TEST( Variable_DTest, Basic )
{
	Variable_D d( "d" );
	d.init( 42.0 );

	EXPECT_TRUE( d.is_Discrete() );
	EXPECT_EQ( Variable::Cat::Discrete, d.cat() );
	EXPECT_EQ( 0, d.order() );
	EXPECT_EQ( 1.0e-4, d.rTol );
	EXPECT_EQ( 1.0e-6, d.aTol );
	EXPECT_EQ( 0.0, d.tX );
	EXPECT_EQ( 0.0, d.tQ );
	EXPECT_EQ( 42.0, d.x() );
	EXPECT_EQ( 42.0, d.q() );
	EXPECT_EQ( 42.0, d.x( 0.0 ) );
	EXPECT_EQ( 42.0, d.q( 0.0 ) );
	EXPECT_EQ( 0.0, d.x1( 0.0 ) );
	EXPECT_EQ( 0.0, d.q1( 0.0 ) );
	EXPECT_EQ( 0.0, d.x2( 0.0 ) );
	EXPECT_EQ( 0.0, d.q2( 0.0 ) );
	EXPECT_EQ( 0.0, d.x3( 0.0 ) );

	d.advance_handler( 2.0, 99.0 );
	EXPECT_EQ( 2.0, d.tX );
	EXPECT_EQ( 2.0, d.tQ );
	EXPECT_EQ( 99.0, d.x() );
	EXPECT_EQ( 99.0, d.q() );
	EXPECT_EQ( 99.0, d.x( 2.0 ) );
	EXPECT_EQ( 99.0, d.q( 2.0 ) );
	events.clear();
}
