#ifndef QSS_Function_NonlinearEx1_ND_hh_INCLUDED
#define QSS_Function_NonlinearEx1_ND_hh_INCLUDED

// Derivative Function for Nonlinear Example 1: Numeric Differentiation
// Problem:  y'( t ) = ( 1 + 2 t ) / ( y + 2 ), y( 0 ) = 2
// Solution: y = sqrt( 2 t^2 + 2 t + 16 ) - 2
// Note:     y''( t ) = ( 2 / ( y + 2 ) ) - ( ( 1 + 2 t )^2 / ( y + 2 )^3 )

// QSS Headers
#include <QSS/globals.hh>
#include <QSS/math.hh>

// C++ Headers
#include <cassert>
#include <cmath>

// Linear Time-Invariant Function
template< typename V > // Template to avoid cyclic inclusion with Variable
class Function_NonlinearEx1_ND
{

public: // Types

	using Coefficient = double;

	using Variable = V;

	using Value = typename Variable::Value;
	using Time = typename Variable::Time;
	using AdvanceSpecsLIQSS1 = typename Variable::AdvanceSpecsLIQSS1;
	using AdvanceSpecsLIQSS2 = typename Variable::AdvanceSpecsLIQSS2;

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

	// Continuous Differentiation Value at Time t
	Value
	xn( Time const t ) const
	{
		return ( 1.0 + ( 2.0 * t ) ) / ( y_->xn( t ) + 2.0 );
	}

	// Continuous First Derivative at Time t
	Value
	x1( Time const t ) const
	{
		return dtn_inv_2_ * ( xn( t + dtn_ ) - xn( t - dtn_ ) );
	}

	// Quantized Value at Time t
	Value
	q( Time const t ) const
	{
		return ( 1.0 + ( 2.0 * t ) ) / ( y_->q( t ) + 2.0 );
	}

	// Quantized Numeric Differentiation Value at Time t
	Value
	qn( Time const t ) const
	{
		return ( 1.0 + ( 2.0 * t ) ) / ( y_->qn( t ) + 2.0 );
	}

	// Quantized First Derivative at Time t
	Value
	q1( Time const t ) const
	{
		return dtn_inv_2_ * ( qn( t + dtn_ ) - qn( t - dtn_ ) );
	}

	// Quantized Second Derivative at Time t
	Value
	q2( Time const t ) const
	{
		return dtn_inv_sq_ * ( qn( t + dtn_ ) - ( 2.0 * qn( t ) ) + qn( t - dtn_ ) );
	}

	// Quantized Sequential Value at Time t
	Value
	qs( Time const t ) const
	{
		return q_t_ = qn( t );
	}

	// Quantized Forward-Difference Sequential First Derivative at Time t
	Value
	qf1( Time const t ) const
	{
		return dtn_inv_ * ( qn( t + dtn_ ) - q_t_ );
	}

	// Quantized Centered-Difference Sequential First Derivative at Time t
	Value
	qc1( Time const t ) const
	{
		return dtn_inv_2_ * ( ( q_p_ = qn( t + dtn_ ) ) - ( q_m_ = qn( t - dtn_ ) ) );
	}

	// Quantized Centered-Difference Sequential Second Derivative at Time t
	Value
	qc2( Time const t ) const
	{
		return dtn_inv_sq_ * ( q_p_ - ( 2.0 * q_t_ ) + q_m_ );
	}

	// Differentiation Time Step
	Time
	dtn() const
	{
		return dtn_;
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
		Time const tm( t - dtn_ );
		Time const tp( t + dtn_ );
		Value const y2m( y_->qn( tm ) + 2.0 );
		Value const y2p( y_->qn( tp ) + 2.0 );
		Value const sl( dtn_inv_2_ * ( ndv( tp, y2p, -del ) - ndv( tm, y2m, -del ) ) );
		Value const su( dtn_inv_2_ * ( ndv( tp, y2p, +del ) - ndv( tm, y2m, +del ) ) );

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
		Time const tm( t - dtn_ );
		Time const tp( t + dtn_ );
		Value const y2m( y_->xn( tm ) + 2.0 );
		Value const y2p( y_->xn( tp ) + 2.0 );
		Value const sl( dtn_inv_2_ * ( ndv( tp, y2p, -del ) - ndv( tm, y2m, -del ) ) );
		Value const su( dtn_inv_2_ * ( ndv( tp, y2p, +del ) - ndv( tm, y2m, +del ) ) );

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

	// Set Variable
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

	// Numeric Differentiation Value at Time t Given y+2 and y Delta
	static
	Value
	ndv( Time const t, Value const y2, Value const del )
	{
		return ( 1.0 + ( 2.0 * t ) ) / ( y2 + del );
	}

private: // Data

	Variable * y_{ nullptr };
	mutable Value q_t_; // Last q(t) computed
	mutable Value q_p_; // Last q(t+dtn) computed
	mutable Value q_m_; // Last q(t-dtn) computed
	Time dtn_{ dtnd }; // Differentiation time step
	Time dtn_inv_{ 1.0 / dtnd }; // Differentiation time step inverse
	Time dtn_inv_2_{ 0.5 / dtnd }; // Differentiation time step half inverse
	Time dtn_inv_sq_{ 1.0 / ( dtnd * dtnd ) }; // Differentiation time step inverse squared

};

#endif
