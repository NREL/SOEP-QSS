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

	// Quantized Value at Initialization Time
	double
	q() const
	{
		return ( 1.0 ) / ( y_->q() + 2.0 );
	}

	// Quantized First Derivative at Initialization Time
	double
	q1() const
	{
		double const v( y_->q() + 2.0 );
		return ( ( 2.0 * v ) - y_->q1() ) / square( v );
	}

	// Quantized Second Derivative at Initialization Time
	double
	q2() const
	{
		double const v( y_->q() + 2.0 );
		return ( ( 2.0 * square( y_->q1() ) ) - ( v * ( y_->q2() + ( 4.0 * y_->q1() ) ) ) ) / cube( v );
	}

	// Continuous Value at Time t
	double
	operator ()( double const t ) const
	{
		return ( 1.0 + ( 2.0 * t ) ) / ( y_->x( t ) + 2.0 );
	}

	// Continuous Value at Time t
	double
	x( double const t ) const
	{
		return ( 1.0 + ( 2.0 * t ) ) / ( y_->x( t ) + 2.0 );
	}

	// Quantized Value at Time t
	double
	q( double const t ) const
	{
		return ( 1.0 + ( 2.0 * t ) ) / ( y_->q( t ) + 2.0 );
	}

	// Quantized First Derivative at Time t
	double
	q1( double const t ) const
	{
		double const v( y_->q( t ) + 2.0 );
		return ( ( 2.0 * v ) - ( y_->q1( t ) * ( 1.0 + ( 2.0 * t ) ) ) ) / square( v );
	}

	// Quantized Second Derivative at Time t
	double
	q2( double const t ) const
	{
		double const v( y_->q( t ) + 2.0 );
		double const w( 1.0 + 2.0 * t );
		return ( ( 2.0 * square( y_->q1( t ) ) * w ) - ( v * ( ( y_->q2( t ) * w ) + ( 4.0 * y_->q1( t ) ) ) ) ) / cube( v );
	}

	// Exact Value of y at Time t
	double
	e( double const t ) const
	{
		return std::sqrt( ( 2.0 * t * t ) + ( 2.0 * t ) + 16.0 ) - 2.0;
	}

private: // Data

	Variable * y_{ nullptr };

};

#endif
