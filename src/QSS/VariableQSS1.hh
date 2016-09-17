#ifndef QSS_VariableQSS1_hh_INCLUDED
#define QSS_VariableQSS1_hh_INCLUDED

// QSS1 Variable

// QSS Headers
#include <QSS/Variable.hh>
#include <QSS/EventQueue.hh>
#include <QSS/Function.hh>
#include <QSS/globals.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cmath>
#include <string>

// QSS1 Variable
class VariableQSS1 final : public Variable
{

public: // Types

	using Time = Variable::Time;
	using Derivative = Function< Variable >;
	using Coefficient = Derivative::Coefficient;

public: // Creation

	// Constructor
	explicit
	VariableQSS1(
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
		return x0_ + ( x1_ * ( t - tCon ) );
	}

	// Quantized Value at Time t
	double
	q( Time const t ) const
	{
		assert( ( tBeg <= t ) && ( t <= tEnd ) );
		(void)t; // Suppress unused parameter warning
		return q_;
	}

	// Quantized Value at tBeg
	double
	q0() const
	{
		return q_;
	}

	// Quantized Slope at tBeg
	double
	q1() const
	{
		return 0.0;
	}

	// Quantized Slope at Time t
	double
	q1( Time const t ) const
	{
		assert( ( tBeg <= t ) && ( t <= tEnd ) );
		(void)t; // Suppress unused parameter warning
		return 0.0;
	}

	// Next End Time on Trigger Update
	Time
	tEndTrigger() const
	{
		return
		 ( x1_ != 0.0 ? tBeg + ( qTol / std::abs( x1_ ) ) : // Slope != 0
		 infinity ); // Slope == 0
	}

	// Next End Time on Observer Update
	Time
	tEndObserver() const
	{
		return
		 ( x1_ > 0.0 ? tCon + ( ( ( q_ - x0_ ) + qTol ) / x1_ ) : // Slope > 0
		 ( x1_ < 0.0 ? tCon + ( ( ( q_ - x0_ ) - qTol ) / x1_ ) : // Slope < 0
		 infinity ) ); // Slope == 0
	}

public: // Methods

	// Initialize Value
	VariableQSS1 &
	init_val()
	{
		q_ = x0_;
		set_qTol();
		return *this;
	}

	// Initialize to Value
	VariableQSS1 &
	init_val( double const xBeg )
	{
		x0_ = q_ = xBeg;
		set_qTol();
		return *this;
	}

	// Add Constant to Derivative
	VariableQSS1 &
	add_der( Coefficient const c0 )
	{
		d_.add( c0 );
		return *this;
	}

	// Add a Coefficient + Variable to Derivative
	VariableQSS1 &
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
	VariableQSS1 &
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
		x1_ = d_.q0();
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
		qTol = std::max( aTol, rTol * std::abs( q_ ) );
		assert( qTol > 0.0 );
	}

	// Advance Trigger to Time tEnd and Requantize
	void
	advance()
	{
		x0_ = q_ = x0_ + ( x1_ * ( tEnd - tCon ) );
		x1_ = d_.q( tBeg = tCon = tEnd );
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
			x0_ = x0_ + ( x1_ * ( t - tCon ) );
			x1_ = d_.q( tCon = t );
			tEnd = tEndObserver();
			event( events.shift( tEnd, event() ) );
		}
	}

private: // Data

	double x0_{ 0.0 }, x1_{ 0.0 }; // Continuous value coefficients for active time segment
	double q_{ 0.0 }; // Quantized value for active time segment
	Derivative d_; // Derivative function

};

#endif
