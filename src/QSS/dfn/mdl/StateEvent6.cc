// StateEvent6 Example Setup
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

//  model StateEvent6
//    // This model has 8 state events at t=1.35s,
//    // t = 2.39s, t = 3.85s, t = 4.9s, t = 6.35s,
//    // t = 7.4s, t = 8.85s, t = 9.9s
//    // when simulated from 0 to 10s.
//    Real x1(start=1.1, fixed=true);
//    Real x2(start=-2.5, fixed=true);
//    Real x3(start=4, fixed=true);
//    discrete Real y;
//    Modelica.Blocks.Interfaces.RealOutput __zc_z "Zero crossing";
//    Modelica.Blocks.Interfaces.RealOutput __zc_der_z
//      "Derivative of Zero crossing";
//  equation
//    der(x1) = cos(2*3.14*time/2.5);
//    der(x2) = 1;
//    der(x3) = -2;
//    __zc_z = x1 - 1;
//    __zc_der_z = der(x1 - 1);
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
#include <QSS/dfn/mdl/StateEvent6.hh>
#include <QSS/dfn/mdl/Function_LTI.hh>
#include <QSS/dfn/Conditional.hh>
#include <QSS/dfn/Variable_D.hh>
#include <QSS/dfn/Variable_LIQSS1.hh>
#include <QSS/dfn/Variable_LIQSS2.hh>
#include <QSS/dfn/Variable_QSS1.hh>
#include <QSS/dfn/Variable_QSS2.hh>
#include <QSS/dfn/Variable_QSS3.hh>
#include <QSS/dfn/Variable_ZC1.hh>
#include <QSS/dfn/Variable_ZC2.hh>
#include <QSS/math.hh>
#include <QSS/options.hh>

// C++ Headers
#include <cassert>
#include <cmath>

namespace QSS {
namespace dfn {
namespace mdl {

// Zero-Crossing Handler for StateEvent6 Model
template< typename V > // Template to avoid cyclic inclusion with Variable
class Handler_StateEvent6 final
{

public: // Types

	using Variable = V;
	using Variables = typename Variable::Variables;
	using Time = typename Variable::Time;
	using Value = typename Variable::Value;
	using Crossing = typename Variable::Crossing;
	using Variable_ZC_LTI = Variable_ZC< Function_LTI >;

public: // Properties

	// Apply at Time t
	void
	operator ()( Time const t )
	{
		Crossing const crossing( z_->crossing_prev );
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
	using Value = typename Variable::Value;
	using Coefficient = double;

public: // Creation

	// Default Constructor
	Function_x1()
	{}

public: // Properties

	// Continuous Value at Time t
	Value
	operator ()( Time const t ) const
	{
		return std::cos( c() * t );
	}

	// Continuous Value at Time t
	Value
	x( Time const t ) const
	{
		return std::cos( c() * t );
	}

	// Continuous First Derivative at Time t
	Value
	x1( Time const t ) const
	{
		return -c() * std::sin( c() * t );
	}

	// Quantized Value at Time t
	Value
	q( Time const t ) const
	{
		return std::cos( c() * t );
	}

	// Quantized First Derivative at Time t
	Value
	q1( Time const t ) const
	{
		return -c() * std::sin( c() * t );
	}

	// Quantized Second Derivative at Time t
	Value
	q2( Time const t ) const
	{
		return -square( c() ) * std::cos( c() * t );
	}

	// Quantized Sequential Value at Time t
	Value
	qs( Time const t ) const
	{
		return q( t );
	}

	// Quantized Forward-Difference Sequential First Derivative at Time t
	Value
	qf1( Time const t ) const
	{
		return q1( t );
	}

	// Quantized Centered-Difference Sequential First Derivative at Time t
	Value
	qc1( Time const t ) const
	{
		return q1( t );
	}

	// Quantized Centered-Difference Sequential Second Derivative at Time t
	Value
	qc2( Time const t ) const
	{
		return q2( t );
	}

	// Simultaneous Value at Time t
	Value
	s( Time const t ) const
	{
		return std::cos( c() * t );
	}

	// Simultaneous First Derivative at Time t
	Value
	s1( Time const t ) const
	{
		return -c() * std::sin( c() * t );
	}

	// Simultaneous Second Derivative at Time t
	Value
	s2( Time const t ) const
	{
		return -square( c() ) * std::cos( c() * t );
	}

	// Simultaneous Sequential Value at Time t
	Value
	ss( Time const t ) const
	{
		return s( t );
	}

	// Simultaneous Forward-Difference Sequential First Derivative at Time t
	Value
	sf1( Time const t ) const
	{
		return s1( t );
	}

	// Simultaneous Centered-Difference Sequential First Derivative at Time t
	Value
	sc1( Time const t ) const
	{
		return s1( t );
	}

	// Simultaneous Centered-Difference Sequential Second Derivative at Time t
	Value
	sc2( Time const t ) const
	{
		return s2( t );
	}

public: // Methods

	// Finalize Function Representation
	bool
	finalize( Variable * v )
	{
		assert( v != nullptr );
		return false; // Not self-observer
	}

	// Finalize Function Representation
	bool
	finalize( Variable & v )
	{
		return finalize( &v );
	}

public: // Static Methods

	// Time factor
	static
	Value
	c()
	{
		static Value const c_( 0.8 * pi );
		return c_;
	}

};

// StateEvent6 Example Setup
void
StateEvent6( Variables & vars, Conditionals & cons )
{
	using namespace options;

	// Timing
	if ( ! options::tEnd_set ) options::tEnd = 10.0;

	vars.clear();
	vars.reserve( 5 );

	// QSS variables
	using V1 = Variable_QSS< Function_x1 >;
	using V = Variable_QSS< Function_LTI >;
	V1 * x1( nullptr );
	V * x2( nullptr );
	V * x3( nullptr );
	if ( qss == QSS::QSS1 ) {
		vars.push_back( x1 = new Variable_QSS1< Function_x1 >( "x1", rTol, aTol, +1.1 ) );
		vars.push_back( x2 = new Variable_QSS1< Function_LTI >( "x2", rTol, aTol, -2.5 ) );
		vars.push_back( x3 = new Variable_QSS1< Function_LTI >( "x3", rTol, aTol, +4.0 ) );
	} else if ( qss == QSS::QSS2 ) {
		vars.push_back( x1 = new Variable_QSS2< Function_x1 >( "x1", rTol, aTol, +1.1 ) );
		vars.push_back( x2 = new Variable_QSS2< Function_LTI >( "x2", rTol, aTol, -2.5 ) );
		vars.push_back( x3 = new Variable_QSS2< Function_LTI >( "x3", rTol, aTol, +4.0 ) );
	} else if ( qss == QSS::QSS3 ) {
		vars.push_back( x1 = new Variable_QSS3< Function_x1 >( "x1", rTol, aTol, +1.1 ) );
		vars.push_back( x2 = new Variable_QSS3< Function_LTI >( "x2", rTol, aTol, -2.5 ) );
		vars.push_back( x3 = new Variable_QSS3< Function_LTI >( "x3", rTol, aTol, +4.0 ) );
	} else if ( qss == QSS::LIQSS1 ) {
		vars.push_back( x1 = new Variable_QSS1< Function_x1 >( "x1", rTol, aTol, +1.1 ) ); // Add q/slu1 to enable LIQSS1
		vars.push_back( x2 = new Variable_LIQSS1< Function_LTI >( "x2", rTol, aTol, -2.5 ) );
		vars.push_back( x3 = new Variable_LIQSS1< Function_LTI >( "x3", rTol, aTol, +4.0 ) );
	} else if ( qss == QSS::LIQSS2 ) {
		vars.push_back( x1 = new Variable_QSS2< Function_x1 >( "x1", rTol, aTol, +1.1 ) ); // Add q/xlu12 to enable LIQSS2
		vars.push_back( x2 = new Variable_LIQSS2< Function_LTI >( "x2", rTol, aTol, -2.5 ) );
		vars.push_back( x3 = new Variable_LIQSS2< Function_LTI >( "x3", rTol, aTol, +4.0 ) );
	} else {
		std::cerr << "Error: Unsupported QSS method" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Derivatives
	x2->d().add( +1.0 );
	x3->d().add( -2.0 );

	// Discrete variable
	Variable_D * y( new Variable_D( "y", 0 ) );
	vars.push_back( y );

	// Zero-crossings
	using Z = Variable_ZC< Function_LTI >;

	// Zero-crossing variable: x1 > 1
	Z * z1( nullptr );
	if ( ( qss == QSS::QSS1 ) || ( qss == QSS::LIQSS1 ) ) {
		vars.push_back( z1 = new Variable_ZC1< Function_LTI >( "z1", rTol, aTol ) );
	} else { // Use QSS2
		vars.push_back( z1 = new Variable_ZC2< Function_LTI >( "z1", rTol, aTol ) );
	}
	z1->add_crossings_Up();
	z1->f().add( x1 ).add( -1.0 );

	// Zero-crossing variable: x1 <= 1
	Z * z2( nullptr );
	if ( ( qss == QSS::QSS1 ) || ( qss == QSS::LIQSS1 ) ) {
		vars.push_back( z2 = new Variable_ZC1< Function_LTI >( "z2", rTol, aTol ) );
	} else { // Use QSS2
		vars.push_back( z2 = new Variable_ZC2< Function_LTI >( "z2", rTol, aTol ) );
	}
	z2->add_crossings_Dn();
	z2->f().add( x1 ).add( -1.0 );

	// Conditional
	using When = WhenV< Variable >;
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
//	if ( ( qss == QSS::QSS1 ) || ( qss == QSS::LIQSS1 ) ) {
//		vars.push_back( z = new Variable_ZC1< Function_LTI >( "z", rTol, aTol ) );
//	} else { // Use QSS2
//		vars.push_back( z = new Variable_ZC2< Function_LTI >( "z", rTol, aTol ) );
//	}
//	z->add_crossings_non_Flat();
//	z->f().add( x1 ).add( -1.0 );
//
//	// Conditional
//	using When = WhenV< Variable >;
//	When * zc( new When() );
//	cons.push_back( zc );
//	When::ClauseH< Handler_StateEvent6 > * zc_clause( zc->add_clause< Handler_StateEvent6 >() );
//	zc_clause->add( z );
//	zc_clause->h.var( y, z );
}

} // mdl
} // dfn
} // QSS
