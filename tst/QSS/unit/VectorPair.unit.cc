// QSS::VectorPair Unit Tests
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2018 Objexx Engineerinc, Inc. All rights reserved.
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
#include <QSS/VectorPair.hh>

// C++ Headers
#include <vector>

using namespace QSS;

TEST( VectorPairTest, Basic )
{
	std::vector< int > v{ { 1, 2, 3 } };
	std::vector< int > w{ { 4, 5, 6 } };
	VectorPair< int > p( v, w );

	{ // Forward iterator for loop
		int j( 0 );
		for ( VectorPair< int >::iterator i = p.begin(), e = p.end(); i != e; ++i ) {
			EXPECT_EQ( ++j, *i );
		}
	}

	{ // Forward range-based for loop
		int j( 0 );
		for ( int & i : p ) {
			EXPECT_EQ( ++j, i );
		}
	}

	{ // Backwards iterator for loop
		int j( w.back() + 1 );
		using RevIt = std::reverse_iterator< VectorPair< int >::iterator >;
		RevIt br( p.end() ), er( p.begin() );
		for ( RevIt i = br; i != er; ++i ) {
			EXPECT_EQ( --j, *i );
		}
	}

	{ // Modify in loop then verify in loop
		int j( 10 );
		for ( VectorPair< int >::iterator i = p.begin(), e = p.end(); i != e; ++i ) {
			*i = ++j;
		}
		j = 10;
		for ( VectorPair< int >::iterator i = p.begin(), e = p.end(); i != e; ++i ) {
			EXPECT_EQ( ++j, *i );
		}
	}
}

TEST( VectorPairTest, Empty1 )
{
	std::vector< int > v;
	std::vector< int > w{ { 1, 2, 3 } };
	VectorPair< int > p( v, w );

	{ // Forward iterator for loop
		int j( 0 );
		for ( VectorPair< int >::iterator i = p.begin(), e = p.end(); i != e; ++i ) {
			EXPECT_EQ( ++j, *i );
		}
	}

	{ // Forward range-based for loop
		int j( 0 );
		for ( int & i : p ) {
			EXPECT_EQ( ++j, i );
		}
	}

	{ // Backwards iterator for loop
		int j( w.back() + 1 );
		using RevIt = std::reverse_iterator< VectorPair< int >::iterator >;
		RevIt br( p.end() ), er( p.begin() );
		for ( RevIt i = br; i != er; ++i ) {
			EXPECT_EQ( --j, *i );
		}
	}

	{ // Modify in loop then verify in loop
		int j( 10 );
		for ( VectorPair< int >::iterator i = p.begin(), e = p.end(); i != e; ++i ) {
			*i = ++j;
		}
		j = 10;
		for ( VectorPair< int >::iterator i = p.begin(), e = p.end(); i != e; ++i ) {
			EXPECT_EQ( ++j, *i );
		}
	}
}

TEST( VectorPairTest, Empty2 )
{
	std::vector< int > v{ { 1, 2, 3 } };
	std::vector< int > w;
	VectorPair< int > p( v, w );

	{ // Forward iterator for loop
		int j( 0 );
		for ( VectorPair< int >::iterator i = p.begin(), e = p.end(); i != e; ++i ) {
			EXPECT_EQ( ++j, *i );
		}
	}

	{ // Forward range-based for loop
		int j( 0 );
		for ( int & i : p ) {
			EXPECT_EQ( ++j, i );
		}
	}

	{ // Backwards iterator for loop
		int j( v.back() + 1 );
		using RevIt = std::reverse_iterator< VectorPair< int >::iterator >;
		RevIt br( p.end() ), er( p.begin() );
		for ( RevIt i = br; i != er; ++i ) {
			EXPECT_EQ( --j, *i );
		}
	}

	{ // Modify in loop then verify in loop
		int j( 10 );
		for ( VectorPair< int >::iterator i = p.begin(), e = p.end(); i != e; ++i ) {
			*i = ++j;
		}
		j = 10;
		for ( VectorPair< int >::iterator i = p.begin(), e = p.end(); i != e; ++i ) {
			EXPECT_EQ( ++j, *i );
		}
	}
}

TEST( VectorPairTest, EmptyBoth )
{
	std::vector< int > v;
	std::vector< int > w;
	VectorPair< int > p( v, w );

	{ // Forward iterator for loop
		int j( 0 );
		for ( VectorPair< int >::iterator i = p.begin(), e = p.end(); i != e; ++i ) {
			EXPECT_EQ( ++j, *i );
		}
	}

	{ // Forward range-based for loop
		int j( 0 );
		for ( int & i : p ) {
			EXPECT_EQ( ++j, i );
		}
	}

	{ // Backwards iterator for loop
		int j( 1 );
		using RevIt = std::reverse_iterator< VectorPair< int >::iterator >;
		RevIt br( p.end() ), er( p.begin() );
		for ( RevIt i = br; i != er; ++i ) {
			EXPECT_EQ( --j, *i );
		}
	}

	{ // Modify in loop then verify in loop
		int j( 10 );
		for ( VectorPair< int >::iterator i = p.begin(), e = p.end(); i != e; ++i ) {
			*i = ++j;
		}
		j = 10;
		for ( VectorPair< int >::iterator i = p.begin(), e = p.end(); i != e; ++i ) {
			EXPECT_EQ( ++j, *i );
		}
	}
}
