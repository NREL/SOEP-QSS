#ifndef QSS_Variable_LIQSS1_hh_INCLUDED
#define QSS_Variable_LIQSS1_hh_INCLUDED

// LIQSS1 Variable
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/Variable_QSS.hh>

// LIQSS1 Variable
template< template< typename > class F >
class Variable_LIQSS1 final : public Variable_QSS< F >
{

public: // Types

	using Super = Variable_QSS< F >;
	using Time = Variable::Time;
	using Value = Variable::Value;
	using AdvanceSpecs_LIQSS1 = typename Variable::AdvanceSpecs_LIQSS1;

	using Super::name;
	using Super::rTol;
	using Super::aTol;
	using Super::qTol;
	using Super::xIni;
	using Super::tQ;
	using Super::tX;
	using Super::tE;
	using Super::dt_min;
	using Super::dt_max;
	using Super::self_observer;

	using Super::event;
	using Super::advance_observers;
	using Super::shrink_observers;

private: // Types

	using Super::observers_;
	using Super::event_;
	using Super::d_;

public: // Creation

	// Constructor
	explicit
	Variable_LIQSS1(
	 std::string const & name,
	 Value const rTol = 1.0e-4,
	 Value const aTol = 1.0e-6,
	 Value const xIni = 0.0
	) :
	 Variable_QSS< F >( name, rTol, aTol, xIni ),
	 x_0_( xIni ),
	 q_c_( xIni ),
	 q_0_( xIni )
	{
		set_qTol();
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

	// Initialize QSS Variable
	void
	init( Value const x )
	{
		init0( x );
		init1_LIQSS();
		init_event();
	}

	// Initialize Constant Term to Given Value
	void
	init0( Value const x )
	{
		x_0_ = q_c_ = q_0_ = x;
		set_qTol();
	}

	// Initialize Linear Coefficient in LIQSS Variable
	void
	init1_LIQSS()
	{ // Call before init1 since it alters q_0_
		self_observer = d_.finalize( this );
		shrink_observers(); // Optional
		if ( self_observer ) {
			advance_x(); // Continuous rep used to avoid cyclic dependency
		} else {
			x_1_ = d_.x( tQ ); // Continuous rep used to avoid cyclic dependency
			q_0_ += signum( x_1_ ) * qTol;
		}
	}

	// Initialize Event in Queue
	void
	init_event()
	{
		set_tE_aligned();
		event( events.add( tE, this ) );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
	}

	// Set Current Tolerance
	void
	set_qTol()
	{
		qTol = std::max( rTol * std::abs( q_c_ ), aTol );
		assert( qTol > 0.0 );
	}

	// Advance Trigger to Time tE and Requantize
	void
	advance()
	{
		q_c_ = q_0_ = x_0_ + ( x_1_ * ( ( tQ = tE ) - tX ) );
		set_qTol();
		if ( self_observer ) {
			x_0_ = q_c_;
			advance_q();
			tX = tE;
		} else {
			q_0_ += signum( x_1_ ) * qTol;
		}
		set_tE_aligned();
		event( events.shift( tE, event() ) );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
		advance_observers();
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 0
	void
	advance0()
	{
		x_0_ = q_c_ = q_0_ = x_0_ + ( x_1_ * ( ( tQ = tE ) - tX ) );
		set_qTol();
		tX = tE;
	}

	// Advance Simultaneous Trigger in LIQSS Variable to Time tE and Requantize: Step 1
	void
	advance1_LIQSS()
	{ // Call before advance1 since it alters q_0_
		if ( self_observer ) {
			advance_x(); // Continuous rep used to avoid cyclic dependency
		} else {
			x_1_ = d_.x( tE ); // Continuous rep used to avoid cyclic dependency
			q_0_ += signum( x_1_ ) * qTol;
		}
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 1
	void
	advance1()
	{
		set_tE_aligned();
		event( events.shift( tE, event() ) );
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
	}

	// Advance Observer to Time t
	void
	advance( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		if ( tX < t ) { // Could observe multiple variables with simultaneous triggering
			x_0_ = x_0_ + ( x_1_ * ( t - tX ) );
			x_1_ = d_.q( tX = t );
			set_tE_unaligned();
			event( events.shift( tE, event() ) );
			if ( options::output::d ) std::cout << "  " << name << '(' << t << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
		}
	}

private: // Methods

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE_aligned()
	{
		assert( tX <= tQ );
		assert( dt_min <= dt_max );
		tE = ( x_1_ != 0.0 ? tQ + ( qTol / std::abs( x_1_ ) ) : infinity );
		if ( dt_max != infinity ) tE = std::min( tE, tQ + dt_max );
		tE = std::max( tE, tQ + dt_min );
	}

	// Set End Time: Quantized and Continuous Unaligned
	void
	set_tE_unaligned()
	{
		assert( tQ <= tX );
		assert( dt_min <= dt_max );
		tE =
		 ( x_1_ > 0.0 ? tX + ( ( q_c_ + qTol - x_0_ ) / x_1_ ) :
		 ( x_1_ < 0.0 ? tX + ( ( q_c_ - qTol - x_0_ ) / x_1_ ) :
		 infinity ) );
		if ( dt_max != infinity ) tE = std::min( tE, tX + dt_max );
		tE = std::max( tE, tX ); // Numeric bulletproofing
	}

	// Advance Self-Observing Trigger using Quantized Derivative
	void
	advance_LIQSS( AdvanceSpecs_LIQSS1 const & specs ) //Do Performance test pass-by-value
	{
		assert( qTol > 0.0 );
		assert( self_observer );

		// Set coefficients based on derivative signs
		int const dls( signum( specs.l ) );
		int const dus( signum( specs.u ) );
		if ( ( dls == -1 ) && ( dus == -1 ) ) { // Downward trajectory
			q_0_ -= qTol;
			x_1_ = specs.l;
		} else if ( ( dls == +1 ) && ( dus == +1 ) ) { // Upward trajectory
			q_0_ += qTol;
			x_1_ = specs.u;
		} else { // Flat trajectory
			q_0_ = std::min( std::max( specs.z, q_0_ - qTol ), q_0_ + qTol ); // Clipped in case of roundoff
			x_1_ = 0.0;
		}
	}

	// Advance Self-Observing Trigger using Quantized Derivative
	void
	advance_q()
	{
		assert( ( tQ == tE ) || ( tQ == 0.0 ) ); // Precondition: tQ set to trigger time
		advance_LIQSS( d_.qlu( tQ, qTol ) ); // Quantized rep used for single trigger
	}

	// Advance Self-Observing Trigger using Continuous Derivative
	void
	advance_x()
	{
		assert( ( tQ == tE ) || ( tQ == 0.0 ) ); // Precondition: tQ set to trigger time
		advance_LIQSS( d_.xlu( tQ, qTol ) ); // Continuous rep used for simultaneous triggers
	}

private: // Data

	Value x_0_{ 0.0 }, x_1_{ 0.0 }; // Continuous rep coefficients
	Value q_c_{ 0.0 }, q_0_{ 0.0 }; // Quantized rep coefficients

};

#endif
