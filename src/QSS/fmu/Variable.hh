// FMU-Based Variable Abstract Base Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2019 Objexx Engineering, Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
// (3) Neither the name of the copyright holder nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, THE UNITED STATES
// GOVERNMENT, OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef QSS_fmu_Variable_hh_INCLUDED
#define QSS_fmu_Variable_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable.fwd.hh>
#include <QSS/fmu/Conditional.hh>
#include <QSS/fmu/FMU_ME.hh>
#include <QSS/fmu/FMU_Variable.hh>
#include <QSS/fmu/Observers.hh>
#include <QSS/math.hh>
#include <QSS/options.hh>
#include <QSS/SmoothToken.hh>
#include <QSS/Target.hh>

// FMI Library Headers
#include <fmilib.h>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <vector>

namespace QSS {
namespace fmu {

// FMU-Based Variable Abstract Base Class
class Variable : public Target
{

public: // Types

	using Super = Target;

	using Boolean = bool;
	using Integer = int;
	using Real = double;
	using Time = double;
	using Reals = std::vector< Real >;
	using Events = FMU_ME::Events;
	using Variables = std::vector< Variable * >;
	using VariableRefs = std::vector< fmi2_value_reference_t >;
	using size_type = Variables::size_type;
	using Indexes = std::vector< size_type >;

	// Zero Crossing Type
	enum class Crossing {
	 DnPN = -4, // Positive to negative
	 DnPZ = -3, // Positive to zero
	 Dn   = -2, // Positive to <= zero
	 DnZN = -1, // Zero to negative
	 Flat =  0, // Flat zero value
	 UpZP = +1, // Zero to positive
	 Up   = +2, // Negative to >= zero
	 UpNZ = +3, // Negative to zero
	 UpNP = +4  // Negative to positive
	};

protected: // Creation

	// Name + Tolerance + Value Constructor
	Variable(
	 int const order,
	 std::string const & name,
	 Real const rTol,
	 Real const aTol,
	 Real const xIni,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Target( name ),
	 order_( order ),
	 rTol( std::max( rTol, 0.0 ) ),
	 aTol( std::max( aTol, std::numeric_limits< Real >::min() ) ),
	 xIni( xIni ),
	 dt_min( options::dtMin ),
	 dt_max( options::dtMax ),
	 dt_inf( options::dtInf ),
	 dt_inf_rlx( options::dtInf == infinity ? infinity : 0.5 * options::dtInf ),
	 fmu_me( fmu_me ),
	 var( var ),
	 der( der ),
	 observers_( fmu_me ),
	 events_( fmu_me->events )
	{}

	// Name + Tolerance Constructor
	Variable(
	 int const order,
	 std::string const & name,
	 Real const rTol,
	 Real const aTol,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Target( name ),
	 order_( order ),
	 rTol( std::max( rTol, 0.0 ) ),
	 aTol( std::max( aTol, std::numeric_limits< Real >::min() ) ),
	 xIni( 0.0 ),
	 dt_min( options::dtMin ),
	 dt_max( options::dtMax ),
	 dt_inf( options::dtInf ),
	 dt_inf_rlx( options::dtInf == infinity ? infinity : 0.5 * options::dtInf ),
	 fmu_me( fmu_me ),
	 var( var ),
	 der( der ),
	 observers_( fmu_me ),
	 events_( fmu_me->events )
	{}

	// Name + Value Constructor
	Variable(
	 int const order,
	 std::string const & name,
	 Real const xIni,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Target( name ),
	 order_( order ),
	 xIni( xIni ),
	 dt_min( options::dtMin ),
	 dt_max( options::dtMax ),
	 dt_inf( options::dtInf ),
	 dt_inf_rlx( options::dtInf == infinity ? infinity : 0.5 * options::dtInf ),
	 fmu_me( fmu_me ),
	 var( var ),
	 der( der ),
	 observers_( fmu_me ),
	 events_( fmu_me->events )
	{}

	// Name Constructor
	Variable(
	 int const order,
	 std::string const & name,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Target( name ),
	 order_( order ),
	 xIni( 0.0 ),
	 dt_min( options::dtMin ),
	 dt_max( options::dtMax ),
	 dt_inf( options::dtInf ),
	 dt_inf_rlx( options::dtInf == infinity ? infinity : 0.5 * options::dtInf ),
	 fmu_me( fmu_me ),
	 var( var ),
	 der( der ),
	 observers_( fmu_me ),
	 events_( fmu_me->events )
	{}

	// Copy Constructor
	Variable( Variable const & ) = default;

	// Move Constructor
	Variable( Variable && ) noexcept = default;

public: // Creation

	// Destructor
	~Variable()
	{
		if ( conditional != nullptr ) conditional->var() = nullptr;
	}

protected: // Assignment

	// Copy Assignment
	Variable &
	operator =( Variable const & ) = default;

	// Move Assignment
	Variable &
	operator =( Variable && ) noexcept = default;

public: // Predicate

	// Discrete Variable?
	virtual
	bool
	is_Discrete() const
	{
		return false;
	}

	// Input Variable?
	virtual
	bool
	is_Input() const
	{
		return false;
	}

	// Connection Input Variable?
	virtual
	bool
	is_connection() const
	{
		return false;
	}

	// QSS Variable?
	virtual
	bool
	is_QSS() const
	{
		return false;
	}

	// Zero-Crossing Variable?
	virtual
	bool
	is_ZC() const
	{
		return false;
	}

	// Non-Zero-Crossing Variable?
	virtual
	bool
	not_ZC() const
	{
		return true;
	}

	// In Conditional?
	virtual
	bool
	in_conditional() const
	{
		return conditional != nullptr;
	}

public: // Properties

	// Order of Method
	int
	order() const
	{
		return order_;
	}

	// Boolean Value
	virtual
	Boolean
	b() const
	{
		assert( false ); // Missing override
		return false;
	}

	// Boolean Value at Time t
	virtual
	Boolean
	b( Time const ) const
	{
		assert( false ); // Missing override
		return false;
	}

	// Integer Value
	virtual
	Integer
	i() const
	{
		assert( false ); // Missing override
		return 0;
	}

	// Integer Value at Time t
	virtual
	Integer
	i( Time const ) const
	{
		assert( false ); // Missing override
		return 0;
	}

	// Real Value
	virtual
	Real
	r() const
	{
		assert( false ); // Missing override
		return 0.0;
	}

	// Real Value at Time t
	virtual
	Real
	r( Time const ) const
	{
		assert( false ); // Missing override
		return 0.0;
	}

	// Continuous Value at Time t
	virtual
	Real
	x( Time const ) const
	{
		assert( false ); // Missing override
		return 0.0;
	}

	// Continuous First Derivative at Time t
	virtual
	Real
	x1( Time const ) const
	{
		return 0.0;
	}

	// Continuous Second Derivative at Time t
	virtual
	Real
	x2( Time const ) const
	{
		return 0.0;
	}

	// Continuous Third Derivative at Time t
	virtual
	Real
	x3( Time const ) const
	{
		return 0.0;
	}

	// Quantized Value at Time t
	virtual
	Real
	q( Time const ) const
	{
		assert( false ); // Missing override
		return 0.0;
	}

	// Quantized First Derivative at Time t
	virtual
	Real
	q1( Time const ) const
	{
		return 0.0;
	}

	// Quantized Second Derivative at Time t
	virtual
	Real
	q2( Time const ) const
	{
		return 0.0;
	}

	// Quantized Third Derivative at Time t
	virtual
	Real
	q3( Time const ) const
	{
		return 0.0;
	}

	// Simultaneous Value at Time t
	virtual
	Real
	s( Time const ) const
	{
		assert( false ); // Missing override
		return 0.0;
	}

	// Simultaneous Numeric Differentiation Value at Time t
	virtual
	Real
	sn( Time const ) const
	{
		assert( false ); // Missing override
		return 0.0;
	}

	// Simultaneous First Derivative at Time t
	virtual
	Real
	s1( Time const ) const
	{
		return 0.0;
	}

	// Simultaneous Second Derivative at Time t
	virtual
	Real
	s2( Time const ) const
	{
		return 0.0;
	}

	// Simultaneous Third Derivative at Time t
	virtual
	Real
	s3( Time const ) const
	{
		return 0.0;
	}

	// SmoothToken at Time t
	SmoothToken
	k( Time const t ) const
	{
		switch ( order_ ) {
		case 0:
			return SmoothToken::order_0( x( t ), tD );
		case 1:
			return SmoothToken::order_1( x( t ), x1( t ), tD );
		case 2:
			return SmoothToken::order_2( x( t ), x1( t ), x2( t ), tD );
		case 3:
			return SmoothToken::order_3( x( t ), x1( t ), x2( t ), x3( t ), tD );
		default: // Should not happen
			assert( false );
			return SmoothToken();
		}
	}

	// Zero-Crossing Time
	virtual
	Time
	tZC() const
	{
		assert( false ); // Not a ZC variable
		return Time( 0.0 );
	}

	// Observers
	Observers< Variable > const &
	observers() const
	{
		return observers_;
	}

	// Observers
	Observers< Variable > &
	observers()
	{
		return observers_;
	}

	// Observees
	Variables const &
	observees() const
	{
		return observees_;
	}

	// Observees
	Variables &
	observees()
	{
		return observees_;
	}

	// Event Queue
	Events const *
	events() const
	{
		return events_;
	}

	// Event Queue
	Events *
	events()
	{
		return events_;
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

	// Add Observee and its Observer
	void
	observe( Variable * v )
	{
		if ( v == this ) { // Flag as self-observer
			self_observer = true;
		} else {
			observees_.push_back( v );
			v->observers_.push_back( this );
		}
	}

	// Add Zero-Crossing Variable as an Observer
	void
	observe_ZC( Variable * v )
	{
		assert( is_ZC() );
		assert( v != this );
		v->observers_.push_back( this );
	}

	// Initialize Observers Collection
	void
	init_observers()
	{
		observers_.init();
		have_observers_ = observers_.have();
		have_observers_2_ = observers_.have2();
		have_observers_NZ_2_ = observers_.nz_have2();
		have_observers_ZC_2_ = observers_.zc_have2();
	}

	// Initialize Observees Collection
	void
	init_observees()
	{
		if ( ! observees_.empty() ) {
			observees_.erase( std::remove_if( observees_.begin(), observees_.end(), []( Variable * v ){ return v->is_Discrete(); } ), observees_.end() ); // Remove discrete variables: Don't need them after ZC drill-thru observees set up
			std::sort( observees_.begin(), observees_.end() );
			observees_.erase( std::unique( observees_.begin(), observees_.end() ), observees_.end() ); // Remove duplicates
			observees_.shrink_to_fit();
		}
	}

	// Time Initialization
	void
	init_time( Time const t )
	{
		tQ = tX = tE = tN = t;
	}

	// Initialization
	virtual
	void
	init()
	{}

	// Initialization to a Value
	virtual
	void
	init( Real const )
	{}

	// Initialization: Stage 0 ZC
	virtual
	void
	init_0_ZC()
	{
		assert( false ); // Not a ZC variable
	}

	// Initialization: Stage 0
	virtual
	void
	init_0()
	{}

	// Initialization to a Value: Stage 0
	virtual
	void
	init_0( Real const )
	{}

	// Initialization: Stage 1
	virtual
	void
	init_1()
	{}

	// Initialization: Stage 2
	virtual
	void
	init_2()
	{}

	// Initialization: Stage 3
	virtual
	void
	init_3()
	{}

	// Discrete Add Event
	void
	add_discrete( Time const t )
	{
		event_ = events_->add_discrete( t, this );
	}

	// Discrete Shift Event to Time t
	void
	shift_discrete( Time const t )
	{
		event_ = events_->shift_discrete( t, event_ );
	}

	// Discrete Advance
	virtual
	void
	advance_discrete()
	{
		assert( false );
	}

	// Discrete Advance Simultaneous
	virtual
	void
	advance_discrete_simultaneous()
	{
		assert( false );
	}

	// QSS Add Event
	void
	add_QSS( Time const t )
	{
		event_ = events_->add_QSS( t, this );
	}

	// QSS Shift Event to Time t
	void
	shift_QSS( Time const t )
	{
		event_ = events_->shift_QSS( t, event_ );
	}

	// QSS ZC Add Event
	void
	add_QSS_ZC( Time const t )
	{
		event_ = events_->add_QSS_ZC( t, this );
	}

	// QSS ZC Shift Event to Time t
	void
	shift_QSS_ZC( Time const t )
	{
		event_ = events_->shift_QSS_ZC( t, event_ );
	}

	// QSS Advance
	virtual
	void
	advance_QSS()
	{
		assert( false );
	}

	// QSS Advance: Stage 0
	virtual
	void
	advance_QSS_0()
	{
		assert( false );
	}

	// QSS Advance: Stage 1
	virtual
	void
	advance_QSS_1()
	{
		assert( false );
	}

	// QSS Advance: Stage 2
	virtual
	void
	advance_QSS_2()
	{}

	// QSS Advance: Stage 3
	virtual
	void
	advance_QSS_3()
	{}

	// Zero-Crossing Add Event
	void
	add_ZC( Time const t )
	{
		event_ = events_->add_ZC( t, this );
	}

	// Zero-Crossing Shift Event to Time t
	void
	shift_ZC( Time const t )
	{
		event_ = events_->shift_ZC( t, event_ );
	}

	// Zero-Crossing Advance
	virtual
	void
	advance_ZC()
	{
		assert( false ); // Not a ZC variable
	}

	// Handler Add Event
	void
	add_handler()
	{
		event_ = events_->add_handler( this );
	}

	// Handler Shift Event to Time t
	void
	shift_handler( Time const t )
	{
		event_ = events_->shift_handler( t, event_ );
	}

	// Handler Shift Event to Time Infinity
	void
	shift_handler()
	{
		event_ = events_->shift_handler( event_ );
	}

	// Handler Advance
	virtual
	void
	advance_handler( Time const )
	{
		assert( false ); // Not a QSS or discrete variable
	}

	// Handler Advance: Stage 0
	virtual
	void
	advance_handler_0( Time const )
	{
		assert( false ); // Not a QSS or discrete variable
	}

	// Handler Advance: Stage 1
	virtual
	void
	advance_handler_1()
	{
		assert( false ); // Not a QSS variable
	}

	// Handler Advance: Stage 2
	virtual
	void
	advance_handler_2()
	{
		assert( false ); // Not a QSS variable
	}

	// Handler No-Advance
	virtual
	void
	no_advance_handler()
	{
		assert( false ); // Not a QSS or discrete variable
	}

	// Advance Observers: Stage 1
	void
	advance_observers_1()
	{
		observers_.advance_1( tQ );
	}

	// Advance Observers: Stage 2
	void
	advance_observers_2()
	{
		assert( tN == tQ + options::dtNum );
		observers_.advance_2( tN );
	}

	// Advance Observers: Non-Zero-Crossing: Stage 2
	void
	advance_observers_NZ_2()
	{
		assert( tN == tQ + options::dtNum );
		observers_.advance_NZ_2( tN );
	}

	// Advance Observers: Zero-Crossing: Stage 2
	void
	advance_observers_ZC_2()
	{
		assert( tN == tQ + options::dtNum );
		observers_.advance_ZC_2( tN );
	}

	// Advance Observers: Stage d
	void
	advance_observers_d()
	{
		assert( options::output::d );
		observers_.advance_d();
	}

	// Advance Observers
	void
	advance_observers()
	{
		observers_.advance( tQ );
	}

	// Observer Advance: Stage 1
	virtual
	void
	advance_observer_1( Time const )
	{
		assert( false );
	}

	// Observer Advance: Stage 1
	virtual
	void
	advance_observer_1( Time const, Real const )
	{
		assert( false );
	}

	// Zero-Crossing Observer Advance: Stage 1
	virtual
	void
	advance_observer_ZC_1( Time const, Real const, Real const )
	{
		assert( false );
	}

	// Observer Advance: Stage 2
	virtual
	void
	advance_observer_2( Time const )
	{
		assert( false );
	}

	// Observer Advance: Stage 2
	virtual
	void
	advance_observer_2( Time const, Real const )
	{
		assert( false );
	}

	// Observer Advance: Stage d
	virtual
	void
	advance_observer_d() const
	{
		assert( false );
	}

public: // Methods: FMU

	// Get FMU Real Variable Value
	Real
	fmu_get_real() const
	{
		assert( fmu_me != nullptr );
		return fmu_me->get_real( var.ref );
	}

	// Set FMU Real Variable to a Value
	void
	fmu_set_real( Real const v ) const
	{
		assert( fmu_me != nullptr );
		fmu_me->set_real( var.ref, v );
	}

	// Get FMU Integer Variable Value
	Integer
	fmu_get_integer() const
	{
		assert( fmu_me != nullptr );
		return fmu_me->get_integer( var.ref );
	}

	// Set FMU Integer Variable to a Value
	void
	fmu_set_integer( Integer const v ) const
	{
		assert( fmu_me != nullptr );
		fmu_me->set_integer( var.ref, v );
	}

	// Get FMU Boolean Variable Value
	bool
	fmu_get_boolean() const
	{
		assert( fmu_me != nullptr );
		return fmu_me->get_boolean( var.ref );
	}

	// Set FMU Boolean Variable to a Value
	void
	fmu_set_boolean( bool const v ) const
	{
		assert( fmu_me != nullptr );
		fmu_me->set_boolean( var.ref, v );
	}

	// Get FMU Variable Derivative
	Real
	fmu_get_deriv() const
	{
		assert( fmu_me != nullptr );
		return fmu_me->get_real( der.ref );
	}

	// Set FMU Variable to Continuous Value at Time t
	void
	fmu_set_x( Time const t ) const
	{
		assert( fmu_me != nullptr );
		fmu_me->set_real( var.ref, x( t ) );
	}

	// Set FMU Variable to Quantized Value at Time t
	void
	fmu_set_q( Time const t ) const
	{
		assert( fmu_me != nullptr );
		fmu_me->set_real( var.ref, q( t ) );
	}

	// Set FMU Variable to Simultaneous Value at Time t
	void
	fmu_set_s( Time const t ) const
	{
		assert( fmu_me != nullptr );
		fmu_me->set_real( var.ref, s( t ) );
	}

	// Set FMU Variable to Simultaneous Numeric Differentiation Value at Time t
	void
	fmu_set_sn( Time const t ) const
	{
		assert( fmu_me != nullptr );
		fmu_me->set_real( var.ref, sn( t ) );
	}

	// Set All Observee FMU Variables to Continuous Value at Time t
	void
	fmu_set_observees_x( Time const t ) const
	{
		for ( auto observee : observees_ ) {
			observee->fmu_set_x( t );
		}
	}

	// Set All Observee FMU Variables to Quantized Value at Time t
	void
	fmu_set_observees_q( Time const t ) const
	{
		for ( auto observee : observees_ ) {
			observee->fmu_set_q( t );
		}
	}

	// Set All Observee FMU Variables to Simultaneous Value at Time t
	void
	fmu_set_observees_s( Time const t ) const
	{
		for ( auto observee : observees_ ) {
			observee->fmu_set_s( t );
		}
	}

	// Set All Observee FMU Variables to Simultaneous Numeric Differentiation Value at Time t
	void
	fmu_set_observees_sn( Time const t ) const
	{
		for ( auto observee : observees_ ) {
			observee->fmu_set_sn( t );
		}
	}

protected: // Methods

	// Infinite Aligned Time Step Processing
	void
	tE_infinity_tQ()
	{
		if ( dt_inf != infinity ) { // Deactivation control is enabled
			if ( tE == infinity ) { // Deactivation has occurred
				if ( dt_inf_rlx < half_infinity ) { // Relax and use deactivation time step
					dt_inf_rlx *= 2.0;
					tE = tQ + dt_inf_rlx;
				}
			} else { // Reset deactivation time step
				dt_inf_rlx = dt_inf;
			}
		}
	}

	// Infinite Unaligned Time Step Processing
	void
	tE_infinity_tX()
	{
		if ( dt_inf != infinity ) { // Deactivation control is enabled
			if ( tE == infinity ) { // Deactivation has occurred
				if ( dt_inf_rlx < half_infinity ) { // Relax and use deactivation time step
					dt_inf_rlx *= 2.0;
					tE = tX + dt_inf_rlx;
				}
			} else { // Reset deactivation time step
				dt_inf_rlx = dt_inf;
			}
		}
	}

protected: // Data

	int order_{ 0 }; // Method order

public: // Data

	Real rTol{ 1.0e-4 }; // Relative tolerance
	Real aTol{ 1.0e-6 }; // Absolute tolerance
	Real qTol{ 1.0e-6 }; // Quantization tolerance
	Real xIni{ 0.0 }; // Initial value
	Time tQ{ 0.0 }; // Quantized time range begin
	Time tX{ 0.0 }; // Continuous time range begin
	Time tE{ 0.0 }; // Time range end: tQ <= tE and tX <= tE
	Time tN{ 0.0 }; // Numeric differentiation time
	Time tD{ infinity }; // Discrete event time: tQ <= tD and tX <= tD
	Time dt_min{ 0.0 }; // Time step min
	Time dt_max{ infinity }; // Time step max
	Time dt_inf{ infinity }; // Time step inf
	Time dt_inf_rlx{ infinity }; // Relaxed time step inf
	bool self_observer{ false }; // Variable appears in its function/derivative?
	Conditional< Variable > * conditional{ nullptr }; // Conditional (non-owning)
	FMU_ME * fmu_me{ nullptr }; // FMU-ME (non-owning) pointer
	FMU_Variable var; // FMU variables specs
	FMU_Variable der; // FMU derivative specs

protected: // Data

	// Observers
	Observers< Variable > observers_; // Variables dependent on this one
	bool have_observers_{ false }; // Have observers?
	bool have_observers_2_{ false }; // Have order 2+ observers?
	bool have_observers_NZ_2_{ false }; // Have order 2+ non-zero-crossing observers?
	bool have_observers_ZC_2_{ false }; // Have order 2+ zero-crossing observers?

	// Observees
	Variables observees_; // Variables this one depends on

	// Event queue
	Events * events_{ nullptr };

};

} // fmu
} // QSS

#endif
