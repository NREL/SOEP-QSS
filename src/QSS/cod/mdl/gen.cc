// Generated Example
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
#include <QSS/cod/mdl/gen.hh>
#include <QSS/cod/mdl/Function_LTI.hh>
#include <QSS/cod/Conditional.hh>
#include <QSS/cod/Variable_all.hh>
#include <QSS/options.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <random>
#include <sstream>
#include <unordered_set>
#include <vector>

namespace QSS {
namespace cod {
namespace mdl {

namespace { // Internal shared global
std::default_random_engine random_generator;
}

// Uniform-Random Real on [l,u]
template< typename T >
T
uniform_random_real( T const l = T( 0 ), T const u = T( 1 ) )
{
	assert( l <= u );
	static std::uniform_real_distribution< T > distribution( T( 0.0 ), std::nextafter( T( 1.0 ), std::numeric_limits< T >::max() ) ); // [0,1]
	return ( distribution( random_generator ) * ( u - l ) ) + l; // Shift from [0,1] to [l,u]
}

// Uniform-Random Integer on [0,(2^15)-1]
template< typename T >
T
uniform_random_integer( T const l = T( 0 ), T const u = T( 1 ) )
{
	assert( l <= u );
	T const b( std::numeric_limits< T >::max() );
	static std::uniform_int_distribution< T > distribution( T( 0 ), b ); // [0,b]
	return static_cast< T >( distribution( random_generator ) * ( double( u - l + 1 ) / ( double( b ) + 1.0 ) ) ) + l; // Shift from [0,b] to [l,u]
}

// Zero-Crossing Handler
template< typename V > // Template to avoid cyclic inclusion with Variable
class Handler_gen final
{

public: // Types

	using Variable = V;
	using Time = typename Variable::Time;
	using Real = typename Variable::Real;
	using Reals = typename Variable::Reals;
	using Variables = typename Variable::Variables;
	using size_type = typename Variables::size_type;
	using Crossing = typename Variable::Crossing;
	using Variable_QSS_LTI = Variable_QSS< Function_LTI >;
	using Variable_ZC_LTI = Variable_ZC< Function_LTI >;

public: // Property

	// Apply at Time t
	void
	operator ()( Time const t )
	{
		Crossing const crossing( z_->crossing_last );
		if ( crossing <= Crossing::Dn ) { // Downward zero-crossing
			for ( size_type i = 0, e = observers_.size(); i < e; ++i ) {
				observers_[ i ]->shift_handler( t, dn_[ i ] );
			}
		} else if ( crossing >= Crossing::Up ) { // Upward zero-crossing
			for ( size_type i = 0, e = observers_.size(); i < e; ++i ) {
				observers_[ i ]->shift_handler( t, up_[ i ] );
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
	var( Variable_ZC_LTI * z )
	{
		z_ = z;
	}

	// Add a QSS Variable
	void
	add(
	 Variable_QSS_LTI * v, // Variable
	 Real const dn, // Value to set on downward crossing
	 Real const up // Value to set on upward crossing
	)
	{
		assert( std::find( observers_.begin(), observers_.end(), v ) == observers_.end() );
		observers_.push_back( v );
		dn_.push_back( dn );
		up_.push_back( up );
	}

private: // Data

	Reals dn_; // Values to set on downward crossing
	Reals up_; // Values to set on upward crossing
	Variable_ZC_LTI * z_{ nullptr }; // Associated zero-crossing variable
	Variables observers_;

};

// Generated Example Setup
void
gen(
 Variable::Variables & vars,
 std::vector< Conditional * > & cons,
 Variable::size_type const nQSS,
 Variable::size_type const nZC,
 Variable::size_type const seed,
 bool const do_seed
)
{
	using namespace options;
	using Real = Variable::Real;
	using size_type = Variable::size_type;
	using Indexes = std::unordered_set< size_type >;

	// Parameters
	Real const x_mag( 100.0 ); // Variable initial value range
	Real const c_mag( 0.5 ); // Derivative coefficient range
	Real const h_mag( x_mag ); // Handler value range
	size_type const deg_QSS( static_cast< size_type >( std::sqrt( Real( nQSS ) ) ) ); // QSS dependency out-degree range
	size_type const deg_ZC( static_cast< size_type >( std::sqrt( Real( nQSS ) ) ) ); // ZC dependency out-degree range
	size_type const deg_handler( static_cast< size_type >( 4 ) ); // Conditional handler dependency out-degree range

	// Initialization
	if ( do_seed ) {
		if ( seed == 0u ) { // Use time as seed
			random_generator.seed( static_cast< uint_fast32_t >( std::chrono::system_clock::now().time_since_epoch().count() ) );
		} else { // Use specified seed
			random_generator.seed( static_cast< uint_fast32_t >( seed ) );
		}
	}
	vars.clear();
	vars.reserve( nQSS + nZC );

	// QSS variables
	using V = Variable_QSS< Function_LTI >;
	for ( size_type i = 1; i <= nQSS; ++i ) {
		V * x( nullptr );
		std::ostringstream i_stream; i_stream << i;
		std::string const x_nam( "x" + i_stream.str() );
		Real const x_ini( uniform_random_real( -x_mag, x_mag ) );
		if ( qss == QSS::QSS1 ) {
			vars.push_back( x = new Variable_QSS1< Function_LTI >( x_nam, rTol, aTol, x_ini ) );
		} else if ( qss == QSS::QSS2 ) {
			vars.push_back( x = new Variable_QSS2< Function_LTI >( x_nam, rTol, aTol, x_ini ) );
		} else if ( qss == QSS::QSS3 ) {
			vars.push_back( x = new Variable_QSS3< Function_LTI >( x_nam, rTol, aTol, x_ini ) );
		} else if ( qss == QSS::LIQSS1 ) {
			vars.push_back( x = new Variable_LIQSS1< Function_LTI >( x_nam, rTol, aTol, x_ini ) );
		} else if ( qss == QSS::LIQSS2 ) {
			vars.push_back( x = new Variable_LIQSS2< Function_LTI >( x_nam, rTol, aTol, x_ini ) );
		} else if ( qss == QSS::LIQSS3 ) {
			vars.push_back( x = new Variable_LIQSS3< Function_LTI >( x_nam, rTol, aTol, x_ini ) );
		} else if ( qss == QSS::xQSS1 ) {
			vars.push_back( x = new Variable_xQSS1< Function_LTI >( x_nam, rTol, aTol, x_ini ) );
		} else if ( qss == QSS::xQSS2 ) {
			vars.push_back( x = new Variable_xQSS2< Function_LTI >( x_nam, rTol, aTol, x_ini ) );
		} else if ( qss == QSS::xQSS3 ) {
			vars.push_back( x = new Variable_xQSS3< Function_LTI >( x_nam, rTol, aTol, x_ini ) );
		} else if ( qss == QSS::xLIQSS1 ) {
			vars.push_back( x = new Variable_xLIQSS1< Function_LTI >( x_nam, rTol, aTol, x_ini ) );
		} else if ( qss == QSS::xLIQSS2 ) {
			vars.push_back( x = new Variable_xLIQSS2< Function_LTI >( x_nam, rTol, aTol, x_ini ) );
		} else if ( qss == QSS::xLIQSS3 ) {
			vars.push_back( x = new Variable_xLIQSS3< Function_LTI >( x_nam, rTol, aTol, x_ini ) );
		} else {
			std::cerr << "Error: Unsupported QSS method" << std::endl;
			std::exit( EXIT_FAILURE );
		}
	}

	// QSS derivatives
	std::cout << "\nQSS Variables:\n";
	for ( size_type i = 0; i < nQSS; ++i ) {
		V * x( static_cast< V * >( vars[ i ] ) );
		Real const c0( uniform_random_real( -c_mag, c_mag ) ); // Constant
		x->add( c0 );
		std::cout << '\n' << x->name << "\n der = " << c0;
		size_type const n_deps( uniform_random_integer( size_type( 1 ), deg_QSS ) ); // Number of QSS derivative dependencies
		Indexes ks;
		for ( size_type j = 0; j < n_deps; ++j ) {
			Real const c( uniform_random_real( -c_mag, c_mag ) ); // Coefficient
			size_type k( uniform_random_integer( size_type( 0 ), nQSS - 1 ) ); // QSS variable index
			while ( ks.find( k ) != ks.end() ) k = uniform_random_integer( size_type( 0 ), nQSS - 1 ); // Avoid duplicates
			assert( k < nQSS );
			ks.insert( k );
			x->add( c, vars[ k ] );
			std::cout << "\n     + ( " << c << " * " << vars[ k ]->name << " )";
		}
		std::cout << '\n';
	}

	// Zero-crossing variables
	using Z = Variable_ZC< Function_LTI >;
	std::cout << "\nZero-Crossing Variables:\n";
	for ( size_type i = 1; i <= nZC; ++i ) {
		Z * z( nullptr );
		std::ostringstream i_stream; i_stream << i;
		std::string const z_nam( "z" + i_stream.str() );
		if ( ( qss == QSS::QSS1 ) || ( qss == QSS::LIQSS1 ) || ( qss == QSS::xQSS1 ) || ( qss == QSS::xLIQSS1 ) ) {
			vars.push_back( z = new Variable_ZC1< Function_LTI >( z_nam, rTol, aTol, zTol ) );
		} else if ( ( qss == QSS::QSS2 ) || ( qss == QSS::LIQSS2 ) || ( qss == QSS::xQSS2 ) || ( qss == QSS::xLIQSS2 ) ) {
			vars.push_back( z = new Variable_ZC2< Function_LTI >( z_nam, rTol, aTol, zTol ) );
		} else {
			vars.push_back( z = new Variable_ZC3< Function_LTI >( z_nam, rTol, aTol, zTol ) );
		}
		z->add_crossings_non_Flat();

		// Function
		Real const c0( uniform_random_real( -c_mag, c_mag ) ); // Constant
		z->add( c0 );
		std::cout << '\n' << z->name << "\n fxn = " << c0;
		size_type const n_deps( uniform_random_integer( size_type( 1 ), deg_ZC ) ); // Number of ZC dependencies
		Indexes ks;
		for ( size_type j = 0; j < n_deps; ++j ) {
			Real const c( uniform_random_real( -c_mag, c_mag ) ); // Coefficient
			size_type k( uniform_random_integer( size_type( 0 ), nQSS - 1 ) ); // QSS variable index
			while ( ks.find( k ) != ks.end() ) k = uniform_random_integer( size_type( 0 ), nQSS - 1 ); // Avoid duplicates
			assert( k < nQSS );
			ks.insert( k );
			z->add( c, vars[ k ] );
			std::cout << "\n     + ( " << c << " * " << vars[ k ]->name << " )";
		}
		std::cout << '\n';

		// Conditional
		using When = Conditional_When< Variable >;
		When * hit( new When( z->name + "_When" ) );
		cons.push_back( hit );
		When::ClauseH< Handler_gen > * hit_clause( hit->add_clause< Handler_gen >() );
		hit_clause->add( z );
		hit_clause->h.var( z );
		size_type const n_hdeps( uniform_random_integer( size_type( 1 ), deg_handler ) ); // Number of handler dependencies
		ks.clear();
		std::cout << "\n Handler:  var: ( dn, up ):";
		for ( size_type j = 0; j < n_hdeps; ++j ) {
			size_type k( uniform_random_integer( size_type( 0 ), nQSS - 1 ) ); // QSS variable index
			while ( ks.find( k ) != ks.end() ) k = uniform_random_integer( size_type( 0 ), nQSS - 1 ); // Avoid duplicates
			assert( k < nQSS );
			ks.insert( k );
			Real const d( uniform_random_real( -h_mag, h_mag ) ); // Downward crossing value
			Real const u( uniform_random_real( -h_mag, h_mag ) ); // Upward crossing value
			hit_clause->h.add( static_cast< V * >( vars[ k ] ), d, u );
			std::cout << "\n  " << vars[ k ]->name << ": ( " << d << ", " << u << " )";
		}
		std::cout << '\n';
	}
}

} // mdl
} // cod
} // QSS
