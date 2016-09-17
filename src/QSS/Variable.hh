#ifndef QSS_Variable_hh_INCLUDED
#define QSS_Variable_hh_INCLUDED

// QSS Variable Abstract Base Class

// QSS Headers
#include <QSS/EventQueue.hh>

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

	// Continuous Value at Time t
	virtual
	double
	x( Time const t ) const = 0;

	// Quantized Value at Time t
	virtual
	double
	q( Time const t ) const = 0;

	// Quantized Value at tBeg
	virtual
	double
	q0() const = 0;

	// Quantized Slope at tBeg
	virtual
	double
	q1() const = 0;

	// Quantized Slope at Time t
	virtual
	double
	q1( Time const t ) const = 0;

public: // Methods

	// Add Observer
	void
	add_observer( Variable & v )
	{
		observers_.push_back( &v );
	}

	// Add Observer
	void
	add_observer( Variable * v )
	{
		observers_.push_back( v );
	}

	// Shrink Observers Collection
	void
	shrink_observers() // May be worth calling after all observers added to improve memory and cache use
	{
		observers_.shrink_to_fit();
	}

	// Initialize Value
	virtual
	Variable &
	init_val() = 0;

	// Initialize to Value
	virtual
	Variable &
	init_val( double const xBeg ) = 0;

	// Initialize First Derivative
	virtual
	void
	init_der() = 0;

	// Initialize Second Derivative
	virtual
	void
	init_der2()
	{}

	// Initialize Event in Queue
	virtual
	void
	init_event() = 0;

	// Advance Trigger to Time tEnd and Requantize
	virtual
	void
	advance() = 0;

	// Advance Observer to Time t
	virtual
	void
	advance( Time const t ) = 0;

public: // Data

	std::string name;
	double aTol{ 1.0e-6 }; // Absolute tolerance
	double rTol{ 1.0e-6 }; // Relative tolerance
	double qTol{ 1.0e-6 }; // Current quantization tolerance
	Time tBeg{ 0.0 }; // Quantized time range begin
	Time tCon{ 0.0 }; // Continuous time range begin
	Time tEnd{ infinity }; // Time range end: tBeg <= tCon <= tEnd

private: // Data

	Variables observers_; // Variables dependent on this Variable
	EventQ::iterator event_; // Iterator to event queue entry

public: // Static Data

	static Time const infinity;

};

#endif
