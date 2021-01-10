// QSS::Depends Unit Tests
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
#include <QSS/Depends.hh>

using namespace QSS;

TEST( DependsTest, Basic )
{
	Depends depends;

	EXPECT_TRUE( depends.empty() );
	EXPECT_FALSE( depends.any() );
	EXPECT_FALSE( depends.all() );
	EXPECT_FALSE( depends.has( "var" ) );
	EXPECT_FALSE( depends.has( "var", "dep" ) );
	EXPECT_EQ( 0u, depends.size() );

	depends.add( Depends::regex( "vol*.T" ), { Depends::regex( "wall*.T" ), Depends::regex( "floor.T" ), Depends::regex( "ceil.T" ) } );

	EXPECT_FALSE( depends.empty() );
	EXPECT_TRUE( depends.any() );
	EXPECT_FALSE( depends.all() );
	EXPECT_TRUE( depends.has( "vol[1].T" ) );
	EXPECT_FALSE( depends.has( "Avol.T" ) );
	EXPECT_TRUE( depends.has( "vol[1].T", "wallSouth.T" ) );
	EXPECT_FALSE( depends.has( "vol[1].T", "WellSouth.T" ) );
	EXPECT_EQ( 1u, depends.size() );

	depends.add( Depends::regex( "mass*.U" ), { Depends::regex( "window*.U" ), Depends::regex( "door*.U" ) } );

	EXPECT_FALSE( depends.empty() );
	EXPECT_TRUE( depends.any() );
	EXPECT_FALSE( depends.all() );
	EXPECT_TRUE( depends.has( "mass[3].U" ) );
	EXPECT_FALSE( depends.has( "moss.U" ) );
	EXPECT_TRUE( depends.has( "mass[3].U", "door_4.U" ) );
	EXPECT_FALSE( depends.has( "mass[3].U", "skylight[55].U" ) );
	EXPECT_EQ( 2u, depends.size() );
}
