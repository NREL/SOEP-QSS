// Bouncing Ball Example
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2019 Objexx Engineering, Inc. All rights reserved.
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
#include <QSS/cod/mdl/bball.hh>
#include <QSS/cod/mdl/Function_LTI.hh>
#include <QSS/cod/Conditional.hh>
#include <QSS/cod/Variable_all.hh>
#include <QSS/options.hh>

// C++ Headers
#include <cmath>

namespace QSS {
namespace cod {
namespace mdl {

// Zero-Crossing Handler for Bouncing Ball Model
template< typename V > // Template to avoid cyclic inclusion with Variable
class Handler_bball final
{

public: // Types

	using Variable = V;
	using Variables = typename Variable::Variables;
	using Time = typename Variable::Time;
	using Real = typename Variable::Real;
	using Crossing = typename Variable::Crossing;
	using Variable_QSS_LTI = Variable_QSS< Function_LTI >;
	using Variable_ZC_LTI = Variable_ZC< Function_LTI >;

public: // Properties

	// Apply at Time t
	void
	operator ()( Time const t )
	{
		Crossing const crossing( z_->crossing_last );
		if ( crossing <= Crossing::Flat ) { // Downward zero-crossing or stationary on floor
			Real const v( v_->x( t ) );
			if ( ( std::abs( v ) <= 0.01 ) && ( h_->x( t ) <= 0.0 ) ) { // Treat as stationary on floor
				v_->add( 0.0 ); // Set velocity derivative to zero
				v_->shift_handler( t, 0.0 );
				h_->shift_handler( t, 0.0 );
			} else {
				if ( v < 0.0 ) v_->shift_handler( t, -0.7 * v ); // Coefficient of restitution = 0.7
				h_->shift_handler( t, 0.0 );
				b_->shift_handler( t, b_->r() + 1 ); // Add to bounce count
			}
		}
	}

	// Handler-Modified Variables
	Variables const &
	observers() const
	{
		return observers_;
	}

	// Handler-Modified Variables
	Variables &
	observers()
	{
		return observers_;
	}

public: // Methods

	// Set Variables
	void
	var(
	 Variable_QSS_LTI * h,
	 Variable_QSS_LTI * v,
//	 Variable_D * b,
	 Variable_I * b,
	 Variable_ZC_LTI * z
	)
	{
		h_ = h;
		v_ = v;
		b_ = b;
		z_ = z;

		observers_.push_back( h );
		observers_.push_back( v );
		observers_.push_back( b );
	}

private: // Data

	Variable_QSS_LTI * h_{ nullptr };
	Variable_QSS_LTI * v_{ nullptr };
//	Variable_D * b_{ nullptr };
	Variable_I * b_{ nullptr };
	Variable_ZC_LTI * z_{ nullptr };
	Variables observers_;

};

// Bouncing Ball Example Setup
void
bball( Variable::Variables & vars, std::vector< Conditional * > & cons )
{
	using namespace options;

	// Timing
	if ( ! options::specified::tEnd ) options::tEnd = 3.0;

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
	} else if ( qss == QSS::LIQSS3 ) {
		vars.push_back( h = new Variable_LIQSS3< Function_LTI >( "h", rTol, aTol, 1.0 ) );
		vars.push_back( v = new Variable_LIQSS3< Function_LTI >( "v", rTol, aTol, 0.0 ) );
	} else if ( qss == QSS::xQSS1 ) {
		vars.push_back( h = new Variable_xQSS1< Function_LTI >( "h", rTol, aTol, 1.0 ) );
		vars.push_back( v = new Variable_xQSS1< Function_LTI >( "v", rTol, aTol, 0.0 ) );
	} else if ( qss == QSS::xQSS2 ) {
		vars.push_back( h = new Variable_xQSS2< Function_LTI >( "h", rTol, aTol, 1.0 ) );
		vars.push_back( v = new Variable_xQSS2< Function_LTI >( "v", rTol, aTol, 0.0 ) );
	} else if ( qss == QSS::xQSS3 ) {
		vars.push_back( h = new Variable_xQSS3< Function_LTI >( "h", rTol, aTol, 1.0 ) );
		vars.push_back( v = new Variable_xQSS3< Function_LTI >( "v", rTol, aTol, 0.0 ) );
	} else if ( qss == QSS::xLIQSS1 ) {
		vars.push_back( h = new Variable_xLIQSS1< Function_LTI >( "h", rTol, aTol, 1.0 ) );
		vars.push_back( v = new Variable_xLIQSS1< Function_LTI >( "v", rTol, aTol, 0.0 ) );
	} else if ( qss == QSS::xLIQSS2 ) {
		vars.push_back( h = new Variable_xLIQSS2< Function_LTI >( "h", rTol, aTol, 1.0 ) );
		vars.push_back( v = new Variable_xLIQSS2< Function_LTI >( "v", rTol, aTol, 0.0 ) );
	} else if ( qss == QSS::xLIQSS3 ) {
		vars.push_back( h = new Variable_xLIQSS3< Function_LTI >( "h", rTol, aTol, 1.0 ) );
		vars.push_back( v = new Variable_xLIQSS3< Function_LTI >( "v", rTol, aTol, 0.0 ) );
	} else {
		std::cerr << "Error: Unsupported QSS method" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Derivatives
	h->add( v );
	v->add( -9.80665 ); // g = 9.80665 m/s^2

	// Discrete variable
//	Variable_D * b( new Variable_D( "b", 0 ) ); // Bounce counter (to demo discrete variables)
	Variable_I * b( new Variable_I( "b", 0 ) ); // Bounce counter (to demo integer variables)
	vars.push_back( b );

	// Zero-crossing variable
	using Z = Variable_ZC< Function_LTI >;
	Z * z( nullptr ); // Height (m) zero-crossing
	if ( ( qss == QSS::QSS1 ) || ( qss == QSS::LIQSS1 ) || ( qss == QSS::xQSS1 ) || ( qss == QSS::xLIQSS1 ) ) {
		vars.push_back( z = new Variable_ZC1< Function_LTI >( "z", rTol, aTol, zTol ) );
	} else if ( ( qss == QSS::QSS2 ) || ( qss == QSS::LIQSS2 ) || ( qss == QSS::xQSS2 ) || ( qss == QSS::xLIQSS2 ) ) {
		vars.push_back( z = new Variable_ZC2< Function_LTI >( "z", rTol, aTol, zTol ) );
	} else {
		vars.push_back( z = new Variable_ZC3< Function_LTI >( "z", rTol, aTol, zTol ) );
	}
	z->add_crossings_Dn_Flat();
	z->add( h );

	// Conditional
	using When = Conditional_When< Variable >;
	When * hit( new When() );
	cons.push_back( hit );
	When::ClauseH< Handler_bball > * hit_floor( hit->add_clause< Handler_bball >() );
	hit_floor->add( z );
	hit_floor->h.var( h, v, b, z );
}

} // mdl
} // cod
} // QSS
