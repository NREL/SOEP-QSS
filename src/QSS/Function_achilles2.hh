#ifndef QSS_Function_achilles2_hh_INCLUDED
#define QSS_Function_achilles2_hh_INCLUDED

// Function for Achilles and the Tortoise Derivative Variable 2

// C++ Headers
#include <cassert>
#include <vector>

// Linear Time-Invariant Function
template< typename V > // Template to avoid cyclic inclusion with Variable
class Function_achilles2
{

public: // Types

	using Coefficient = double;

	using Variable = V;

public: // Methods

	// Set Variables
	void
	var( Variable & x1 )
	{
		x1_ = &x1;
	}

	// Finalize Function Representation
	bool
	finalize( Variable * v )
	{
		assert( v != nullptr );

		// Add variables as observer of owning variable
		x1_->add_observer( v );
		return false;
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
		return ( c1_ * x1_->q() );
	}

	// Quantized First Derivative at Initialization Time
	double
	q1() const
	{
		return ( c1_ * x1_->q1() );
	}

	// Quantized Second Derivative at Initialization Time
	double
	q2() const
	{
		return ( c1_ * x1_->q2() );
	}

	// Continuous Value at Time t
	double
	operator ()( double const t ) const
	{
		return ( c1_ * x1_->x( t ) );
	}

	// Continuous Value at Time t
	double
	x( double const t ) const
	{
		return ( c1_ * x1_->x( t ) );
	}

	// Quantized Value at Time t
	double
	q( double const t ) const
	{
		return ( c1_ * x1_->q( t ) );
	}

	// Quantized First Derivative at Time t
	double
	q1( double const t ) const
	{
		return ( c1_ * x1_->q1( t ) );
	}

	// Quantized Second Derivative at Time t
	double
	q2( double const t ) const
	{
		return ( c1_ * x1_->q2( t ) );
	}

private: // Data

	Coefficient c0_{ 0.0 }, c1_{ -1.0 };
	Variable * x1_{ nullptr };

};

#endif
