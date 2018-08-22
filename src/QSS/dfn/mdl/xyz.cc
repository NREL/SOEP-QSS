// Simple xyz Example
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

// QSS Headers
#include <QSS/dfn/mdl/xyz.hh>
#include <QSS/dfn/mdl/Function_LTI.hh>
#include <QSS/dfn/Variable_all.hh>
#include <QSS/options.hh>

namespace QSS {
namespace dfn {
namespace mdl {

using Variables = std::vector< Variable * >;

// Simple xyz Example Setup
void
xyz( Variables & vars )
{
	using namespace options;

	// Timing
	if ( ! options::specified::tEnd ) options::tEnd = 2.0;

	// Variables
	using V = Variable_QSS< Function_LTI >;
	V * x( nullptr );
	V * y( nullptr );
	V * z( nullptr );
	vars.clear();
	vars.reserve( 3 );
	if ( qss == QSS::QSS1 ) {
		vars.push_back( x = new Variable_QSS1< Function_LTI >( "x", rTol, aTol, 0.0 ) );
		vars.push_back( y = new Variable_QSS1< Function_LTI >( "y", rTol, aTol, 0.0 ) );
		vars.push_back( z = new Variable_QSS1< Function_LTI >( "z", rTol, aTol, 0.0 ) );
	} else if ( qss == QSS::QSS2 ) {
		vars.push_back( x = new Variable_QSS2< Function_LTI >( "x", rTol, aTol, 0.0 ) );
		vars.push_back( y = new Variable_QSS2< Function_LTI >( "y", rTol, aTol, 0.0 ) );
		vars.push_back( z = new Variable_QSS2< Function_LTI >( "z", rTol, aTol, 0.0 ) );
	} else if ( qss == QSS::QSS3 ) {
		vars.push_back( x = new Variable_QSS3< Function_LTI >( "x", rTol, aTol, 0.0 ) );
		vars.push_back( y = new Variable_QSS3< Function_LTI >( "y", rTol, aTol, 0.0 ) );
		vars.push_back( z = new Variable_QSS3< Function_LTI >( "z", rTol, aTol, 0.0 ) );
	} else if ( qss == QSS::LIQSS1 ) {
		vars.push_back( x = new Variable_LIQSS1< Function_LTI >( "x", rTol, aTol, 0.0 ) );
		vars.push_back( y = new Variable_LIQSS1< Function_LTI >( "y", rTol, aTol, 0.0 ) );
		vars.push_back( z = new Variable_LIQSS1< Function_LTI >( "z", rTol, aTol, 0.0 ) );
	} else if ( qss == QSS::LIQSS2 ) {
		vars.push_back( x = new Variable_LIQSS2< Function_LTI >( "x", rTol, aTol, 0.0 ) );
		vars.push_back( y = new Variable_LIQSS2< Function_LTI >( "y", rTol, aTol, 0.0 ) );
		vars.push_back( z = new Variable_LIQSS2< Function_LTI >( "z", rTol, aTol, 0.0 ) );
	} else {
		std::cerr << "Error: Unsupported QSS method" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Derivatives
	x->add( 1.0 );
	y->add( 1.0, x );
	z->add( 1.0, y );
}

} // mdl
} // dfn
} // QSS
