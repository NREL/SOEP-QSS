// StateEvent6 Example
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2022 Objexx Engineering, Inc. All rights reserved.
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

//  model StateEvent6
//    // This model has 8 state events at t=1.35s,
//    // t = 2.39s, t = 3.85s, t = 4.9s, t = 6.35s,
//    // t = 7.4s, t = 8.85s, t = 9.9s
//    // when simulated from 0 to 10s.
//    Real x1(start=1.1, fixed=true);
//    Real x2(start=-2.5, fixed=true);
//    Real x3(start=4, fixed=true);
//    discrete Real y;
//  equation
//    der(x1) = cos(2*3.14*time/2.5);
//    der(x2) = 1;
//    der(x3) = -2;
//    when (x1 > 1) then
//      y = 1;
//    elsewhen (x1 <= 1) then
//      y = 0;
//    end when annotation (Documentation(info="<html>
//<p>
//This model has 8 state event at 1.35, 2.39,
//3.85, 4.9, 6.35, 7.4, 8.85, 9.9s when simulated from 0 to 10s.
//</p>
//</html>"));
//
//  end StateEvent6;

// Note: x1 has "accidental" deactivation at t=0: Suggest --dtInf=0.001

// QSS Headers
#include <QSS/cod/mdl/StateEvent6.hh>
#include <QSS/cod/mdl/Function_LTI.hh>
#include <QSS/cod/Conditional.hh>
#include <QSS/cod/Variable_all.hh>
#include <QSS/math.hh>
#include <QSS/options.hh>

// C++ Headers
#include <cassert>

namespace QSS {
namespace cod {
namespace mdl {

// Zero-Crossing Handler for StateEvent6 Model
template< typename V > // Template to avoid cyclic inclusion with Variable
class Handler_StateEvent6 final
{

public: // Types

	using Variable = V;
	using Variables = typename Variable::Variables;
	using Time = typename Variable::Time;
	using Real = typename Variable::Real;
	using Crossing = typename Variable::Crossing;
	using Variable_ZC_LTI = Variable_ZC< Function_LTI >;

public: // Property

	// Apply at Time t
	void
	operator ()( Time const t )
	{
		Crossing const crossing( z_->crossing_last );
		if ( crossing >= Crossing::Up ) { // Upward crossing
			y_->shift_handler( t, 1.0 );
		} else if ( crossing <= Crossing::Dn ) { // Downward crossing
			y_->shift_handler( t, 0.0 );
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
	 Variable_D * y,
	 Variable_ZC_LTI * z
	)
	{
		y_ = y;
		z_ = z;

		observers_.push_back( y );
	}

private: // Data

	Variable_D * y_{ nullptr };
	Variable_ZC_LTI * z_{ nullptr };
	Variables observers_;

};

// x1 Derivative Function
template< typename V > // Template to avoid cyclic inclusion with Variable
class Function_x1 final
{

public: // Types

	using Variable = V;
	using Time = typename Variable::Time;
	using Real = typename Variable::Real;
	using Coefficient = double;

public: // Creation

	// Default Constructor
	Function_x1() = default;

public: // Property

	// Continuous Value at Time t
	Real
	operator ()( Time const t ) const
	{
		return std::cos( c() * t );
	}

	// Continuous Value at Time t
	Real
	x( Time const t ) const
	{
		return std::cos( c() * t );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const t ) const
	{
		return -c() * std::sin( c() * t );
	}

	// Quantized Value at Time t
	Real
	q( Time const t ) const
	{
		return std::cos( c() * t );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const t ) const
	{
		return -c() * std::sin( c() * t );
	}

	// Quantized Second Derivative at Time t
	Real
	q2( Time const t ) const
	{
		return -square( c() ) * std::cos( c() * t );
	}

	// Quantized Sequential Value at Time t
	Real
	qs( Time const t ) const
	{
		return q( t );
	}

	// Quantized Forward-Difference Sequential First Derivative at Time t
	Real
	qf1( Time const t ) const
	{
		return q1( t );
	}

	// Quantized Centered-Difference Sequential First Derivative at Time t
	Real
	qc1( Time const t ) const
	{
		return q1( t );
	}

	// Quantized Centered-Difference Sequential Second Derivative at Time t
	Real
	qc2( Time const t ) const
	{
		return q2( t );
	}

public: // Static Methods

	// Time factor
	static
	Real
	c()
	{
		static Real const c_( 0.8 * pi );
		return c_;
	}

};

// StateEvent6 Example Setup
void
StateEvent6( Variable::Variables & vars, std::vector< Conditional * > & cons )
{
	using namespace options;

	// Timing
	if ( !options::specified::tEnd ) options::tEnd = 10.0;

	vars.clear();
	vars.reserve( 5 );

	// QSS variables
	using V1 = Variable_QSS< Function_x1 >;
	using V = Variable_QSS< Function_LTI >;
	V1 * x1( nullptr );
	V * x2( nullptr );
	V * x3( nullptr );
	// Note: No variables are self-observers so LIQSS methods offer no benefit
	if ( qss == QSS::QSS1 ) {
		vars.push_back( x1 = new Variable_QSS1< Function_x1 >( "x1", rTol, aTol, 1.0e-9, +1.1 ) );
		vars.push_back( x2 = new Variable_QSS1< Function_LTI >( "x2", rTol, aTol, 1.0e-9, -2.5 ) );
		vars.push_back( x3 = new Variable_QSS1< Function_LTI >( "x3", rTol, aTol, 1.0e-9, +4.0 ) );
	} else if ( qss == QSS::QSS2 ) {
		vars.push_back( x1 = new Variable_QSS2< Function_x1 >( "x1", rTol, aTol, 1.0e-9, +1.1 ) );
		vars.push_back( x2 = new Variable_QSS2< Function_LTI >( "x2", rTol, aTol, 1.0e-9, -2.5 ) );
		vars.push_back( x3 = new Variable_QSS2< Function_LTI >( "x3", rTol, aTol, 1.0e-9, +4.0 ) );
	} else if ( qss == QSS::QSS3 ) {
		vars.push_back( x1 = new Variable_QSS3< Function_x1 >( "x1", rTol, aTol, 1.0e-9, +1.1 ) );
		vars.push_back( x2 = new Variable_QSS3< Function_LTI >( "x2", rTol, aTol, 1.0e-9, -2.5 ) );
		vars.push_back( x3 = new Variable_QSS3< Function_LTI >( "x3", rTol, aTol, 1.0e-9, +4.0 ) );
	} else if ( qss == QSS::LIQSS1 ) {
		vars.push_back( x1 = new Variable_QSS1< Function_x1 >( "x1", rTol, aTol, 1.0e-9, +1.1 ) ); // Add qlu1 to enable LIQSS1
		vars.push_back( x2 = new Variable_LIQSS1< Function_LTI >( "x2", rTol, aTol, 1.0e-9, -2.5 ) );
		vars.push_back( x3 = new Variable_LIQSS1< Function_LTI >( "x3", rTol, aTol, 1.0e-9, +4.0 ) );
	} else if ( qss == QSS::LIQSS2 ) {
		vars.push_back( x1 = new Variable_QSS2< Function_x1 >( "x1", rTol, aTol, 1.0e-9, +1.1 ) ); // Add qlu1/2 to enable LIQSS2
		vars.push_back( x2 = new Variable_LIQSS2< Function_LTI >( "x2", rTol, aTol, 1.0e-9, -2.5 ) );
		vars.push_back( x3 = new Variable_LIQSS2< Function_LTI >( "x3", rTol, aTol, 1.0e-9, +4.0 ) );
	} else if ( qss == QSS::LIQSS3 ) {
		vars.push_back( x1 = new Variable_QSS3< Function_x1 >( "x1", rTol, aTol, 1.0e-9, +1.1 ) ); // Add qlu1/2/3 to enable LIQSS23
		vars.push_back( x2 = new Variable_LIQSS3< Function_LTI >( "x2", rTol, aTol, 1.0e-9, -2.5 ) );
		vars.push_back( x3 = new Variable_LIQSS3< Function_LTI >( "x3", rTol, aTol, 1.0e-9, +4.0 ) );
	} else if ( qss == QSS::xQSS1 ) {
		vars.push_back( x1 = new Variable_xQSS1< Function_x1 >( "x1", rTol, aTol, 1.0e-9, +1.1 ) );
		vars.push_back( x2 = new Variable_xQSS1< Function_LTI >( "x2", rTol, aTol, 1.0e-9, -2.5 ) );
		vars.push_back( x3 = new Variable_xQSS1< Function_LTI >( "x3", rTol, aTol, 1.0e-9, +4.0 ) );
	} else if ( qss == QSS::xQSS2 ) {
		vars.push_back( x1 = new Variable_xQSS2< Function_x1 >( "x1", rTol, aTol, 1.0e-9, +1.1 ) );
		vars.push_back( x2 = new Variable_xQSS2< Function_LTI >( "x2", rTol, aTol, 1.0e-9, -2.5 ) );
		vars.push_back( x3 = new Variable_xQSS2< Function_LTI >( "x3", rTol, aTol, 1.0e-9, +4.0 ) );
	} else if ( qss == QSS::xQSS3 ) {
		vars.push_back( x1 = new Variable_xQSS3< Function_x1 >( "x1", rTol, aTol, 1.0e-9, +1.1 ) );
		vars.push_back( x2 = new Variable_xQSS3< Function_LTI >( "x2", rTol, aTol, 1.0e-9, -2.5 ) );
		vars.push_back( x3 = new Variable_xQSS3< Function_LTI >( "x3", rTol, aTol, 1.0e-9, +4.0 ) );
	} else if ( qss == QSS::xLIQSS1 ) {
		vars.push_back( x1 = new Variable_xQSS1< Function_x1 >( "x1", rTol, aTol, 1.0e-9, +1.1 ) ); // Add qlu1 to enable xLIQSS1
		vars.push_back( x2 = new Variable_xLIQSS1< Function_LTI >( "x2", rTol, aTol, 1.0e-9, -2.5 ) );
		vars.push_back( x3 = new Variable_xLIQSS1< Function_LTI >( "x3", rTol, aTol, 1.0e-9, +4.0 ) );
	} else if ( qss == QSS::xLIQSS2 ) {
		vars.push_back( x1 = new Variable_xQSS2< Function_x1 >( "x1", rTol, aTol, 1.0e-9, +1.1 ) ); // Add qlu1/2 to enable xLIQSS2
		vars.push_back( x2 = new Variable_xLIQSS2< Function_LTI >( "x2", rTol, aTol, 1.0e-9, -2.5 ) );
		vars.push_back( x3 = new Variable_xLIQSS2< Function_LTI >( "x3", rTol, aTol, 1.0e-9, +4.0 ) );
	} else if ( qss == QSS::xLIQSS3 ) {
		vars.push_back( x1 = new Variable_xQSS3< Function_x1 >( "x1", rTol, aTol, 1.0e-9, +1.1 ) ); // Add qlu1/2/3 to enable xLIQSS3
		vars.push_back( x2 = new Variable_xLIQSS3< Function_LTI >( "x2", rTol, aTol, 1.0e-9, -2.5 ) );
		vars.push_back( x3 = new Variable_xLIQSS3< Function_LTI >( "x3", rTol, aTol, 1.0e-9, +4.0 ) );
	} else {
		std::cerr << "Error: Unsupported QSS method" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Derivatives
	x2->add( +1.0 );
	x3->add( -2.0 );

	// Discrete variable
	Variable_D * y( new Variable_D( "y", 0 ) );
	vars.push_back( y );

	// Zero-crossings
	using Z = Variable_ZC< Function_LTI >;

	// Zero-crossing variable: x1 > 1
	Z * z1( nullptr );
	if ( ( qss == QSS::QSS1 ) || ( qss == QSS::LIQSS1 ) || ( qss == QSS::xQSS1 ) || ( qss == QSS::xLIQSS1 ) ) {
		vars.push_back( z1 = new Variable_ZC1< Function_LTI >( "z1", rTol, aTol, zTol ) );
	} else if ( ( qss == QSS::QSS2 ) || ( qss == QSS::LIQSS2 ) || ( qss == QSS::xQSS2 ) || ( qss == QSS::xLIQSS2 ) ) {
		vars.push_back( z1 = new Variable_ZC2< Function_LTI >( "z1", rTol, aTol, zTol ) );
	} else {
		vars.push_back( z1 = new Variable_ZC3< Function_LTI >( "z1", rTol, aTol, zTol ) );
	}
	z1->add_crossings_Up();
	z1->add( x1 ).add( -1.0 );

	// Zero-crossing variable: x1 <= 1
	Z * z2( nullptr );
	if ( ( qss == QSS::QSS1 ) || ( qss == QSS::LIQSS1 ) || ( qss == QSS::xQSS1 ) || ( qss == QSS::xLIQSS1 ) ) {
		vars.push_back( z2 = new Variable_ZC1< Function_LTI >( "z2", rTol, aTol, zTol ) );
	} else if ( ( qss == QSS::QSS2 ) || ( qss == QSS::LIQSS2 ) || ( qss == QSS::xQSS2 ) || ( qss == QSS::xLIQSS2 ) ) {
		vars.push_back( z2 = new Variable_ZC2< Function_LTI >( "z2", rTol, aTol, zTol ) );
	} else {
		vars.push_back( z2 = new Variable_ZC3< Function_LTI >( "z2", rTol, aTol, zTol ) );
	}
	z2->add_crossings_Dn();
	z2->add( x1 ).add( -1.0 );

	// Conditional
	using When = Conditional_When< Variable >;
	When * zc( new When() );
	cons.push_back( zc );
	// Using the same handler for both crossings but in general would need separate handlers
	When::ClauseH< Handler_StateEvent6 > * zc1_clause( zc->add_clause< Handler_StateEvent6 >() );
	zc1_clause->add( z1 );
	zc1_clause->h.var( y, z1 );
	When::ClauseH< Handler_StateEvent6 > * zc2_clause( zc->add_clause< Handler_StateEvent6 >() );
	zc2_clause->add( z2 );
	zc2_clause->h.var( y, z2 );

// Alternative with one zero-crossing variable and clause
//  Works because the zero-crossings have the same function and the handler processes by crossing type
//  Two variable/clause method above shows use of an elsewhen clause for more general situations
//
//	// Zero-crossing variable: x1 - 1
//	Z * z( nullptr );
//	if ( ( qss == QSS::QSS1 ) || ( qss == QSS::LIQSS1 ) || ( qss == QSS::xQSS1 ) || ( qss == QSS::xLIQSS1 ) ) {
//		vars.push_back( z = new Variable_ZC1< Function_LTI >( "z", rTol, aTol, zTol ) );
//	} else if ( ( qss == QSS::QSS2 ) || ( qss == QSS::LIQSS2 ) || ( qss == QSS::xQSS2 ) || ( qss == QSS::xLIQSS2 ) ) {
//		vars.push_back( z = new Variable_ZC2< Function_LTI >( "z", rTol, aTol, zTol ) );
//	} else {
//		vars.push_back( z = new Variable_ZC3< Function_LTI >( "z", rTol, aTol, zTol ) );
//	}
//	z->add_crossings_non_Flat();
//	z->add( x1 ).add( -1.0 );
//
//	// Conditional
//	using When = Conditional_When< Variable >;
//	When * zc( new When() );
//	cons.push_back( zc );
//	When::ClauseH< Handler_StateEvent6 > * zc_clause( zc->add_clause< Handler_StateEvent6 >() );
//	zc_clause->add( z );
//	zc_clause->h.var( y, z );
}

} // mdl
} // cod
} // QSS
