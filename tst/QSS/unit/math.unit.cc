// QSS::math Unit Tests
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2020 Objexx Engineering, Inc. All rights reserved.
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

TEST( MathTest, Quad )
{
	EXPECT_EQ( 81.0, quad( 3.0 ) );
	EXPECT_EQ( 16, quad( 2 ) );
}

TEST( MathTest, Infinityish )
{
	EXPECT_EQ( infinity, infinityish< double >() );
	EXPECT_EQ( std::numeric_limits< double >::infinity(), infinityish< double >() );
	EXPECT_EQ( std::numeric_limits< float >::infinity(), infinityish< float >() );
	EXPECT_EQ( std::numeric_limits< int >::max(), infinityish< int >() );
}

TEST( MathTest, PositiveOrInfinity )
{
	EXPECT_EQ( 3.0, positive_or_infinity( 3.0 ) );
	EXPECT_EQ( infinity, positive_or_infinity( -5.0 ) );
	EXPECT_EQ( infinity, positive_or_infinity( 0.0 ) );
	EXPECT_EQ( 2, positive_or_infinity( 2 ) );
}

TEST( MathTest, MinMax )
{
	EXPECT_EQ( 3.0, min( 44.0, 77.0, 3.0 ) );
	EXPECT_EQ( 5, min( 8, 99, 38, 5 ) );
	EXPECT_EQ( 5, min( 8, 99, 38, 5, 373 ) );
	EXPECT_EQ( 4, min( 8, 99, 38, 5, 373, 4 ) );

	EXPECT_EQ( 77.0, max( 44.0, 77.0, 3.0 ) );
	EXPECT_EQ( 373, max( 8, 99, 38, 373 ) );
	EXPECT_EQ( 373, max( 8, 99, 38, 5, 373 ) );
	EXPECT_EQ( 555, max( 8, 99, 38, 5, 555, 373 ) );

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

TEST( MathTest, MinRootQuadratic )
{
	EXPECT_DOUBLE_EQ( 0.32287565553229536, min_root_quadratic( -4.0, -8.0, 3.0 ) );
	EXPECT_DOUBLE_EQ( 0.87082869338697070, min_root_quadratic( -2.0, -4.0, 5.0 ) );
	EXPECT_DOUBLE_EQ( 0.42214438511238006, min_root_quadratic( -2.0, -11.0, 5.0 ) );
	EXPECT_DOUBLE_EQ( 0.0, min_root_quadratic( -2.0, -11.0, -0.001 ) );
	EXPECT_DOUBLE_EQ( 0.32287565553229536, min_positive_root_quadratic( -4.0, -8.0, 3.0 ) );
	EXPECT_DOUBLE_EQ( 0.87082869338697070, min_positive_root_quadratic( -2.0, -4.0, 5.0 ) );
	EXPECT_DOUBLE_EQ( 0.42214438511238006, min_positive_root_quadratic( -2.0, -11.0, 5.0 ) );
	EXPECT_DOUBLE_EQ( infinity, min_positive_root_quadratic( -2.0, -11.0, -0.001 ) );
	EXPECT_DOUBLE_EQ( 0.32287565553229536, min_root_quadratic_lower( -4.0, -8.0, 3.0 ) );
	EXPECT_DOUBLE_EQ( 0.87082869338697070, min_root_quadratic_lower( -2.0, -4.0, 5.0 ) );
	EXPECT_DOUBLE_EQ( 0.42214438511238006, min_root_quadratic_lower( -2.0, -11.0, 5.0 ) );
	EXPECT_DOUBLE_EQ( 0.0, min_root_quadratic_lower( -2.0, -11.0, -0.001 ) );
	EXPECT_DOUBLE_EQ( 0.32287565553229536, min_root_quadratic_upper( 4.0, 8.0, -3.0 ) );
	EXPECT_DOUBLE_EQ( 0.87082869338697070, min_root_quadratic_upper( 2.0, 4.0, -5.0 ) );
	EXPECT_DOUBLE_EQ( 0.42214438511238006, min_root_quadratic_upper( 2.0, 11.0, -5.0 ) );
	EXPECT_DOUBLE_EQ( 0.0, min_root_quadratic_upper( 2.0, 11.0, 0.001 ) );
	EXPECT_DOUBLE_EQ( 0.5, min_root_quadratic_both( -4.0, 8.0, 3.0, -3.0 ) );
	EXPECT_DOUBLE_EQ( 0.8708286933869707, min_root_quadratic_both( 2.0, 4.0, 5.0, -5.0 ) );
	EXPECT_DOUBLE_EQ( 0.5, min_root_quadratic_both( -2.0, 11.0, 5.0, -5.0 ) );
}

TEST( MathTest, CubicUtils )
{
	EXPECT_EQ( 3.0, peak_mag_quadratic( 0.0, 0.0, 3.0 ) );
	EXPECT_EQ( infinity, peak_mag_quadratic( 0.0, 1.0, 3.0 ) );
	EXPECT_EQ( 0.0, peak_mag_quadratic( 2.0, 0.0, 0.0 ) );
	EXPECT_EQ( 3.0, peak_mag_quadratic( 1.0, 2.0, 4.0 ) );
	EXPECT_EQ( 7.0, peak_mag_quadratic( 1.0, 2.0, 8.0 ) );

	EXPECT_EQ( 0.0, cubic_cull( 1.0, 2.0, -8.0 ) );
	EXPECT_EQ( 8.0, cubic_cull( 1.0, 2.0, 8.0 ) );

	EXPECT_EQ( 0.0, cubic_cull_lower( 1.0, 2.0, -8.0, -1.5 ) );
	EXPECT_EQ( 8.0, cubic_cull_lower( 9.0, 8.0, 8.0, -1.5 ) );

	EXPECT_EQ( 0.0, cubic_cull_upper( 1.0, 2.0, -8.0, 1.5 ) );
	EXPECT_EQ( 8.0, cubic_cull_upper( 1.0, 2.0, 8.0, 1.5 ) );
}

TEST( MathTest, MinRootCubic )
{
	EXPECT_NEAR( 0.7073498763104409, min_root_cubic_lower( -2.25, -6.5, -7.0, 9.0 ), 1.0e-14 );
	EXPECT_DOUBLE_EQ( 0.0, min_root_cubic_lower( -2.25, -6.5, -7.0, -0.01 ) ); // d < 0 => Precision loss
	EXPECT_NEAR( 0.7073498763104409, min_root_cubic_upper( 2.25, 6.5, 7.0, -9.0 ), 1.0e-14 );
	EXPECT_DOUBLE_EQ( 0.0, min_root_cubic_upper( 2.25, 6.5, 7.0, 0.01 ) ); // d > 0 => Precision loss
	EXPECT_DOUBLE_EQ( 1.359787450380789, min_root_cubic_both( -2.0, 3.0, -7.0, 9.0, 9.0 ) );
	EXPECT_DOUBLE_EQ( 1.4175965758288351, min_root_cubic_both( -2.0, 4.0, -8.0, 9.0, 9.0 ) );
	EXPECT_NEAR( 0.29037158997385715, min_root_cubic_both( -9.0, 3.0, -7.0, 2.0, 2.0 ), 1.0e-15 );
	EXPECT_DOUBLE_EQ( 1.060647778684131, min_root_cubic_both( -9.0, 3.0, 6.0, 1.0, 1.0 ) );
}
