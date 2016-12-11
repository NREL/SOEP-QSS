#ifndef QSS_Function_NonlinearEx1_hh_INCLUDED
#define QSS_Function_NonlinearEx1_hh_INCLUDED

// Derivative Function for Nonlinear Example 1
// Problem:  y'( t ) = ( 1 + 2 t ) / ( y + 2 ), y( 0 ) = 2
// Solution: y = sqrt( 2 t^2 + 2 t + 16 ) - 2

// QSS Headers
#include <QSS/math.hh>

// C++ Headers
#include <cassert>
#include <cmath>
#include <vector>

// Linear Time-Invariant Function
template< typename V > // Template to avoid cyclic inclusion with Variable
class Function_NonlinearEx1
{

public: // Types

	using Coefficient = double;

	using Variable = V;

	using Value = typename Variable::Value;
	using Time = typename Variable::Time;
	using AdvanceSpecsLIQSS1 = typename Variable::AdvanceSpecsLIQSS1;
	using AdvanceSpecsLIQSS2 = typename Variable::AdvanceSpecsLIQSS2;

public: // Properties

	// Continuous Value at Initialization Time
	Value
	x() const
	{
		return 1.0 / ( y_->x() + 2.0 );
	}

	// Continuous First Derivative at Initialization Time
	Value
	x1() const
	{
		Value const v( y_->x() + 2.0 );
		return ( ( 2.0 * v ) - y_->x1() ) / square( v );
	}

	// Quantized Value at Initialization Time
	Value
	q() const
	{
		return 1.0 / ( y_->q() + 2.0 );
	}

	// Quantized First Derivative at Initialization Time
	Value
	q1() const
	{
		Value const v( y_->q() + 2.0 );
		return ( ( 2.0 * v ) - y_->q1() ) / square( v );
	}

	// Quantized Second Derivative at Initialization Time
	Value
	q2() const
	{
		Value const v( y_->q() + 2.0 );
		return ( ( 2.0 * square( y_->q1() ) ) - ( v * ( y_->q2() + ( 4.0 * y_->q1() ) ) ) ) / cube( v );
	}

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

	// Quantized Values at Time t and at Variable +/- Delta
	AdvanceSpecsLIQSS1
	qlu( Time const t, Value const del ) const
	{
		// Value at +/- del
		Value const num( 1.0 + ( 2.0 * t ) );
		Value const y2( y_->q( t ) + 2.0 );
		Value const vl( num / ( y2 - del ) );
		Value const vu( num / ( y2 + del ) );

		// Zero point: No y gives zero function value at any t >= 0
		Value const z( 0.0 ); // No y value gives zero slope at any t >= 0

		return AdvanceSpecsLIQSS1{ vl, vu, z };
	}

	// Continuous Values at Time t and at Variable +/- Delta
	AdvanceSpecsLIQSS1
	xlu( Time const t, Value const del ) const
	{
		// Value at +/- del
		Value const num( 1.0 + ( 2.0 * t ) );
		Value const y2( y_->q( t ) + 2.0 );
		Value const vl( num / ( y2 - del ) );
		Value const vu( num / ( y2 + del ) );

		// Zero point: No y gives zero function value at any t >= 0
		Value const z( 0.0 );

		return AdvanceSpecsLIQSS1{ vl, vu, z };
	}

	// Quantized Values and Derivatives at Time t and at Variable +/- Delta
	AdvanceSpecsLIQSS2
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

		return AdvanceSpecsLIQSS2{ vl, vu, z1, sl, su, z2 };
	}

	// Continuous Values and Derivatives at Time t and at Variable +/- Delta
	AdvanceSpecsLIQSS2
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

		return AdvanceSpecsLIQSS2{ vl, vu, z1, sl, su, z2 };
	}

	// Exact Value of y at Time t
	Value
	e( Time const t ) const
	{
		return std::sqrt( ( 2.0 * t * ( t + 1.0 ) ) + 16.0 ) - 2.0;
	}

public: // Methods

	// Set Variables
	void
	var( Variable & y )
	{
		y_ = &y;
	}

	// Finalize Function Representation
	bool
	finalize( Variable * v )
	{
		assert( v != nullptr );

		// Add variables as observer of owning variable
		y_->add_observer( v );
		return true;
	}

	// Finalize Function Representation
	bool
	finalize( Variable & v )
	{
		return finalize( &v );
	}

private: // Methods

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

#endif
