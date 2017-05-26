#ifndef QSS_fmu_Variable_QSS1_hh_INCLUDED
#define QSS_fmu_Variable_QSS1_hh_INCLUDED

// FMU-Based QSS1 Variable
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

// FMU-Based QSS1 Variable
class Variable_QSS1 final : public Variable_QSS
{

public: // Types

	using Super = Variable_QSS;

public: // Creation

	// Constructor
	explicit
	Variable_QSS1(
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
	x1( Time const ) const
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
		init_1();
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
		x_1_ = fmu_get_deriv();
		set_tE_aligned();
		event( events.add_QSS( tE, this ) );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
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
		x_0_ = q_0_ = x_0_ + ( x_1_ * ( ( tQ = tE ) - tX ) );
		set_qTol();
		advance_observers_1();
		fmu_set_observees_q( tX = tQ );
		if ( observers_.empty() ) fmu_set_value( q_0_ );
		x_1_ = fmu_get_deriv();
		if ( observers_max_order_ >= 2 ) {
			fmu::set_time( tD = tQ + options::dtND );
			advance_observers_2();
		}
		set_tE_aligned();
		event( events.shift_QSS( tE, event() ) );
		if ( options::output::d ) {
			std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
			advance_observers_d();
		}
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0()
	{
		x_0_ = q_0_ = x_0_ + ( x_1_ * ( ( tQ = tE ) - tX ) );
		tX = tE;
		set_qTol();
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1()
	{
		fmu_set_observees_q( tQ );
		if ( ( self_observer ) && ( observers_.empty() ) ) fmu_set_value( q_0_ );
		x_1_ = fmu_get_deriv();
		set_tE_aligned();
		event( events.shift_QSS( tE, event() ) );
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
	}

	// Observer Advance: Stage 1
	void
	advance_observer_1( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		x_0_ = x_0_ + ( x_1_ * ( t - tX ) );
		tX = t;
		x_1_ = fmu_get_deriv();
		set_tE_unaligned();
		event( events.shift_QSS( tE, event() ) );
	}

	// Observer Advance: Stage d
	void
	advance_observer_d()
	{
		std::cout << "  " << name << '(' << tX << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
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
		x_1_ = fmu_get_deriv();
		if ( observers_max_order_ >= 2 ) {
			fmu::set_time( tD = tQ + options::dtND );
			advance_observers_2();
		}
		set_tE_aligned();
		event( events.shift_QSS( tE, event() ) );
		if ( options::output::d ) {
			std::cout << "* " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
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
		x_1_ = fmu_get_deriv();
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
		 ( x_1_ > 0.0 ? tX + ( ( q_0_ + qTol - x_0_ ) / x_1_ ) :
		 ( x_1_ < 0.0 ? tX + ( ( q_0_ - qTol - x_0_ ) / x_1_ ) :
		 infinity ) );
		if ( dt_max != infinity ) tE = std::min( tE, tX + dt_max );
		tE = std::max( tE, tX ); // Numeric bulletproofing
	}

private: // Data

	Value x_0_{ 0.0 }, x_1_{ 0.0 }; // Continuous rep coefficients
	Value q_0_{ 0.0 }; // Quantized rep coefficients

};

} // fmu
} // QSS

#endif
