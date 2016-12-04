#ifndef QSS_VariableQSS1_hh_INCLUDED
#define QSS_VariableQSS1_hh_INCLUDED

// QSS1 Variable

// QSS Headers
#include <QSS/Variable.hh>
#include <QSS/globals.hh>

// QSS1 Variable
template< template< typename > typename F >
class VariableQSS1 final : public Variable
{

public: // Types

	using Time = Variable::Time;
	template< typename V > using Function = F< V >;
	using Derivative = Function< Variable >;
	using Coefficient = typename Derivative::Coefficient;

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
		return 1;
	}

	// Continuous Value at Time t
	double
	x( Time const t ) const
	{
		assert( ( tC <= t ) && ( t <= tE ) );
		return x0_ + ( x1_ * ( t - tC ) );
	}

	// Quantized Value at Time tQ
	double
	q() const
	{
		return q0_;
	}

	// Quantized Value at Time t
	double
	q( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		(void)t; // Suppress unused parameter warning
		return q0_;
	}

	// Next End Time: Quantized and Continuous Aligned
	Time
	tEndAligned() const
	{
		assert( tQ == tC ); // Quantized and continuous reps should be rooted at same time
		return ( x1_ != 0.0 ? tQ + ( qTol / std::abs( x1_ ) ) : infinity );
	}

	// Next End Time: Quantized and Continuous Unaligned
	Time
	tEndUnaligned() const
	{
		return
		 ( x1_ > 0.0 ? tC + ( ( ( q0_ - x0_ ) + qTol ) / x1_ ) :
		 ( x1_ < 0.0 ? tC + ( ( ( q0_ - x0_ ) - qTol ) / x1_ ) :
		 infinity ) );
	}

public: // Methods

	// Initialize Constant Term
	VariableQSS1 &
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
		x1_ = d_.q();
	}

	// Initialize Event in Queue
	void
	init_event()
	{
		tE = tEndAligned();
		event( events.add( tE, this ) );
		if ( diag ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q0_ << " quantized, " << x0_ << "+" << x1_ << "*t internal   tE=" << tE << '\n';
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
		q0_ = x0_ + ( x1_ * ( ( tQ = tE ) - tC ) );
		set_qTol();
		if ( self_observer ) {
			x0_ = q0_;
			x1_ = d_.q( tC = tE );
			tE = tEndAligned();
		} else {
			tE = tEndUnaligned();
		}
		event( events.shift( tE, event() ) );
		if ( diag ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q0_ << " quantized, " << x0_ << "+" << x1_ << "*t internal   tE=" << tE << '\n';
		for ( Variable * observer : observers() ) { // Advance (other) observers
			observer->advance( tQ );
		}
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 0
	void
	advance0()
	{
		q0_ = x0_ + ( x1_ * ( ( tQ = tE ) - tC ) );
		set_qTol();
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 1
	void
	advance1()
	{
		if ( self_observer ) {
			x0_ = q0_;
			x1_ = d_.q( tC = tE );
			tE = tEndAligned();
		} else {
			tE = tEndUnaligned();
		}
		event( events.shift( tE, event() ) );
		if ( diag ) std::cout << "= " << name << '(' << tQ << ')' << " = " << q0_ << " quantized, " << x0_ << "+" << x1_ << "*t internal   tE=" << tE << '\n';
	}

	// Advance Observer to Time t
	void
	advance( Time const t )
	{
		assert( ( tC <= t ) && ( t <= tE ) );
		if ( tC < t ) { // Could observe multiple variables with simultaneous triggering
			x0_ = x0_ + ( x1_ * ( t - tC ) );
			x1_ = d_.q( tC = t );
			tE = tEndUnaligned();
			event( events.shift( tE, event() ) );
			if ( diag ) std::cout << "  " << name << '(' << t << ')' << " = " << q0_ << " quantized, " << x0_ << "+" << x1_ << "*t internal   tE=" << tE << '\n';
		}
	}

private: // Data

	Derivative d_; // Derivative function
	double x0_{ 0.0 }, x1_{ 0.0 }; // Continuous value coefficients for active time segment
	double q0_{ 0.0 }; // Quantized value for active time segment

};

#endif
