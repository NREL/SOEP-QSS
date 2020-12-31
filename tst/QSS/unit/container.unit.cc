// QSS::container Unit Tests
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
#include <QSS/container.hh>
#include <QSS/cod/Variable_QSS1.hh>
#include <QSS/cod/Variable_QSS2.hh>
#include <QSS/cod/Variable_ZC1.hh>
#include <QSS/cod/Variable_ZC2.hh>
#include <QSS/cod/mdl/Function_LTI.hh>

// C++ Headers
#include <map>

using namespace QSS;
using namespace QSS::cod;
using namespace QSS::cod::mdl;
using Variables = Variable::Variables;
using size_type = Variables::size_type;
using QSS1 = Variable_QSS1< Function_LTI >;
using QSS2 = Variable_QSS2< Function_LTI >;
using ZC1 = Variable_ZC1< Function_LTI >;
using ZC2 = Variable_ZC2< Function_LTI >;

TEST( ContainerTest, SortByOrder )
{
	ZC2 zc2( "ZC2" );
	ZC1 zc1( "ZC1" );
	QSS2 qss2( "QSS2" );
	QSS1 qss1( "QSS1" );

	Variables variables;

	variables.push_back( &zc2 );
	variables.push_back( &zc1 );
	variables.push_back( &qss2 );
	variables.push_back( &qss1 );

	EXPECT_FALSE( std::is_sorted( variables.begin(), variables.end(), []( Variable const * v1, Variable const * v2 ){ return v1->order() < v2->order(); } ) );
	sort_by_order( variables );
	EXPECT_TRUE( is_sorted_by_order( variables ) );
	EXPECT_TRUE( std::is_sorted( variables.begin(), variables.end(), []( Variable const * v1, Variable const * v2 ){ return v1->order() < v2->order(); } ) );

	EXPECT_EQ( 0u, begin_order_index( variables, 1 ) );
	EXPECT_EQ( 2u, begin_order_index( variables, 2 ) );
	EXPECT_EQ( 4u, begin_order_index( variables, 3 ) );
}

TEST( ContainerTest, SortByTypeAndORder )
{
	ZC2 zc2( "ZC2" );
	ZC1 zc1( "ZC1" );
	QSS2 qss2( "QSS2" );
	QSS1 qss1( "QSS1" );

	Variables variables;

	variables.push_back( &zc2 );
	variables.push_back( &zc1 );
	variables.push_back( &qss2 );
	variables.push_back( &qss1 );

	EXPECT_FALSE( std::is_sorted( variables.begin(), variables.end(), []( Variable const * v1, Variable const * v2 ){ return v1->not_ZC() && v2->is_ZC(); } ) );
	sort_by_type_and_order( variables );
	EXPECT_TRUE( std::is_sorted( variables.begin(), variables.end(), []( Variable const * v1, Variable const * v2 ){ return v1->not_ZC() && v2->is_ZC(); } ) );
}

TEST( ContainerTest, BeginOrderIndex )
{
	Variables variables;
	variables.push_back( new ZC1( "ZC1" ) );
	variables.push_back( new ZC1( "ZC1" ) );
	variables.push_back( new ZC1( "ZC1" ) );
	variables.push_back( new QSS1( "QSS1" ) );
	variables.push_back( new QSS1( "QSS1" ) );
	variables.push_back( new QSS2( "QSS2" ) );
	variables.push_back( new QSS2( "QSS2" ) );
	variables.push_back( new ZC2( "ZC2" ) );
	variables.push_back( new ZC2( "ZC2" ) );
	variables.push_back( new ZC2( "ZC2" ) );
	EXPECT_EQ( 5u, begin_order_index( variables, 2 ) );
	for ( Variable * var : variables ) delete var; // Clean up
}

TEST( ContainerTest, Uniquify )
{
	ZC2 zc2( "ZC2" );
	ZC1 zc1( "ZC1" );
	QSS2 qss2( "QSS2" );
	QSS1 qss1( "QSS1" );

	Variables variables;

	variables.push_back( &zc2 );
	variables.push_back( &zc1 );
	variables.push_back( &qss2 );
	variables.push_back( &qss1 );

	variables.push_back( &zc2 );
	variables.push_back( &zc1 );
	variables.push_back( &qss2 );
	variables.push_back( &qss1 );

	EXPECT_FALSE( is_unique( variables ) );
	uniquify( variables );
	EXPECT_TRUE( is_unique( variables ) );
}

TEST( ContainerTest, VariablesObservers )
{
	QSS2 v( "v" );
	QSS2 h( "h" );
	QSS2 z( "z" );

	h.observers().push_back( &v );
	h.observers().push_back( &z );

	Variables triggers;

	triggers.push_back( &v );
	triggers.push_back( &h );

	Variables observers;

	variables_observers( triggers, observers );

	EXPECT_EQ( 1u, observers.size() ); // Only z is a non-trigger observer
}

TEST( ContainerTest, VectorRemoveValue )
{
	std::vector< int > v{ 99, 42, 8, 72 };

	EXPECT_EQ( 4u, v.size() );
	vector_remove_value( v, 33 );
	EXPECT_EQ( 4u, v.size() );
	vector_remove_value( v, 42 );
	EXPECT_EQ( 3u, v.size() );
	EXPECT_EQ( std::vector< int >( { 99, 8, 72 } ), v );
}

TEST( ContainerTest, VectorNullifyValue )
{
	int a{ 99 };
	int b{ 42 };
	int c{ 88 };
	int d{ 72 };
	std::vector< int * > v{ &a, &b, &c, &d };

	EXPECT_EQ( 4u, v.size() );
	vector_nullify_value( v, &b );
	EXPECT_EQ( 4u, v.size() );
	EXPECT_EQ( std::vector< int * >( { &a, nullptr, &c, &d } ), v );
}

TEST( ContainerTest, MapRemoveValue )
{
	std::map< int, int > m{ { 3, 99 }, { 7, 42 }, { 8, 88 }, { 4, 72 } };

	EXPECT_EQ( 4u, m.size() );
	map_remove_value( m, 33 );
	EXPECT_EQ( 4u, m.size() );
	map_remove_value( m, 42 );
	EXPECT_EQ( 3u, m.size() );
	std::map< int, int > m2{ { 3, 99 }, { 8, 88 }, { 4, 72 } };
	EXPECT_EQ( m2, m );
}
