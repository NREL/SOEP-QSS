#ifndef QSS_VariableQSS2_hh_INCLUDED
#define QSS_VariableQSS2_hh_INCLUDED

// QSS2 Variable

// QSS Headers
#include <QSS/Variable.hh>
#include <QSS/globals.hh>

// QSS2 Variable
template< template< typename > typename F >
class VariableQSS2 final : public Variable
{

public: // Types

	using Time = Variable::Time;
	template< typename V > using Function = F< V >;
	using Derivative = Function< Variable >;
	using Coefficient = typename Derivative::Coefficient;

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

	// Order of QSS Method
	int
	order() const
	{
		return 2;
	}

	// Continuous Value at Time t
	double
	x( Time const t ) const
	{
		assert( ( tC <= t ) && ( t <= tE ) );
		Time const tDel( t - tC );
		return x0_ + ( ( x1_ + ( x2_ * tDel ) ) * tDel );
	}

	// Quantized Value at Time tQ
	double
	q() const
	{
		return q0_;
	}

	// Quantized First Derivative at Time tQ
	double
	q1() const
	{
		return q1_;
	}

	// Quantized Value at Time t
	double
	q( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		return q0_ + ( q1_ * ( t - tQ ) );
	}

	// Quantized First Derivative at Time t
	double
	q1( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		(void)t; // Suppress unused parameter warning
		return q1_;
	}

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE_aligned()
	{
		assert( tC <= tQ ); // Quantized and continuous trajectories align at tQ
		tE = ( x2_ != 0.0 ? tQ + std::sqrt( qTol / std::abs( x2_ ) ) : infinity );
	}

	// Set End Time: Quantized and Continuous Unaligned
	void
	set_tE_unaligned()
	{
		assert( tQ <= tC );
		double const d0( x0_ - ( q0_ + ( q1_ * ( tC - tQ ) ) ) );
		double const d1( x1_ - q1_ );
		if ( d1 >= 0.0 ) {
			Time const tPosQ( min_root_quadratic( x2_, d1, d0 - qTol ) );
			if ( x2_ >= 0.0 ) { // Only need to check +qTol
				tE = ( tPosQ == infinity ? infinity : tC + tPosQ );
			} else {
				Time const tNegQ( min_root_quadratic( x2_, d1, d0 + qTol ) );
				Time const tMinQ( std::min( tPosQ, tNegQ ) );
				tE = ( tMinQ == infinity ? infinity : tC + tMinQ );
			}
		} else { // d1 < 0
			Time const tNegQ( min_root_quadratic( x2_, d1, d0 + qTol ) );
			if ( x2_ <= 0.0 ) { // Only need to check -qTol
				tE = ( tNegQ == infinity ? infinity : tC + tNegQ );
			} else {
				Time const tPosQ( min_root_quadratic( x2_, d1, d0 - qTol ) );
				Time const tMinQ( std::min( tPosQ, tNegQ ) );
				tE = ( tMinQ == infinity ? infinity : tC + tMinQ );
			}
		}
	}

public: // Methods

	// Initialize Constant Term
	VariableQSS2 &
	init0( double const x )
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
		x2_ = one_half * d_.q1();
	}

	// Initialize Event in Queue
	void
	init_event()
	{
		set_tE_aligned();
		event( events.add( tE, this ) );
		if ( diag ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q0_ << "+" << q1_ << "*t quantized, " << x0_ << "+" << x1_ << "*t+" << x2_ << "*t^2 internal   tE=" << tE << '\n';
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
		Time const tDel( ( tQ = tE ) - tC );
		q0_ = x0_ + ( ( x1_ + ( x2_ * tDel ) ) * tDel );
		q1_ = d_.q( tE );
		set_qTol();
		if ( self_observer ) {
			x0_ = q0_;
			x1_ = q1_;
			x2_ = one_half * d_.q1( tC = tE );
		}
		set_tE_aligned();
		event( events.shift( tE, event() ) );
		if ( diag ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q0_ << "+" << q1_ << "*t quantized, " << x0_ << "+" << x1_ << "*t+" << x2_ << "*t^2 internal   tE=" << tE << '\n';
		for ( Variable * observer : observers() ) { // Advance (other) observers
			observer->advance( tQ );
		}
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 0
	void
	advance0()
	{
		Time const tDel( ( tQ = tE ) - tC );
		q0_ = x0_ + ( ( x1_ + ( x2_ * tDel ) ) * tDel );
		set_qTol();
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 1
	void
	advance1()
	{
		q1_ = d_.q( tE );
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 2
	void
	advance2()
	{
		if ( self_observer ) {
			x0_ = q0_;
			x1_ = q1_;
			x2_ = one_half * d_.q1( tC = tE );
		}
		set_tE_aligned();
		event( events.shift( tE, event() ) );
		if ( diag ) std::cout << "= " << name << '(' << tQ << ')' << " = " << q0_ << "+" << q1_ << "*t quantized, " << x0_ << "+" << x1_ << "*t+" << x2_ << "*t^2 internal   tE=" << tE << '\n';
	}

	// Advance Observer to Time t
	void
	advance( Time const t )
	{
		assert( ( tC <= t ) && ( t <= tE ) );
		if ( tC < t ) { // Could observe multiple variables with simultaneous triggering
			Time const tDel( t - tC );
			x0_ = x0_ + ( ( x1_ + ( x2_ * tDel ) ) * tDel );
			x1_ = d_.q( t );
			x2_ = one_half * d_.q1( tC = t );
			set_tE_unaligned();
			event( events.shift( tE, event() ) );
			if ( diag ) std::cout << "  " << name << '(' << t << ')' << " = " << q0_ << "+" << q1_ << "*t quantized, " << x0_ << "+" << x1_ << "*t+" << x2_ << "*t^2 internal   tE=" << tE << '\n';
		}
	}

private: // Data

	Derivative d_; // Derivative function
	double x0_{ 0.0 }, x1_{ 0.0 }, x2_{ 0.0 }; // Continuous value coefficients for active time segment
	double q0_{ 0.0 }, q1_{ 0.0 }; // Quantized value coefficients for active time segment

};

#endif
