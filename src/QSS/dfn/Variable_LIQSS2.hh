#ifndef QSS_dfn_Variable_LIQSS2_hh_INCLUDED
#define QSS_dfn_Variable_LIQSS2_hh_INCLUDED

// LIQSS2 Variable
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

// LIQSS2 Variable
template< template< typename > class F >
class Variable_LIQSS2 final : public Variable_QSS< F >
{

public: // Types

	using Super = Variable_QSS< F >;
	using Time = Variable::Time;
	using Value = Variable::Value;
	using AdvanceSpecs_LIQSS2 = typename Variable::AdvanceSpecs_LIQSS2;

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
	Variable_LIQSS2(
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
		return 2;
	}

	// Continuous Value at Time t
	Value
	x( Time const t ) const
	{
		Time const tDel( t - tX );
		return x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// Continuous First Derivative at Time t
	Value
	x1( Time const t ) const
	{
		return x_1_ + ( two * x_2_ * ( t - tX ) );
	}

	// Continuous Second Derivative at Time t
	Value
	x2( Time const t ) const
	{
		return two * x_2_;
	}

	// Quantized Value at Time t
	Value
	q( Time const t ) const
	{
		return q_0_ + ( q_1_ * ( t - tQ ) );
	}

	// Quantized First Derivative at Time t
	Value
	q1( Time const ) const
	{
		return q_1_;
	}

public: // Methods

	// Initialization to a Value
	void
	init( Value const x )
	{
		init_0( x );
		init_1();
		init_LIQSS_2();
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

	// Initialization: Stage 1
	void
	init_1()
	{
		self_observer = d_.finalize( this );
		shrink_observers(); // Optional
		// For self-observer this is a first pass to set a reasonable x_1_ = q_1_ for init_2 calls
		// This avoids variable processing order dependency but not inconsistent rep usage
		x_1_ = q_1_ = d_.x( tQ ); // Continuous rep used to reduce impact of cyclic dependency
	}

	// Initialization: LIQSS Stage 2
	void
	init_LIQSS_2()
	{ // Call before init_2 since it alters q_0_
		if ( self_observer ) {
			advance_x( tQ ); // Continuous rep used to reduce impact of cyclic dependency
		} else {
			x_2_ = one_half * d_.x1( tQ ); // Continuous rep used to reduce impact of cyclic dependency
			q_0_ += signum( x_2_ ) * qTol;
		}
		set_tE_aligned();
		event( events.add_QSS( tE, this ) );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
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
		Time const tDel( ( tQ = tE ) - tX );
		x_0_ = q_c_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
		set_qTol();
		if ( self_observer ) {
			advance_q( tX = tE );
		} else {
			x_1_ = q_1_ = d_.q( tE );
			x_2_ = one_half * d_.q1( tX = tE );
			q_0_ += signum( x_2_ ) * qTol;
		}
		set_tE_aligned();
		event( events.shift_QSS( tE, event() ) );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
		advance_observers();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0()
	{
		Time const tDel( ( tQ = tE ) - tX );
		x_0_ = q_c_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
		tX = tE;
		set_qTol();
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1()
	{
		x_1_ = q_1_ = d_.x( tE ); // Continuous rep used to reduce impact of cyclic dependency
	}

	// LIQSS Advance: Stage 2
	void
	advance_LIQSS_2()
	{ // Call before advance_QSS_2 since it alters q_0_
		if ( self_observer ) {
			advance_x( tE ); // Continuous rep used to reduce impact of cyclic dependency
		} else {
			x_2_ = one_half * d_.x1( tE ); // Continuous rep used to reduce impact of cyclic dependency
			q_0_ += signum( x_2_ ) * qTol;
		}
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2()
	{
		set_tE_aligned();
		event( events.shift_QSS( tE, event() ) );
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
	}

	// Observer Advance
	void
	advance_observer( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		x_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
		x_1_ = d_.qs( t );
		x_2_ = one_half * d_.qf1( tX = t );
		set_tE_unaligned();
		event( events.shift_QSS( tE, event() ) );
		if ( options::output::d ) std::cout << "  " << name << '(' << t << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
	}

	// Handler Advance
	void
	advance_handler( Time const t, Value const x )
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		x_0_ = q_c_ = q_0_ = x;
		set_qTol();
		x_1_ = q_1_ = d_.qs( tX = tQ = t );
		x_2_ = one_half * d_.qf1( t );
		set_tE_aligned();
		event( events.shift_QSS( tE, event() ) );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
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
		x_1_ = q_1_ = d_.qs( tQ );
	}

	// Handler Advance: Stage 2
	void
	advance_handler_2()
	{
		x_2_ = one_half * d_.qf1( tQ );
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
		Value const d0( x_0_ - ( q_c_ + ( q_1_ * ( tX - tQ ) ) ) );
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

	// Advance Self-Observing Trigger using Quantized Derivative
	void
	advance_LIQSS( AdvanceSpecs_LIQSS2 const & specs )
	{
		assert( qTol > 0.0 );
		assert( self_observer );

		// Set coefficients based on second derivative signs
		int const dls( signum( specs.l2 ) );
		int const dus( signum( specs.u2 ) );
		if ( ( dls == -1 ) && ( dus == -1 ) ) { // Downward curving trajectory
			q_0_ -= qTol;
			x_1_ = q_1_ = specs.l1;
			x_2_ = one_half * specs.l2;
		} else if ( ( dls == +1 ) && ( dus == +1 ) ) { // Upward curving trajectory
			q_0_ += qTol;
			x_1_ = q_1_ = specs.u1;
			x_2_ = one_half * specs.u2;
		} else { // Straight trajectory
			q_0_ = std::min( std::max( specs.z2, q_0_ - qTol ), q_0_ + qTol ); // Clipped in case of roundoff
			x_1_ = q_1_ = specs.z1;
			x_2_ = 0.0;
		}
	}

	// Advance Self-Observing Trigger using Quantized Derivative
	void
	advance_q( Time const t )
	{
		advance_LIQSS( d_.qlu2( t, qTol ) ); // Quantized rep used for single trigger
	}

	// Advance Self-Observing Trigger using Continuous Derivative
	void
	advance_x( Time const t )
	{
		advance_LIQSS( d_.xlu2( t, qTol ) ); // Continuous rep used for simultaneous triggers
	}

private: // Data

	Value x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }; // Continuous rep coefficients
	Value q_c_{ 0.0 }, q_0_{ 0.0 }, q_1_{ 0.0 }; // Quantized rep coefficients

};

} // dfn
} // QSS

#endif
