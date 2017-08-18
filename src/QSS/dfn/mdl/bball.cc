// Bouncing Ball Example Setup
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
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, THE UNITED STATES
// GOVERNMENT, OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// QSS Headers
#include <QSS/dfn/mdl/bball.hh>
#include <QSS/dfn/mdl/Function_LTI.hh>
#include <QSS/dfn/Variable_D.hh>
#include <QSS/dfn/Variable_LIQSS1.hh>
#include <QSS/dfn/Variable_LIQSS2.hh>
#include <QSS/dfn/Variable_QSS1.hh>
#include <QSS/dfn/Variable_QSS2.hh>
#include <QSS/dfn/Variable_QSS3.hh>
#include <QSS/dfn/Variable_ZC1.hh>
#include <QSS/dfn/Variable_ZC2.hh>
#include <QSS/options.hh>

// C++ Headers
#include <cmath>

namespace QSS {
namespace dfn {
namespace mdl {

// Zero-Crossing Handler for Bouncing Ball Model
template< typename V > // Template to avoid cyclic inclusion with Variable
class Handler_bball
{

public: // Types

	using Variable = V;
	using Time = typename Variable::Time;
	using Value = typename Variable::Value;
	using Crossing = typename Variable::Crossing;

public: // Properties

	// Apply at Time t
	void
	operator ()( Time const t, Crossing const crossing )
	{
		if ( crossing <= Crossing::Flat ) { // Downward zero-crossing or stationary on floor
			Value const v( v_->x( t ) );
			if ( ( std::abs( v ) <= 0.01 ) && ( h_->x( t ) <= 0.0 ) ) { // Treat as stationary on floor
				dynamic_cast< Variable_QSS< Function_LTI > * >( v_ )->d().add( 0.0 );
				v_->shift_handler( t, 0.0 );
				h_->shift_handler( t, 0.0 );
			} else {
				if ( v < 0.0 ) v_->shift_handler( t, -0.7 * v ); // Coefficient of restitution = 0.7
				h_->shift_handler( t, 0.0 );
				b_->shift_handler( t, b_->x() + 1 ); // Add to bounce count
			}
		}
	}

public: // Methods

	// Set Variables
	void
	var(
	 Variable_QSS< Function_LTI >* h,
	 Variable_QSS< Function_LTI >* v,
	 Variable_D * b
	)
	{
		h_ = h;
		v_ = v;
		b_ = b;
	}

private: // Data

	Variable_QSS< Function_LTI > * h_{ nullptr };
	Variable_QSS< Function_LTI > * v_{ nullptr };
	Variable_D * b_{ nullptr };

};

// Bouncing Ball Example Setup
void
bball( Variables & vars )
{
	using namespace options;

	// Timing
	if ( ! options::tEnd_set ) options::tEnd = 3.0;

	// QSS variables
	using V = Variable_QSS< Function_LTI >;
	V * h( nullptr ); // Height (m)
	V * v( nullptr ); // Velocity (m/s)
	vars.clear();
	vars.reserve( 4 );
	if ( qss == QSS::QSS1 ) {
		vars.push_back( h = new Variable_QSS1< Function_LTI >( "h", rTol, aTol, 1.0 ) );
		vars.push_back( v = new Variable_QSS1< Function_LTI >( "v", rTol, aTol, 0.0 ) );
	} else if ( qss == QSS::QSS2 ) {
		vars.push_back( h = new Variable_QSS2< Function_LTI >( "h", rTol, aTol, 1.0 ) );
		vars.push_back( v = new Variable_QSS2< Function_LTI >( "v", rTol, aTol, 0.0 ) );
	} else if ( qss == QSS::QSS3 ) {
		vars.push_back( h = new Variable_QSS3< Function_LTI >( "h", rTol, aTol, 1.0 ) );
		vars.push_back( v = new Variable_QSS3< Function_LTI >( "v", rTol, aTol, 0.0 ) );
	} else if ( qss == QSS::LIQSS1 ) {
		vars.push_back( h = new Variable_LIQSS1< Function_LTI >( "h", rTol, aTol, 1.0 ) );
		vars.push_back( v = new Variable_LIQSS1< Function_LTI >( "v", rTol, aTol, 0.0 ) );
	} else if ( qss == QSS::LIQSS2 ) {
		vars.push_back( h = new Variable_LIQSS2< Function_LTI >( "h", rTol, aTol, 1.0 ) );
		vars.push_back( v = new Variable_LIQSS2< Function_LTI >( "v", rTol, aTol, 0.0 ) );
	} else {
		std::cerr << "Error: Unsupported QSS method" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Derivatives
	h->d().add( v );
	v->d().add( -9.80665 ); // g = 9.80665 m/s^2

	// Discrete variable
	Variable_D * b( new Variable_D( "b", 0 ) ); // Bounce counter (to demo discrete variables)
	vars.push_back( b );

	// Zero-crossing variable
	using Z = Variable_ZC< Function_LTI, Handler_bball >;
	Z * z( nullptr ); // Height (m) zero-crossing
	if ( ( qss == QSS::QSS1 ) || ( qss == QSS::LIQSS1 ) ) {
		vars.push_back( z = new Variable_ZC1< Function_LTI, Handler_bball >( "z", rTol, aTol ) );
	} else { // Use QSS2
		vars.push_back( z = new Variable_ZC2< Function_LTI, Handler_bball >( "z", rTol, aTol ) );
	}
	z->add_crossings_Dn_Flat();
	z->f().add( h );
	z->h().var( h, v, b );
}

} // mdl
} // dfn
} // QSS
