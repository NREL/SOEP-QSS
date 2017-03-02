#ifndef QSS_Variable_hh_INCLUDED
#define QSS_Variable_hh_INCLUDED

// QSS Variable Abstract Base Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/EventQueue.hh>
#include <QSS/globals.hh>
#include <QSS/math.hh>
#include <QSS/options.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

// QSS Variable Abstract Base Class
class Variable
{

public: // Types

	using Time = double;
	using Value = double;
	using Variables = std::vector< Variable * >;
	using EventQ = EventQueue< Variable >;

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
	explicit
	Variable(
	 std::string const & name,
	 Value const rTol = 1.0e-4,
	 Value const aTol = 1.0e-6,
	 Value const xIni = 0.0
	) :
	 name( name ),
	 rTol( std::max( rTol, 0.0 ) ),
	 aTol( std::max( aTol, std::numeric_limits< Value >::min() ) ),
	 xIni( xIni )
	{}

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

public: // Properties

	// Order of Method
	virtual
	int
	order() const = 0;

	// Continuous Value at Time t
	virtual
	Value
	x( Time const t ) const = 0;

	// Continuous Numeric Differenentiation Value at Time t: Allow t Outside of [tX,tE]
	virtual
	Value
	xn( Time const t ) const = 0;

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

	// Quantized Numeric Differenentiation Value at Time t: Allow t Outside of [tQ,tE]
	virtual
	Value
	qn( Time const t ) const = 0;

	// Quantized First Derivative at Time t
	virtual
	Value
	q1( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		(void)t; // Suppress unused parameter warning
		return 0.0;
	}

	// Quantized Second Derivative at Time t
	virtual
	Value
	q2( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		(void)t; // Suppress unused parameter warning
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

	// Set Event Queue Iterator
	void
	event( EventQ::iterator const i )
	{
		event_ = i;
		assert( event_->second == this );
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
	shrink_observers() // May be worth calling after all observers added to improve memory and cache use
	{
		observers_.shrink_to_fit();
	}

	// Initialize Input Variable
	virtual
	void
	init()
	{}

	// Initialize QSS Variable
	virtual
	void
	init( Value const )
	{}

	// Initialize Constant Term in Input Variable
	virtual
	void
	init0()
	{}

	// Initialize Constant Term to Given Value
	virtual
	void
	init0( Value const )
	{}

	// Initialize Linear Coefficient in LIQSS Variable
	virtual
	void
	init1_LIQSS()
	{}

	// Initialize Linear Coefficient
	virtual
	void
	init1()
	{}

	// Initialize Linear Coefficient for FMU
	virtual
	void
	init1_fmu()
	{}

	// Initialize Quadratic Coefficient in LIQSS Variable
	virtual
	void
	init2_LIQSS()
	{}

	// Initialize Quadratic Coefficient
	virtual
	void
	init2()
	{}

	// Initialize Cubic Coefficient
	virtual
	void
	init3()
	{}

	// Initialize Event in Queue
	virtual
	void
	init_event() = 0;

	// Advance Trigger to Time tE and Requantize
	virtual
	void
	advance() = 0;

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 0
	virtual
	void
	advance0()
	{}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 1.FMU
	virtual
	void
	advance1_fmu()
	{}

	// Advance Simultaneous Trigger in LIQSS Variable to Time tE and Requantize: Step 1
	virtual
	void
	advance1_LIQSS()
	{}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 1
	virtual
	void
	advance1()
	{}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 2.FMU
	virtual
	void
	advance2_fmu( Time const )
	{}

	// Advance Simultaneous Trigger in LIQSS Variable to Time tE and Requantize: Step 2
	virtual
	void
	advance2_LIQSS()
	{}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 2
	virtual
	void
	advance2()
	{}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 3
	virtual
	void
	advance3()
	{}

	// Advance non-Self Observers to New Time tQ
	void
	advance_observers()
	{
		for ( Variable * observer : observers_ ) {
			observer->advance( tQ );
		}
	}

	// Advance non-Self Observers to Time t: Stage 2
	void
	advance_observers_2( Time const t )
	{
		for ( Variable * observer : observers_ ) {
			observer->advance_2( t, tQ );
		}
	}

	// Advance Observer to Time t
	virtual
	void
	advance( Time const )
	{}

	// Advance Observer to Time t
	virtual
	void
	advance_2( Time const, Time const )
	{}

	// Set All Observee FMU Variable to Quantized Value at Time t
	virtual
	void
	fmu_set_observees_q( Time const ) const
	{}

	// Set All Observee FMU Variable to Quantized Numeric Differentiation Value at Time t
	virtual
	void
	fmu_set_observees_qn( Time const ) const
	{}

	// Set All Observee FMU Variables to Quantized Value at Time t > tX
	virtual
	void
	fmu_set_observees_q_tX( Time const ) const
	{}

	// Set All Observee FMU Variables to Quantized Numeric Differentiation Value at Time t > tX
	virtual
	void
	fmu_set_observees_qn_tX( Time const, Time const ) const
	{}

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
	bool self_observer{ false }; // Variable appears in its derivative?

protected: // Data

	Variables observers_; // Variables dependent on this Variable
	EventQ::iterator event_; // Iterator to event queue entry

};

#endif
