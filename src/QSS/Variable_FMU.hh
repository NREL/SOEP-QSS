#ifndef QSS_Variable_FMU_hh_INCLUDED
#define QSS_Variable_FMU_hh_INCLUDED

// FMU-Based QSS Variable Abstract Base Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/Variable.hh>
#include <QSS/FMU.hh>
#include <QSS/FMU_Variable.hh>

// FMU-Based QSS Variable Abstract Base Class
class Variable_FMU : public Variable
{

public: // Types

	using Time = Variable::Time;
	using Value = Variable::Value;
	using Variables = Variable::Variables;
	using Variables_FMU = std::vector< Variable_FMU * >;
	using EventQ = Variable::EventQ;

protected: // Creation

	// Constructor
	explicit
	Variable_FMU(
	 std::string const & name,
	 Value const rTol = 1.0e-4,
	 Value const aTol = 1.0e-6,
	 Value const xIni = 0.0,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Variable( name, rTol, aTol, xIni ),
	 var( var ),
	 der( der )
	{}

	// Copy Constructor
	Variable_FMU( Variable_FMU const & ) = default;

	// Move Constructor
	Variable_FMU( Variable_FMU && ) noexcept = default;

public: // Creation

	// Destructor
	virtual
	~Variable_FMU()
	{}

protected: // Assignment

	// Copy Assignment
	Variable_FMU &
	operator =( Variable_FMU const & ) = default;

	// Move Assignment
	Variable_FMU &
	operator =( Variable_FMU && ) noexcept = default;

public: // Properties

	// Observees
	Variables_FMU const &
	observees() const
	{
		return observees_;
	}

	// Observees
	Variables_FMU &
	observees()
	{
		return observees_;
	}

public: // Methods

	// Add Observee
	void
	add_observee( Variable_FMU & v )
	{
		if ( &v != this ) observees_.push_back( &v ); // Don't need to self-observe
	}

	// Add Observee
	void
	add_observee( Variable_FMU * v )
	{
		if ( v != this ) observees_.push_back( v ); // Don't need to self-observe
	}

	// Shrink Observees Collection
	void
	shrink_observees() // May be worth calling after all observees added to improve memory and cache use
	{
		observees_.shrink_to_fit();
	}

	// Set All Observer's Observee FMU Variables to Quantized Value at Time t
	void
	fmu_set_observers_observees_q( Time const t ) const
	{
		for ( Variable const * observer : observers_ ) {
			observer->fmu_set_observees_q_tX( t ); //Do Elim virtual call
		}
	}

	// Set All Observer's Observee FMU Variables to Quantized Numeric Differentiation Value at Time t
	void
	fmu_set_observers_observees_qn( Time const t, Time const t_check ) const
	{
		for ( Variable const * observer : observers_ ) {
			observer->fmu_set_observees_qn_tX( t, t_check ); //Do Elim virtual call
		}
	}

	// Set FMU Variable to Continuous Value at Time t
	void
	fmu_set_x( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		FMU::set_real( var.ref, x( t ) );
	}

	// Set FMU Variable to Quantized Value at Time t
	void
	fmu_set_q( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		FMU::set_real( var.ref, q( t ) );
	}

	// Set FMU Variable to Quantized Numeric Differentiation Value at Time t
	void
	fmu_set_qn( Time const t ) const
	{
		FMU::set_real( var.ref, qn( t ) );
	}

	// Set All Observee FMU Variables to Quantized Value at Time t
	void
	fmu_set_observees_q( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		fmu_set_q( t ); // Set self state also
		for ( auto observee : observees_ ) {
			observee->fmu_set_q( t );
		}
	}

	// Set All Observee FMU Variables to Quantized Numeric Differentiation Value at Time t
	void
	fmu_set_observees_qn( Time const t ) const
	{
		fmu_set_qn( t ); // Set self state also
		for ( auto observee : observees_ ) {
			observee->fmu_set_qn( t );
		}
	}

	// Set All Observee FMU Variables to Quantized Value at Time t > tX
	void
	fmu_set_observees_q_tX( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		if ( tX < t ) {
			fmu_set_q( t ); // Set self state also
			for ( auto observee : observees_ ) {
				observee->fmu_set_q( t );
			}
		}
	}

	// Set All Observee FMU Variables to Quantized Numeric Differentiation Value at Time t > tX
	void
	fmu_set_observees_qn_tX( Time const t, Time const t_check ) const
	{
		if ( tX < t_check ) {
			fmu_set_qn( t ); // Set self state also
			for ( auto observee : observees_ ) {
				observee->fmu_set_qn( t );
			}
		}
	}

public: // Data

	FMU_Variable var;
	FMU_Variable der;

protected: // Data

	Variables_FMU observees_; // Variables this one dependent on

};

#endif
