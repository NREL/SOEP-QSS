// QSS::dfn::Variable_Inp1 Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// QSS Headers
#include <QSS/dfn/mdl/Function_sin.hh>
#include <QSS/dfn/Variable_Inp1.hh>

// C++ Headers
#include <algorithm>

using namespace QSS;
using namespace QSS::dfn;
using namespace QSS::dfn::mdl;

TEST( Variable_Inp1Test, Basic )
{
	Variable_Inp1< Function_sin > u( "u" );
	u.f().c( 0.05 ).s( 0.5 );
	u.init();
	EXPECT_EQ( 1.0e-4, u.rTol );
	EXPECT_EQ( 1.0e-6, u.aTol );
	EXPECT_EQ( 0.0, u.f()( 0.0 ) );
	EXPECT_DOUBLE_EQ( 0.025, u.f().d1( 0.0 ) );
	EXPECT_EQ( 0.0, u.f().d2( 0.0 ) );
	EXPECT_DOUBLE_EQ( -0.00625, u.f().d3( 0.0 ) );
	EXPECT_DOUBLE_EQ( 0.0, u.x( 0.0 ) );
	EXPECT_DOUBLE_EQ( 0.025, u.x( 1.0 ) );
	EXPECT_DOUBLE_EQ( 0.025, u.x1( 0.0 ) );
	EXPECT_DOUBLE_EQ( 0.0, u.q( 0.0 ) );
	EXPECT_DOUBLE_EQ( 0.0, u.q( 1.0 ) );
	EXPECT_EQ( 0.0, u.tQ );
	EXPECT_DOUBLE_EQ( std::max( u.rTol * 0.0, u.aTol ) / 0.025, u.tE );
	double const u_tE( u.tE );
	u.advance_QSS();
	EXPECT_EQ( u_tE, u.tQ );
	EXPECT_EQ( 1U, events.size() );
	events.clear();
}
