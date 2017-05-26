#ifndef QSS_dfn_Variable_LIQSS1_hh_INCLUDED
#define QSS_dfn_Variable_LIQSS1_hh_INCLUDED

// LIQSS1 Variable
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/dfn/Variable_QSS.hh>

namespace QSS {
namespace dfn {

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

	using Super::advance_observers;
	using Super::event;
	using Super::shrink_observers;

private: // Types

	using Super::d_;
	using Super::event_;
	using Super::observers_;

public: // Creation

	// Constructor
	explicit
	Variable_LIQSS1(
	 std::string const & name,
	 Value const rTol = 1.0e-4,
	 Value const aTol = 1.0e-6,
	 Value const xIni = 0.0
	) :
	 Super( name, rTol, aTol, xIni ),
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
		return x_0_ + ( x_1_ * ( t - tX ) );
	}

	// Continuous First Derivative at Time t
	Value
	x1( Time const t ) const
	{
		return x_1_;
	}

	// Quantized Value at Time t
	Value
	q( Time const ) const
	{
		return q_0_;
	}

public: // Methods

	// Initialization to a Value
	void
	init( Value const x )
	{
		init_0( x );
		init_LIQSS_1();
	}

	// Initialization to a Value: Stage 0
	void
	init_0( Value const x )
	{
		x_0_ = q_c_ = q_0_ = x;
		set_qTol();
	}

	// Initialization: Stage 0
	void
	init_0()
	{
		x_0_ = q_c_ = q_0_ = xIni;
		set_qTol();
	}

	// Initialization: LIQSS Stage 1
	void
	init_LIQSS_1()
	{ // Call before init_1 since it alters q_0_
		self_observer = d_.finalize( this );
		shrink_observers(); // Optional
		if ( self_observer ) {
			advance_x( tQ ); // Continuous rep used to avoid cyclic dependency
		} else {
			x_1_ = d_.x( tQ ); // Continuous rep used to avoid cyclic dependency
			q_0_ += signum( x_1_ ) * qTol;
		}
		set_tE_aligned();
		event( events.add_QSS( tE, this ) );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
	}

	// Set Current Tolerance
	void
	set_qTol()
	{
		qTol = std::max( rTol * std::abs( q_c_ ), aTol );
		assert( qTol > 0.0 );
	}

	// QSS Advance
	void
	advance_QSS()
	{
		x_0_ = q_c_ = q_0_ = x_0_ + ( x_1_ * ( ( tQ = tE ) - tX ) );
		set_qTol();
		if ( self_observer ) {
			advance_q( tX = tE );
		} else {
			x_1_ = d_.q( tX = tE );
			q_0_ += signum( x_1_ ) * qTol;
		}
		set_tE_aligned();
		event( events.shift_QSS( tE, event() ) );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
		advance_observers();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0()
	{
		x_0_ = q_c_ = q_0_ = x_0_ + ( x_1_ * ( ( tQ = tE ) - tX ) );
		tX = tE;
		set_qTol();
	}

	// LIQSS Trigger Advance: Stage 1
	void
	advance_LIQSS_1()
	{ // Call before advance_QSS_1 since it alters q_0_
		if ( self_observer ) {
			advance_x( tE ); // Continuous rep used to avoid cyclic dependency
		} else {
			x_1_ = d_.x( tE ); // Continuous rep used to avoid cyclic dependency
			q_0_ += signum( x_1_ ) * qTol;
		}
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1()
	{
		set_tE_aligned();
		event( events.shift_QSS( tE, event() ) );
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
	}

	// Observer Advance
	void
	advance_observer( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		x_0_ = x_0_ + ( x_1_ * ( t - tX ) );
		x_1_ = d_.q( tX = t );
		set_tE_unaligned();
		event( events.shift_QSS( tE, event() ) );
		if ( options::output::d ) std::cout << "  " << name << '(' << t << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
	}

	// Handler Advance
	void
	advance_handler( Time const t, Value const x )
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		x_0_ = q_c_ = q_0_ = x;
		set_qTol();
		x_1_ = d_.q( tX = tQ = t );
		set_tE_aligned();
		event( events.shift_QSS( tE, event() ) );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
		advance_observers();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t, Value const x )
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_c_ = q_0_ = x;
		set_qTol();
	}

	// Handler Advance: Stage 1
	void
	advance_handler_1()
	{
		x_1_ = d_.q( tQ );
		set_tE_aligned();
		event( events.shift_QSS( tE, event() ) );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
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
	advance_LIQSS( AdvanceSpecs_LIQSS1 const & specs )
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
	advance_q( Time const t )
	{
		advance_LIQSS( d_.qlu1( t, qTol ) ); // Quantized rep used for single trigger
	}

	// Advance Self-Observing Trigger using Continuous Derivative
	void
	advance_x( Time const t )
	{
		advance_LIQSS( d_.xlu1( t, qTol ) ); // Continuous rep used for simultaneous triggers
	}

private: // Data

	Value x_0_{ 0.0 }, x_1_{ 0.0 }; // Continuous rep coefficients
	Value q_c_{ 0.0 }, q_0_{ 0.0 }; // Quantized rep coefficients

};

} // dfn
} // QSS

#endif
