// Sine Function Using Numeric Differentiation
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

#ifndef QSS_dfn_mdl_Function_sin_ND_hh_INCLUDED
#define QSS_dfn_mdl_Function_sin_ND_hh_INCLUDED

// QSS Headers
#include <QSS/options.hh>

// C++ Headers
#include <cassert>
#include <cmath>

namespace QSS {
namespace dfn {
namespace mdl {

// Sine Function Using Numeric Differentiation
class Function_sin_ND
{

public: // Types

	using Time = double;
	using Value = double;
	using Coefficient = double;

public: // Creation

	// Default Constructor
	Function_sin_ND()
	{}

	// Constructor
	explicit
	Function_sin_ND(
	 Coefficient const c,
	 Coefficient const s = 1.0
	) :
	 c_( c ),
	 s_( s )
	{}

public: // Properties

	// Value Scaling
	Coefficient
	c() const
	{
		return c_;
	}

	// Time Scaling
	Coefficient
	s() const
	{
		return s_;
	}

	// Value at Time t
	Value
	operator ()( Time const t ) const
	{
		return c_ * std::sin( s_ * t );
	}

	// Value at Time t
	Value
	v( Time const t ) const
	{
		return c_ * std::sin( s_ * t );
	}

	// First Derivative at Time t
	Value
	d1( Time const t ) const
	{
		return dtn_inv_2_ * ( v( t + dtn_ ) - v( t - dtn_ ) );
	}

	// Second Derivative at Time t
	Value
	d2( Time const t ) const
	{
		return dtn_inv_sq_ * ( v( t + dtn_ ) - ( 2.0 * v( t ) ) + v( t - dtn_ ) );
	}

	// Third Derivative at Time t
	Value
	d3( Time const t ) const
	{
		return dtn_inv_cb_2_ * ( v( t + dtn_2_ ) - ( 2.0 * ( v( t + dtn_ ) - v( t - dtn_ ) ) ) - v( t - dtn_2_ ) );
	}

	// Sequential Value at Time t
	Value
	vs( Time const t ) const
	{
		return v_t_ = v( t );
	}

	// Forward-Difference Sequential First Derivative at Time t
	Value
	df1( Time const t ) const
	{
		return dtn_inv_ * ( v( t + dtn_ ) - v_t_ );
	}

	// Centered-Difference Sequential First Derivative at Time t
	Value
	dc1( Time const t ) const
	{
		return dtn_inv_2_ * ( ( v_p_ = v( t + dtn_ ) ) - ( v_m_ = v( t - dtn_ ) ) );
	}

	// Centered-Difference Sequential Second Derivative at Time t
	Value
	dc2( Time const t ) const
	{
		return dtn_inv_sq_ * ( v_p_ - ( 2.0 * v_t_ ) + v_m_ );
	}

	// Centered-Difference Sequential Third Derivative at Time t
	Value
	dc3( Time const t ) const
	{
		return dtn_inv_cb_2_ * ( v( t + dtn_2_ ) - ( 2.0 * ( v_p_ - v_m_ ) ) - v( t - dtn_2_ ) );
	}

	// Differentiation Time Step
	Time
	dtn() const
	{
		return dtn_;
	}

public: // Methods

	// Set Value Scaling
	Function_sin_ND &
	c( Coefficient const c )
	{
		c_ = c;
		return *this;
	}

	// Set Time Scaling
	Function_sin_ND &
	s( Coefficient const s )
	{
		s_ = s;
		return *this;
	}

	// Set Differentiation Time Step
	void
	dtn( Time const dtn )
	{
		assert( dtn > 0.0 );
		dtn_ = dtn;
		dtn_2_ = 2.0 * dtn_;
		dtn_inv_ = 1.0 / dtn_;
		dtn_inv_2_ = 0.5 / dtn_;
		dtn_inv_sq_ = dtn_inv_ * dtn_inv_;
		dtn_inv_cb_2_ = 0.5 * ( dtn_inv_ * dtn_inv_ * dtn_inv_ );
	}

public: // Static Data

	static int const max_order = 3; // Max QSS order supported

private: // Data

	Coefficient c_{ 1.0 }; // Value scaling
	Coefficient s_{ 1.0 }; // Time scaling
	mutable Value v_t_; // Last v(t) computed
	mutable Value v_p_; // Last v(t+dtn) computed
	mutable Value v_m_; // Last v(t-dtn) computed
	Time dtn_{ options::dtND }; // Differentiation time step
	Time dtn_2_{ 2 * options::dtND }; // Differentiation time step doubled
	Time dtn_inv_{ 1.0 / options::dtND }; // Differentiation time step inverse
	Time dtn_inv_2_{ 0.5 / options::dtND }; // Differentiation time step half inverse
	Time dtn_inv_sq_{ 1.0 / ( options::dtND * options::dtND ) }; // Differentiation time step inverse squared
	Time dtn_inv_cb_2_{ 0.5 / ( options::dtND * options::dtND * options::dtND ) }; // Differentiation time step half inverse cubed

};

} // mdl
} // dfn
} // QSS

#endif
