#ifndef QSS_VariableQSS3_hh_INCLUDED
#define QSS_VariableQSS3_hh_INCLUDED

// QSS3 Variable

// QSS Headers
#include <QSS/Variable.hh>
#include <QSS/EventQueue.hh>
#include <QSS/Function.hh>
#include <QSS/globals.hh>
#include <QSS/math.hh>

// QSS3 Variable
class VariableQSS3 final : public Variable
{

public: // Types

	using Time = Variable::Time;
	using Derivative = Function< Variable >;
	using Coefficient = Derivative::Coefficient;

public: // Creation

	// Constructor
	explicit
	VariableQSS3(
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
		Time const tDel( t - tCon );
		return x0_ + ( x1_ * tDel ) + ( x2_ * square( tDel ) + ( x3_ * cube( tDel ) ) );
	}

	// Quantized Value at Time t
	double
	q( Time const t ) const
	{
		assert( ( tBeg <= t ) && ( t <= tEnd ) );
		return q0_ + ( q1_ * ( t - tBeg ) ) + ( q2_ * square( t - tBeg ) );
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
		return q1_ + ( two * q2_ * ( t - tBeg ) );
	}

	// Quantized Curvature at tBeg
	double
	q2() const
	{
		return q2_;
	}

	// Quantized Curvature at Time t
	double
	q2( Time const t ) const
	{
		assert( ( tBeg <= t ) && ( t <= tEnd ) );
		(void)t; // Suppress unused parameter warning
		return q2_;
	}

	// Next End Time on Trigger Update
	Time
	tEndTrigger() const
	{
		return
		 ( x3_ != 0.0 ? tBeg + std::cbrt( qTol / std::abs( x3_ ) ) : // 3rd deriv != 0
		 infinity ); // Curvature == 0
	}

	// Next End Time on Observer Update
	Time
	tEndObserver() const
	{
		double const tCB( tCon - tBeg );
		double const d0( x0_ - ( q0_ + ( q1_ * tCB ) + ( q2_ * ( tCB * tCB ) ) ) );
		double const d1( x1_ - ( q1_ + ( two * q2_ * tCB ) ) );
		double const d2( x2_ - q2_ );
		if ( ( x3_ >= 0.0 ) && ( d2 >= 0.0 ) && ( d1 >= 0.0 ) ) { // Only need to check +qTol
			Time const tPosQ( min_root_cubic( x3_, d2, d1, d0 - qTol ) );
			return ( tPosQ == infinity ? infinity : tCon + tPosQ );
		} else if ( ( x3_ <= 0.0 ) && ( d2 <= 0.0 ) && ( d1 <= 0.0 ) ) { // Only need to check -qTol
			Time const tNegQ( min_root_cubic( x3_, d2, d1, d0 + qTol ) );
			return ( tNegQ == infinity ? infinity : tCon + tNegQ );
		} else { // Check +qTol and -qTol
			Time const tPosQ( min_root_cubic( x3_, d2, d1, d0 - qTol ) );
			Time const tNegQ( min_root_cubic( x3_, d2, d1, d0 + qTol ) );
			Time const tMinQ( std::min( tPosQ, tNegQ ) );
			return ( tMinQ == infinity ? infinity : tCon + tMinQ );
		}
	}

public: // Methods

	// Initialize Value
	VariableQSS3 &
	init_val()
	{
		q0_ = x0_;
		set_qTol();
		return *this;
	}

	// Initialize to Value
	VariableQSS3 &
	init_val( double const xBeg )
	{
		x0_ = q0_ = xBeg;
		set_qTol();
		return *this;
	}

	// Add Constant to Derivative
	VariableQSS3 &
	add_der( Coefficient const c0 )
	{
		d_.add( c0 );
		return *this;
	}

	// Add a Coefficient + Variable to Derivative
	VariableQSS3 &
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
	VariableQSS3 &
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
		x2_ = q2_ = one_half * d_.q1();
	}

	// Initialize Third Derivative
	void
	init_der3()
	{
		x3_ = one_third * d_.q2();
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
		Time const tDel( tEnd - tCon );
		Time const tDel_sq( tDel * tDel );
		x0_ = q0_ = x0_ + ( x1_ * tDel ) + ( x2_ * tDel_sq ) + ( x3_ * ( tDel_sq * tDel ) );
		x1_ = q1_ = d_.q( tBeg = tCon = tEnd );
		x2_ = q2_ = one_half * d_.q1( tBeg );
		x3_ = one_third * d_.q2( tBeg );
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
			Time const tDel( t - tCon );
			Time const tDel_sq( tDel * tDel );
			x0_ = x0_ + ( x1_ * tDel ) + ( x2_ * tDel_sq ) + ( x3_ * ( tDel_sq * tDel ) );
			x1_ = d_.q( t );
			x2_ = one_half * d_.q1( t );
			x3_ = one_third * d_.q2( t );
			tCon = t;
			tEnd = tEndObserver();
			event( events.shift( tEnd, event() ) );
		}
	}

private: // Data

	double x0_{ 0.0 }, x1_{ 0.0 }, x2_{ 0.0 }, x3_{ 0.0 }; // Continuous value coefficients for active time segment
	double q0_{ 0.0 }, q1_{ 0.0 }, q2_{ 0.0 }; // Quantized value coefficients for active time segment
	Derivative d_; // Derivative function

};

#endif
