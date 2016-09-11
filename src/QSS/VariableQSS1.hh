#ifndef QSS_VariableQSS1_hh_INCLUDED
#define QSS_VariableQSS1_hh_INCLUDED

// QSS1 Variable

// QSS Headers
#include <QSS/QSS.hh>
#include <QSS/Variable.hh>
#include <QSS/Function.hh>
#include <QSS/EventQueue.hh>

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

	// Continuous Constant on Active Segment
	double
	x0() const
	{
		return x0_;
	}

	// Continuous Slope on Active Segment
	double
	x1() const
	{
		return x1_;
	}

	// Derivative Function
	Derivative const &
	d() const
	{
		return d_;
	}

	// Derivative Function
	Derivative &
	d()
	{
		return d_;
	}

public: // Methods

	// Initialize Value
	VariableQSS1 &
	init_val()
	{
		q_ = x0_;
		return *this;
	}

	// Initialize to Value
	VariableQSS1 &
	init_val( double const xBeg )
	{
		x0_ = q_ = xBeg;
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

	// Initialize Derivative and Quantize
	void
	init_der()
	{
		x1_ = d_( tBeg ); // Assumes tBeg shared by all Variables at init time and init
		tEnd = tEndNext();
		event( events.add( tEnd, this ) );
	}

	// Continuous Value at Time t
	double
	x( Time const t ) const
	{
		assert( ( tBeg <= t ) && ( t <= tEnd ) );
		return x0_ + x1_ * ( t - tBeg );
	}

	// Quantized Value at Time t
	double
	q( Time const t ) const
	{
		assert( ( tBeg <= t ) && ( t <= tEnd ) );
		return q_;
	}

	// Continuous Derivative Value at Time t
	double
	d_x( Time const t ) const
	{
		assert( ( tBeg <= t ) && ( t <= tEnd ) );
		return d_.x( t );
	}

	// Quantized Derivative Value at Time t
	double
	d_q( Time const t ) const
	{
		assert( ( tBeg <= t ) && ( t <= tEnd ) );
		return d_.q( t );
	}

	// Next End Time
	Time
	tEndNext()
	{
		return
		 ( x1_ > 0.0 ? tBeg + ( ( ( q_ - x0_ ) + std::max( aTol, std::abs( rTol * x0_ ) ) ) / x1_ ) : // Slope > 0
		 ( x1_ < 0.0 ? tBeg + ( ( ( q_ - x0_ ) - std::max( aTol, std::abs( rTol * x0_ ) ) ) / x1_ ) : // Slope < 0
		 infinity ) ); // Slope == 0
	}

	// Advance Trigger to Time tEnd
	void
	advance()
	{
		x0_ = q_ = x0_ + ( x1_ * ( tEnd - tBeg ) );
		x1_ = d_.q( tBeg = tEnd );
		tEnd = tEndNext();
		event( events.shift( tEnd, event() ) );
		for ( Variable * observer : observers() ) { // Advance observers
			observer->advance( tBeg );
		}
	}

	// Advance Observer to Time t
	void
	advance( Time const t )
	{
		assert( ( tBeg <= t ) && ( t <= tEnd ) );
		if ( tBeg < t ) { // Could observe multiple variables with simultaneous triggering
			x0_ = x0_ + ( x1_ * ( t - tBeg ) );
			x1_ = d_.q( tBeg = t );
			tEnd = tEndNext();
			event( events.shift( tEnd, event() ) );
		}
	}

private: // Data

	double x0_{ 0.0 }, x1_{ 0.0 }; // Continuous value coefficients for active time segment
	double q_{ 0.0 }; // Quantized value for active time segment
	Derivative d_; // Derivative function

};

#endif
