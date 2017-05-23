#ifndef QSS_dfn_Variable_hh_INCLUDED
#define QSS_dfn_Variable_hh_INCLUDED

// Variable Abstract Base Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/dfn/Variable.fwd.hh>
#include <QSS/dfn/globals_dfn.hh>
#include <QSS/EventQueue.hh>
#include <QSS/math.hh>
#include <QSS/options.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace QSS {
namespace dfn {

// Variable Abstract Base Class
class Variable
{

public: // Types

	using Time = double;
	using Value = double;
	using Variables = std::vector< Variable * >;
	using EventQ = EventQueue< Variable >;
	using size_type = Variables::size_type;

	// Variable category
	enum class Cat {
	 Discrete,
	 Input,
	 QSS,
	 ZC
	};

	// Zero Crossing Type
	enum class Crossing {
	 DnPN = -4, // Downward: Positive to negative
	 DnPZ = -3, // Downward: Positive to zero
	 DnZN = -2, // Downward: Zero to negative
	 Dn = -1,   // Downward
	 Flat =  0, // Flat zero value
	 Up = +1,   // Upward
	 UpZP = +2, // Upward: Zero to positive
	 UpNZ = +3, // Upward: Negative to zero
	 UpNP = +4  // Upward: Negative to positive
	};

	struct AdvanceSpecs_LIQSS1
	{
		Value l;
		Value u;
		Value z;
	};

	struct AdvanceSpecs_LIQSS2
	{
		Value l1;
		Value u1;
		Value z1;
		Value l2;
		Value u2;
		Value z2;
	};

protected: // Creation

	// Constructor
	Variable(
	 std::string const & name,
	 Value const rTol,
	 Value const aTol,
	 Value const xIni = 0.0
	) :
	 name( name ),
	 rTol( std::max( rTol, 0.0 ) ),
	 aTol( std::max( aTol, std::numeric_limits< Value >::min() ) ),
	 xIni( xIni )
	{
		dt_min = options::dtMin;
		dt_max = options::dtMax;
	}

	// Default-Tolerance Constructor
	explicit
	Variable(
	 std::string const & name,
	 Value const xIni = 0.0
	) :
	 name( name ),
	 xIni( xIni )
	{
		dt_min = options::dtMin;
		dt_max = options::dtMax;
	}

	// Copy Constructor
	Variable( Variable const & ) = default;

	// Move Constructor
	Variable( Variable && ) noexcept = default;

public: // Creation

	// Destructor
	virtual
	~Variable()
	{}

protected: // Assignment

	// Copy Assignment
	Variable &
	operator =( Variable const & ) = default;

	// Move Assignment
	Variable &
	operator =( Variable && ) noexcept = default;

public: // Predicate

	// Discrete Variable?
	virtual
	bool
	is_Discrete() const
	{ // Default implementation
		return false;
	}

	// Input Variable?
	virtual
	bool
	is_Input() const
	{ // Default implementation
		return false;
	}

	// QSS Variable?
	virtual
	bool
	is_QSS() const
	{ // Default implementation
		return false;
	}

	// Zero-Crossing Variable?
	virtual
	bool
	is_ZC() const
	{ // Default implementation
		return false;
	}

public: // Properties

	// Category
	virtual
	Cat
	cat() const = 0;

	// Order of Method
	virtual
	int
	order() const = 0;

	// Continuous Value at Time t
	virtual
	Value
	x( Time const t ) const = 0;

	// Continuous First Derivative at Time t
	virtual
	Value
	x1( Time const t ) const = 0;

	// Continuous Second Derivative at Time t
	virtual
	Value
	x2( Time const ) const
	{
		return 0.0;
	}

	// Continuous Third Derivative at Time t
	virtual
	Value
	x3( Time const ) const
	{
		return 0.0;
	}

	// Quantized Value at Time t
	virtual
	Value
	q( Time const t ) const = 0;

	// Quantized First Derivative at Time t
	virtual
	Value
	q1( Time const ) const
	{
		return 0.0;
	}

	// Quantized Second Derivative at Time t
	virtual
	Value
	q2( Time const ) const
	{
		return 0.0;
	}

	// Observers
	Variables const &
	observers() const
	{
		return observers_;
	}

	// Observers
	Variables &
	observers()
	{
		return observers_;
	}

	// Event Queue Iterator
	EventQ::iterator &
	event()
	{
		return event_;
	}

	// Event Queue Iterator Assignment
	void
	event( EventQ::iterator const i )
	{
		event_ = i;
		assert( event_->second.var() == this );
	}

	// Zero-Crossing Time
	virtual
	Time
	tZC() const
	{
		assert( false ); // Not a ZC variable
		return Time( 0.0 );
	}

public: // Methods

	// Set Max Time Step
	void
	set_dt_min( Time const dt )
	{
		assert( dt >= 0.0 );
		dt_min = dt;
	}

	// Set Max Time Step
	void
	set_dt_max( Time const dt )
	{
		assert( dt > 0.0 );
		dt_max = dt;
	}

	// Add Observer
	void
	add_observer( Variable & v )
	{
		if ( &v != this ) observers_.push_back( &v ); // Don't need to self-observe: Observers called at the end of self requantization
	}

	// Add Observer
	void
	add_observer( Variable * v )
	{
		if ( v != this ) observers_.push_back( v ); // Don't need to self-observe: Observers called at the end of self requantization
	}

	// Shrink Observers Collection
	void
	shrink_observers()
	{
		observers_.shrink_to_fit();
	}

	// Add Handler Event
	void
	add_handler()
	{
		event_ = events.add_handler( this );
	}

	// Initialization
	virtual
	void
	init()
	{}

	// Initialization to a Value
	virtual
	void
	init( Value const )
	{}

	// Initialization: Stage 0
	virtual
	void
	init_0()
	{}

	// Initialization to a Value: Stage 0
	virtual
	void
	init_0( Value const )
	{}

	// Initialization: LIQSS Stage 1
	virtual
	void
	init_LIQSS_1()
	{}

	// Initialization: Stage 1
	virtual
	void
	init_1()
	{}

	// Initialization: LIQSS Stage 2
	virtual
	void
	init_LIQSS_2()
	{}

	// Initialization: Stage 2
	virtual
	void
	init_2()
	{}

	// Initialization: Stage 3
	virtual
	void
	init_3()
	{}

	// QSS Advance
	virtual
	void
	advance_QSS()
	{
		assert( false );
	}

	// QSS Advance: Simultaneous
	virtual
	void
	advance_QSS_simultaneous()
	{
		assert( false );
	}

	// QSS Advance: Stage 0
	virtual
	void
	advance_QSS_0()
	{}

	// LIQSS Trigger Advance: Stage 1
	virtual
	void
	advance_LIQSS_1()
	{}

	// QSS Advance: Stage 1
	virtual
	void
	advance_QSS_1()
	{}

	// LIQSS Trigger Advance: Stage 2
	virtual
	void
	advance_LIQSS_2()
	{}

	// QSS Advance: Stage 2
	virtual
	void
	advance_QSS_2()
	{}

	// QSS Advance: Stage 3
	virtual
	void
	advance_QSS_3()
	{}

	// Advance Non-Self Observers to Time tQ
	void
	advance_observers()
	{
		for ( Variable * observer : observers_ ) {
			observer->advance_observer( tQ );
		}
	}

	// Observer Advance
	virtual
	void
	advance_observer( Time const )
	{
		assert( false ); // Not a QSS or ZC variable
	}

	// Zero-Crossing Trigger Advance
	virtual
	void
	advance_ZC()
	{
		assert( false ); // Not a ZC variable
	}

	// Handler Advance
	virtual
	void
	advance_handler( Time const, Value const )
	{
		assert( false ); // Not a QSS or Discrete variable
	}

	// Handler Advance: Stage 0
	virtual
	void
	advance_handler_0( Time const, Value const )
	{
		assert( false ); // Not a QSS or Discrete variable
	}

	// Handler Advance: Stage 1
	virtual
	void
	advance_handler_1()
	{
		assert( false ); // Not a QSS or Discrete variable
	}

	// Handler Advance: Stage 2
	virtual
	void
	advance_handler_2()
	{
		assert( false ); // Not a QSS or Discrete variable
	}

	// Handler Advance: Stage 3
	virtual
	void
	advance_handler_3()
	{
		assert( false ); // Not a QSS or Discrete variable
	}

	// Shift Observer to Time t
	void
	shift_observer( Time const t )
	{
		event_ = events.shift_observer( t, event_ );
	}

	// Shift Handler to Time t
	void
	shift_handler( Time const t, Value const val )
	{
		event_ = events.shift_handler( t, val, event_ );
	}

	// Shift Handler to Time Infinity
	void
	shift_handler()
	{
		event_ = events.shift_handler( event_ );
	}

public: // Data

	std::string name;
	Value rTol{ 1.0e-4 }; // Relative tolerance
	Value aTol{ 1.0e-6 }; // Absolute tolerance
	Value qTol{ 1.0e-6 }; // Quantization tolerance
	Value xIni{ 0.0 }; // Initial value
	Time tQ{ 0.0 }; // Quantized time range begin
	Time tX{ 0.0 }; // Continuous time range begin
	Time tE{ infinity }; // Time range end: tQ <= tE and tX <= tE
	Time dt_min{ 0.0 }; // Time step min
	Time dt_max{ infinity }; // Time step max
	bool self_observer{ false }; // Variable appears in its function/derivative?

protected: // Data

	Variables observers_; // Variables dependent on this Variable
	EventQ::iterator event_; // Iterator to event queue entry

};

} // dfn
} // QSS

#endif
