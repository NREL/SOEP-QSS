#ifndef QSS_FunctionABC_hh_INCLUDED
#define QSS_FunctionABC_hh_INCLUDED

class Function
{

public: // Types

protected: // Creation

	// Constructor
	Function()
	{}

	// Copy Constructor
	Function( Function const & ) = default;

	// Move Constructor
	Function( Function && ) = default;

public: // Creation

	// Destructor
	virtual
	~Function()
	{}

protected: // Assignment

	// Copy Constructor
	Function &
	operator =( Function const & ) = default;

public: // Methods

	// Value at Time t
	virtual
	double
	operator ()( double const t ) const = 0;

};

#endif
