#ifndef QSS_Variable_Inp3_hh_INCLUDED
#define QSS_Variable_Inp3_hh_INCLUDED

// QSS Input Variable with Quantization Order 3
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/Variable_Inp.hh>

// QSS Input Variable with Quantization Order 3
template< class F >
class Variable_Inp3 final : public Variable_Inp< F >
{

public: // Types

	using Super = Variable_Inp< F >;
	using Time = Variable::Time;
	using Value = Variable::Value;
	using Function = typename Super::Function;
	using Coefficient = typename Super::Coefficient;

	using Super::name;
	using Super::rTol;
	using Super::aTol;
	using Super::qTol;
	using Super::tQ;
	using Super::tX;
	using Super::tE;
	using Super::dt_min;
	using Super::dt_max;

	using Super::event;
	using Super::advance_observers;
	using Super::shrink_observers;

private: // Types

	using Super::f_;

public: // Creation

	// Constructor
	explicit
	Variable_Inp3(
	 std::string const & name,
	 Value const rTol = 1.0e-4,
	 Value const aTol = 1.0e-6
	) :
	 Variable_Inp< F >( name, rTol, aTol )
	{
		init0();
	}

public: // Properties

	// Order of Method
	int
	order() const
	{
		return 3;
	}

	// Continuous Value at Time t
	Value
	x( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		return x_0_ + ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
	}

	// Continuous Numeric Differenentiation Value at Time t: Allow t Outside of [tX,tE]
	Value
	xn( Time const t ) const
	{
		Time const tDel( t - tX );
		return x_0_ + ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
	}

	// Continuous First Derivative at Time t
	Value
	x1( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		return x_1_ + ( ( ( two * x_2_ ) + ( three * x_3_ * tDel ) ) * tDel );
	}

	// Continuous Second Derivative at Time t
	Value
	x2( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		return ( two * x_2_ ) + ( six * x_3_ * ( t - tX ) );
	}

	// Continuous Third Derivative at Time t
	Value
	x3( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		return six * x_3_;
	}

	// Quantized Value at Time t
	Value
	q( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		Time const tDel( t - tQ );
		return q_0_ + ( ( q_1_ + ( q_2_ * tDel ) ) * tDel );
	}

	// Quantized Numeric Differenentiation Value at Time t: Allow t Outside of [tQ,tE]
	Value
	qn( Time const t ) const
	{
		Time const tDel( t - tQ );
		return q_0_ + ( ( q_1_ + ( q_2_ * tDel ) ) * tDel );
	}

	// Quantized First Derivative at Time t
	Value
	q1( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		return q_1_ + ( two * q_2_ * ( t - tQ ) );
	}

	// Quantized Second Derivative at Time t
	Value
	q2( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		(void)t; // Suppress unused parameter warning
		return two * q_2_;
	}

public: // Methods

	// Initialize Input Variable
	void
	init()
	{
		init0();
		init1();
		init2();
		init3();
		init_event();
	}

	// Initialize Constant Term
	void
	init0()
	{
		x_0_ = q_0_ = f_.vs( tQ );
		set_qTol();
	}

	// Initialize Linear Coefficient
	void
	init1()
	{
		shrink_observers(); // Optional
		x_1_ = q_1_ = f_.dc1( tQ );
	}

	// Initialize Quadratic Coefficient
	void
	init2()
	{
		x_2_ = q_2_ = one_half * f_.dc2( tQ );
	}

	// Initialize Cubic Coefficient
	void
	init3()
	{
		x_3_ = one_sixth * f_.dc3( tQ );
	}

	// Initialize Event in Queue
	void
	init_event()
	{
		set_tE();
		event( events.add( tE, this ) );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t+" << q_2_ << "*t^2 quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2+" << x_3_ << "*t^3 internal   tE=" << tE << '\n';
	}

	// Set Current Tolerance
	void
	set_qTol()
	{
		qTol = std::max( rTol * std::abs( q_0_ ), aTol );
		assert( qTol > 0.0 );
	}

	// Advance Trigger to Time tE and Requantize
	void
	advance()
	{
		x_0_ = q_0_ = f_.vs( tX = tQ = tE );
		set_qTol();
		x_1_ = q_1_ = f_.dc1( tE );
		x_2_ = q_2_ = one_half * f_.dc2( tE );
		x_3_ = one_sixth * f_.dc3( tX = tE );
		set_tE();
		event( events.shift( tE, event() ) );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t+" << q_2_ << "*t^2 quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2+" << x_3_ << "*t^3 internal   tE=" << tE << '\n';
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
		x_2_ = q_2_ = one_half * f_.dc2( tE );
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 3
	void
	advance3()
	{
		x_3_ = one_sixth * f_.dc3( tE );
		set_tE();
		event( events.shift( tE, event() ) );
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t+" << q_2_ << "*t^2 quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2+" << x_3_ << "*t^3 internal   tE=" << tE << '\n';
	}

private: // Methods

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE()
	{
		assert( tX <= tQ );
		assert( dt_min <= dt_max );
		tE = ( x_3_ != 0.0 ? tQ + std::cbrt( qTol / std::abs( x_3_ ) ) : infinity );
		if ( dt_max != infinity ) tE = std::min( tE, tQ + dt_max );
		tE = std::max( tE, tQ + dt_min );
		if ( ( options::inflection ) && ( x_3_ != 0.0 ) && ( signum( x_2_ ) != signum( x_3_ ) ) ) {
			Time const tI( tX - ( x_2_ / ( three * x_3_ ) ) );
			if ( tQ < tI ) tE = std::min( tE, tI );
		}
	}

private: // Data

	Value x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }, x_3_{ 0.0 }; // Continuous rep coefficients
	Value q_0_{ 0.0 }, q_1_{ 0.0 }, q_2_{ 0.0 }; // Quantized rep coefficients

};

#endif
