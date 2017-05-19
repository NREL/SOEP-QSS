#ifndef QSS_dfn_Variable_QSS_hh_INCLUDED
#define QSS_dfn_Variable_QSS_hh_INCLUDED

// QSS Variable Abstract Base Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/dfn/Variable.hh>

namespace QSS {
namespace dfn {

// QSS Variable Abstract Base Class
template< template< typename > class F >
class Variable_QSS : public Variable
{

public: // Types

	using Super = Variable;
	using Time = Variable::Time;
	using Value = Variable::Value;
	template< typename V > using Function = F< V >;
	using Derivative = Function< Variable >;

protected: // Creation

	// Constructor
	explicit
	Variable_QSS(
	 std::string const & name,
	 Value const rTol = 1.0e-4,
	 Value const aTol = 1.0e-6,
	 Value const xIni = 0.0
	) :
	 Super( name, rTol, aTol, xIni )
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

public: // Predicate

	// QSS Variable?
	bool
	is_QSS() const
	{
		return true;
	}

public: // Properties

	// Category
	Cat
	cat() const
	{
		return Cat::QSS;
	}

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

} // dfn
} // QSS

#endif
