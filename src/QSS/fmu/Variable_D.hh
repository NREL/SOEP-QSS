#ifndef QSS_fmu_Variable_D_hh_INCLUDED
#define QSS_fmu_Variable_D_hh_INCLUDED

// FMU-Based QSS Discrete Variable
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/fmu/Variable.hh>

namespace QSS {
namespace fmu {

// FMU-Based QSS Discrete Variable
class Variable_D final : public Variable
{

public: // Types

	using Super = Variable;

public: // Creation

	// Constructor
	explicit
	Variable_D(
	 std::string const & name,
	 Value const xIni,
	 FMU_Variable const var = FMU_Variable()
	) :
	 Super( name, xIni, var ),
	 x_( xIni )
	{}

public: // Predicate

	// Discrete Variable?
	bool
	is_Discrete() const
	{
		return true;
	}

public: // Properties

	// Category
	Cat
	cat() const
	{
		return Cat::Discrete;
	}

	// Order of Method
	int
	order() const
	{
		return 0;
	}

	// Value
	Value
	x() const
	{
		return x_;
	}

	// Continuous Value at Time t
	Value
	x( Time const ) const
	{
		return x_;
	}

	// Continuous First Derivative at Time t
	Value
	x1( Time const ) const
	{
		return 0.0;
	}

	// Quantized Value
	Value
	q() const
	{
		return x_;
	}

	// Quantized Value at Time t
	Value
	q( Time const ) const
	{
		return x_;
	}

public: // Methods

	// Initialization to a Value
	void
	init( Value const x )
	{
		init_0( x );
	}

	// Initialization to a Value: Stage 0
	void
	init_0( Value const x )
	{
		assert( observees_.empty() );
		shrink_observers(); // Optional
		sort_observers();
		add_handler();
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << x_ << '\n';
	}

	// Initialization: Stage 0
	void
	init_0()
	{
		assert( observees_.empty() );
		shrink_observers(); // Optional
		sort_observers();
		add_handler();
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << x_ << '\n';
	}

	// Handler Advance
	void
	advance_handler( Time const t )
	{
		assert( tX <= t );
		tX = tQ = t;
		x_ = fmu_get_value(); // Assume FMU ran zero-crossing handler
		advance_observers_1();
		if ( observers_max_order_ >= 2 ) {
			fmu::set_time( tD = tQ + options::dtND );
			advance_observers_2();
		}
		shift_handler();
		if ( options::output::d ) {
			std::cout << "* " << name << '(' << tQ << ')' << " = " << x_ << '\n';
			advance_observers_d();
		}
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t )
	{
		assert( tX <= t );
		tX = tQ = t;
		x_ = fmu_get_value(); // Assume FMU ran zero-crossing handler
		shift_handler();
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << x_ << '\n';
	}

private: // Data

	Value x_; // Value

};

} // fmu
} // QSS

#endif