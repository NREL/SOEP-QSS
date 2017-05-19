#ifndef QSS_dfn_Variable_QSS3_hh_INCLUDED
#define QSS_dfn_Variable_QSS3_hh_INCLUDED

// QSS3 Variable
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

// QSS3 Variable
template< template< typename > class F >
class Variable_QSS3 final : public Variable_QSS< F >
{

public: // Types

	using Super = Variable_QSS< F >;
	using Time = Variable::Time;
	using Value = Variable::Value;

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
	Variable_QSS3(
	 std::string const & name,
	 Value const rTol = 1.0e-4,
	 Value const aTol = 1.0e-6,
	 Value const xIni = 0.0
	) :
	 Super( name, rTol, aTol, xIni ),
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
		return 3;
	}

	// Continuous Value at Time t
	Value
	x( Time const t ) const
	{
		Time const tDel( t - tX );
		return x_0_ + ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
	}

	// Continuous First Derivative at Time t
	Value
	x1( Time const t ) const
	{
		Time const tDel( t - tX );
		return x_1_ + ( ( ( two * x_2_ ) + ( three * x_3_ * tDel ) ) * tDel );
	}

	// Continuous Second Derivative at Time t
	Value
	x2( Time const t ) const
	{
		return ( two * x_2_ ) + ( six * x_3_ * ( t - tX ) );
	}

	// Continuous Third Derivative at Time t
	Value
	x3( Time const t ) const
	{
		return six * x_3_;
	}

	// Quantized Value at Time t
	Value
	q( Time const t ) const
	{
		Time const tDel( t - tQ );
		return q_0_ + ( ( q_1_ + ( q_2_ * tDel ) ) * tDel );
	}

	// Quantized First Derivative at Time t
	Value
	q1( Time const t ) const
	{
		return q_1_ + ( two * q_2_ * ( t - tQ ) );
	}

	// Quantized Second Derivative at Time t
	Value
	q2( Time const ) const
	{
		return two * q_2_;
	}

public: // Methods

	// Initialization to a Value
	void
	init( Value const x )
	{
		init_0( x );
		init_1();
		init_2();
		init_3();
	}

	// Initialization to a Value: Stage 0
	void
	init_0( Value const x )
	{
		x_0_ = q_0_ = x;
		set_qTol();
	}

	// Initialization: Stage 0
	void
	init_0()
	{
		x_0_ = q_0_ = xIni;
		set_qTol();
	}

	// Initialization: Stage 1
	void
	init_1()
	{
		self_observer = d_.finalize( this );
		shrink_observers(); // Optional
		x_1_ = q_1_ = d_.q( tQ );
	}

	// Initialization: Stage 2
	void
	init_2()
	{
		x_2_ = q_2_ = one_half * d_.q1( tQ );
	}

	// Initialization: Stage 3
	void
	init_3()
	{
		x_3_ = one_sixth * d_.q2( tQ );
		set_tE_aligned();
		event( events.add_QSS( tE, this ) );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t+" << q_2_ << "*t^2 quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2+" << x_3_ << "*t^3 internal   tE=" << tE << '\n';
	}

	// Set Current Tolerance
	void
	set_qTol()
	{
		qTol = std::max( rTol * std::abs( q_0_ ), aTol );
		assert( qTol > 0.0 );
	}

	// QSS Advance
	void
	advance_QSS()
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
		event( events.shift_QSS( tE, event() ) );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t+" << q_2_ << "*t^2 quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2+" << x_3_ << "*t^3 internal   tE=" << tE << '\n';
		advance_observers();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0()
	{
		Time const tDel( ( tQ = tE ) - tX );
		x_0_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
		tX = tE;
		set_qTol();
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1()
	{
		x_1_ = q_1_ = d_.qs( tE );
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2()
	{
		x_2_ = q_2_ = one_half * d_.qc1( tE );
	}

	// QSS Advance: Stage 3
	void
	advance_QSS_3()
	{
		x_3_ = one_sixth * d_.qc2( tE );
		set_tE_aligned();
		event( events.shift_QSS( tE, event() ) );
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t+" << q_2_ << "*t^2 quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2+" << x_3_ << "*t^3 internal   tE=" << tE << '\n';
	}

	// Observer Advance
	void
	advance_observer( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		x_0_ = x_0_ + ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
		x_1_ = d_.qs( t );
		x_2_ = one_half * d_.qc1( t );
		x_3_ = one_sixth * d_.qc2( tX = t );
		set_tE_unaligned();
		event( events.shift_QSS( tE, event() ) );
		if ( options::output::d ) std::cout << "  " << name << '(' << t << ')' << " = " << q_0_ << "+" << q_1_ << "*t+" << q_2_ << "*t^2 quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2+" << x_3_ << "*t^3 internal   tE=" << tE << '\n';
	}

	// Handler Advance
	void
	advance_handler( Time const t, Value const x )
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_0_ = x;
		set_qTol();
		x_1_ = q_1_ = d_.qs( t );
		x_2_ = q_2_ = one_half * d_.qc1( t );
		x_3_ = one_sixth * d_.qc2( t );
		set_tE_aligned();
		event( events.shift_QSS( tE, event() ) );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t+" << q_2_ << "*t^2 quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2+" << x_3_ << "*t^3 internal   tE=" << tE << '\n';
		advance_observers();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t, Value const x )
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_0_ = x;
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
		x_2_ = q_2_ = one_half * d_.qc1( tQ );
	}

	// Handler Advance: Stage 3
	void
	advance_handler_3()
	{
		x_3_ = one_sixth * d_.qc2( tQ );
		set_tE_aligned();
		event( events.shift_QSS( tE, event() ) );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t+" << q_2_ << "*t^2 quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2+" << x_3_ << "*t^3 internal   tE=" << tE << '\n';
	}

private: // Methods

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE_aligned()
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

	// Set End Time: Quantized and Continuous Unaligned
	void
	set_tE_unaligned()
	{
		assert( tQ <= tX );
		assert( dt_min <= dt_max );
		Time const tXQ( tX - tQ );
		Value const d0( x_0_ - ( q_0_ + ( q_1_ + ( q_2_ * tXQ ) ) * tXQ ) );
		Value const d1( x_1_ - ( q_1_ + ( two * q_2_ * tXQ ) ) );
		Value const d2( x_2_ - q_2_ );
		Time dtX;
		if ( ( x_3_ >= 0.0 ) && ( d2 >= 0.0 ) && ( d1 >= 0.0 ) ) { // Upper boundary crossing
			dtX = min_root_cubic_upper( x_3_, d2, d1, d0 - qTol );
		} else if ( ( x_3_ <= 0.0 ) && ( d2 <= 0.0 ) && ( d1 <= 0.0 ) ) { // Lower boundary crossing
			dtX = min_root_cubic_lower( x_3_, d2, d1, d0 + qTol );
		} else { // Both boundaries can have crossings
			dtX = min_root_cubic_both( x_3_, d2, d1, d0 + qTol, d0 - qTol );
		}
		tE = ( dtX == infinity ? infinity : tX + std::min( dtX, dt_max ) );
		if ( ( options::inflection ) && ( x_3_ != 0.0 ) && ( signum( x_2_ ) != signum( x_3_ ) ) && ( signum( x_2_ ) == signum( q_2_ ) ) ) {
			Time const tI( tX - ( x_2_ / ( three * x_3_ ) ) );
			if ( tX < tI ) tE = std::min( tE, tI );
		}
	}

private: // Data

	Value x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }, x_3_{ 0.0 }; // Continuous rep coefficients
	Value q_0_{ 0.0 }, q_1_{ 0.0 }, q_2_{ 0.0 }; // Quantized rep coefficients

};

} // dfn
} // QSS

#endif
