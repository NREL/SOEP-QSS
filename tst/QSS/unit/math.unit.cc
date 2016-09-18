// QSS::math Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// QSS Headers
#include <QSS/math.hh>

// C++ Headers
#include <algorithm>
#include <cmath>

TEST( MathTest, MinRootQuadratic )
{
	EXPECT_DOUBLE_EQ( 2.3228756555322954, min_root_quadratic( -4.0, 8.0, 3.0 ) );
	EXPECT_DOUBLE_EQ( 2.8708286933869704, min_root_quadratic( -2.0, 4.0, 5.0 ) );
	EXPECT_DOUBLE_EQ( 5.9221443851123805, min_root_quadratic( -2.0, 11.0, 5.0 ) );
}

TEST( MathTest, MinRootCubic )
{
	EXPECT_DOUBLE_EQ( 1.359787450380789, min_root_cubic( -2.0, 3.0, -7.0, 9.0 ) );
	EXPECT_DOUBLE_EQ( 1.4175965758288351, min_root_cubic( -2.0, 4.0, -8.0, 9.0 ) );
	EXPECT_DOUBLE_EQ( 0.29037158997385715, min_root_cubic( -9.0, 3.0, -7.0, 2.0 ) );
	EXPECT_DOUBLE_EQ( 1.060647778684131, min_root_cubic( -9.0, 3.0, 6.0, 1.0 ) );
}
