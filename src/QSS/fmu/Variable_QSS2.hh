#ifndef QSS_fmu_Variable_QSS2_hh_INCLUDED
#define QSS_fmu_Variable_QSS2_hh_INCLUDED

// FMU-Based QSS2 Variable
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/fmu/Variable_QSS.hh>

namespace QSS {
namespace fmu {

// FMU-Based QSS2 Variable
class Variable_QSS2 final : public Variable_QSS
{

public: // Types

	using Super = Variable_QSS;

public: // Creation

	// Constructor
	explicit
	Variable_QSS2(
	 std::string const & name,
	 Value const rTol = 1.0e-4,
	 Value const aTol = 1.0e-6,
	 Value const xIni = 0.0,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Super( name, rTol, aTol, xIni, var, der ),
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
		init_2();
	}

	// Initialization to a Value: Stage 0
	void
	init_0( Value const x )
	{
		init_observers();
		fmu_set_value( x_0_ = q_0_ = x );
		set_qTol();
	}

	// Initialization: Stage 0
	void
	init_0()
	{
		init_observers();
		fmu_set_value( x_0_ = q_0_ = xIni );
		set_qTol();
	}

	// Initialization: Stage 1
	void
	init_1()
	{
		x_1_ = q_1_ = fmu_get_deriv();
	}

	// Initialization: Stage 2
	void
	init_2()
	{
		x_2_ = options::one_half_over_dtND * ( fmu_get_deriv() - x_1_ ); // Forward Euler //API one_half * fmu_get_deriv2() when 2nd derivative is available
		set_tE_aligned();
		event( events.add_QSS( tE, this ) );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
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
		q_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
		set_qTol();
		advance_observers_1();
		if ( self_observer ) {
			x_0_ = q_0_;
			fmu_set_observees_q( tX = tQ );
			if ( observers_.empty() ) fmu_set_value( q_0_ );
			x_1_ = q_1_ = fmu_get_deriv();
		} else {
			q_1_ = x_1_ + ( two * x_2_ * tDel );
		}
		if ( ( self_observer ) || ( observers_max_order_ >= 2 ) ) {
			fmu::set_time( tD = tQ + options::dtND );
			if ( observers_max_order_ >= 2 ) advance_observers_2();
			if ( self_observer ) {
				fmu_set_observees_q( tD );
				if ( observers_max_order_ <= 1 ) fmu_set_q( tD );
				x_2_ = options::one_half_over_dtND * ( fmu_get_deriv() - x_1_ ); // Forward Euler //API one_half * fmu_get_deriv2() when 2nd derivative is available
			}
		}
		set_tE_aligned();
		event( events.shift_QSS( tE, event() ) );
		if ( options::output::d ) {
			std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
			advance_observers_d();
		}
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0()
	{
		Time const tDel( ( tQ = tE ) - tX );
		x_0_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
		tX = tE;
		set_qTol();
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1()
	{
		fmu_set_observees_q( tQ );
		if ( ( self_observer ) && ( observers_.empty() ) ) fmu_set_value( q_0_ );
		x_1_ = q_1_ = fmu_get_deriv();
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2()
	{
		fmu_set_observees_q( tD = tQ + options::dtND );
		if ( ( self_observer ) && ( observers_max_order_ <= 1 ) ) fmu_set_q( tD );
		x_2_ = options::one_half_over_dtND * ( fmu_get_deriv() - x_1_ ); // Forward Euler //API one_half * fmu_get_deriv2() when 2nd derivative is available
		set_tE_aligned();
		event( events.shift_QSS( tE, event() ) );
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
	}

	// Observer Advance: Stage 1
	void
	advance_observer_1( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		tX = t;
		x_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
		x_1_ = fmu_get_deriv();
	}

	// Observer Advance: Stage 2
	void
	advance_observer_2()
	{
		x_2_ = options::one_half_over_dtND * ( fmu_get_deriv() - x_1_ ); // Forward Euler //API one_half * fmu_get_deriv2() when 2nd derivative is available
		set_tE_unaligned();
		event( events.shift_QSS( tE, event() ) );
	}

	// Observer Advance: Stage d
	void
	advance_observer_d()
	{
		std::cout << "  " << name << '(' << tX << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
	}

	// Handler Advance
	void
	advance_handler( Time const t )
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_0_ = fmu_get_value(); // Assume FMU ran zero-crossing handler
		set_qTol();
		advance_observers_1();
		fmu_set_observees_q( tQ );
		if ( ( self_observer ) && ( observers_.empty() ) ) fmu_set_value( q_0_ );
		x_1_ = q_1_ = fmu_get_deriv();
		fmu::set_time( tD = tQ + options::dtND );
		if ( observers_max_order_ >= 2 ) advance_observers_2();
		fmu_set_observees_q( tD );
		if ( ( self_observer ) && ( observers_max_order_ <= 1 ) ) fmu_set_q( tD );
		x_2_ = options::one_half_over_dtND * ( fmu_get_deriv() - x_1_ ); // Forward Euler
		set_tE_aligned();
		event( events.shift_QSS( tE, event() ) );
		if ( options::output::d ) {
			std::cout << "* " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
			advance_observers_d();
		}
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t )
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_0_ = fmu_get_value(); // Assume FMU ran zero-crossing handler
		set_qTol();
	}

	// Handler Advance: Stage 1
	void
	advance_handler_1()
	{
		fmu_set_observees_q( tQ );
		if ( ( self_observer ) && ( observers_.empty() ) ) fmu_set_value( q_0_ );
		x_1_ = q_1_ = fmu_get_deriv();
	}

	// Handler Advance: Stage 2
	void
	advance_handler_2()
	{
		fmu_set_observees_q( tD = tQ + options::dtND );
		if ( ( self_observer ) && ( observers_max_order_ <= 1 ) ) fmu_set_q( tD );
		x_2_ = options::one_half_over_dtND * ( fmu_get_deriv() - x_1_ ); // Forward Euler
		set_tE_aligned();
		event( events.shift_QSS( tE, event() ) );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
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

} // fmu
} // QSS

#endif
