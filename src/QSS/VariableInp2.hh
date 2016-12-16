#ifndef QSS_VariableInp2_hh_INCLUDED
#define QSS_VariableInp2_hh_INCLUDED

// QSS Input Variable with Quantization Order 2

// QSS Headers
#include <QSS/Variable.hh>
#include <QSS/globals.hh>

// QSS Input Variable with Quantization Order 2
template< template< typename > typename F >
class VariableInp2 final : public Variable
{

public: // Types

	using Value = Variable::Value;
	using Time = Variable::Time;
	using Function = F< Variable >;
	using Coefficient = typename Function::Coefficient;

public: // Creation

	// Constructor
	explicit
	VariableInp2(
	 std::string const & name,
	 Value const aTol = 1.0e-6,
	 Value const rTol = 1.0e-6
	) :
	 Variable( name, aTol, rTol )
	{}

public: // Properties

	// Order of Method
	int
	order() const
	{
		return 2;
	}

	// Continuous Value at Time t
	Value
	x( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		return x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// Continuous Numeric Differenentiation Value at Time t: Allow t Outside of [tX,tE]
	Value
	xn( Time const t ) const
	{
		Time const tDel( t - tX );
		return x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// Continuous First Derivative at Time t
	Value
	x1( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		return x_1_ + ( two * x_2_ * ( t - tX ) );
	}

	// Continuous Second Derivative at Time t
	Value
	x2( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		return two * x_2_;
	}

	// Quantized Value at Time t
	Value
	q( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		return q_0_ + ( q_1_ * ( t - tQ ) );
	}

	// Quantized Numeric Differenentiation Value at Time t: Allow t Outside of [tQ,tE]
	Value
	qn( Time const t ) const
	{
		return q_0_ + ( q_1_ * ( t - tQ ) );
	}

	// Quantized First Derivative at Time t
	Value
	q1( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		(void)t; // Suppress unused parameter warning
		return q_1_;
	}

	// Function
	Function const &
	f() const
	{
		return f_;
	}

	// Function
	Function &
	f()
	{
		return f_;
	}

public: // Methods

	// Initialize Constant Term
	void
	init0()
	{
		x_0_ = q_0_ = f_( tQ );
		set_qTol();
	}

	// Initialize Linear Coefficient
	void
	init1()
	{
		shrink_observers(); // Optional
		x_1_ = q_1_ = f_.d1( tQ );
	}

	// Initialize Quadratic Coefficient
	void
	init2()
	{
		x_2_ = one_half * f_.d2( tQ );
	}

	// Initialize Event in Queue
	void
	init_event()
	{
		set_tE();
		event( events.add( tE, this ) );
		if ( diag ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
	}

	// Set Current Tolerance
	void
	set_qTol()
	{
		qTol = std::max( aTol, rTol * std::abs( q_0_ ) );
		assert( qTol > 0.0 );
	}

	// Advance Trigger to Time tE and Requantize
	void
	advance()
	{
		x_0_ = q_0_ = f_.vs( tX = tQ = tE );
		set_qTol();
		x_1_ = q_1_ = f_.dc1( tE );
		x_2_ = one_half * f_.dc2( tX = tE );
		set_tE();
		event( events.shift( tE, event() ) );
		if ( diag ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
		advance_observers();
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 0
	void
	advance0()
	{
		x_0_ = q_0_ = f_.vs( tX = tQ = tE );
		set_qTol();
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 1
	void
	advance1()
	{
		x_1_ = q_1_ = f_.dc1( tE );
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 2
	void
	advance2()
	{
		x_2_ = one_half * f_.dc2( tE );
		set_tE();
		event( events.shift( tE, event() ) );
		if ( diag ) std::cout << "= " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
	}

private: // Methods

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE()
	{
		assert( tX <= tQ );
		tE = ( x_2_ != 0.0 ? tQ + std::sqrt( qTol / std::abs( x_2_ ) ) : infinity );
		if ( dt_max != infinity ) tE = std::min( tE, tQ + dt_max );
		if ( ( inflection_steps ) && ( x_2_ != 0.0 ) && ( signum( x_1_ ) != signum( x_2_ ) ) ) {
			Time const tI( tX - ( x_1_ / ( two * x_2_ ) ) );
			if ( tQ < tI ) tE = std::min( tE, tI );
		}
	}

private: // Data

	Value x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }; // Continuous rep coefficients
	Value q_0_{ 0.0 }, q_1_{ 0.0 }; // Quantized rep coefficients
	Function f_; // Value function

};

#endif
