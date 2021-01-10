// QSS::BinOptimizer Unit Tests
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
#include <QSS/BinOptimizer.hh>

// C++ Headers
#include <iterator>
#include <vector>

using namespace QSS;

// Variable Mock
class V final {};

// Types

TEST( BinOptimizerTest, Basic )
{
	{
	BinOptimizer optimizer( 100u );
	EXPECT_FALSE( optimizer.valid() );
	optimizer.add( 2u, 8.0 );
	EXPECT_FALSE( optimizer.valid() );
	optimizer.add( 6u, 4.0 );
	EXPECT_FALSE( optimizer.valid() );
	EXPECT_EQ( 9u, optimizer.rec_bin_size() );
	}
	{
	BinOptimizer optimizer( 100u );
	EXPECT_FALSE( optimizer.valid() );
	optimizer.add( 1u, 14.0 );
	EXPECT_FALSE( optimizer.valid() );
	optimizer.add( 5u, 44.0 );
	EXPECT_FALSE( optimizer.valid() );
	optimizer.add( 10u, 22.0 );
	EXPECT_TRUE( optimizer.valid() );
	EXPECT_EQ( 6u, optimizer.rec_bin_size() );
	}
	{
	BinOptimizer optimizer( 100u );
	EXPECT_FALSE( optimizer.valid() );
	optimizer.add( 5u, 18.0 );
	EXPECT_FALSE( optimizer.valid() );
	optimizer.add( 3u, 14.0 );
	EXPECT_FALSE( optimizer.valid() );
	optimizer.add( 9u, 17.0 );
	EXPECT_TRUE( optimizer.valid() );
	EXPECT_EQ( 7u, optimizer.rec_bin_size() );
	}
	{
	BinOptimizer optimizer( 100u );
	EXPECT_FALSE( optimizer.valid() );
	optimizer.add( 6u, 15.0 );
	EXPECT_FALSE( optimizer.valid() );
	optimizer.add( 3u, 17.0 );
	EXPECT_FALSE( optimizer.valid() );
	optimizer.add( 9u, 14.0 );
	EXPECT_TRUE( optimizer.valid() );
	EXPECT_EQ( 2u, optimizer.rec_bin_size() );
	}
	{
	BinOptimizer optimizer( 100u );
	EXPECT_FALSE( optimizer.valid() );
	optimizer.add( 2u, 8.0 );
	EXPECT_FALSE( optimizer.valid() );
	optimizer.add( 4u, 4.0 );
	EXPECT_FALSE( optimizer.valid() );
	optimizer.add( 8u, 8.0 );
	EXPECT_TRUE( optimizer.valid() );
	EXPECT_EQ( 1u, optimizer.rec_bin_size() );
	}
	{
	BinOptimizer optimizer( 100u );
	EXPECT_FALSE( optimizer.valid() );
	optimizer.add( 2u, 8.0 );
	EXPECT_FALSE( optimizer.valid() );
	optimizer.add( 6u, 4.0 );
	EXPECT_FALSE( optimizer.valid() );
	optimizer.add( 8u, 8.0 );
	EXPECT_TRUE( optimizer.valid() );
	EXPECT_EQ( 12u, optimizer.rec_bin_size() );
	}
}
