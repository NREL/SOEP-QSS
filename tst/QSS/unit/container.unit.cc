// QSS::container Unit Tests
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2023 Objexx Engineering, Inc. All rights reserved.
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
#include <QSS/Variable_QSS1.hh>
#include <QSS/Variable_QSS2.hh>
#include <QSS/Variable_ZC1.hh>
#include <QSS/Variable_ZC2.hh>

// C++ Headers
#include <map>
#include <vector>

using namespace QSS;
using Variables = Variable::Variables;
using size_type = Variables::size_type;
using QSS1 = Variable_QSS1;
using QSS2 = Variable_QSS2;
using ZC1 = Variable_ZC1;
using ZC2 = Variable_ZC2;

TEST( ContainerTest, SortByType )
{
	FMU_ME fmu;
	ZC2 zc2( &fmu, "ZC2" );
	ZC1 zc1( &fmu, "ZC1" );
	QSS2 qss2( &fmu, "QSS2" );
	QSS1 qss1( &fmu, "QSS1" );

	Variables variables( { &zc2, &zc1, &qss2, &qss1 } );

	sort_by_type( variables );
	EXPECT_TRUE( std::is_sorted( variables.begin(), variables.end(), []( Variable const * v1, Variable const * v2 ){ return v1->not_ZC() && v2->is_ZC(); } ) );
	EXPECT_TRUE( variables[ 0 ]->is_QSS() );
	EXPECT_TRUE( variables[ 1 ]->is_QSS() );
	EXPECT_TRUE( variables[ 2 ]->is_ZC() );
	EXPECT_TRUE( variables[ 3 ]->is_ZC() );
}

TEST( ContainerTest, SortByName )
{
	FMU_ME fmu;
	ZC2 zc2( &fmu, "ZC2" );
	ZC1 zc1( &fmu, "ZC1" );
	QSS2 qss2( &fmu, "QSS2" );
	QSS1 qss1( &fmu, "QSS1" );

	Variables variables( { &zc2, &zc1, &qss2, &qss1 } );

	EXPECT_FALSE( std::is_sorted( variables.begin(), variables.end(), []( Variable const * v1, Variable const * v2 ){ return v1->name() < v2->name(); } ) );
	sort_by_name( variables );
	EXPECT_TRUE( std::is_sorted( variables.begin(), variables.end(), []( Variable const * v1, Variable const * v2 ){ return v1->name() < v2->name(); } ) );
}

TEST( ContainerTest, Uniquify )
{
	FMU_ME fmu;
	ZC2 zc2( &fmu, "ZC2" );
	ZC1 zc1( &fmu, "ZC1" );
	QSS2 qss2( &fmu, "QSS2" );
	QSS1 qss1( &fmu, "QSS1" );

	Variables variables( { &zc2, &zc1, &qss2, &qss1, &zc2, &zc1, &qss2, &qss1 } );

	EXPECT_FALSE( is_unique( variables ) );
	uniquify( variables );
	EXPECT_TRUE( is_unique( variables ) );
}

TEST( ContainerTest, VariablesObservers )
{
	FMU_ME fmu;
	QSS2 v( &fmu, "v" );
	QSS2 h( &fmu, "h" );
	QSS2 z( &fmu, "z" );

	h.observers().push_back( &v );
	h.observers().push_back( &z );

	Variables triggers( { &v, &h } );

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
