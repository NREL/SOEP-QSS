// QSS::math Unit Tests
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2025 Objexx Engineering, Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
// (3) Neither the name of the copyright holder nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, THE UNITED STATES
// GOVERNMENT, OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Google Test Headers
#include <gtest/gtest.h>

// QSS Headers
#include <QSS/math.hh>

using namespace QSS;

TEST( MathTest, BoolSign )
{
	EXPECT_TRUE( bool_sign( 0.0 ) );
	EXPECT_TRUE( bool_sign( -0.0 ) );
	EXPECT_TRUE( bool_sign( 3.0 ) );
	EXPECT_FALSE( bool_sign( -5.0 ) );
}

TEST( MathTest, SignsSame )
{
	EXPECT_TRUE( signs_same( 3.0, 5.0 ) );
	EXPECT_TRUE( signs_same( -5.0, -2.0 ) );
	EXPECT_TRUE( signs_same( 0, 0 ) );
	EXPECT_TRUE( signs_same( 0.0, -0.0 ) );
	EXPECT_FALSE( signs_same( 3.0, -5.0 ) );
	EXPECT_FALSE( signs_same( -3.0, 5.0 ) );
}

TEST( MathTest, SignsDiffer )
{
	EXPECT_FALSE( signs_differ( 3.0, 5.0 ) );
	EXPECT_FALSE( signs_differ( -5.0, -2.0 ) );
	EXPECT_FALSE( signs_differ( 0, 0 ) );
	EXPECT_FALSE( signs_differ( 0.0, -0.0 ) );
	EXPECT_TRUE( signs_differ( 3.0, -5.0 ) );
	EXPECT_TRUE( signs_differ( -3.0, 5.0 ) );
}

TEST( MathTest, NonZeroAndSignsDiffer )
{
	EXPECT_FALSE( nonzero_and_signs_differ( 3.0, 5.0 ) );
	EXPECT_FALSE( nonzero_and_signs_differ( -5.0, -2.0 ) );
	EXPECT_FALSE( nonzero_and_signs_differ( 0, 0 ) );
	EXPECT_FALSE( nonzero_and_signs_differ( 0.0, -0.0 ) );
	EXPECT_FALSE( nonzero_and_signs_differ( 5.0, -0.0 ) );
	EXPECT_TRUE( nonzero_and_signs_differ( 3.0, -5.0 ) );
	EXPECT_TRUE( nonzero_and_signs_differ( -3.0, 5.0 ) );
}

TEST( MathTest, Sign )
{
	EXPECT_EQ( 1.0, sign( 3.0 ) );
	EXPECT_EQ( -1.0, sign( -5.0 ) );
	EXPECT_EQ( 1.0, sign( 0.0 ) );
	EXPECT_EQ( 1.0, sign( -0.0 ) );
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

TEST( MathTest, Quad )
{
	EXPECT_EQ( 81.0, quad( 3.0 ) );
	EXPECT_EQ( 16, quad( 2 ) );
}

TEST( MathTest, Min )
{
	EXPECT_EQ( 3.0, min( 44.0, 77.0, 3.0 ) );
	EXPECT_EQ( 5, min( 8, 99, 38, 5 ) );
	EXPECT_EQ( 5, min( 8, 99, 38, 5, 373 ) );
	EXPECT_EQ( 4, min( 8, 99, 38, 5, 373, 4 ) );

	EXPECT_EQ( 3.0, min_nonnegative_or_zero( 77.0, 3.0 ) );
	EXPECT_EQ( 77.0, min_nonnegative_or_zero( 77.0, -3.0 ) );
	EXPECT_EQ( 0.0, min_nonnegative_or_zero( -77.0, -3.0 ) );

	EXPECT_EQ( 3.0, min_positive_or_infinity( 77.0, 3.0 ) );
	EXPECT_EQ( 77.0, min_positive_or_infinity( 77.0, -3.0 ) );
	EXPECT_EQ( infinity, min_positive_or_infinity( -77.0, -3.0 ) );

	EXPECT_EQ( 3.0, min_positive_or_infinity( 77.0, 3.0, -22.0 ) );
	EXPECT_EQ( 77.0, min_positive_or_infinity( 77.0, -3.0, 96.0 ) );
	EXPECT_EQ( infinity, min_positive_or_infinity( -77.0, -3.0, -2.0 ) );
}

TEST( MathTest, Max )
{
	EXPECT_EQ( 77.0, max( 44.0, 77.0, 3.0 ) );
	EXPECT_EQ( 373, max( 8, 99, 38, 373 ) );
	EXPECT_EQ( 373, max( 8, 99, 38, 5, 373 ) );
	EXPECT_EQ( 555, max( 8, 99, 38, 5, 555, 373 ) );
}

TEST( MathTest, Nonnegative )
{
	EXPECT_EQ( 42, nonnegative( 42 ) );
	EXPECT_EQ( 0, nonnegative( -42 ) );
	EXPECT_EQ( 44.0, nonnegative( 44.0 ) );
	EXPECT_EQ( 0.0, nonnegative( -44.0 ) );
}

TEST( MathTest, Infinityish )
{
	EXPECT_EQ( infinity, inf< double >() );
	EXPECT_EQ( std::numeric_limits< int >::max(), inf< int >() );
}

TEST( MathTest, PositiveOrInfinity )
{
	EXPECT_EQ( 3.0, positive_or_infinity( 3.0 ) );
	EXPECT_EQ( infinity, positive_or_infinity( -5.0 ) );
	EXPECT_EQ( infinity, positive_or_infinity( 0.0 ) );
	EXPECT_EQ( 2, positive_or_infinity( 2 ) );
}

TEST( MathTest, SortedPositive )
{
	using AD3 = std::array< double, 3 >;
	EXPECT_EQ( AD3( { 1.0, 2.0, 3.0 } ), sorted_positive( 1.0, 2.0, 3.0 ) );
	EXPECT_EQ( AD3( { infinity, infinity, infinity } ), sorted_positive( -1.0, -2.0, -3.0 ) );
	EXPECT_EQ( AD3( { 1.0, infinity, infinity } ), sorted_positive( 1.0, 0.0, -1.0 ) );
	EXPECT_EQ( AD3( { 2.0, 3.0, 5.0 } ), sorted_positive( 3.0, 2.0, 5.0 ) );
	EXPECT_EQ( AD3( { 2.0, 3.0, 5.0 } ), sorted_positive( 5.0, 3.0, 2.0 ) );
}

TEST( MathTest, ZCRootCull )
{
	EXPECT_EQ( 3.0, zc_root_cull( 3.0, 1.0e-6, 1.0e-6 ) );
	EXPECT_EQ( 3.0, zc_root_cull( 3.0, 1.0e-5, 1.0e-6 ) );
	EXPECT_EQ( -3.0, zc_root_cull( -3.0, 1.0e-5, 1.0e-5 ) );
	EXPECT_EQ( inf< double >(), zc_root_cull( 3.0, 1.0e-7 ) );
	EXPECT_EQ( inf< double >(), zc_root_cull( 3.0, 1.0e-7, 1.0e-6 ) );
	EXPECT_EQ( 3.0, zc_root_cull( 3.0, -1.0e-5, 1.0e-6 ) );
	EXPECT_EQ( inf< double >(), zc_root_cull( 3.0, -1.0e-7 ) );
}

TEST( MathTest, ZCRootCullMag )
{
	EXPECT_EQ( 3.0, zc_root_cull_mag( 3.0, 1.0e-6, 1.0e-6 ) );
	EXPECT_EQ( 3.0, zc_root_cull_mag( 3.0, 1.0e-5, 1.0e-6 ) );
	EXPECT_EQ( -3.0, zc_root_cull_mag( -3.0, 1.0e-5, 1.0e-5 ) );
	EXPECT_EQ( inf< double >(), zc_root_cull_mag( 3.0, 1.0e-7 ) );
	EXPECT_EQ( inf< double >(), zc_root_cull_mag( 3.0, 1.0e-7, 1.0e-6 ) );
}

TEST( MathTest, RootClass )
{
	{
	Root< double > root;
	EXPECT_EQ( 0.0, root.x );
	EXPECT_EQ( 0.0, root.v );
	EXPECT_FALSE( root.valid );
	EXPECT_FALSE( bool( root ) );
	}

	{
	Root< double > root( 3.0, -0.001 );
	EXPECT_EQ( 3.0, root.x );
	EXPECT_EQ( -0.001, root.v );
	EXPECT_TRUE( root.valid );
	EXPECT_TRUE( bool( root ) );
	}

	{
	Root< double > root( 3.0, 0.001, 0.01 );
	EXPECT_EQ( 3.0, root.x );
	EXPECT_EQ( 0.001, root.v );
	EXPECT_TRUE( root.valid );
	EXPECT_TRUE( bool( root ) );
	}

	{
	Root< double > root( 3.0, -0.002, 0.001 );
	EXPECT_EQ( 3.0, root.x );
	EXPECT_EQ( -0.002, root.v );
	EXPECT_FALSE( root.valid );
	EXPECT_FALSE( bool( root ) );
	}
}

TEST( MathTest, ZCRootLinear )
{
	double const inf_double( inf< double >() );
	EXPECT_EQ( inf_double, zc_root_linear( 0.0, 3.0 ) );
	EXPECT_EQ( inf_double, zc_root_linear( -0.0, -3.0 ) );
	EXPECT_EQ( inf_double, zc_root_linear( 3.0, 0.0 ) );
	EXPECT_EQ( inf_double, zc_root_linear( -3.0, -0.0 ) );
	EXPECT_EQ( inf_double, zc_root_linear( 3.0, 5.0 ) );
	EXPECT_EQ( inf_double, zc_root_linear( -3.0, -2.0 ) );
	EXPECT_EQ( 2.0, zc_root_linear( 3.0, -6.0 ) );
	EXPECT_EQ( 2.0, zc_root_linear( -3.0, 6.0 ) );
}

TEST( MathTest, NewtonPositiveRootQuadratic )
{
	EXPECT_DOUBLE_EQ( 1.0 + std::sqrt( 2.5 ), newton_positive_root_quadratic( 2.0, -4.0, -3.0, 2.5 ).x );
	EXPECT_DOUBLE_EQ( 5.0 / 3.0, newton_positive_root_quadratic( -3.0, 5.0, 0.0, 1.5 ).x );
	EXPECT_EQ( 0.0, newton_positive_root_quadratic( 0.0, 0.0, 2.0, 1.5 ).x );
	EXPECT_EQ( 0.0, newton_positive_root_quadratic( 0.0, 5.0, 2.0, 1.5 ).x );
	EXPECT_FALSE( newton_positive_root_quadratic( 0.0, 0.0, 2.0, 1.5 ).valid );
	EXPECT_FALSE( newton_positive_root_quadratic( 0.0, 5.0, 2.0, 1.5 ).valid );
	EXPECT_DOUBLE_EQ( 0.4, newton_positive_root_quadratic( 0.0, 5.0, -2.0, 0.0 ).x );
	EXPECT_DOUBLE_EQ( 0.4, newton_positive_root_quadratic( 0.0, -5.0, 2.0, 0.5 ).x );
}

TEST( MathTest, HalleyPositiveRootQuadratic )
{
	EXPECT_DOUBLE_EQ( 1.0 + std::sqrt( 2.5 ), halley_positive_root_quadratic( 2.0, -4.0, -3.0, 2.5 ).x );
	EXPECT_DOUBLE_EQ( 5.0 / 3.0, halley_positive_root_quadratic( -3.0, 5.0, 0.0, 1.5 ).x );
	EXPECT_EQ( 0.0, halley_positive_root_quadratic( 0.0, 0.0, 2.0, 1.5 ).x );
	EXPECT_EQ( 0.0, halley_positive_root_quadratic( 0.0, 5.0, 2.0, 1.5 ).x );
	EXPECT_FALSE( halley_positive_root_quadratic( 0.0, 0.0, 2.0, 1.5 ).valid );
	EXPECT_FALSE( halley_positive_root_quadratic( 0.0, 5.0, 2.0, 1.5 ).valid );
	EXPECT_DOUBLE_EQ( 0.4, halley_positive_root_quadratic( 0.0, 5.0, -2.0, 0.0 ).x );
	EXPECT_DOUBLE_EQ( 0.4, halley_positive_root_quadratic( 0.0, -5.0, 2.0, 0.5 ).x );
}

TEST( MathTest, IterativePositiveRootQuadratic )
{
	EXPECT_DOUBLE_EQ( 1.0 + std::sqrt( 2.5 ), iterative_positive_root_quadratic( 2.0, -4.0, -3.0, 2.5 ).x );
	EXPECT_DOUBLE_EQ( 5.0 / 3.0, iterative_positive_root_quadratic( -3.0, 5.0, 0.0, 1.5 ).x );
	EXPECT_EQ( 0.0, iterative_positive_root_quadratic( 0.0, 0.0, 2.0, 1.5 ).x );
	EXPECT_EQ( 0.0, iterative_positive_root_quadratic( 0.0, 5.0, 2.0, 1.5 ).x );
	EXPECT_FALSE( iterative_positive_root_quadratic( 0.0, 0.0, 2.0, 1.5 ).valid );
	EXPECT_FALSE( iterative_positive_root_quadratic( 0.0, 5.0, 2.0, 1.5 ).valid );
	EXPECT_DOUBLE_EQ( 0.4, iterative_positive_root_quadratic( 0.0, 5.0, -2.0, 0.0 ).x );
	EXPECT_DOUBLE_EQ( 0.4, iterative_positive_root_quadratic( 0.0, -5.0, 2.0, 0.5 ).x );
}

TEST( MathTest, CriticalPointMagnitudeQuadratic )
{
	EXPECT_DOUBLE_EQ( 4.0, critical_point_magnitude_quadratic( -3.0, 6.0, 1.0, 2.0 ) );
	EXPECT_DOUBLE_EQ( 1.2, critical_point_magnitude_quadratic( 5.0, -8.0, 2.0, 2.0 ) );
	EXPECT_EQ( 0.0, critical_point_magnitude_quadratic( 0.0, -8.0, 2.0, 2.0 ) );
	EXPECT_EQ( 0.0, critical_point_magnitude_quadratic( 5.0, 0.0, 2.0, 2.0 ) );
	EXPECT_EQ( 0.0, critical_point_magnitude_quadratic( 5.0, 8.0, 2.0, 2.0 ) );
	EXPECT_EQ( 0.0, critical_point_magnitude_quadratic( 5.0, -8.0, 2.0, 0.75 ) );
}

TEST( MathTest, ZCRootQuadratic )
{
	double const inf_double( inf< double >() );
	EXPECT_DOUBLE_EQ( 1.0 + std::sqrt( 2.5 ), zc_root_quadratic( 2.0, -4.0, -3.0 ) );
	EXPECT_DOUBLE_EQ( 5.0 / 3.0, zc_root_quadratic( -3.0, 5.0, 0.0 ) );
	EXPECT_EQ( inf_double, zc_root_quadratic( 0.0, 0.0, 2.0 ) );
	EXPECT_EQ( inf_double, zc_root_quadratic( 0.0, 5.0, 2.0 ) );
	EXPECT_DOUBLE_EQ( 0.4, zc_root_quadratic( 0.0, 5.0, -2.0 ) );
	EXPECT_DOUBLE_EQ( 0.4, zc_root_quadratic( 0.0, -5.0, 2.0 ) );
}

TEST( MathTest, MinRootQuadratic )
{
	double const inf_double( inf< double >() );

	EXPECT_DOUBLE_EQ( std::sqrt( 2.5 ) - 1.0, min_root_quadratic_lower( -2.0, -4.0, 3.0 ) );
	EXPECT_NEAR( ( 2.0 / std::sqrt( 3 ) ) - 1.0, min_root_quadratic_lower( -3.0, -6.0, 1.0 ), 1.0e-14 );
	EXPECT_EQ( inf_double, min_root_quadratic_lower( 0.0, 0.0, 2.0 ) );
	EXPECT_DOUBLE_EQ( 0.4, min_root_quadratic_lower( 0.0, -5.0, 2.0 ) );

	EXPECT_DOUBLE_EQ( std::sqrt( 2.5 ) - 1.0, min_root_quadratic_upper( 2.0, 4.0, -3.0 ) );
	EXPECT_NEAR( ( 2.0 / std::sqrt( 3 ) ) - 1.0, min_root_quadratic_upper( 3.0, 6.0, -1.0 ), 1.0e-14 );
	EXPECT_EQ( inf_double, min_root_quadratic_upper( 0.0, 0.0, -2.0 ) );
	EXPECT_DOUBLE_EQ( 0.4, min_root_quadratic_upper( 0.0, 5.0, -2.0 ) );

	EXPECT_DOUBLE_EQ( std::sqrt( 2.5 ) - 1.0, min_root_quadratic_both( -2.0, -4.0, 3.0, -9.0 ) );
	EXPECT_NEAR( ( 2.0 / std::sqrt( 3 ) ) - 1.0, min_root_quadratic_both( -3.0, -6.0, 1.0, -9.0 ), 1.0e-14 );
	EXPECT_EQ( inf_double, min_root_quadratic_both( 0.0, 0.0, 2.0, -9.0 ) );
	EXPECT_DOUBLE_EQ( 0.4, min_root_quadratic_both( 0.0, -5.0, 2.0, -9.0 ) );

	EXPECT_DOUBLE_EQ( std::sqrt( 2.5 ) - 1.0, min_root_quadratic_both( 2.0, 4.0, 9.0, -3.0 ) );
	EXPECT_NEAR( ( 2.0 / std::sqrt( 3 ) ) - 1.0, min_root_quadratic_both( 3.0, 6.0, 9.0, -1.0 ), 1.0e-14 );
	EXPECT_EQ( inf_double, min_root_quadratic_both( 0.0, 0.0, 9.0, -2.0 ) );
	EXPECT_DOUBLE_EQ( 0.4, min_root_quadratic_both( 0.0, 5.0, 9.0, -2.0 ) );
}

TEST( MathTest, CubicUtils )
{
	EXPECT_EQ( 4.0, cubic( 1.0, 2.0, -8.0, 4.0, 0.0 ) );
	EXPECT_EQ( -1.0, cubic( 1.0, 2.0, -8.0, 4.0, 1.0 ) );
	EXPECT_EQ( 4.0, cubic( 1.0, 2.0, -8.0, 4.0, 2.0 ) );

	EXPECT_EQ( 4.0, cubic_monic( 2.0, -8.0, 4.0, 0.0 ) );
	EXPECT_EQ( -1.0, cubic_monic( 2.0, -8.0, 4.0, 1.0 ) );
	EXPECT_EQ( 4.0, cubic_monic( 2.0, -8.0, 4.0, 2.0 ) );

	EXPECT_EQ( 0.0, cubic_cull( 1.0, 2.0, -8.0 ) );
	EXPECT_EQ( 8.0, cubic_cull( 1.0, 2.0, 8.0 ) );

	EXPECT_EQ( 0.0, cubic_cull_lower( 1.0, 2.0, -8.0 ) );
	EXPECT_EQ( 0.0, cubic_cull_lower( 9.0, 8.0, 8.0 ) );
	EXPECT_EQ( 4.0, cubic_cull_lower( -9.0, 3.0, 4.0 ) );

	EXPECT_EQ( 0.0, cubic_cull_upper( 1.0, 2.0, -8.0 ) );
	EXPECT_EQ( 0.0, cubic_cull_upper( -9.0, 3.0, 4.0 ) );
	EXPECT_EQ( 8.0, cubic_cull_upper( 1.0, 2.0, 8.0 ) );
}

TEST( MathTest, NewtonSmallPositiveRootCubicMonic )
{
	EXPECT_EQ( 0.0, newton_small_positive_root_cubic_monic( -3.0, 6.0, 1.0 ).x );
	EXPECT_FALSE( newton_small_positive_root_cubic_monic( -3.0, 6.0, 1.0 ).valid );
	EXPECT_DOUBLE_EQ( 1.32218535462608559, newton_small_positive_root_cubic_monic( -3.0, 6.0, -5.0 ).x );
	EXPECT_TRUE( newton_small_positive_root_cubic_monic( -3.0, 6.0, -5.0 ).valid );
}

TEST( MathTest, HalleySmallPositiveRootCubicMonic )
{
	EXPECT_EQ( 0.0, halley_small_positive_root_cubic_monic( -3.0, 6.0, 1.0 ).x );
	EXPECT_FALSE( halley_small_positive_root_cubic_monic( -3.0, 6.0, 1.0 ).valid );
	EXPECT_DOUBLE_EQ( 1.32218535462608559, halley_small_positive_root_cubic_monic( -3.0, 6.0, -5.0 ).x );
	EXPECT_TRUE( halley_small_positive_root_cubic_monic( -3.0, 6.0, -5.0 ).valid );
}

TEST( MathTest, IterativeSmallPositiveRootCubicMonic )
{
	EXPECT_EQ( 0.0, iterative_small_positive_root_cubic_monic( -3.0, 6.0, 1.0 ).x );
	EXPECT_FALSE( iterative_small_positive_root_cubic_monic( -3.0, 6.0, 1.0 ).valid );
	EXPECT_DOUBLE_EQ( 1.32218535462608559, iterative_small_positive_root_cubic_monic( -3.0, 6.0, -5.0 ).x );
	EXPECT_TRUE( iterative_small_positive_root_cubic_monic( -3.0, 6.0, -5.0 ).valid );
}

TEST( MathTest, NewtonPositiveRootCubicMonic )
{
	EXPECT_FALSE( newton_positive_root_cubic_monic( -3.0, 6.0, 1.0, 1.0 ).valid );
	EXPECT_DOUBLE_EQ( 1.32218535462608559, newton_positive_root_cubic_monic( -3.0, 6.0, -5.0, 1.3 ).x );
	EXPECT_TRUE( newton_positive_root_cubic_monic( -3.0, 6.0, -5.0, 1.3 ).valid );
	EXPECT_DOUBLE_EQ( 2.0915403681203739, newton_positive_root_cubic_monic( -0.2, -3.0, -2.0, 1.9 ).x );
}

TEST( MathTest, HalleyPositiveRootCubicMonic )
{
	EXPECT_FALSE( halley_positive_root_cubic_monic( -3.0, 6.0, 1.0, 1.0 ).valid );
	EXPECT_DOUBLE_EQ( 1.32218535462608559, halley_positive_root_cubic_monic( -3.0, 6.0, -5.0, 1.3 ).x );
	EXPECT_TRUE( halley_positive_root_cubic_monic( -3.0, 6.0, -5.0, 1.3 ).valid );
	EXPECT_DOUBLE_EQ( 2.0915403681203739, halley_positive_root_cubic_monic( -0.2, -3.0, -2.0, 1.9 ).x );
}

TEST( MathTest, IterativePositiveRootCubicMonic )
{
	EXPECT_FALSE( iterative_positive_root_cubic_monic( -3.0, 6.0, 1.0, 1.0 ).valid );
	EXPECT_DOUBLE_EQ( 1.32218535462608559, iterative_positive_root_cubic_monic( -3.0, 6.0, -5.0, 1.3 ).x );
	EXPECT_TRUE( iterative_positive_root_cubic_monic( -3.0, 6.0, -5.0, 1.3 ).valid );
	EXPECT_DOUBLE_EQ( 2.0915403681203739, iterative_positive_root_cubic_monic( -0.2, -3.0, -2.0, 1.9 ).x );
}

TEST( MathTest, CriticalPointMagnitudeCubic )
{
	EXPECT_DOUBLE_EQ( 1.7362735784511805, critical_point_magnitude_cubic( 9.0, -3.0, -6.0, 1.0, 1.0 ) );
	EXPECT_DOUBLE_EQ( 26.426101068499275, critical_point_magnitude_cubic( 1.0, -4.0, -6.0, 1.0, 5.0 ) );
}

TEST( MathTest, CriticalPointMagnitudeCubicMonic )
{
	EXPECT_DOUBLE_EQ( 1.7362735784511805/9.0, critical_point_magnitude_cubic_monic( -3.0/9.0, -6.0/9.0, 1.0/9.0, 1.0 ) );
	EXPECT_DOUBLE_EQ( 26.426101068499275, critical_point_magnitude_cubic_monic( -4.0, -6.0, 1.0, 5.0 ) );
}

TEST( MathTest, ZCRootCullCubicMonic )
{
	EXPECT_DOUBLE_EQ( 5.0, zc_root_cull_cubic_monic( -4.0, -6.0, 1.0, 5.0, 1.0, 0.0 ) );
	EXPECT_DOUBLE_EQ( 5.0, zc_root_cull_cubic_monic( -4.0, -6.0, 1.0, 5.0, 1.0, 26.0 ) ); // Extrema is 26.426101068499275
	EXPECT_DOUBLE_EQ( infinity, zc_root_cull_cubic_monic( -4.0, -6.0, 1.0, 5.0, 1.0, 27.0 ) ); // Extrema is 26.426101068499275
	EXPECT_DOUBLE_EQ( 5.0, zc_root_cull_cubic_monic( -4.0, -6.0, 1.0, 5.0, 27.0, 27.0 ) ); // Extrema is 26.426101068499275
}

TEST( MathTest, ZCPositiveRootCullCubicMonic )
{
	EXPECT_DOUBLE_EQ( 5.0, zc_positive_root_cull_cubic_monic( -4.0, -6.0, 1.0, 5.0, 1.0, 0.0 ) );
	EXPECT_DOUBLE_EQ( 5.0, zc_positive_root_cull_cubic_monic( -4.0, -6.0, 1.0, 5.0, 1.0, 26.0 ) ); // Extrema is 26.426101068499275
	EXPECT_DOUBLE_EQ( infinity, zc_positive_root_cull_cubic_monic( -4.0, -6.0, 1.0, 5.0, 1.0, 27.0 ) ); // Extrema is 26.426101068499275
	EXPECT_DOUBLE_EQ( 5.0, zc_positive_root_cull_cubic_monic( -4.0, -6.0, 1.0, 5.0, 27.0, 27.0 ) ); // Extrema is 26.426101068499275

	EXPECT_DOUBLE_EQ( infinity, zc_positive_root_cull_cubic_monic( -4.0, -6.0, 1.0, -5.0, 1.0, 0.0 ) );
	EXPECT_DOUBLE_EQ( infinity, zc_positive_root_cull_cubic_monic( -4.0, -6.0, 1.0, -5.0, 1.0, 26.0 ) ); // Extrema is 26.426101068499275
	EXPECT_DOUBLE_EQ( infinity, zc_positive_root_cull_cubic_monic( -4.0, -6.0, 1.0, -5.0, 1.0, 27.0 ) ); // Extrema is 26.426101068499275
	EXPECT_DOUBLE_EQ( infinity, zc_positive_root_cull_cubic_monic( -4.0, -6.0, 1.0, -5.0, 27.0, 27.0 ) ); // Extrema is 26.426101068499275
}

TEST( MathTest, ZCRootCubic )
{
	EXPECT_DOUBLE_EQ( 0.7073498763104491, zc_root_cubic( -2.25, -6.5, -7.0, 9.0 ) );
	EXPECT_DOUBLE_EQ( 0.7073498763104491, zc_root_cubic( 2.25, 6.5, 7.0, -9.0 ) );
	EXPECT_DOUBLE_EQ( 0.021503603166631264, zc_root_cubic( 1.0, 2000.0, 50.0, -2.0 ) ); // Near quadratic but Halley small root converges
	EXPECT_DOUBLE_EQ( 0.04650293690494123, zc_root_cubic( 1.0, 2000.0, -50.0, -2.0 ) ); // Near quadratic
}

TEST( MathTest, MinRootCubicMonicAnalytical )
{
	EXPECT_DOUBLE_EQ( 0.15417149518144127, min_root_cubic_monic_boundary_analytical( 3.0, 6.0, -1.0 ) );
	EXPECT_DOUBLE_EQ( 0.609695494016669, min_root_cubic_monic_boundary_analytical( 3.0, 6.0, -5.0 ) );
	EXPECT_DOUBLE_EQ( 0.57943586314575579, min_root_cubic_monic_boundary_analytical( 0.2, 3.0, -2.0 ) );
}

TEST( MathTest, MinRootCubic )
{
	EXPECT_DOUBLE_EQ( 0.7073498763104491, min_root_cubic_lower( -2.25, -6.5, -7.0, 9.0 ) );
	EXPECT_DOUBLE_EQ( 0.0, min_root_cubic_lower( -2.25, -6.5, -7.0, -0.01 ) ); // d < 0 => Precision loss

	EXPECT_DOUBLE_EQ( 0.7073498763104491, min_root_cubic_upper( 2.25, 6.5, 7.0, -9.0 ) );
	EXPECT_DOUBLE_EQ( 0.0, min_root_cubic_upper( 2.25, 6.5, 7.0, 0.01 ) ); // d > 0 => Precision loss

	EXPECT_DOUBLE_EQ( 1.359787450380789, min_root_cubic_both( -2.0, 3.0, -7.0, 9.0, -9.0 ) );
	EXPECT_DOUBLE_EQ( 1.4175965758288351, min_root_cubic_both( -2.0, 4.0, -8.0, 9.0, -9.0 ) );
	EXPECT_DOUBLE_EQ( 0.29037158997385715, min_root_cubic_both( -9.0, 3.0, -7.0, 2.0, -2.0 ) );
	EXPECT_DOUBLE_EQ( 1.060647778684131, min_root_cubic_both( -9.0, 3.0, 6.0, 1.0, -3.0 ) );

	EXPECT_DOUBLE_EQ( 2.4141969797051361, min_root_cubic_both( 0.00001, 3.0, -6.0, 6.0, -3.0 ) ); // Near quadratic
	EXPECT_DOUBLE_EQ( 2.4142301455300395, min_root_cubic_both( -0.00001, 3.0, -6.0, 6.0, -3.0 ) ); // Near quadratic
}
