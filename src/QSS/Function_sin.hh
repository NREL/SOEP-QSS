#ifndef QSS_Function_sin_hh_INCLUDED
#define QSS_Function_sin_hh_INCLUDED

// Sine Function
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// C++ Headers
#include <cassert>
#include <cmath>

// Sine Function
class Function_sin
{

public: // Types

	using Time = double;
	using Value = double;
	using Coefficient = double;

public: // Creation

	// Default Constructor
	Function_sin()
	{}

	// Constructor
	explicit
	Function_sin(
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
		return c_ * s_ * std::cos( s_ * t );
	}

	// Second Derivative at Time t
	Value
	d2( Time const t ) const
	{
		return -c_ * ( s_ * s_ ) * std::sin( s_ * t );
	}

	// Third Derivative at Time t
	Value
	d3( Time const t ) const
	{
		return -c_ * ( s_ * s_ * s_ ) * std::cos( s_ * t );
	}

	// Sequential Value at Time t
	Value
	vs( Time const t ) const
	{
		return v( t );
	}

	// Forward-Difference Sequential First Derivative at Time t
	Value
	df1( Time const t ) const
	{
		return d1( t );
	}

	// Centered-Difference Sequential First Derivative at Time t
	Value
	dc1( Time const t ) const
	{
		return d1( t );
	}

	// Centered-Difference Sequential Second Derivative at Time t
	Value
	dc2( Time const t ) const
	{
		return d2( t );
	}

	// Centered-Difference Sequential Third Derivative at Time t
	Value
	dc3( Time const t ) const
	{
		return d3( t );
	}

public: // Methods

	// Set Value Scaling
	Function_sin &
	c( Coefficient const c )
	{
		c_ = c;
		return *this;
	}

	// Set Time Scaling
	Function_sin &
	s( Coefficient const s )
	{
		s_ = s;
		return *this;
	}

public: // Static Data

	static int const max_order = 3; // Max QSS order supported

private: // Data

	Coefficient c_{ 1.0 }; // Value scaling
	Coefficient s_{ 1.0 }; // Time scaling

};

#endif
