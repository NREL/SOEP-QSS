#ifndef QSS_Function_achilles2_hh_INCLUDED
#define QSS_Function_achilles2_hh_INCLUDED

// Function for Achilles and the Tortoise Derivative Variable 2

// C++ Headers
#include <cassert>

// Linear Time-Invariant Function
template< typename V > // Template to avoid cyclic inclusion with Variable
class Function_achilles2
{

public: // Types

	using Coefficient = double;

	using Variable = V;

	using Value = typename Variable::Value;
	using Time = typename Variable::Time;

public: // Properties

	// Continuous Value at Time t
	Value
	operator ()( Time const t ) const
	{
		return c1_ * x1_->x( t );
	}

	// Continuous Value at Time t
	Value
	x( Time const t ) const
	{
		return c1_ * x1_->x( t );
	}

	// Quantized Value at Time t
	Value
	q( Time const t ) const
	{
		return c1_ * x1_->q( t );
	}

	// Quantized First Derivative at Time t
	Value
	q1( Time const t ) const
	{
		return c1_ * x1_->q1( t );
	}

	// Quantized Second Derivative at Time t
	Value
	q2( Time const t ) const
	{
		return c1_ * x1_->q2( t );
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
		assert( v != x1_ );
		x1_->add_observer( v ); // Add self variable as observer of non-self variable
		return false; // Not a self-observer
	}

	// Finalize Function Representation
	bool
	finalize( Variable & v )
	{
		return finalize( &v );
	}

private: // Data

	Coefficient c0_{ 0.0 }, c1_{ -1.0 };
	Variable * x1_{ nullptr };

};

#endif
