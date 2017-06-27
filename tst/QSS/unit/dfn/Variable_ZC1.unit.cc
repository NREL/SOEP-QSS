// QSS::dfn::Variable_ZC1 Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// QSS Headers
#include <QSS/dfn/mdl/Function_LTI.hh>
#include <QSS/dfn/Variable_ZC1.hh>
#include <QSS/dfn/Variable_QSS2.hh>
#include <QSS/math.hh>

// C++ Headers
#include <algorithm>

using namespace QSS;
using namespace QSS::dfn;
using namespace QSS::dfn::mdl;

// Zero-Crossing Handler
template< typename V >
class Handler_ZC1Test
{

public: // Types

	using Variable = V;
	using Time = typename Variable::Time;
	using Value = typename Variable::Value;
	using Crossing = typename Variable::Crossing;

public: // Properties

	// Apply at Time t
	void
	operator ()( Time const t, Crossing const crossing )
	{
		if ( crossing <= Crossing::Dn ) { // Downward zero-crossing
			x_->advance_handler( t, 1.0 );
		}
	}

public: // Methods

	// Set Variables
	void
	var( Variable & x )
	{
		x_ = &x;
	}

private: // Data

	Variable * x_{ nullptr };

};

TEST( Variable_ZC1Test, Basic )
{
	Variable_QSS2< Function_LTI > x( "x" );
	x.d().add( -1.0 );
	x.init( 1.0 );
	EXPECT_EQ( 1.0e-4, x.rTol );
	EXPECT_EQ( 1.0e-6, x.aTol );
	EXPECT_EQ( 1.0, x.x( 0.0 ) );
	EXPECT_EQ( 1.0, x.q( 0.0 ) );
	EXPECT_DOUBLE_EQ( 1.0 - 1.0e-7, x.x( 1.0e-7 ) );
	EXPECT_DOUBLE_EQ( 1.0 - 1.0e-7, x.q( 1.0e-7 ) );
	EXPECT_EQ( infinity, x.tE );

	Variable_ZC1< Function_LTI, Handler_ZC1Test > z( "z", 2.0, 2.0 ); // Big tolerances to make tE > tZ
	z.add_crossings_Dn();
	z.f().add( x );
	z.h().var( x );
	z.init();
	EXPECT_EQ( 2.0, z.rTol );
	EXPECT_EQ( 2.0, z.aTol );
	EXPECT_EQ( 1.0, z.x( 0.0 ) );
	EXPECT_EQ( 1.0, z.q( 0.0 ) );
	EXPECT_DOUBLE_EQ( 1.0 - 1.0e-7, z.x( 1.0e-7 ) );
	EXPECT_DOUBLE_EQ( 1.0, z.q( 1.0e-7 ) );
	EXPECT_DOUBLE_EQ( 0.0 + std::max( z.rTol * 1.0, z.aTol ) / 1.0, z.tE );
	EXPECT_DOUBLE_EQ( 1.0, z.tZ );
	EXPECT_EQ( Variable::Crossing::DnPN, z.crossing );

	EXPECT_DOUBLE_EQ( 0.0, x.x( 1.0 ) );
	EXPECT_DOUBLE_EQ( 0.0, x.q( 1.0 ) );
	z.advance_ZC();
	EXPECT_EQ( 1.0, x.x( 1.0 ) );
	EXPECT_EQ( 1.0, x.q( 1.0 ) );

	events.clear();
}
