// Linear Time-Invariant Function
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

#ifndef QSS_dfn_mdl_Function_LTI_hh_INCLUDED
#define QSS_dfn_mdl_Function_LTI_hh_INCLUDED

// QSS Headers
#include <QSS/math.hh>

// C++ Headers
//#include <algorithm> // std::stable_sort
#include <cassert>
//#include <numeric> // std::iota
#include <vector>

namespace QSS {
namespace dfn {
namespace mdl {

// Linear Time-Invariant Function
template< typename V > // Template to avoid cyclic inclusion with Variable
class Function_LTI
{

public: // Types

	using Coefficient = double;
	using Coefficients = std::vector< Coefficient >;

	using Variable = V;
	using Variables = typename V::Variables;

	using Time = typename Variable::Time;
	using Value = typename Variable::Value;
	using AdvanceSpecs_LIQSS1 = typename Variable::AdvanceSpecs_LIQSS1;
	using AdvanceSpecs_LIQSS2 = typename Variable::AdvanceSpecs_LIQSS2;
	using size_type = Coefficients::size_type;

public: // Creation

	// Default Constructor
	Function_LTI()
	{}

	// Constructor
	Function_LTI(
	 Coefficients const & c,
	 Variables const & x
	) :
	 c_( c ),
	 x_( x )
	{
		assert( c_.size() == x_.size() );
	}

public: // Properties

	// Continuous Value at Time t
	Value
	operator ()( Time const t ) const
	{
		assert( c_.size() == x_.size() );
		Value v( c0_ );
		for ( size_type i = 0, n = c_.size(); i < n; ++i ) {
			v += c_[ i ] * x_[ i ]->x( t );
		}
		return v;
	}

	// Continuous Value at Time t
	Value
	x( Time const t ) const
	{
		assert( c_.size() == x_.size() );
		Value v( c0_ );
		for ( size_type i = 0, n = c_.size(); i < n; ++i ) {
			v += c_[ i ] * x_[ i ]->x( t );
		}
		return v;
	}

	// Continuous First Derivative at Time t
	Value
	x1( Time const t ) const
	{
		assert( c_.size() == x_.size() );
		Value s( 0.0 );
		for ( size_type i = 0, n = c_.size(); i < n; ++i ) {
			s += c_[ i ] * x_[ i ]->x1( t );
		}
		return s;
	}

	// Continuous Second Derivative at Time t
	Value
	x2( Time const t ) const
	{
		assert( c_.size() == x_.size() );
		Value s( 0.0 );
		for ( size_type i = 0, n = c_.size(); i < n; ++i ) {
			s += c_[ i ] * x_[ i ]->x2( t );
		}
		return s;
	}

	// Quantized Value at Time t
	Value
	q( Time const t ) const
	{
		assert( c_.size() == x_.size() );
		Value v( c0_ );
		for ( size_type i = 0, n = c_.size(); i < n; ++i ) {
			v += c_[ i ] * x_[ i ]->q( t );
		}
		return v;
	}

	// Quantized First Derivative at Time t
	Value
	q1( Time const t ) const
	{
		assert( c_.size() == x_.size() );
		Value s( 0.0 );
		for ( size_type i = iBeg[ 2 ], n = c_.size(); i < n; ++i ) {
			s += c_[ i ] * x_[ i ]->q1( t );
		}
		return s;
	}

	// Quantized Second Derivative at Time t
	Value
	q2( Time const t ) const
	{
		assert( c_.size() == x_.size() );
		Value c( 0.0 );
		for ( size_type i = iBeg[ 3 ], n = c_.size(); i < n; ++i ) {
			c += c_[ i ] * x_[ i ]->q2( t );
		}
		return c;
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
		assert( c_.size() == x_.size() );
		Value v( c0_ );
		for ( size_type i = 0, n = c_.size(); i < n; ++i ) {
			v += c_[ i ] * x_[ i ]->s( t );
		}
		return v;
	}

	// Simultaneous First Derivative at Time t
	Value
	s1( Time const t ) const
	{
		assert( c_.size() == x_.size() );
		Value s( 0.0 );
		for ( size_type i = iBeg[ 2 ], n = c_.size(); i < n; ++i ) {
			s += c_[ i ] * x_[ i ]->s1( t );
		}
		return s;
	}

	// Simultaneous Second Derivative at Time t
	Value
	s2( Time const t ) const
	{
		assert( c_.size() == x_.size() );
		Value s( 0.0 );
		for ( size_type i = 0, n = c_.size(); i < n; ++i ) {
			s += c_[ i ] * x_[ i ]->s2( t );
		}
		return s;
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

	// Continuous Values at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS1
	xlu1( Time const t, Value const del ) const
	{
		assert( co_.size() == xo_.size() );

		// Value at +/- del
		Value v( c0_ );
		for ( size_type i = 0, n = co_.size(); i < n; ++i ) {
			v += co_[ i ] * xo_[ i ]->x( t );
		}
		Value const vc( cv_ == 0.0 ? v : v + ( cv_ * xv_->x( t ) ) );
		Value const cv_del( cv_ * del );
		Value const vl( vc - cv_del );
		Value const vu( vc + cv_del );

		// Zero point
		Value const z( signum( vl ) != signum( vu ) ? -( v * cv_inv_ ) : 0.0 );

		return AdvanceSpecs_LIQSS1{ vl, vu, z };
	}

	// Quantized Values at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS1
	qlu1( Time const t, Value const del ) const
	{
		assert( co_.size() == xo_.size() );

		// Value at +/- del
		Value v( c0_ );
		for ( size_type i = 0, n = co_.size(); i < n; ++i ) {
			v += co_[ i ] * xo_[ i ]->q( t );
		}
		Value const vc( cv_ == 0.0 ? v : v + ( cv_ * xv_->q( t ) ) );
		Value const cv_del( cv_ * del );
		Value const vl( vc - cv_del );
		Value const vu( vc + cv_del );

		// Zero point
		Value const z( signum( vl ) != signum( vu ) ? -( v * cv_inv_ ) : 0.0 );

		return AdvanceSpecs_LIQSS1{ vl, vu, z };
	}

	// Simultaneous Values at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS1
	slu1( Time const t, Value const del ) const
	{
		assert( co_.size() == xo_.size() );

		// Value at +/- del
		Value v( c0_ );
		for ( size_type i = 0, n = co_.size(); i < n; ++i ) {
			v += co_[ i ] * xo_[ i ]->s( t );
		}
		Value const vc( cv_ == 0.0 ? v : v + ( cv_ * xv_->x( t ) ) );
		Value const cv_del( cv_ * del );
		Value const vl( vc - cv_del );
		Value const vu( vc + cv_del );

		// Zero point
		Value const z( signum( vl ) != signum( vu ) ? -( v * cv_inv_ ) : 0.0 );

		return AdvanceSpecs_LIQSS1{ vl, vu, z };
	}

	// Continuous Values and Derivatives at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS2
	xlu2( Time const t, Value const del ) const
	{
		assert( co_.size() == xo_.size() );

		// Value at +/- del
		Value v( c0_ );
		for ( size_type i = 0, n = co_.size(); i < n; ++i ) {
			v += co_[ i ] * xo_[ i ]->x( t );
		}
		Value const vc( cv_ == 0.0 ? v : v + ( cv_ * xv_->x( t ) ) );
		Value const cv_del( cv_ * del );
		Value const vl( vc - cv_del );
		Value const vu( vc + cv_del );

		// Derivative at +/- del
		Value s( 0.0 );
		for ( size_type i = ioBeg[ 2 ], n = co_.size(); i < n; ++i ) {
			s += co_[ i ] * xo_[ i ]->x1( t );
		}
		Value const sl( s + ( cv_ * vl ) );
		Value const su( s + ( cv_ * vu ) );

		// Zero point
		bool const signs_differ( signum( sl ) != signum( su ) );
		Value const z1( signs_differ ? -( s * cv_inv_ ) : 0.0 );
		Value const z2( signs_differ ? ( z1 - v ) * cv_inv_ : 0.0 );

		return AdvanceSpecs_LIQSS2{ vl, vu, z1, sl, su, z2 };
	}

	// Quantized Values and Derivatives at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS2
	qlu2( Time const t, Value const del ) const
	{
		assert( co_.size() == xo_.size() );

		// Value at +/- del
		Value v( c0_ );
		for ( size_type i = 0, n = co_.size(); i < n; ++i ) {
			v += co_[ i ] * xo_[ i ]->q( t );
		}
		Value const vc( cv_ == 0.0 ? v : v + ( cv_ * xv_->q( t ) ) );
		Value const cv_del( cv_ * del );
		Value const vl( vc - cv_del );
		Value const vu( vc + cv_del );

		// Derivative at +/- del
		Value s( 0.0 );
		for ( size_type i = ioBeg[ 2 ], n = co_.size(); i < n; ++i ) {
			s += co_[ i ] * xo_[ i ]->q1( t );
		}
		Value const sl( s + ( cv_ * vl ) );
		Value const su( s + ( cv_ * vu ) );

		// Zero point
		bool const signs_differ( signum( sl ) != signum( su ) );
		Value const z1( signs_differ ? -( s * cv_inv_ ) : 0.0 );
		Value const z2( signs_differ ? ( z1 - v ) * cv_inv_ : 0.0 );

		return AdvanceSpecs_LIQSS2{ vl, vu, z1, sl, su, z2 };
	}

	// Simultaneous Values and Derivatives at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS2
	slu2( Time const t, Value const del ) const
	{
		assert( co_.size() == xo_.size() );

		// Value at +/- del
		Value v( c0_ );
		for ( size_type i = 0, n = co_.size(); i < n; ++i ) {
			v += co_[ i ] * xo_[ i ]->s( t );
		}
		Value const vc( cv_ == 0.0 ? v : v + ( cv_ * xv_->x( t ) ) );
		Value const cv_del( cv_ * del );
		Value const vl( vc - cv_del );
		Value const vu( vc + cv_del );

		// Derivative at +/- del
		Value s( 0.0 );
		for ( size_type i = ioBeg[ 2 ], n = co_.size(); i < n; ++i ) {
			s += co_[ i ] * xo_[ i ]->s1( t );
		}
		Value const sl( s + ( cv_ * vl ) );
		Value const su( s + ( cv_ * vu ) );

		// Zero point
		bool const signs_differ( signum( sl ) != signum( su ) );
		Value const z1( signs_differ ? -( s * cv_inv_ ) : 0.0 );
		Value const z2( signs_differ ? ( z1 - v ) * cv_inv_ : 0.0 );

		return AdvanceSpecs_LIQSS2{ vl, vu, z1, sl, su, z2 };
	}

public: // Methods

	// Add Constant
	Function_LTI &
	add( Coefficient const c0 )
	{
		c0_ = c0;
		return *this;
	}

	// Add a Variable
	Function_LTI &
	add( Variable * x )
	{
		assert( c_.size() == x_.size() );
		c_.push_back( 1.0 );
		x_.push_back( x );
		assert( c_.size() == x_.size() );
		return *this;
	}

	// Add a Variable
	Function_LTI &
	add( Variable & x )
	{
		assert( c_.size() == x_.size() );
		c_.push_back( 1.0 );
		x_.push_back( &x );
		assert( c_.size() == x_.size() );
		return *this;
	}

	// Add a Coefficient + Variable
	Function_LTI &
	add(
	 Coefficient const c,
	 Variable & x
	)
	{
		assert( c_.size() == x_.size() );
		c_.push_back( c );
		x_.push_back( &x );
		assert( c_.size() == x_.size() );
		return *this;
	}

	// Add a Variable + Coefficient
	Function_LTI &
	add(
	 Variable & x,
	 Coefficient const c
	)
	{
		assert( c_.size() == x_.size() );
		c_.push_back( c );
		x_.push_back( &x );
		assert( c_.size() == x_.size() );
		return *this;
	}

	// Add a Coefficient + Variable
	Function_LTI &
	add(
	 Coefficient const c,
	 Variable * x
	)
	{
		assert( c_.size() == x_.size() );
		c_.push_back( c );
		x_.push_back( x );
		assert( c_.size() == x_.size() );
		return *this;
	}

	// Add a Variable + Coefficient
	Function_LTI &
	add(
	 Variable * x,
	 Coefficient const c
	)
	{
		assert( c_.size() == x_.size() );
		c_.push_back( c );
		x_.push_back( x );
		assert( c_.size() == x_.size() );
		return *this;
	}

	// Finalize Function Representation
	bool
	finalize( Variable * v )
	{
		assert( v != nullptr );
		assert( c_.size() == x_.size() );
		size_type n( c_.size() );

		// Sort elements by QSS method order (not max efficiency!)
		Coefficients c;
		c.reserve( n );
		Variables x;
		x.reserve( n );
		for ( int order = 0; order <= max_order; ++order ) {
			iBeg[ order ] = c.size();
			ioBeg[ order ] = co_.size();
			for ( size_type i = 0; i < n; ++i ) {
				if ( x_[ i ]->order() == order ) {
					c.push_back( c_[ i ] );
					x.push_back( x_[ i ] );
					if ( x_[ i ] == v ) {
						cv_ = c_[ i ];
						cv_inv_ = ( cv_ != 0.0 ? 1.0 / cv_ : infinity );
					} else {
						co_.push_back( c_[ i ] );
						xo_.push_back( x_[ i ] );
					}
				}
			}
		}
		xv_ = v;
		c_.swap( c );
		x_.swap( x );
// Consider doing an in-place permutation if this is a bottleneck
//		std::vector< size_type > p( n ); // Permutation
//		std::iota( p.begin(), p.end(), 0u );
//		std::stable_sort( p.begin(), p.end(), [&]( size_type i, size_type j ){ return x_[ i ]->order() < x_[ j ]->order() } );
//		...

		// Add variables as observees of self variable
		bool self_observer( false );
		for ( Variable * x : x_ ) {
			if ( x == v ) {
				self_observer = true;
			} else {
				x->add_observer( v );
			}
		}
		return self_observer;
	}

	// Finalize Function Representation
	bool
	finalize( Variable & v )
	{
		return finalize( &v );
	}

public: // Static Data

	static int const max_order = 3; // Max QSS order supported

private: // Data

	size_type iBeg[ max_order + 1 ]; // Index of first Variable of each QSS order
	size_type ioBeg[ max_order + 1 ]; // Index of first non-self Variable of each QSS order
	Coefficient c0_{ 0.0 }; // Constant term
	Coefficients c_; // Coefficients
	Variables x_; // Variables
	Coefficient cv_{ 0.0 }; // Coefficient of self Variable
	Coefficient cv_inv_{ 0.0 }; // Inverse of coefficient of self Variable
	Variable * xv_{ nullptr }; // Self Variable
	Coefficients co_; // Coefficients for Variables other than self Variable
	Variables xo_; // Variables other than self Variable

};

} // mdl
} // dfn
} // QSS

#endif
