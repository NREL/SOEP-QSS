#ifndef QSS_dfn_mdl_Function_nonlinear_hh_INCLUDED
#define QSS_dfn_mdl_Function_nonlinear_hh_INCLUDED

// Derivative Function for Nonlinear Example
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// Problem:  y'( t ) = ( 1 + 2 t ) / ( y + 2 ), y( 0 ) = 2
// Solution: y = sqrt( 2 t^2 + 2 t + 16 ) - 2
// Note:     y''( t ) = ( 2 / ( y + 2 ) ) - ( ( 1 + 2 t )^2 / ( y + 2 )^3 )

// QSS Headers
#include <QSS/math.hh>

// C++ Headers
#include <cassert>
#include <cmath>

namespace QSS {
namespace dfn {
namespace mdl {

// Derivative Function for Nonlinear Example
template< typename V > // Template to avoid cyclic inclusion with Variable
class Function_nonlinear
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
		Value const v( y_->x( t ) + 2.0 );
		return ( ( 2.0 * v ) - ( y_->x1( t ) * ( 1.0 + ( 2.0 * t ) ) ) ) / square( v );
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
		Value const v( y_->q( t ) + 2.0 );
		return ( ( 2.0 * v ) - ( y_->q1( t ) * ( 1.0 + ( 2.0 * t ) ) ) ) / square( v );
	}

	// Quantized Second Derivative at Time t
	Value
	q2( Time const t ) const
	{
		Value const v( y_->q( t ) + 2.0 );
		Value const w( 1.0 + 2.0 * t );
		return ( ( 2.0 * square( y_->q1( t ) ) * w ) - ( v * ( ( y_->q2( t ) * w ) + ( 4.0 * y_->q1( t ) ) ) ) ) / cube( v );
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

	// Continuous Values at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS1
	xlu1( Time const t, Value const del ) const
	{
		// Value at +/- del
		Value const num( 1.0 + ( 2.0 * t ) );
		Value const y2( y_->q( t ) + 2.0 );
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
		Value const ts( square( 1.0 + ( 2.0 * t ) ) );
		Value const sl( derivative( ts, y2 - del ) );
		Value const su( derivative( ts, y2 + del ) );

		// Zero point: No solution points have zero function derivative
		assert( signum( sl ) == signum( su ) );
		assert( signum( sl ) != 0 );
		Value const z1( 0.0 );
		Value const z2( 0.0 );

		return AdvanceSpecs_LIQSS2{ vl, vu, z1, sl, su, z2 };
	}

	// Continuous Values and Derivatives at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS2
	xlu2( Time const t, Value const del ) const
	{
		// Value at +/- del
		Value const num( 1.0 + ( 2.0 * t ) );
		Value const y2( y_->x( t ) + 2.0 );
		Value const vl( num / ( y2 - del ) );
		Value const vu( num / ( y2 + del ) );

		// Derivative at +/- del
		Value const ts( square( 1.0 + ( 2.0 * t ) ) );
		Value const sl( derivative( ts, y2 - del ) );
		Value const su( derivative( ts, y2 + del ) );

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

	// Set Variable
	void
	var( Variable & y )
	{
		y_ = &y;
	}

	// Set Variable
	void
	var( Variable * y )
	{
		y_ = y;
	}

	// Finalize Function Representation
	bool
	finalize( Variable * v )
	{
		assert( v != nullptr );
		assert( v == y_ );
		return true; // Self-observer
	}

	// Finalize Function Representation
	bool
	finalize( Variable & v )
	{
		return finalize( &v );
	}

private: // Static Methods

	// Derivative at Time t Given ( 1 + 2*t )^2 and y+2
	static
	Value
	derivative( Time const ts, Value const y2 )
	{
		return ( 2.0 / y2 ) - ( ts / cube( y2 ) );
	}

private: // Data

	Variable * y_{ nullptr };

};

} // mdl
} // dfn
} // QSS

#endif
