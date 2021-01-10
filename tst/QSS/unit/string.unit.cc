// QSS String Function Unit Tests
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2021 Objexx Engineering, Inc. All rights reserved.
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
#include <QSS/string.hh>

using namespace QSS;

TEST( StringTest, Predicate )
{
	EXPECT_TRUE( is_tail( "" ) );
	EXPECT_TRUE( is_tail( "   " ) );
	EXPECT_TRUE( is_tail( "\t\t\t" ) );
	EXPECT_FALSE( is_tail( "TAIL" ) );

	EXPECT_TRUE( is_int( "123" ) );
	EXPECT_FALSE( is_int( "1223.456" ) );
	EXPECT_FALSE( is_int( "Fish" ) );

	EXPECT_TRUE( is_size( "123" ) );
	EXPECT_TRUE( is_size( "-123" ) );
	EXPECT_FALSE( is_size( "1223.456" ) );
	EXPECT_FALSE( is_size( "Fish" ) );

	EXPECT_TRUE( is_double( "123" ) );
	EXPECT_TRUE( is_double( "123.456" ) );
	EXPECT_TRUE( is_double( "123.456e2" ) );

	EXPECT_TRUE( is_any_of( 'k', "cake" ) );
	EXPECT_FALSE( is_any_of( 'k', "CAKE" ) );

	EXPECT_TRUE( has( "cake", 'k' ) );
	EXPECT_FALSE( has( "cake", 'K' ) );

	EXPECT_TRUE( HAS( "cake", 'k' ) );
	EXPECT_TRUE( HAS( "cake", 'K' ) );

	EXPECT_FALSE( has_any_not_of( "x", "xyz" ) );
	EXPECT_FALSE( has_any_not_of( "x", "xyz" ) );
	EXPECT_TRUE( has_any_not_of( "xbz", "xyz" ) );
	EXPECT_TRUE( has_any_not_of( "b", "X" ) );

	EXPECT_TRUE( has_prefix( "Cat and Dog", "Cat" ) );
	EXPECT_TRUE( has_prefix( "Cat and Dog", "Cat" ) );
	EXPECT_TRUE( has_prefix( "Cat and Dog", "C" ) );
	EXPECT_FALSE( has_prefix( "Cat and Dog", "Bat" ) );
	EXPECT_FALSE( has_prefix( "Cat and Dog", "Bat" ) );
	EXPECT_FALSE( has_prefix( "Cat and Dog", "B" ) );
	EXPECT_TRUE( has_prefix( "Fish Tank", "Fi" ) );
	EXPECT_FALSE( has_prefix( "Fish Tank", "Fin" ) );

	EXPECT_TRUE( has_suffix( "Cat and Dog", "Dog" ) );
	EXPECT_TRUE( has_suffix( "Cat and Dog", "Dog" ) );
	EXPECT_TRUE( has_suffix( "Cat and Dog", "g" ) );
	EXPECT_FALSE( has_suffix( "Cat and Dog", "Bat" ) );
	EXPECT_FALSE( has_suffix( "Cat and Dog", "Bat" ) );
	EXPECT_FALSE( has_suffix( "Cat and Dog", "B" ) );
	EXPECT_TRUE( has_suffix( "A cat is a cat", "cat" ) );
	EXPECT_FALSE( has_suffix( "A cat is a cat", "bat" ) );

	EXPECT_TRUE( has_option( "--run", "run" ) );
	EXPECT_FALSE( has_option( "-run", "run" ) );
	EXPECT_FALSE( has_option( "--yes", "run" ) );

	EXPECT_TRUE( has_value_option( "--run=now", "run" ) );
	EXPECT_TRUE( has_value_option( "--run:now", "run" ) );
	EXPECT_TRUE( has_value_option( "--run=", "run" ) );
	EXPECT_FALSE( has_value_option( "--run", "run" ) );
	EXPECT_FALSE( has_value_option( "--opt=val", "run" ) );
}

TEST( StringTest, ConversionTo )
{
	EXPECT_EQ( 123, int_of( "123" ) );
	EXPECT_NE( 123, int_of( "124" ) );

	EXPECT_EQ( std::size_t( 123u ), size_of( "123" ) );
	EXPECT_NE( std::size_t( 123u ), size_of( "124" ) );

	EXPECT_EQ( 123.0, double_of( "123" ) );
	EXPECT_EQ( 123.75, double_of( "123.75" ) );
	EXPECT_NE( 123.76, double_of( "123.75" ) );
}

TEST( StringTest, Case )
{
	EXPECT_EQ( "big dog", lowercased( "Big Dog" ) );
	EXPECT_EQ( "BIG DOG", uppercased( "Big Dog" ) );
}

TEST( StringTest, Split )
{
	EXPECT_EQ( std::vector< std::string >( { "A", "big", "fish" } ), split( "A big fish" ) );
}

TEST( StringTest, Strip )
{
	std::string s( "  Fish " );
	EXPECT_EQ( "Fish", strip( s ) );
}
