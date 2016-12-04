#ifndef QSS_Variable_hh_INCLUDED
#define QSS_Variable_hh_INCLUDED

// QSS Variable Abstract Base Class

// QSS Headers
#include <QSS/EventQueue.hh>
#include <QSS/math.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

// QSS Variable Abstract Base Class
class Variable
{

public: // Types

	using Time = double;
	using Variables = std::vector< Variable * >;
	using EventQ = EventQueue< Variable >;

protected: // Creation

	// Constructor
	explicit
	Variable(
	 std::string const & name,
	 double const aTol = 1.0e-6,
	 double const rTol = 1.0e-6
	) :
	 name( name ),
	 aTol( std::max( aTol, 0.0 ) ),
	 rTol( std::max( rTol, 0.0 ) )
	{
		assert( aTol > 0.0 ); // Prevent infinite loop if value is zero
		assert( rTol >= 0.0 );
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

public: // Properties

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

	// Order of QSS Method
	virtual
	int
	order() const = 0;

	// Continuous Value at Time t
	virtual
	double
	x( Time const t ) const = 0;

	// Quantized Value at Time tQ
	virtual
	double
	q() const = 0;

	// Quantized First Derivative at Time tQ
	virtual
	double
	q1() const
	{
		return 0.0;
	}

	// Quantized Second Derivative at Time tQ
	virtual
	double
	q2() const
	{
		return 0.0;
	}

	// Quantized Value at Time t
	virtual
	double
	q( Time const t ) const = 0;

	// Quantized First Derivative at Time t
	virtual
	double
	q1( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		(void)t; // Suppress unused parameter warning
		return 0.0;
	}

	// Quantized Second Derivative at Time t
	virtual
	double
	q2( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		(void)t; // Suppress unused parameter warning
		return 0.0;
	}

public: // Methods

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

	// Initialize Constant Term
	virtual
	Variable &
	init0( double const x ) = 0;

	// Initialize Linear Coefficient
	virtual
	void
	init1() = 0;

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

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 1
	virtual
	void
	advance1()
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

	// Advance Simultaneous Trigger to Time tE and Requantize: Step Observers
	void
	advance_observers()
	{
		for ( Variable * observer : observers() ) { // Advance (other) observers
			observer->advance( tQ );
		}
	}

	// Advance Observer to Time t
	virtual
	void
	advance( Time const t ) = 0;

public: // Data

	std::string name;
	double aTol{ 1.0e-6 }; // Absolute tolerance
	double rTol{ 1.0e-6 }; // Relative tolerance
	double qTol{ 1.0e-6 }; // Quantization tolerance
	Time tQ{ 0.0 }; // Quantized time range begin
	Time tC{ 0.0 }; // Continuous time range begin
	Time tE{ infinity }; // Time range end: tQ <= tE and tC <= tE
	bool self_observer{ false }; // Variable appears in its derivative?

protected: // Data

	Variables observers_; // Variables dependent on this Variable
	EventQ::iterator event_; // Iterator to event queue entry

};

#endif
