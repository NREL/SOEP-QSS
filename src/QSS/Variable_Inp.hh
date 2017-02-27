#ifndef QSS_Variable_Inp_hh_INCLUDED
#define QSS_Variable_Inp_hh_INCLUDED

// QSS Input Variable Abstract Base Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/Variable.hh>

// QSS Input Variable with Quantization Order 1
template< class F >
class Variable_Inp : public Variable
{

public: // Types

	using Super = Variable;
	using Value = Variable::Value;
	using Time = Variable::Time;
	using Function = F;
	using Coefficient = typename Function::Coefficient;

protected: // Creation

	// Constructor
	explicit
	Variable_Inp(
	 std::string const & name,
	 Value const rTol = 1.0e-4,
	 Value const aTol = 1.0e-6
	) :
	 Variable( name, rTol, aTol )
	{}

	// Copy Constructor
	Variable_Inp( Variable_Inp const & ) = default;

	// Move Constructor
	Variable_Inp( Variable_Inp && ) noexcept = default;

public: // Creation

	// Destructor
	virtual
	~Variable_Inp()
	{}

protected: // Assignment

	// Copy Assignment
	Variable_Inp &
	operator =( Variable_Inp const & ) = default;

	// Move Assignment
	Variable_Inp &
	operator =( Variable_Inp && ) noexcept = default;

public: // Properties

	// Function
	Function const &
	f() const
	{
		return f_;
	}

	// Function
	Function &
	f()
	{
		return f_;
	}

protected: // Data

	Function f_; // Value function

};

#endif
