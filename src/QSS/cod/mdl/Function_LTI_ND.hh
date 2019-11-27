// Linear Time-Invariant Function Using Numeric Differentiation
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

#ifndef QSS_cod_mdl_Function_LTI_ND_hh_INCLUDED
#define QSS_cod_mdl_Function_LTI_ND_hh_INCLUDED

// QSS Headers
#include <QSS/options.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

namespace QSS {
namespace cod {
namespace mdl {

// Linear Time-Invariant Function Using Numeric Differentiation
//
// Note: Adding many terms into a sorted vector could be a performance issue
//
// Note: Not set up for use with LIQSS methods
template< typename V > // Template to avoid cyclic inclusion with Variable
class Function_LTI_ND final
{

public: // Types

	using Coefficient = double;
	using Variable = V;

	struct Term
	{
		Coefficient c;
		Variable * v;

		// Term < Term: Partial ordering by Variable order
		friend
		bool
		operator <( Term const & l, Term const & r )
		{
			return ( l.v->order() < r.v->order() );
		}

	}; // Term
	using Terms = std::vector< Term >;
	using size_type = typename Terms::size_type;

	using Time = typename Variable::Time;
	using Real = typename Variable::Real;

public: // Property

	// Continuous Value at Time t
	Real
	operator ()( Time const t ) const
	{
		Real r( c0_ );
		for ( Term const & term : terms_ ) {
			r += term.c * term.v->x( t );
		}
		return r;
	}

	// Continuous Value at Time t
	Real
	x( Time const t ) const
	{
		Real r( c0_ );
		for ( Term const & term : terms_ ) {
			r += term.c * term.v->x( t );
		}
		return r;
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const t ) const
	{
		return dtn_inv_2_ * ( x( t + dtn_ ) - x( t - dtn_ ) );
	}

	// Continuous Second Derivative at Time t
	Real
	x2( Time const t ) const
	{
		return dtn_inv_sq_ * ( x( t + dtn_ ) - ( 2.0 * x( t ) ) + x( t - dtn_ ) );
	}

	// Continuous Third Derivative at Time t
	Real
	x3( Time const t ) const
	{
		return dtn_inv_cb_2_ * ( x( t + dtn_2_ ) - x( t - dtn_2_ ) - ( 2.0 * ( x( t + dtn_ ) - x( t - dtn_ ) ) ) );
	}

	// Quantized Value at Time t
	Real
	q( Time const t ) const
	{
		Real r( c0_ );
		for ( Term const & term : terms_ ) {
			r += term.c * term.v->q( t );
		}
		return r;
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const t ) const
	{
		return dtn_inv_2_ * ( q( t + dtn_ ) - q( t - dtn_ ) );
	}

	// Quantized Second Derivative at Time t
	Real
	q2( Time const t ) const
	{
		return dtn_inv_sq_ * ( q( t + dtn_ ) - ( 2.0 * q( t ) ) + q( t - dtn_ ) );
	}

	// Quantized Sequential Value at Time t
	Real
	qs( Time const t ) const
	{
		return v_t_ = q( t );
	}

	// Quantized Forward-Difference Sequential First Derivative at Time t
	Real
	qf1( Time const t ) const
	{
		return dtn_inv_ * ( q( t + dtn_ ) - v_t_ );
	}

	// Quantized Centered-Difference Sequential First Derivative at Time t
	Real
	qc1( Time const t ) const
	{
		return dtn_inv_2_ * ( ( v_p_ = q( t + dtn_ ) ) - ( v_m_ = q( t - dtn_ ) ) );
	}

	// Quantized Centered-Difference Sequential Second Derivative at Time t
	Real
	qc2( Time const ) const
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
	void
	add( Coefficient const c0 )
	{
		c0_ = c0;
	}

	// Add Variable
	void
	add( Variable * v, Variable * = nullptr )
	{
		assert( v != nullptr );
		Term term{ 1.0, v };
		terms_.insert( std::upper_bound( terms_.begin(), terms_.end(), term ), std::move( term ) );
	}

	// Add Coefficient * Variable
	void
	add(
	 Coefficient const c,
	 Variable * v,
	 Variable * = nullptr
	)
	{
		Term term{ c, v };
		terms_.insert( std::upper_bound( terms_.begin(), terms_.end(), term ), std::move( term ) );
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

private: // Data

	Coefficient c0_{ 0.0 }; // Constant term
	Terms terms_; // Coefficient * Variable terms

	mutable Real v_t_; // Last value(t) computed
	mutable Real v_p_; // Last value(t+dtn) computed
	mutable Real v_m_; // Last value(t-dtn) computed

	Time dtn_{ options::dtNum }; // Differentiation time step
	Time dtn_2_{ 2.0 * options::dtNum }; // Differentiation time step x 2
	Time dtn_inv_{ 1.0 / options::dtNum }; // Differentiation time step inverse
	Time dtn_inv_2_{ 0.5 / options::dtNum }; // Differentiation time step half inverse
	Time dtn_inv_sq_{ 1.0 / ( options::dtNum * options::dtNum ) }; // Differentiation time step inverse squared
	Time dtn_inv_cb_2_{ 0.5 / ( options::dtNum * options::dtNum * options::dtNum ) }; // Differentiation time step half inverse cubed

};

} // mdl
} // cod
} // QSS

#endif
