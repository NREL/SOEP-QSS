#ifndef QSS_Variable_QSS_hh_INCLUDED
#define QSS_Variable_QSS_hh_INCLUDED

// QSS Variable Abstract Base Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/Variable.hh>

// QSS1 Variable
template< template< typename > class F >
class Variable_QSS : public Variable
{

public: // Types

	using Super = Variable;
	using Time = Variable::Time;
	using Value = Variable::Value;
	template< typename V > using Function = F< V >;
	using Derivative = Function< Variable >;
	using Coefficient = typename Derivative::Coefficient;

protected: // Creation

	// Constructor
	explicit
	Variable_QSS(
	 std::string const & name,
	 Value const rTol = 1.0e-4,
	 Value const aTol = 1.0e-6,
	 Value const xIni = 0.0
	) :
	 Variable( name, rTol, aTol, xIni )
	{}

	// Copy Constructor
	Variable_QSS( Variable_QSS const & ) = default;

	// Move Constructor
	Variable_QSS( Variable_QSS && ) noexcept = default;

public: // Creation

	// Destructor
	virtual
	~Variable_QSS()
	{}

protected: // Assignment

	// Copy Assignment
	Variable_QSS &
	operator =( Variable_QSS const & ) = default;

	// Move Assignment
	Variable_QSS &
	operator =( Variable_QSS && ) noexcept = default;

public: // Properties

	// Derivative Function
	Derivative const &
	d() const
	{
		return d_;
	}

	// Derivative Function
	Derivative &
	d()
	{
		return d_;
	}

protected: // Data

	Derivative d_; // Derivative function

};

#endif
