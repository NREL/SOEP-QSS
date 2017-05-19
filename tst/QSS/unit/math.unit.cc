// QSS::math Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// QSS Headers
#include <QSS/math.hh>

// C++ Headers
#include <algorithm>
#include <cmath>

using namespace QSS;

TEST( MathTest, Sign )
{
	EXPECT_EQ( 1.0, sign( 3.0 ) );
	EXPECT_EQ( -1.0, sign( -5.0 ) );
	EXPECT_EQ( 1.0, sign( 0.0 ) );
	EXPECT_EQ( -1.0, sign( -0.0 ) );
}

TEST( MathTest, Sgn )
{
	EXPECT_EQ( 1.0, sgn( 3.0 ) );
	EXPECT_EQ( -1.0, sgn( -5.0 ) );
	EXPECT_EQ( 0.0, sgn( 0.0 ) );
	EXPECT_EQ( 0.0, sgn( -0.0 ) );
	EXPECT_EQ( 1, sgn( 3 ) );
	EXPECT_EQ( -1, sgn( -5 ) );
	EXPECT_EQ( 0, sgn( 0 ) );
	EXPECT_EQ( 0, sgn( -0 ) );
}

TEST( MathTest, Signum )
{
	EXPECT_EQ( 1, sgn( 3.0 ) );
	EXPECT_EQ( -1, sgn( -5.0 ) );
	EXPECT_EQ( 0, sgn( 0.0 ) );
	EXPECT_EQ( 0, sgn( -0.0 ) );
	EXPECT_EQ( 1, sgn( 3 ) );
	EXPECT_EQ( -1, sgn( -5 ) );
	EXPECT_EQ( 0, sgn( 0 ) );
	EXPECT_EQ( 0, sgn( -0 ) );
}

TEST( MathTest, Square )
{
	EXPECT_EQ( 9.0, square( 3.0 ) );
	EXPECT_EQ( 4, square( 2 ) );
}

TEST( MathTest, Cube )
{
	EXPECT_EQ( 27.0, cube( 3.0 ) );
	EXPECT_EQ( 8, cube( 2 ) );
}

TEST( MathTest, MinRootQuadraticLower )
{
	EXPECT_DOUBLE_EQ( 0.32287565553229536, min_root_quadratic_lower( -4.0, -8.0, 3.0 ) );
	EXPECT_DOUBLE_EQ( 0.87082869338697070, min_root_quadratic_lower( -2.0, -4.0, 5.0 ) );
	EXPECT_DOUBLE_EQ( 0.42214438511238006, min_root_quadratic_lower( -2.0, -11.0, 5.0 ) );
	EXPECT_DOUBLE_EQ( 0.0, min_root_quadratic_lower( -2.0, -11.0, -0.001 ) );
}

TEST( MathTest, MinRootQuadraticUpper )
{
	EXPECT_DOUBLE_EQ( 0.32287565553229536, min_root_quadratic_upper( 4.0, 8.0, -3.0 ) );
	EXPECT_DOUBLE_EQ( 0.87082869338697070, min_root_quadratic_upper( 2.0, 4.0, -5.0 ) );
	EXPECT_DOUBLE_EQ( 0.42214438511238006, min_root_quadratic_upper( 2.0, 11.0, -5.0 ) );
	EXPECT_DOUBLE_EQ( 0.0, min_root_quadratic_upper( 2.0, 11.0, 0.001 ) );
}

TEST( MathTest, MinRootQuadraticBoth )
{
	EXPECT_DOUBLE_EQ( 0.5, min_root_quadratic_both( -4.0, 8.0, 3.0, -3.0 ) );
	EXPECT_DOUBLE_EQ( 0.8708286933869707, min_root_quadratic_both( 2.0, 4.0, 5.0, -5.0 ) );
	EXPECT_DOUBLE_EQ( 0.5, min_root_quadratic_both( -2.0, 11.0, 5.0, -5.0 ) );
}

TEST( MathTest, MinRootCubic )
{
	EXPECT_DOUBLE_EQ( 1.359787450380789, min_root_cubic_both( -2.0, 3.0, -7.0, 9.0, 9.0 ) );
	EXPECT_DOUBLE_EQ( 1.4175965758288351, min_root_cubic_both( -2.0, 4.0, -8.0, 9.0, 9.0 ) );
	EXPECT_NEAR( 0.29037158997385715, min_root_cubic_both( -9.0, 3.0, -7.0, 2.0, 2.0 ), 1.0e-15 );
	EXPECT_DOUBLE_EQ( 1.060647778684131, min_root_cubic_both( -9.0, 3.0, 6.0, 1.0, 1.0 ) );
	EXPECT_NEAR( 0.7073498763104409, min_root_cubic_lower( -2.25, -6.5, -7.0, 9.0 ), 1.0e-14 );
	EXPECT_DOUBLE_EQ( 0.0, min_root_cubic_lower( -2.25, -6.5, -7.0, -0.01 ) ); // d < 0 => Precision loss
	EXPECT_NEAR( 0.7073498763104409, min_root_cubic_upper( 2.25, 6.5, 7.0, -9.0 ), 1.0e-14 );
	EXPECT_DOUBLE_EQ( 0.0, min_root_cubic_upper( 2.25, 6.5, 7.0, 0.01 ) ); // d > 0 => Precision loss
}
