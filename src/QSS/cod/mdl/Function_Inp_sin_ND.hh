// Sine Input Function Using Numeric Differentiation
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2020 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_cod_mdl_Function_Inp_sin_ND_hh_INCLUDED
#define QSS_cod_mdl_Function_Inp_sin_ND_hh_INCLUDED

// QSS Headers
#include <QSS/math.hh>
#include <QSS/options.hh>

// C++ Headers
#include <cassert>

namespace QSS {
namespace cod {
namespace mdl {

// Sine Input Function Using Numeric Differentiation
class Function_Inp_sin_ND final
{

public: // Types

	using Time = double;
	using Real = double;
	using Coefficient = double;

public: // Creation

	// Constructor
	explicit
	Function_Inp_sin_ND(
	 Coefficient const c = 1.0,
	 Coefficient const s = 1.0
	) :
	 c_( c ),
	 s_( s )
	{}

public: // Property

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
	Real
	operator ()( Time const t ) const
	{
		return c_ * std::sin( s_ * t );
	}

	// Value at Time t
	Real
	v( Time const t ) const
	{
		return c_ * std::sin( s_ * t );
	}

	// First Derivative at Time t
	Real
	d1( Time const t ) const
	{
		return dtn_inv_2_ * ( v( t + dtn_ ) - v( t - dtn_ ) );
	}

	// Second Derivative at Time t
	Real
	d2( Time const t ) const
	{
		return dtn_inv_sq_ * ( v( t + dtn_ ) - ( 2.0 * v( t ) ) + v( t - dtn_ ) );
	}

	// Third Derivative at Time t
	Real
	d3( Time const t ) const
	{
		return dtn_inv_cb_2_ * ( v( t + dtn_2_ ) - ( 2.0 * ( v( t + dtn_ ) - v( t - dtn_ ) ) ) - v( t - dtn_2_ ) );
	}

	// Sequential Value at Time t
	Real
	vs( Time const t ) const
	{
		return v_t_ = v( t );
	}

	// Forward-Difference Sequential First Derivative at Time t
	Real
	df1( Time const t ) const
	{
		return dtn_inv_ * ( v( t + dtn_ ) - v_t_ );
	}

	// Centered-Difference Sequential First Derivative at Time t
	Real
	dc1( Time const t ) const
	{
		return dtn_inv_2_ * ( ( v_p_ = v( t + dtn_ ) ) - ( v_m_ = v( t - dtn_ ) ) );
	}

	// Centered-Difference Sequential Second Derivative at Time t
	Real
	dc2( Time const ) const
	{
		return dtn_inv_sq_ * ( v_p_ - ( 2.0 * v_t_ ) + v_m_ );
	}

	// Centered-Difference Sequential Third Derivative at Time t
	Real
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

	// Discrete Event after Time t
	Time
	tD( Time const ) const
	{
		return infinity;
	}

public: // Methods

	// Set Value Scaling
	Function_Inp_sin_ND &
	c( Coefficient const c )
	{
		c_ = c;
		return *this;
	}

	// Set Time Scaling
	Function_Inp_sin_ND &
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

private: // Data

	Coefficient c_{ 1.0 }; // Value scaling
	Coefficient s_{ 1.0 }; // Time scaling
	mutable Real v_t_; // Last v(t) computed
	mutable Real v_p_; // Last v(t+dtn) computed
	mutable Real v_m_; // Last v(t-dtn) computed
	Time dtn_{ options::dtND }; // Numeric differentiation time step
	Time dtn_2_{ 2.0 * options::dtND }; // Numeric differentiation time step doubled
	Time dtn_inv_{ 1.0 / options::dtND }; // Numeric differentiation time step inverse
	Time dtn_inv_2_{ 0.5 / options::dtND }; // Numeric differentiation time step half inverse
	Time dtn_inv_sq_{ 1.0 / ( options::dtND * options::dtND ) }; // Numeric differentiation time step inverse squared
	Time dtn_inv_cb_2_{ 0.5 / ( options::dtND * options::dtND * options::dtND ) }; // Numeric differentiation time step half inverse cubed

};

} // mdl
} // cod
} // QSS

#endif
