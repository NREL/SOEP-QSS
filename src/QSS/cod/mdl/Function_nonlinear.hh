// Derivative Function for Nonlinear Example
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

#ifndef QSS_cod_mdl_Function_nonlinear_hh_INCLUDED
#define QSS_cod_mdl_Function_nonlinear_hh_INCLUDED

// QSS Headers
#include <QSS/math.hh>

// C++ Headers
#include <cassert>

namespace QSS {
namespace cod {
namespace mdl {

// Problem:  y'(t) = ( 1 + 2 t ) / ( y + 2 ), y( 0 ) = 2
// Solution: y = sqrt( 2 t^2 + 2 t + 16 ) - 2
// Notes:
//  y'(t) = ( 1 + 2 t ) / sqrt( 2 t^2 + 2 t + 16 )
//
//  y''(t) = ( 2 / ( y + 2 ) ) - ( ( 1 + 2 t )^2 / ( y + 2 )^3 )
//         = ( 2 / ( y + 2 ) ) - ( ( 1 + 2 t ) / ( y + 2 )^2 ) y'
//         = 31 / ( 2 t^2 + 2 t + 16 )^(3/2)
//
//  y'''(t) = 3 ( 1 + 2 t )^3 / ( y + 2 )^5 - 6 ( 1 + 2 t ) / ( y + 2 )^3
//          = ( -4 / ( y + 2 )^2 ) y' + 2 ( ( 1 + 2 t ) / ( y + 2 )^3 ) y'^2 - ( ( 1 + 2 t ) / ( y + 2 )^2 ) y''
//          = -( 93 + 186 t ) / ( 2 t^2 + 2 t + 16 )^(5/2)
//
//  y''''(t) = ( 4 ( y + 2 ) ( 2 + y' ) y' - 6 ( 1 + 2 t ) y'^3 + ( 2 ( 1 + 2 t ) - 6 ( y + 2 ) ) ( y + 2 ) y'' + ( ( 1 + 2 t ) ( y + 2 )^2 y''' ) ) / ( y + 2 )^4
//           = ( 1488 t^2 + 1488 t - 2511 ) / ( 2 t^2 + 2 t + 16 )^(7/2)

// Derivative Function for Nonlinear Example
template< typename V > // Template to avoid cyclic inclusion with Variable
class Function_nonlinear final
{

public: // Types

	using Variable = V;
	using Time = typename Variable::Time;
	using Real = typename Variable::Real;
	using Coefficient = double;
	using AdvanceSpecs_LIQSS1 = typename Variable::AdvanceSpecs_LIQSS1;
	using AdvanceSpecs_LIQSS2 = typename Variable::AdvanceSpecs_LIQSS2;

public: // Property

	// Continuous Value at Time t
	Real
	operator ()( Time const t ) const
	{
		return ( 1.0 + ( 2.0 * t ) ) / ( y_->x( t ) + 2.0 );
	}

	// Continuous Value at Time t
	Real
	x( Time const t ) const
	{
		return ( 1.0 + ( 2.0 * t ) ) / ( y_->x( t ) + 2.0 );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const t ) const
	{
		Real const yp2( y_->x( t ) + 2.0 );
		return ( ( 2.0 * yp2 ) - ( y_->x1( t ) * ( 1.0 + ( 2.0 * t ) ) ) ) / square( yp2 );
	}

	// Continuous Second Derivative at Time t
	Real
	x2( Time const t ) const
	{
		Real const yp2( y_->x( t ) + 2.0 );
		Real const w( 1.0 + 2.0 * t );
		return ( ( 2.0 * square( y_->x1( t ) ) * w ) - ( yp2 * ( ( y_->x2( t ) * w ) + ( 4.0 * y_->x1( t ) ) ) ) ) / cube( yp2 );
	}

	// Continuous Third Derivative at Time t
	Real
	x3( Time const t ) const
	{
		Real const yp2( y_->x( t ) + 2.0 );
		Real const y1( y_->x1( t ) );
		Real const y2( y_->x2( t ) );
		Real const y3( y_->x3( t ) );
		Real const w( 1.0 + 2.0 * t );
		return (
		 ( 4.0 * yp2 * ( 2.0 + y1 ) * y1 ) -
		 ( 6.0 * w * cube( y1 ) ) +
		 ( ( ( 2.0 * w ) - ( 6.0 * yp2 ) ) * yp2 * y2 ) +
		 ( w * square( yp2 ) * y3 )
		 ) / quad( yp2 );
	}

	// Quantized Value at Time t
	Real
	q( Time const t ) const
	{
		return ( 1.0 + ( 2.0 * t ) ) / ( y_->q( t ) + 2.0 );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const t ) const
	{
		Real const yp2( y_->q( t ) + 2.0 );
		return ( ( 2.0 * yp2 ) - ( y_->q1( t ) * ( 1.0 + ( 2.0 * t ) ) ) ) / square( yp2 );
	}

	// Quantized Second Derivative at Time t
	Real
	q2( Time const t ) const
	{
		Real const yp2( y_->q( t ) + 2.0 );
		Real const w( 1.0 + 2.0 * t );
		return ( ( 2.0 * square( y_->q1( t ) ) * w ) - ( yp2 * ( ( y_->q2( t ) * w ) + ( 4.0 * y_->q1( t ) ) ) ) ) / cube( yp2 );
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

	// Quantized Values at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS1
	qlu1( Time const t, Real const del, Real const = 0.0 ) const
	{
		// Value at +/- del
		Real const num( 1.0 + ( 2.0 * t ) );
		Real const y2( y_->q( t ) + 2.0 );
		Real const vl( num / ( y2 - del ) );
		Real const vu( num / ( y2 + del ) );

		// Zero point: No y gives zero function value at any t >= 0
		Real const z( 0.0 ); // No y value gives zero slope at any t >= 0

		return AdvanceSpecs_LIQSS1{ vl, vu, z };
	}

	// Quantized Values and Derivatives at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS2
	qlu2( Time const t, Real const del, Real const = 0.0, Real const = 0.0 ) const
	{
		// Value at +/- del
		Real const num( 1.0 + ( 2.0 * t ) );
		Real const y2( y_->q( t ) + 2.0 );
		Real const vl( num / ( y2 - del ) );
		Real const vu( num / ( y2 + del ) );

		// Derivative at +/- del
		Real const ts( square( 1.0 + ( 2.0 * t ) ) );
		Real const sl( derivative( ts, y2 - del ) );
		Real const su( derivative( ts, y2 + del ) );

		// Zero point: No solution points have zero function derivative
		assert( signum( sl ) == signum( su ) );
		assert( signum( sl ) != 0 );
		Real const z1( 0.0 );
		Real const z0( 0.0 );

		return AdvanceSpecs_LIQSS2{ vl, vu, sl, su, z0, z1 };
	}

	// Exact Value of y at Time t
	Real
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

private: // Static Methods

	// Derivative at Time t Given ( 1 + 2*t )^2 and y+2
	static
	Real
	derivative( Time const ts, Real const y2 )
	{
		return ( 2.0 / y2 ) - ( ts / cube( y2 ) );
	}

private: // Data

	Variable * y_{ nullptr };

};

} // mdl
} // cod
} // QSS

#endif
