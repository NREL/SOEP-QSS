#ifndef QSS_Function_sin_ND_hh_INCLUDED
#define QSS_Function_sin_ND_hh_INCLUDED

// Sine Function Using Numeric Differentiation

// QSS Headers
#include <QSS/globals.hh>

// C++ Headers
#include <cassert>
#include <cmath>

// Sine Function
template< typename V > // Template to avoid cyclic inclusion with Variable
class Function_sin_ND
{

public: // Types

	using Coefficient = double;

	using Variable = V;

	using Value = typename Variable::Value;
	using Time = typename Variable::Time;

public: // Creation

	// Default Constructor
	Function_sin_ND()
	{}

	// Constructor
	explicit
	Function_sin_ND(
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
		return dtn_inv_2_ * ( v( t + dtn_ ) - v( t - dtn_ ) );
	}

	// Second Derivative at Time t
	Value
	d2( Time const t ) const
	{
		return dtn_inv_sq_ * ( v( t + dtn_ ) - ( 2.0 * v( t ) ) + v( t - dtn_ ) );
	}

	// Third Derivative at Time t
	Value
	d3( Time const t ) const
	{
		return dtn_inv_cb_2_ * ( v( t + dtn_2_ ) - ( 2.0 * ( v( t + dtn_ ) - v( t - dtn_ ) ) ) - v( t - dtn_2_ ) );
	}

	// Sequential Value at Time t
	Value
	vs( Time const t ) const
	{
		return v_t_ = v( t );
	}

	// Forward-Difference Sequential First Derivative at Time t
	Value
	df1( Time const t ) const
	{
		return dtn_inv_ * ( v( t + dtn_ ) - v_t_ );
	}

	// Centered-Difference Sequential First Derivative at Time t
	Value
	dc1( Time const t ) const
	{
		return dtn_inv_2_ * ( ( v_p_ = v( t + dtn_ ) ) - ( v_m_ = v( t - dtn_ ) ) );
	}

	// Centered-Difference Sequential Second Derivative at Time t
	Value
	dc2( Time const t ) const
	{
		return dtn_inv_sq_ * ( v_p_ - ( 2.0 * v_t_ ) + v_m_ );
	}

	// Centered-Difference Sequential Third Derivative at Time t
	Value
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

public: // Methods

	// Set Value Scaling
	Function_sin_ND &
	c( Coefficient const c )
	{
		c_ = c;
		return *this;
	}

	// Set Time Scaling
	Function_sin_ND &
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

public: // Static Data

	static int const max_order = 3; // Max QSS order supported

private: // Data

	Coefficient c_{ 1.0 }; // Value scaling
	Coefficient s_{ 1.0 }; // Time scaling
	mutable Value v_t_; // Last v(t) computed
	mutable Value v_p_; // Last v(t+dtn) computed
	mutable Value v_m_; // Last v(t-dtn) computed
	Time dtn_{ dtnd }; // Differentiation time step
	Time dtn_2_{ 2 * dtnd }; // Differentiation time step doubled
	Time dtn_inv_{ 1.0 / dtnd }; // Differentiation time step inverse
	Time dtn_inv_2_{ 0.5 / dtnd }; // Differentiation time step half inverse
	Time dtn_inv_sq_{ 1.0 / ( dtnd * dtnd ) }; // Differentiation time step inverse squared
	Time dtn_inv_cb_2_{ 0.5 / ( dtnd * dtnd * dtnd ) }; // Differentiation time step inverse squared

};

#endif
