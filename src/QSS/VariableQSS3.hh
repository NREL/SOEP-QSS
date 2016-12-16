#ifndef QSS_VariableQSS3_hh_INCLUDED
#define QSS_VariableQSS3_hh_INCLUDED

// QSS3 Variable

// QSS Headers
#include <QSS/Variable.hh>
#include <QSS/globals.hh>

// QSS3 Variable
template< template< typename > typename F >
class VariableQSS3 final : public Variable
{

public: // Types

	using Value = Variable::Value;
	using Time = Variable::Time;
	template< typename V > using Function = F< V >;
	using Derivative = Function< Variable >;
	using Coefficient = typename Derivative::Coefficient;

public: // Creation

	// Constructor
	explicit
	VariableQSS3(
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

	// Initialize QSS Variable
	void
	init( Value const x )
	{
		init0( x );
		init1();
		init2();
		init3();
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
		self_observer = d_.finalize( this );
		shrink_observers(); // Optional
		x_1_ = q_1_ = d_.q( tQ );
	}

	// Initialize Quadratic Coefficient
	void
	init2()
	{
		x_2_ = q_2_ = one_half * d_.q1( tQ );
	}

	// Initialize Cubic Coefficient
	void
	init3()
	{
		x_3_ = one_sixth * d_.q2( tQ );
	}

	// Initialize Event in Queue
	void
	init_event()
	{
		set_tE_aligned();
		event( events.add( tE, this ) );
		if ( diag ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t+" << q_2_ << "*t^2 quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2+" << x_3_ << "*t^3 internal   tE=" << tE << '\n';
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
		Time const tDel( ( tQ = tE ) - tX );
		q_0_ = x_0_ + ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
		set_qTol();
		if ( self_observer ) {
			x_0_ = q_0_;
			x_1_ = q_1_ = d_.qs( tE );
			x_2_ = q_2_ = one_half * d_.qc1( tE );
			x_3_ = one_sixth * d_.qc2( tX = tE );
		} else {
			q_1_ = x_1_ + ( ( ( two * x_2_ ) + ( three * x_3_ * tDel ) ) * tDel );
			q_2_ = x_2_ + ( three * x_3_ * tDel );
		}
		set_tE_aligned();
		event( events.shift( tE, event() ) );
		if ( diag ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t+" << q_2_ << "*t^2 quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2+" << x_3_ << "*t^3 internal   tE=" << tE << '\n';
		advance_observers();
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 0
	void
	advance0()
	{
		Time const tDel( ( tQ = tE ) - tX );
		x_0_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
		set_qTol();
		tX = tE;
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 1
	void
	advance1()
	{
		x_1_ = q_1_ = d_.qs( tE );
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 2
	void
	advance2()
	{
		x_2_ = q_2_ = one_half * d_.qc1( tE );
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 3
	void
	advance3()
	{
		x_3_ = one_sixth * d_.qc2( tE );
		set_tE_aligned();
		event( events.shift( tE, event() ) );
		if ( diag ) std::cout << "= " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t+" << q_2_ << "*t^2 quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2+" << x_3_ << "*t^3 internal   tE=" << tE << '\n';
	}

	// Advance Observer to Time t
	void
	advance( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		if ( tX < t ) { // Could observe multiple variables with simultaneous triggering
			Time const tDel( t - tX );
			x_0_ = x_0_ + ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
			x_1_ = d_.qs( t );
			x_2_ = one_half * d_.qc1( t );
			x_3_ = one_sixth * d_.qc2( tX = t );
			set_tE_unaligned();
			event( events.shift( tE, event() ) );
			if ( diag ) std::cout << "  " << name << '(' << t << ')' << " = " << q_0_ << "+" << q_1_ << "*t+" << q_2_ << "*t^2 quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2+" << x_3_ << "*t^3 internal   tE=" << tE << '\n';
		}
	}

private: // Methods

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE_aligned()
	{
		assert( tX <= tQ );
		tE = ( x_3_ != 0.0 ? tQ + std::cbrt( qTol / std::abs( x_3_ ) ) : infinity );
		if ( dt_max != infinity ) tE = std::min( tE, tQ + dt_max );
		if ( ( inflection_steps ) && ( x_3_ != 0.0 ) && ( signum( x_2_ ) != signum( x_3_ ) ) ) {
			Time const tI( tX - ( x_2_ / ( three * x_3_ ) ) );
			if ( tQ < tI ) tE = std::min( tE, tI );
		}
	}

	// Set End Time: Quantized and Continuous Unaligned
	void
	set_tE_unaligned()
	{
		assert( tQ <= tX );
		Time const tXQ( tX - tQ );
		Value const d0( x_0_ - ( q_0_ + ( q_1_ + ( q_2_ * tXQ ) ) * tXQ ) );
		Value const d1( x_1_ - ( q_1_ + ( two * q_2_ * tXQ ) ) );
		Value const d2( x_2_ - q_2_ );
		if ( ( x_3_ >= 0.0 ) && ( d2 >= 0.0 ) && ( d1 >= 0.0 ) ) { // Only need to check +qTol
			Time const tPosQ( min_root_cubic( x_3_, d2, d1, d0 - qTol ) );
			tE = ( tPosQ == infinity ? infinity : tX + tPosQ );
		} else if ( ( x_3_ <= 0.0 ) && ( d2 <= 0.0 ) && ( d1 <= 0.0 ) ) { // Only need to check -qTol
			Time const tNegQ( min_root_cubic( x_3_, d2, d1, d0 + qTol ) );
			tE = ( tNegQ == infinity ? infinity : tX + tNegQ );
		} else { // Check +qTol and -qTol
			Time const tPosQ( min_root_cubic( x_3_, d2, d1, d0 - qTol ) );
			Time const tNegQ( min_root_cubic( x_3_, d2, d1, d0 + qTol ) );
			Time const tMinQ( std::min( tPosQ, tNegQ ) );
			tE = ( tMinQ == infinity ? infinity : tX + tMinQ );
		}
		if ( dt_max != infinity ) tE = std::min( tE, tX + dt_max );
		if ( ( inflection_steps ) && ( x_3_ != 0.0 ) && ( signum( x_2_ ) != signum( x_3_ ) ) && ( signum( x_2_ ) == signum( q_2_ ) ) ) {
			Time const tI( tX - ( x_2_ / ( three * x_3_ ) ) );
			if ( tX < tI ) tE = std::min( tE, tI );
		}
	}

private: // Data

	Value x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }, x_3_{ 0.0 }; // Continuous rep coefficients
	Value q_0_{ 0.0 }, q_1_{ 0.0 }, q_2_{ 0.0 }; // Quantized rep coefficients
	Derivative d_; // Derivative function

};

#endif
