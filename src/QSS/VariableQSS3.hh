#ifndef QSS_VariableQSS3_hh_INCLUDED
#define QSS_VariableQSS3_hh_INCLUDED

// QSS3 Variable

// QSS Headers
#include <QSS/Variable.hh>
#include <QSS/globals.hh>
#include <QSS/math.hh>

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

	// Order of QSS Method
	int
	order() const
	{
		return 3;
	}

	// Continuous Value at Time tX
	Value
	x() const
	{
		return x0_;
	}

	// Continuous Value at Time tX
	Value
	x0() const
	{
		return x0_;
	}

	// Continuous First Derivative at Time tX
	Value
	x1() const
	{
		return x1_;
	}

	// Continuous Value at Time t
	Value
	x( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		return x0_ + ( ( x1_ + ( x2_ + ( x3_ * tDel ) ) * tDel ) * tDel );
	}

	// Continuous First Derivative at Time t
	Value
	x1( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		return x1_ + ( ( ( two * x2_ ) + ( three * x3_ * tDel ) ) * tDel );
	}

	// Quantized Value at Time tQ
	Value
	q() const
	{
		return q0_;
	}

	// Quantized Value at Time tQ
	Value
	q0() const
	{
		return q0_;
	}

	// Quantized First Derivative at Time tQ
	Value
	q1() const
	{
		return q1_;
	}

	// Quantized Second Derivative at Time tQ
	Value
	q2() const
	{
		return two * q2_;
	}

	// Quantized Value at Time t
	Value
	q( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		Time const tDel( t - tQ );
		return q0_ + ( ( q1_ + ( q2_ * tDel ) ) * tDel );
	}

	// Quantized First Derivative at Time t
	Value
	q1( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		return q1_ + ( two * q2_ * ( t - tQ ) );
	}

	// Quantized Second Derivative at Time t
	Value
	q2( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		(void)t; // Suppress unused parameter warning
		return two * q2_;
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

	// Initialize Constant Term
	VariableQSS3 &
	init0( Value const x )
	{
		x0_ = q0_ = x;
		set_qTol();
		return *this;
	}

	// Initialize Linear Coefficient
	void
	init1()
	{
		self_observer = d_.finalize( this );
		shrink_observers(); // Optional
		x1_ = q1_ = d_.q();
	}

	// Initialize Quadratic Coefficient
	void
	init2()
	{
		x2_ = q2_ = one_half * d_.q1();
	}

	// Initialize Cubic Coefficient
	void
	init3()
	{
		x3_ = one_sixth * d_.q2();
	}

	// Initialize Event in Queue
	void
	init_event()
	{
		set_tE_aligned();
		event( events.add( tE, this ) );
		if ( diag ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q0_ << "+" << q1_ << "*t+" << q2_ << "*t^2 quantized, " << x0_ << "+" << x1_ << "*t+" << x2_ << "*t^2+" << x3_ << "*t^3 internal   tE=" << tE << '\n';
	}

	// Set Current Tolerance
	void
	set_qTol()
	{
		qTol = std::max( aTol, rTol * std::abs( q0_ ) );
		assert( qTol > 0.0 );
	}

	// Advance Trigger to Time tE and Requantize
	void
	advance()
	{
		Time const tDel( ( tQ = tE ) - tX );
		q0_ = x0_ + ( ( x1_ + ( x2_ + ( x3_ * tDel ) ) * tDel ) * tDel );
		set_qTol();
		if ( self_observer ) {
			x0_ = q0_;
			x1_ = q1_ = d_.q( tE );
			x2_ = q2_ = one_half * d_.q1( tE );
			x3_ = one_sixth * d_.q2( tX = tE );
		} else {
			q1_ = x1_ + ( ( ( two * x2_ ) + ( three * x3_ * tDel ) ) * tDel );
			q2_ = x2_ + ( three * x3_ * tDel );
		}
		set_tE_aligned();
		event( events.shift( tE, event() ) );
		if ( diag ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q0_ << "+" << q1_ << "*t+" << q2_ << "*t^2 quantized, " << x0_ << "+" << x1_ << "*t+" << x2_ << "*t^2+" << x3_ << "*t^3 internal   tE=" << tE << '\n';
		for ( Variable * observer : observers() ) { // Advance (other) observers
			observer->advance( tQ );
		}
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 0
	void
	advance0()
	{
		Time const tDel( ( tQ = tE ) - tX );
		x0_ = q0_ = x0_ + ( ( x1_ + ( x2_ + ( x3_ * tDel ) ) * tDel ) * tDel );
		set_qTol();
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 1
	void
	advance1()
	{
		x1_ = q1_ = d_.q( tE );
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 2
	void
	advance2()
	{
		x2_ = q2_ = one_half * d_.q1( tE );
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 3
	void
	advance3()
	{
		x3_ = one_sixth * d_.q2( tX = tE );
		set_tE_aligned();
		event( events.shift( tE, event() ) );
		if ( diag ) std::cout << "= " << name << '(' << tQ << ')' << " = " << q0_ << "+" << q1_ << "*t+" << q2_ << "*t^2 quantized, " << x0_ << "+" << x1_ << "*t+" << x2_ << "*t^2+" << x3_ << "*t^3 internal   tE=" << tE << '\n';
	}

	// Advance Observer to Time t
	void
	advance( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		if ( tX < t ) { // Could observe multiple variables with simultaneous triggering
			Time const tDel( t - tX );
			x0_ = x0_ + ( ( x1_ + ( x2_ + ( x3_ * tDel ) ) * tDel ) * tDel );
			x1_ = d_.q( t );
			x2_ = one_half * d_.q1( t );
			x3_ = one_sixth * d_.q2( tX = t );
			set_tE_unaligned();
			event( events.shift( tE, event() ) );
			if ( diag ) std::cout << "  " << name << '(' << t << ')' << " = " << q0_ << "+" << q1_ << "*t+" << q2_ << "*t^2 quantized, " << x0_ << "+" << x1_ << "*t+" << x2_ << "*t^2+" << x3_ << "*t^3 internal   tE=" << tE << '\n';
		}
	}

private: // Methods

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE_aligned()
	{
		assert( tX <= tQ );
		tE = ( x3_ != 0.0 ? tQ + std::cbrt( qTol / std::abs( x3_ ) ) : infinity );
		if ( ( inflection_steps ) && ( x3_ != 0.0 ) && ( signum( x2_ ) != signum( x3_ ) ) ) {
			tE = std::min( tE, tQ - ( x2_ / ( three * x3_ ) ) );
		}
	}

	// Set End Time: Quantized and Continuous Unaligned
	void
	set_tE_unaligned()
	{
		assert( tQ <= tX );
		Time const tXQ( tX - tQ );
		Value const d0( x0_ - ( q0_ + ( q1_ + ( q2_ * tXQ ) ) * tXQ ) );
		Value const d1( x1_ - ( q1_ + ( two * q2_ * tXQ ) ) );
		Value const d2( x2_ - q2_ );
		if ( ( x3_ >= 0.0 ) && ( d2 >= 0.0 ) && ( d1 >= 0.0 ) ) { // Only need to check +qTol
			Time const tPosQ( min_root_cubic( x3_, d2, d1, d0 - qTol ) );
			tE = ( tPosQ == infinity ? infinity : tX + tPosQ );
		} else if ( ( x3_ <= 0.0 ) && ( d2 <= 0.0 ) && ( d1 <= 0.0 ) ) { // Only need to check -qTol
			Time const tNegQ( min_root_cubic( x3_, d2, d1, d0 + qTol ) );
			tE = ( tNegQ == infinity ? infinity : tX + tNegQ );
		} else { // Check +qTol and -qTol
			Time const tPosQ( min_root_cubic( x3_, d2, d1, d0 - qTol ) );
			Time const tNegQ( min_root_cubic( x3_, d2, d1, d0 + qTol ) );
			Time const tMinQ( std::min( tPosQ, tNegQ ) );
			tE = ( tMinQ == infinity ? infinity : tX + tMinQ );
		}
		if ( ( inflection_steps ) && ( x3_ != 0.0 ) && ( signum( x2_ ) != signum( x3_ ) ) && ( signum( x2_ ) == signum( q2_ ) ) ) {
			tE = std::min( tE, tQ - ( x2_ / ( three * x3_ ) ) );
		}
	}

private: // Data

	Value x0_{ 0.0 }, x1_{ 0.0 }, x2_{ 0.0 }, x3_{ 0.0 }; // Continuous value coefficients for active time segment
	Value q0_{ 0.0 }, q1_{ 0.0 }, q2_{ 0.0 }; // Quantized value coefficients for active time segment
	Derivative d_; // Derivative function

};

#endif
