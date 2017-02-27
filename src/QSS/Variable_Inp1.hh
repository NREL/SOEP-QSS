#ifndef QSS_Variable_Inp1_hh_INCLUDED
#define QSS_Variable_Inp1_hh_INCLUDED

// QSS Input Variable with Quantization Order 1
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/Variable_Inp.hh>

// QSS Input Variable with Quantization Order 1
template< class F >
class Variable_Inp1 final : public Variable_Inp< F >
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
	Variable_Inp1(
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
		return 1;
	}

	// Continuous Value at Time t
	Value
	x( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		return x_0_ + ( x_1_ * ( t - tX ) );
	}

	// Continuous Numeric Differenentiation Value at Time t: Allow t Outside of [tX,tE]
	Value
	xn( Time const t ) const
	{
		return x_0_ + ( x_1_ * ( t - tX ) );
	}

	// Continuous First Derivative at Time t
	Value
	x1( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		return x_1_;
	}

	// Quantized Value at Time t
	Value
	q( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		(void)t; // Suppress unused parameter warning
		return q_0_;
	}

	// Quantized Numeric Differenentiation Value at Time t: Allow t Outside of [tQ,tE]
	Value
	qn( Time const t ) const
	{
		(void)t; // Suppress unused parameter warning
		return q_0_;
	}

public: // Methods

	// Initialize Input Variable
	void
	init()
	{
		init0();
		init1();
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
		x_1_ = f_.df1( tQ );
	}

	// Initialize Event in Queue
	void
	init_event()
	{
		set_tE();
		event( events.add( tE, this ) );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
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
		x_1_ = f_.df1( tE );
		set_tE();
		event( events.shift( tE, event() ) );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
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
		x_1_ = f_.df1( tE );
		set_tE();
		event( events.shift( tE, event() ) );
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
	}

private: // Methods

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE()
	{
		assert( tX <= tQ );
		assert( dt_min <= dt_max );
		tE = ( x_1_ != 0.0 ? tQ + ( qTol / std::abs( x_1_ ) ) : infinity );
		if ( dt_max != infinity ) tE = std::min( tE, tQ + dt_max );
		tE = std::max( tE, tQ + dt_min );
	}

private: // Data

	Value x_0_{ 0.0 }, x_1_{ 0.0 }; // Continuous rep coefficients
	Value q_0_{ 0.0 }; // Quantized rep coefficients

};

#endif
