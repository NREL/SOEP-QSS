#ifndef QSS_Variable_FMU_QSS2_hh_INCLUDED
#define QSS_Variable_FMU_QSS2_hh_INCLUDED

// FMU-Based QSS2 Variable
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/Variable_FMU.hh>

// FMU-Based QSS2 Variable
class Variable_FMU_QSS2 final : public Variable_FMU
{

public: // Types

	using Time = Variable::Time;
	using Value = Variable::Value;

public: // Creation

	// Constructor
	explicit
	Variable_FMU_QSS2(
	 std::string const & name,
	 Value const rTol = 1.0e-4,
	 Value const aTol = 1.0e-6,
	 Value const xIni = 0.0,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Variable_FMU( name, rTol, aTol, xIni, var, der ),
	 x_0_( xIni ),
	 q_0_( xIni )
	{
		set_qTol();
	}

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

public: // Methods

	// Initialize QSS Variable
	void
	init( Value const x )
	{
		init0( x );
		init1();
		init2();
		init_event();
	}

	// Initialize Constant Term to Given Value
	void
	init0( Value const x )
	{
		x_0_ = q_0_ = x;
		set_qTol();
	}

	// Initialize Linear Coefficient
	void
	init1()
	{
//		self_observer = d_.finalize( this ); // Handled in main for FMU run
		shrink_observers(); // Optional
		shrink_observees(); // Optional
		fmu_set_observees_q( tQ );
	}

	// Initialize Linear Coefficient for FMU
	void
	init1_fmu()
	{
		x_1_ = q_1_ = FMU::get_derivative( der.ics );
	}

	// Initialize Quadratic Coefficient
	void
	init2()
	{
//		x_2_ = one_half * FMU::get_derivative2( der.ics ); //Do Add 2nd derivative
	}

	// Initialize Event in Queue
	void
	init_event()
	{
		set_tE_aligned();
		event( events.add( tE, this ) );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
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
		Time const tDel( ( tQ = tE ) - tX );
		q_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
		set_qTol();
		if ( self_observer ) {
			x_0_ = q_0_;
			fmu_set_observees_q( tE );
		} else {
			q_1_ = x_1_ + ( two * x_2_ * tDel );
		}
		fmu_set_observers_observees_q();
		FMU::get_derivatives(); //API Eliminate need for this with FMI per-variable get_derivative call
		if ( self_observer ) {
			tX = tE;
			x_1_ = q_1_ = FMU::get_derivative( der.ics );
//			x_2_ = one_half * FMU::get_derivative2( der.ics ); //Do
		}
		set_tE_aligned();
		event( events.shift( tE, event() ) );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
		advance_observers();
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 0
	void
	advance0()
	{
		Time const tDel( ( tQ = tE ) - tX );
		x_0_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
		set_qTol();
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step FMU
	void
	advance_fmu()
	{
		fmu_set_observees_q( tE );
		fmu_set_observers_observees_q();
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 1
	void
	advance1()
	{
		tX = tE;
		x_1_ = q_1_ = FMU::get_derivative( der.ics );
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 2
	void
	advance2()
	{
//		x_2_ = one_half * FMU::get_derivative2( der.ics ); //Do
		set_tE_aligned();
		event( events.shift( tE, event() ) );
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
	}

	// Advance Observer to Time t
	void
	advance( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		if ( tX < t ) { // Could observe multiple variables with simultaneous triggering
			Time const tDel( t - tX );
			x_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
			x_1_ = FMU::get_derivative( der.ics );
//			x_2_ = one_half * FMU::get_derivative2( der.ics ); //Do
			tX = t;
			set_tE_unaligned();
			event( events.shift( tE, event() ) );
			if ( options::output::d ) std::cout << "  " << name << '(' << t << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
		}
	}

private: // Methods

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE_aligned()
	{
		assert( tX <= tQ );
		assert( dt_min <= dt_max );
		tE = ( x_2_ != 0.0 ? tQ + std::sqrt( qTol / std::abs( x_2_ ) ) : infinity );
		if ( dt_max != infinity ) tE = std::min( tE, tQ + dt_max );
		tE = std::max( tE, tQ + dt_min );
		if ( ( options::inflection ) && ( x_2_ != 0.0 ) && ( signum( x_1_ ) != signum( x_2_ ) ) ) {
			Time const tI( tX - ( x_1_ / ( two * x_2_ ) ) );
			if ( tQ < tI ) tE = std::min( tE, tI );
		}
	}

	// Set End Time: Quantized and Continuous Unaligned
	void
	set_tE_unaligned()
	{
		assert( tQ <= tX );
		assert( dt_min <= dt_max );
		Value const d0( x_0_ - ( q_0_ + ( q_1_ * ( tX - tQ ) ) ) );
		Value const d1( x_1_ - q_1_ );
		Time dtX;
		if ( ( d1 >= 0.0 ) && ( x_2_ >= 0.0 ) ) { // Upper boundary crossing
			dtX = min_root_quadratic_upper( x_2_, d1, d0 - qTol );
		} else if ( ( d1 <= 0.0 ) && ( x_2_ <= 0.0 ) ) { // Lower boundary crossing
			dtX = min_root_quadratic_lower( x_2_, d1, d0 + qTol );
		} else { // Both boundaries can have crossings
			dtX = min_root_quadratic_both( x_2_, d1, d0 + qTol, d0 - qTol );
		}
		tE = ( dtX == infinity ? infinity : tX + std::min( dtX, dt_max ) );
		if ( ( options::inflection ) && ( x_2_ != 0.0 ) && ( signum( x_1_ ) != signum( x_2_ ) ) && ( signum( x_1_ ) == signum( q_1_ ) ) ) {
			Time const tI( tX - ( x_1_ / ( two * x_2_ ) ) );
			if ( tX < tI ) tE = std::min( tE, tI );
		}
	}

private: // Data

	Value x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }; // Continuous rep coefficients
	Value q_0_{ 0.0 }, q_1_{ 0.0 }; // Quantized rep coefficients

};

#endif
