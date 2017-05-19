#ifndef QSS_fmu_Variable_QSS_hh_INCLUDED
#define QSS_fmu_Variable_QSS_hh_INCLUDED

// FMU-Based QSS Variable Abstract Base Class
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

// FMU-Based QSS Variable Abstract Base Class
class Variable_QSS : public Variable
{

public: // Types

	using Super = Variable;

protected: // Creation

	// Constructor
	explicit
	Variable_QSS(
	 std::string const & name,
	 Value const rTol = 1.0e-4,
	 Value const aTol = 1.0e-6,
	 Value const xIni = 0.0,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Super( name, rTol, aTol, xIni, var, der )
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

};

} // fmu
} // QSS

#endif
