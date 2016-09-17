#ifndef QSS_VariableQSS2_hh_INCLUDED
#define QSS_VariableQSS2_hh_INCLUDED

// QSS2 Variable

// QSS Headers
#include <QSS/Variable.hh>
#include <QSS/EventQueue.hh>
#include <QSS/Function.hh>
#include <QSS/globals.hh>
#include <QSS/math.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cmath>
#include <string>

// QSS2 Variable
class VariableQSS2 final : public Variable
{

public: // Types

	using Time = Variable::Time;
	using Derivative = Function< Variable >;
	using Coefficient = Derivative::Coefficient;

public: // Creation

	// Constructor
	explicit
	VariableQSS2(
	 std::string const & name,
	 double const aTol = 1.0e-6,
	 double const rTol = 1.0e-6
	) :
	 Variable( name, aTol, rTol )
	{}

public: // Properties

	// Continuous Value at Time t
	double
	x( Time const t ) const
	{
		assert( ( tCon <= t ) && ( t <= tEnd ) );
		return x0_ + ( x1_ * ( t - tCon ) ) + ( x2_ * square( t - tCon ) );
	}

	// Quantized Value at Time t
	double
	q( Time const t ) const
	{
		assert( ( tBeg <= t ) && ( t <= tEnd ) );
		return q0_ + ( q1_ * ( t - tBeg ) );
	}

	// Quantized Value at tBeg
	double
	q0() const
	{
		return q0_;
	}

	// Quantized Slope at tBeg
	double
	q1() const
	{
		return q1_;
	}

	// Quantized Slope at Time t
	double
	q1( Time const t ) const
	{
		assert( ( tBeg <= t ) && ( t <= tEnd ) );
		(void)t; // Suppress unused parameter warning
		return q1_;
	}

	// Next End Time on Trigger Update
	Time
	tEndTrigger() const
	{
		return
		 ( x2_ != 0.0 ? tBeg + std::sqrt( qTol / std::abs( x2_ ) ) : // Curvature != 0
		 infinity ); // Curvature == 0
	}

	// Next End Time on Observer Update
	Time
	tEndObserver() const
	{
		double const d0( x0_ - ( q0_ + ( q1_ * ( tCon - tBeg ) ) ) );
		if ( x1_ - q1_ >= 0.0 ) {
			Time const tPosQ( min_root( x2_, x1_ - q1_, d0 - qTol ) );
			if ( x2_ >= 0.0 ) { // Only need to check +qTol
				return ( tPosQ == infinity ? infinity : tCon + tPosQ );
			} else {
				Time const tNegQ( min_root( x2_, x1_ - q1_, d0 + qTol ) );
				Time const tMinQ( std::min( tPosQ, tNegQ ) );
				return ( tMinQ == infinity ? infinity : tCon + tMinQ );
			}
		} else { // x1 - q1 < 0
			Time const tNegQ( min_root( x2_, x1_ - q1_, d0 + qTol ) );
			if ( x2_ <= 0.0 ) { // Only need to check -qTol
				return ( tNegQ == infinity ? infinity : tCon + tNegQ );
			} else {
				Time const tPosQ( min_root( x2_, x1_ - q1_, d0 - qTol ) );
				Time const tMinQ( std::min( tPosQ, tNegQ ) );
				return ( tMinQ == infinity ? infinity : tCon + tMinQ );
			}
		}
	}

public: // Methods

	// Initialize Value
	VariableQSS2 &
	init_val()
	{
		q0_ = x0_;
		set_qTol();
		return *this;
	}

	// Initialize to Value
	VariableQSS2 &
	init_val( double const xBeg )
	{
		x0_ = q0_ = xBeg;
		set_qTol();
		return *this;
	}

	// Add Constant to Derivative
	VariableQSS2 &
	add_der( Coefficient const c0 )
	{
		d_.add( c0 );
		return *this;
	}

	// Add a Coefficient + Variable to Derivative
	VariableQSS2 &
	add_der(
	 Coefficient const c_i,
	 Variable & x_i
	)
	{
		d_.add( c_i, x_i );
		x_i.add_observer( this );
		return *this;
	}

	// Add a Coefficient + Variable to Derivative
	VariableQSS2 &
	add_der(
	 Coefficient const c_i,
	 Variable * x_i
	)
	{
		d_.add( c_i, x_i );
		x_i->add_observer( this );
		return *this;
	}

	// Initialize First Derivative
	void
	init_der()
	{
		x1_ = q1_ = d_.q0();
	}

	// Initialize Second Derivative
	void
	init_der2()
	{
		x2_ = 0.5 * d_.q1();
	}

	// Initialize Event in Queue
	void
	init_event()
	{
		tEnd = tEndTrigger();
		event( events.add( tEnd, this ) );
	}

	// Set Current Tolerance
	void
	set_qTol()
	{
		qTol = std::max( aTol, rTol * std::abs( q0_ ) );
		assert( qTol > 0.0 );
	}

	// Advance Trigger to Time tEnd and Requantize
	void
	advance()
	{
		x0_ = q0_ = x0_ + ( x1_ * ( tEnd - tCon ) ) + ( x2_ * square( tEnd - tCon ) );
		x1_ = q1_ = d_.q( tBeg = tCon = tEnd );
		x2_ = 0.5 * d_.q1( tBeg );
		set_qTol();
		tEnd = tEndTrigger();
		event( events.shift( tEnd, event() ) );
		for ( Variable * observer : observers() ) { // Advance observers
			observer->advance( tBeg );
		}
	}

	// Advance Observer to Time t
	void
	advance( Time const t )
	{
		assert( ( tCon <= t ) && ( t <= tEnd ) );
		if ( tCon < t ) { // Could observe multiple variables with simultaneous triggering
			x0_ = x0_ + ( x1_ * ( t - tCon ) ) + ( x2_ * square( t - tCon ) );
			x1_ = d_.q( t );
			x2_ = 0.5 * d_.q1( t );
			tCon = t;
			tEnd = tEndObserver();
			event( events.shift( tEnd, event() ) );
		}
	}

private: // Methods

	// Min Nonnegative Root of Quadratic Equation
	double
	min_root( double const a, double const b, double const c ) const
	{
		if ( a == 0.0 ) { // Linear
			if ( b == 0.0 ) { // Degenerate
				return infinity;
			} else {
				double const root( -( c / b ) );
				return ( root >= 0.0 ? root : infinity );
			}
		} else { // Quadratic
			double const disc( square( b ) - ( 4.0 * a * c ) );
			if ( disc < 0.0 ) { // No real roots
				return infinity;
			} else if ( disc == 0.0 ) { // One real root
				double const root( -( b / ( 2.0 * a ) ) );
				return ( root >= 0.0 ? root : infinity );
			} else { // Two real roots: From http://mathworld.wolfram.com/QuadraticEquation.html for precision
				double const q( -0.5 * ( b + ( sign( b ) * std::sqrt( disc ) ) ) );
				double const root1( q / a );
				double const root2( c / q );
				return ( root1 >= 0.0 ? ( root2 >= 0.0 ? std::min( root1, root2 ) : root1 ) : ( root2 >= 0.0 ? root2 : infinity ) );
			}
		}
	}

private: // Data

	double x0_{ 0.0 }, x1_{ 0.0 }, x2_{ 0.0 }; // Continuous value coefficients for active time segment
	double q0_{ 0.0 }, q1_{ 0.0 }; // Quantized value coefficients for active time segment
	Derivative d_; // Derivative function

};

#endif
