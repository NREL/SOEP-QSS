#ifndef QSS_Quantizer_hh_INCLUDED
#define QSS_Quantizer_hh_INCLUDED

// Quantizer Class

// QSS Headers

// C++ Headers
#include <cassert>

// Quantizer
class Quantizer // : public QuantizerABC
{

public: // Types


public: // Creation

	// Constructor
	explicit
	Quantizer(
	 double const aTol = 1.0e-6,
	 double const rTol = 1.0e-6
	) :
	 aTol_( aTol ),
	 rTol_( rTol )
	{}

//	// Copy Constructor
//	Quantizer( Quantizer const & ) = default;
//
//	// Move Constructor
//	Quantizer( Quantizer && ) noexcept = default;
//
//public: // Assignment
//
//	// Copy Assignment
//	Quantizer &
//	operator =( Quantizer const & ) = default;
//
//	// Move Assignment
//	Quantizer &
//	operator =( Quantizer && ) noexcept = default;

public: // Properties

	// Absolute Tolerance
	double
	aTol() const
	{
		return aTol_;
	}

	// Relative Tolerance
	double
	rTol() const
	{
		return rTol_;
	}

public: // Methods

	// Next Quantized Time
	double
	next(
	 double const q //
	) const
	{
	}

private: // Data

	double aTol_{ 1.0e-6 }, rTol_{ 1.0e-6 }; // Absolute and relative tolerances

};

#endif
