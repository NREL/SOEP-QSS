#ifndef QSS_Function_achilles1_hh_INCLUDED
#define QSS_Function_achilles1_hh_INCLUDED

// Function for Achilles and the Tortoise Derivative Variable 1

// C++ Headers
#include <cassert>
#include <vector>

// Linear Time-Invariant Function
template< typename V > // Template to avoid cyclic inclusion with Variable
class Function_achilles1
{

public: // Types

	using Coefficient = double;

	using Variable = V;

public: // Methods

	// Set Variables
	void
	var(
	 Variable & x1,
	 Variable & x2
	)
	{
		x1_ = &x1;
		x2_ = &x2;
	}

	// Finalize Function Representation
	bool
	finalize( Variable * v )
	{
		assert( v != nullptr );

		// Add variables as observer of owning variable
		x1_->add_observer( v );
		x2_->add_observer( v );
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
		return ( c1_ * x1_->q() ) + ( c2_ * x2_->q() );
	}

	// Quantized First Derivative at Initialization Time
	double
	q1() const
	{
		return ( c1_ * x1_->q1() ) + ( c2_ * x2_->q1() );
	}

	// Quantized Second Derivative at Initialization Time
	double
	q2() const
	{
		return ( c1_ * x1_->q2() ) + ( c2_ * x2_->q2() );
	}

	// Continuous Value at Time t
	double
	operator ()( double const t ) const
	{
		return ( c1_ * x1_->x( t ) ) + ( c2_ * x2_->x( t ) );
	}

	// Continuous Value at Time t
	double
	x( double const t ) const
	{
		return ( c1_ * x1_->x( t ) ) + ( c2_ * x2_->x( t ) );
	}

	// Quantized Value at Time t
	double
	q( double const t ) const
	{
		return ( c1_ * x1_->q( t ) ) + ( c2_ * x2_->q( t ) );
	}

	// Quantized First Derivative at Time t
	double
	q1( double const t ) const
	{
		return ( c1_ * x1_->q1( t ) ) + ( c2_ * x2_->q1( t ) );
	}

	// Quantized Second Derivative at Time t
	double
	q2( double const t ) const
	{
		return ( c1_ * x1_->q2( t ) ) + ( c2_ * x2_->q2( t ) );
	}

private: // Data

	Coefficient c0_{ 0.0 }, c1_{ -0.5 }, c2_{ 1.5 };
	Variable * x1_{ nullptr };
	Variable * x2_{ nullptr };

};

#endif
