// QSS::cod::container Unit Tests
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
#include <QSS/cod/container.hh>
#include <QSS/cod/Variable_QSS1.hh>
#include <QSS/cod/Variable_QSS2.hh>
#include <QSS/cod/Variable_ZC1.hh>
#include <QSS/cod/Variable_ZC2.hh>
#include <QSS/cod/mdl/Function_LTI.hh>

// C++ Headers
#include <algorithm>

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
	Variables variables;
	for ( size_type i = 0; i < 3; ++i ) { // Add variables out of order
		variables.push_back( new ZC2( "ZC2" ) );
		variables.push_back( new ZC1( "ZC1" ) );
		variables.push_back( new QSS2( "QSS2" ) );
		variables.push_back( new QSS1( "QSS1" ) );
	}
	EXPECT_FALSE( std::is_sorted( variables.begin(), variables.end(), []( Variable const * v1, Variable const * v2 ){ return v1->order() < v2->order(); } ) );
	sort_by_order( variables );
	EXPECT_TRUE( std::is_sorted( variables.begin(), variables.end(), []( Variable const * v1, Variable const * v2 ){ return v1->order() < v2->order(); } ) );
}

TEST( ContainerTest, SortByZC )
{
	Variables variables;
	for ( size_type i = 0; i < 3; ++i ) { // Add variables out of order
		variables.push_back( new ZC2( "ZC2" ) );
		variables.push_back( new ZC1( "ZC1" ) );
		variables.push_back( new QSS2( "QSS2" ) );
		variables.push_back( new QSS1( "QSS1" ) );
	}
	EXPECT_FALSE( std::is_sorted( variables.begin(), variables.end(), []( Variable const * v1, Variable const * v2 ){ return ( ! v1->is_ZC() ) && ( v2->is_ZC() ); } ) );
	sort_by_ZC( variables );
	EXPECT_TRUE( std::is_sorted( variables.begin(), variables.end(), []( Variable const * v1, Variable const * v2 ){ return ( ! v1->is_ZC() ) && ( v2->is_ZC() ); } ) );
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
}
