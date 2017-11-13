// Linear Time-Invariant Function Using Numeric Differentiation
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

#ifndef QSS_dfn_mdl_Function_LTI_ND_hh_INCLUDED
#define QSS_dfn_mdl_Function_LTI_ND_hh_INCLUDED

// QSS Headers
#include <QSS/options.hh>

// C++ Headers
//#include <algorithm> // std::stable_sort
#include <cassert>
//#include <numeric> // std::iota
#include <vector>

namespace QSS {
namespace dfn {
namespace mdl {

// Linear Time-Invariant Function Using Numeric Differentiation
//
// Note: Not set up for use with LIQSS methods
template< typename V > // Template to avoid cyclic inclusion with Variable
class Function_LTI_ND final
{

public: // Types

	using Coefficient = double;
	using Coefficients = std::vector< Coefficient >;

	using Variable = V;
	using Variables = typename V::Variables;

	using Time = typename Variable::Time;
	using Value = typename Variable::Value;
	using size_type = Coefficients::size_type;

public: // Creation

	// Default Constructor
	Function_LTI_ND()
	{}

	// Constructor
	Function_LTI_ND(
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
		return dtn_inv_2_ * ( x( t + dtn_ ) - x( t - dtn_ ) );
	}

	// Continuous Second Derivative at Time t
	Value
	x2( Time const t ) const
	{
		return dtn_inv_sq_ * ( x( t + dtn_ ) - ( 2.0 * x( t ) ) + x( t - dtn_ ) );
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
		return dtn_inv_2_ * ( q( t + dtn_ ) - q( t - dtn_ ) );
	}

	// Quantized Second Derivative at Time t
	Value
	q2( Time const t ) const
	{
		return dtn_inv_sq_ * ( q( t + dtn_ ) - ( 2.0 * q( t ) ) + q( t - dtn_ ) );
	}

	// Quantized Sequential Value at Time t
	Value
	qs( Time const t ) const
	{
		return v_t_ = q( t );
	}

	// Quantized Forward-Difference Sequential First Derivative at Time t
	Value
	qf1( Time const t ) const
	{
		return dtn_inv_ * ( q( t + dtn_ ) - v_t_ );
	}

	// Quantized Centered-Difference Sequential First Derivative at Time t
	Value
	qc1( Time const t ) const
	{
		return dtn_inv_2_ * ( ( v_p_ = q( t + dtn_ ) ) - ( v_m_ = q( t - dtn_ ) ) );
	}

	// Quantized Centered-Difference Sequential Second Derivative at Time t
	Value
	qc2( Time const t ) const
	{
		return dtn_inv_sq_ * ( v_p_ - ( 2.0 * v_t_ ) + v_m_ );
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

	// Simultaneous Numeric Differentiation Value at Time t
	Value
	sn( Time const t ) const
	{
		assert( c_.size() == x_.size() );
		Value v( c0_ );
		for ( size_type i = 0, n = c_.size(); i < n; ++i ) {
			v += c_[ i ] * x_[ i ]->sn( t );
		}
		return v;
	}

	// Simultaneous First Derivative at Time t
	Value
	s1( Time const t ) const
	{
		return dtn_inv_2_ * ( sn( t + dtn_ ) - sn( t - dtn_ ) );
	}

	// Simultaneous Second Derivative at Time t
	Value
	s2( Time const t ) const
	{
		return dtn_inv_sq_ * ( sn( t + dtn_ ) - ( 2.0 * s( t ) ) + sn( t - dtn_ ) );
	}

	// Simultaneous Sequential Value at Time t
	Value
	ss( Time const t ) const
	{
		return v_t_ = s( t );
	}

	// Simultaneous Forward-Difference Sequential First Derivative at Time t
	Value
	sf1( Time const t ) const
	{
		return dtn_inv_ * ( sn( t + dtn_ ) - v_t_ );
	}

	// Simultaneous Centered-Difference Sequential First Derivative at Time t
	Value
	sc1( Time const t ) const
	{
		return dtn_inv_2_ * ( ( v_p_ = sn( t + dtn_ ) ) - ( v_m_ = sn( t - dtn_ ) ) );
	}

	// Simultaneous Centered-Difference Sequential Second Derivative at Time t
	Value
	sc2( Time const t ) const
	{
		return dtn_inv_sq_ * ( v_p_ - ( 2.0 * v_t_ ) + v_m_ );
	}

	// Differentiation Time Step
	Time
	dtn() const
	{
		return dtn_;
	}

public: // Methods

	// Add Constant
	Function_LTI_ND &
	add( Coefficient const c0 )
	{
		c0_ = c0;
		return *this;
	}

	// Add a Variable
	Function_LTI_ND &
	add( Variable * x )
	{
		assert( c_.size() == x_.size() );
		c_.push_back( 1.0 );
		x_.push_back( x );
		assert( c_.size() == x_.size() );
		return *this;
	}

	// Add a Variable
	Function_LTI_ND &
	add( Variable & x )
	{
		assert( c_.size() == x_.size() );
		c_.push_back( 1.0 );
		x_.push_back( &x );
		assert( c_.size() == x_.size() );
		return *this;
	}

	// Add a Coefficient + Variable
	Function_LTI_ND &
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
	Function_LTI_ND &
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
	Function_LTI_ND &
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
	Function_LTI_ND &
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
			for ( size_type i = 0; i < n; ++i ) {
				if ( x_[ i ]->order() == order ) {
					c.push_back( c_[ i ] );
					x.push_back( x_[ i ] );
				}
			}
		}
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

	// Set Differentiation Time Step
	void
	dtn( Time const dtn )
	{
		assert( dtn > 0.0 );
		dtn_ = dtn;
		dtn_inv_ = 1.0 / dtn_;
		dtn_inv_2_ = 0.5 / dtn_;
		dtn_inv_sq_ = dtn_inv_ * dtn_inv_;
	}

public: // Static Data

	static int const max_order = 3; // Max QSS order supported

private: // Data

	size_type iBeg[ max_order + 1 ]; // Index of first Variable of each QSS order
	Coefficient c0_{ 0.0 }; // Constant term
	Coefficients c_; // Coefficients
	Variables x_; // Variables
	mutable Value v_t_; // Last value(t) computed
	mutable Value v_p_; // Last value(t+dtn) computed
	mutable Value v_m_; // Last value(t-dtn) computed
	Time dtn_{ options::dtNum }; // Differentiation time step
	Time dtn_inv_{ 1.0 / options::dtNum }; // Differentiation time step inverse
	Time dtn_inv_2_{ 0.5 / options::dtNum }; // Differentiation time step half inverse
	Time dtn_inv_sq_{ 1.0 / ( options::dtNum * options::dtNum ) }; // Differentiation time step inverse squared

};

} // mdl
} // dfn
} // QSS

#endif
