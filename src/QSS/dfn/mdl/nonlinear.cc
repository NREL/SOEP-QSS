// Nonlinear Derivative Example Setup
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017 Objexx Engineerinc, Inc. All rights reserved.
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
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// QSS Headers
#include <QSS/dfn/mdl/nonlinear.hh>
#include <QSS/dfn/mdl/Function_nonlinear.hh>
#include <QSS/dfn/Variable_LIQSS1.hh>
#include <QSS/dfn/Variable_LIQSS2.hh>
#include <QSS/dfn/Variable_QSS1.hh>
#include <QSS/dfn/Variable_QSS2.hh>
#include <QSS/dfn/Variable_QSS3.hh>
#include <QSS/options.hh>

// C++ Headers
#include <cstddef>
#include <fstream>

namespace QSS {
namespace dfn {
namespace mdl {

using Variables = std::vector< Variable * >;

// Nonlinear Derivative Example Setup
void
nonlinear( Variables & vars )
{
	using namespace options;

	// Timing
	if ( ! options::tEnd_set ) options::tEnd = 5.0;

	// Variables
	using V = Variable_QSS< Function_nonlinear >;
	V * y( nullptr );
	vars.clear();
	vars.reserve( 1 );
	if ( qss == QSS::QSS1 ) {
		vars.push_back( y = new Variable_QSS1< Function_nonlinear >( "y", rTol, aTol, 2.0 ) );
	} else if ( qss == QSS::QSS2 ) {
		vars.push_back( y = new Variable_QSS2< Function_nonlinear >( "y", rTol, aTol, 2.0 ) );
	} else if ( qss == QSS::QSS3 ) {
		vars.push_back( y = new Variable_QSS3< Function_nonlinear >( "y", rTol, aTol, 2.0 ) );
	} else if ( qss == QSS::LIQSS1 ) {
		vars.push_back( y = new Variable_LIQSS1< Function_nonlinear >( "y", rTol, aTol, 2.0 ) );
	} else if ( qss == QSS::LIQSS2 ) {
		vars.push_back( y = new Variable_LIQSS2< Function_nonlinear >( "y", rTol, aTol, 2.0 ) );
	} else {
		std::cerr << "Unsupported QSS method" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Derivatives
	y->d().var( y );

	// Analytical solution output
	std::ofstream e_stream( "y.e.out" );
	std::size_t iOut( 0 );
	double tOut( 0.0 );
	while ( tOut <= options::tEnd * ( 1.0 + 1.0e-14 ) ) {
		e_stream << tOut << '\t' << y->d().e( tOut ) << '\n';
		tOut = ( ++iOut ) * options::dtOut;
	}
	e_stream.close();
}

} // mdl
} // dfn
} // QSS
