// Derivative Function for Nonlinear Example: Numeric Differentiation
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2018 Objexx Engineerinc, Inc. All rights reserved.
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

#ifndef QSS_cod_mdl_Function_nonlinear_ND_hh_INCLUDED
#define QSS_cod_mdl_Function_nonlinear_ND_hh_INCLUDED

// QSS Headers
#include <QSS/math.hh>
#include <QSS/options.hh>

// C++ Headers
#include <cassert>

namespace QSS {
namespace cod {
namespace mdl {

// Problem:  y'( t ) = ( 1 + 2 t ) / ( y + 2 ), y( 0 ) = 2
// Solution: y = sqrt( 2 t^2 + 2 t + 16 ) - 2
// Note:     y''( t ) = ( 2 / ( y + 2 ) ) - ( ( 1 + 2 t )^2 / ( y + 2 )^3 )

// Derivative Function for Nonlinear Example: Numeric Differentiation
template< typename V > // Template to avoid cyclic inclusion with Variable
class Function_nonlinear_ND final
{

public: // Types

	using Variable = V;
	using Time = typename Variable::Time;
	using Value = typename Variable::Value;
	using Coefficient = double;
	using AdvanceSpecs_LIQSS1 = typename Variable::AdvanceSpecs_LIQSS1;
	using AdvanceSpecs_LIQSS2 = typename Variable::AdvanceSpecs_LIQSS2;

public: // Properties

	// Continuous Value at Time t
	Value
	operator ()( Time const t ) const
	{
		return ( 1.0 + ( 2.0 * t ) ) / ( y_->x( t ) + 2.0 );
	}

	// Continuous Value at Time t
	Value
	x( Time const t ) const
	{
		return ( 1.0 + ( 2.0 * t ) ) / ( y_->x( t ) + 2.0 );
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

	// Continuous Third Derivative at Time t
	Value
	x3( Time const t ) const
	{
		return dtn_inv_cb_2_ * ( x( t + dtn_2_ ) - x( t - dtn_2_ ) - ( 2.0 * ( x( t + dtn_ ) - x( t - dtn_ ) ) ) );
	}

	// Quantized Value at Time t
	Value
	q( Time const t ) const
	{
		return ( 1.0 + ( 2.0 * t ) ) / ( y_->q( t ) + 2.0 );
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
	qc2( Time const ) const
	{
		return dtn_inv_sq_ * ( v_p_ - ( 2.0 * v_t_ ) + v_m_ );
	}

	// Simultaneous Value at Time t
	Value
	s( Time const t ) const
	{
		return ( 1.0 + ( 2.0 * t ) ) / ( y_->s( t ) + 2.0 );
	}

	// Simultaneous Numeric Differentiation Value at Time t
	Value
	sn( Time const t ) const
	{
		return ( 1.0 + ( 2.0 * t ) ) / ( y_->sn( t ) + 2.0 );
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
	sc2( Time const ) const
	{
		return dtn_inv_sq_ * ( v_p_ - ( 2.0 * v_t_ ) + v_m_ );
	}

	// Differentiation Time Step
	Time
	dtn() const
	{
		return dtn_;
	}

	// Quantized Values at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS1
	qlu1( Time const t, Value const del ) const
	{
		// Value at +/- del
		Value const num( 1.0 + ( 2.0 * t ) );
		Value const y2( y_->q( t ) + 2.0 );
		Value const vl( num / ( y2 - del ) );
		Value const vu( num / ( y2 + del ) );

		// Zero point: No y gives zero function value at any t >= 0
		Value const z( 0.0 ); // No y value gives zero slope at any t >= 0

		return AdvanceSpecs_LIQSS1{ vl, vu, z };
	}

	// Simultaneous Values at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS1
	slu1( Time const t, Value const del ) const
	{
		// Value at +/- del
		Value const num( 1.0 + ( 2.0 * t ) );
		Value const y2( y_->s( t ) + 2.0 );
		Value const vl( num / ( y2 - del ) );
		Value const vu( num / ( y2 + del ) );

		// Zero point: No y gives zero function value at any t >= 0
		Value const z( 0.0 );

		return AdvanceSpecs_LIQSS1{ vl, vu, z };
	}

	// Quantized Values and Derivatives at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS2
	qlu2( Time const t, Value const del ) const
	{
		// Value at +/- del
		Value const num( 1.0 + ( 2.0 * t ) );
		Value const y2( y_->q( t ) + 2.0 );
		Value const vl( num / ( y2 - del ) );
		Value const vu( num / ( y2 + del ) );

		// Derivative at +/- del
		Time const tm( t - dtn_ );
		Time const tp( t + dtn_ );
		Value const y2m( y_->q( tm ) + 2.0 );
		Value const y2p( y_->q( tp ) + 2.0 );
		Value const sl( dtn_inv_2_ * ( ndv( tp, y2p, -del ) - ndv( tm, y2m, -del ) ) );
		Value const su( dtn_inv_2_ * ( ndv( tp, y2p, +del ) - ndv( tm, y2m, +del ) ) );

		// Zero point: No solution points have zero function derivative
		assert( signum( sl ) == signum( su ) );
		assert( signum( sl ) != 0 );
		Value const z1( 0.0 );
		Value const z2( 0.0 );

		return AdvanceSpecs_LIQSS2{ vl, vu, z1, sl, su, z2 };
	}

	// Simultaneous Values and Derivatives at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS2
	slu2( Time const t, Value const del, Value const ) const
	{
		// Value at +/- del
		Value const num( 1.0 + ( 2.0 * t ) );
		Value const y2( y_->s( t ) + 2.0 );
		Value const vl( num / ( y2 - del ) );
		Value const vu( num / ( y2 + del ) );

		// Derivative at +/- del
		Time const tm( t - dtn_ );
		Time const tp( t + dtn_ );
		Value const y2m( y_->s( tm ) + 2.0 );
		Value const y2p( y_->s( tp ) + 2.0 );
		Value const sl( dtn_inv_2_ * ( ndv( tp, y2p, -del ) - ndv( tm, y2m, -del ) ) );
		Value const su( dtn_inv_2_ * ( ndv( tp, y2p, +del ) - ndv( tm, y2m, +del ) ) );

		// Zero point: No solution points have zero function derivative
		assert( signum( sl ) == signum( su ) );
		assert( signum( sl ) != 0 );
		Value const z1( 0.0 );
		Value const z2( 0.0 );

		return AdvanceSpecs_LIQSS2{ vl, vu, z1, sl, su, z2 };
	}

	// Exact Value of y at Time t
	Value
	e( Time const t ) const
	{
		return std::sqrt( ( 2.0 * t * ( t + 1.0 ) ) + 16.0 ) - 2.0;
	}

public: // Methods

	// Add Variable
	void
	add( Variable * y, Variable * = nullptr )
	{
		y_ = y;
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

private: // Static Methods

	// Numeric Differentiation Value at Time t Given y+2 and y Delta
	static
	Value
	ndv( Time const t, Value const y2, Value const del )
	{
		return ( 1.0 + ( 2.0 * t ) ) / ( y2 + del );
	}

private: // Data

	Variable * y_{ nullptr };
	mutable Value v_t_; // Last value(t) computed
	mutable Value v_p_; // Last value(t+dtn) computed
	mutable Value v_m_; // Last value(t-dtn) computed
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
