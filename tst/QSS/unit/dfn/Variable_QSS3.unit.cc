// QSS::dfn::Variable_QSS3 Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// QSS Headers
#include <QSS/dfn/mdl/Function_LTI.hh>
#include <QSS/dfn/Variable_QSS3.hh>

// C++ Headers
#include <algorithm>
#include <cmath>

using namespace QSS;
using namespace QSS::dfn;
using namespace QSS::dfn::mdl;

TEST( Variable_QSS3Test, Basic )
{
	Variable_QSS3< Function_LTI > x1( "x1" );
	x1.d().add( 12.0 ).add( 2.0, x1 );
	x1.init( 2.5 );
	EXPECT_EQ( 1.0e-4, x1.rTol );
	EXPECT_EQ( 1.0e-6, x1.aTol );
	EXPECT_DOUBLE_EQ( 2.5 + 17.0e-6 + 17.0e-12, x1.q( 1.0e-6 ) );
	EXPECT_DOUBLE_EQ( 17.0 + ( 2.0 * 17.0e-6 ), x1.q1( 1.0e-6 ) );
	EXPECT_DOUBLE_EQ( 2.5 + 17.0e-6 + 17.0e-12 + ( 17.0e-18 / 3.0 ), x1.x( 1.0e-6 ) );
	EXPECT_EQ( 0.0, x1.tQ );
	EXPECT_DOUBLE_EQ( std::cbrt( std::max( x1.rTol * 2.5, x1.aTol ) / ( 34.0 / 3.0 ) ), x1.tE );
	double const x1_tE( x1.tE );
	x1.advance_QSS();
	EXPECT_EQ( x1_tE, x1.tQ );

	Variable_QSS3< Function_LTI > x2( "x2", 1.0e-4, 1.0e-3 );
	x2.d().add( 12.0 ).add( 2.0, x2 );
	x2.init( 2.5 );
	EXPECT_EQ( 1.0e-4, x2.rTol );
	EXPECT_EQ( 1.0e-3, x2.aTol );
	EXPECT_EQ( 0.0, x2.tQ );
	EXPECT_DOUBLE_EQ( std::cbrt( std::max( x2.rTol * 2.5, x2.aTol ) / ( 34.0 / 3.0 ) ), x2.tE );

	EXPECT_EQ( 2U, events.size() );
	events.clear();
}
