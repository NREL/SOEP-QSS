// FMU-Based Variable Abstract Base Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2020 Objexx Engineering, Inc. All rights reserved.
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
#include <QSS/Target.hh>
#include <QSS/fmu/Conditional.hh>
#include <QSS/fmu/FMU_ME.hh>
#include <QSS/fmu/FMU_Variable.hh>
#include <QSS/fmu/Observers.hh>
#include <QSS/container.hh>
#include <QSS/globals.hh>
#include <QSS/math.hh>
#include <QSS/options.hh>
#include <QSS/Output.hh>
#include <QSS/SmoothToken.hh>

// FMI Library Headers
#include <fmilib.h>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <limits>
#include <vector>

namespace QSS {
namespace fmu {

// Forward
class Variable_Con;

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
	using EventQ = FMU_ME::EventQ;
	using Variables = std::vector< Variable * >;
	using Variable_Cons = std::vector< Variable_Con * >;
	using VariableRef = fmi2_value_reference_t;
	using VariableRefs = std::vector< VariableRef >;
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

	// Copy Constructor
	Variable( Variable const & ) = default;

	// Move Constructor
	Variable( Variable && ) noexcept = default;

	// Name + Tolerance + Value Constructor
	Variable(
	 int const order,
	 std::string const & name,
	 Real const rTol_,
	 Real const aTol_,
	 Real const xIni_,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Target( name ),
	 order_( order ),
	 is_time_( name == "time" ),
	 rTol( std::max( rTol_, 0.0 ) ),
	 aTol( std::max( aTol_, std::numeric_limits< Real >::min() ) ),
	 xIni( xIni_ ),
	 dt_min( is_time_ ? 0.0 : options::dtMin ),
	 dt_max( is_time_ ? infinity : options::dtMax ),
	 dt_inf_( is_time_ ? infinity : options::dtInf ),
	 dt_inf_rlx_( dt_inf_ == infinity ? infinity : 0.5 * dt_inf_ ),
	 observers_( fmu_me ),
	 fmu_me_( fmu_me ),
	 var_( var ),
	 der_( der ),
	 eventq_( fmu_me->eventq ),
	 out_x_( name, 'x', false ),
	 out_q_( name, 'q', false )
	{}

	// Name + Tolerance Constructor
	Variable(
	 int const order,
	 std::string const & name,
	 Real const rTol_,
	 Real const aTol_,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Target( name ),
	 order_( order ),
	 is_time_( name == "time" ),
	 rTol( std::max( rTol_, 0.0 ) ),
	 aTol( std::max( aTol_, std::numeric_limits< Real >::min() ) ),
	 xIni( 0.0 ),
	 dt_min( is_time_ ? 0.0 : options::dtMin ),
	 dt_max( is_time_ ? infinity : options::dtMax ),
	 dt_inf_( is_time_ ? infinity : options::dtInf ),
	 dt_inf_rlx_( dt_inf_ == infinity ? infinity : 0.5 * dt_inf_ ),
	 observers_( fmu_me ),
	 fmu_me_( fmu_me ),
	 var_( var ),
	 der_( der ),
	 eventq_( fmu_me->eventq ),
	 out_x_( name, 'x', false ),
	 out_q_( name, 'q', false )
	{}

	// Name + Value Constructor
	Variable(
	 int const order,
	 std::string const & name,
	 Real const xIni_,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Target( name ),
	 order_( order ),
	 is_time_( name == "time" ),
	 xIni( xIni_ ),
	 dt_min( is_time_ ? 0.0 : options::dtMin ),
	 dt_max( is_time_ ? infinity : options::dtMax ),
	 dt_inf_( is_time_ ? infinity : options::dtInf ),
	 dt_inf_rlx_( dt_inf_ == infinity ? infinity : 0.5 * dt_inf_ ),
	 observers_( fmu_me ),
	 fmu_me_( fmu_me ),
	 var_( var ),
	 der_( der ),
	 eventq_( fmu_me->eventq ),
	 out_x_( name, 'x', false ),
	 out_q_( name, 'q', false )
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
	 is_time_( name == "time" ),
	 xIni( 0.0 ),
	 dt_min( is_time_ ? 0.0 : options::dtMin ),
	 dt_max( is_time_ ? infinity : options::dtMax ),
	 dt_inf_( is_time_ ? infinity : options::dtInf ),
	 dt_inf_rlx_( dt_inf_ == infinity ? infinity : 0.5 * dt_inf_ ),
	 observers_( fmu_me ),
	 fmu_me_( fmu_me ),
	 var_( var ),
	 der_( der ),
	 eventq_( fmu_me->eventq ),
	 out_x_( name, 'x', false ),
	 out_q_( name, 'q', false )
	{}

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

	// Not Discrete Variable?
	bool
	not_Discrete() const
	{
		return ( ! is_Discrete() );
	}

	// Input Variable?
	virtual
	bool
	is_Input() const
	{
		return false;
	}

	// Not Input Variable?
	bool
	not_Input() const
	{
		return ( ! is_Input() );
	}

	// Connection Input Variable?
	virtual
	bool
	is_connection() const
	{
		return false;
	}

	// Not Connection Input Variable?
	bool
	not_connection() const
	{
		return ( ! is_connection() );
	}

	// Connected?
	bool
	connected() const
	{
		return connected_;
	}

	// QSS Variable?
	virtual
	bool
	is_QSS() const
	{
		return false;
	}

	// Not QSS Variable?
	bool
	not_QSS() const
	{
		return ( ! is_QSS() );
	}

	// State Variable?
	bool
	is_state() const
	{
		return is_QSS();
	}

	// Not State Variable?
	bool
	not_state() const
	{
		return ( ! is_QSS() );
	}

	// LIQSS Variable?
	virtual
	bool
	is_LIQSS() const
	{
		return false;
	}

	// Not LIQSS Variable?
	bool
	not_LIQSS() const
	{
		return ( ! is_LIQSS() );
	}

	// Zero-Crossing Variable?
	virtual
	bool
	is_ZC() const
	{
		return false;
	}

	// Not Zero-Crossing Variable?
	bool
	not_ZC() const
	{
		return ( ! is_ZC() );
	}

	// Explicit Zero-Crossing Variable?
	virtual
	bool
	is_ZCe() const
	{
		return false;
	}

	// Not Explicit Zero-Crossing Variable?
	bool
	not_ZCe() const
	{
		return ( ! is_ZCe() );
	}

	// In Conditional?
	virtual
	bool
	in_conditional() const
	{
		return conditional != nullptr;
	}

	// B|I|D|R Variable?
	virtual
	bool
	is_BIDR() const
	{
		return false;
	}

	// Self-Observer?
	bool
	self_observer() const
	{
		return self_observer_;
	}

	// Observed?
	bool
	observed() const
	{
		return observed_;
	}

	// Observes?
	bool
	observes() const
	{
		return observes_;
	}

public: // Property

	// Order
	int
	order() const
	{
		return order_;
	}

	// State|ZC|Other + Order Sorting Index
	int
	state_order() const
	{
		return order_ + ( is_state() ? 0 : max_rep_order + ( is_ZC() ? 0 : max_rep_order + 1 ) );
	}

	// State Sorting Index
	int
	state_sort_index() const
	{
		return ( is_state() ? 0 : 1 );
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

	// FMU Variable Specs
	FMU_Variable const &
	var() const
	{
		return var_;
	}

	// FMU Variable Specs
	FMU_Variable &
	var()
	{
		return var_;
	}

	// FMU Derivative Specs
	FMU_Variable const &
	der() const
	{
		return der_;
	}

	// FMU Derivative Specs
	FMU_Variable &
	der()
	{
		return der_;
	}

	// Observees
	Variables &
	observees()
	{
		return observees_;
	}

	// Connections
	Variable_Cons const &
	connections() const
	{
		return connections_;
	}

	// Connections
	Variable_Cons &
	connections()
	{
		return connections_;
	}

	// Event Queue
	EventQ const *
	eventq() const
	{
		return eventq_;
	}

	// Event Queue
	EventQ *
	eventq()
	{
		return eventq_;
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

	// Self-Observe
	void
	self_observe()
	{
		self_observer_ = true;
	}

	// Add Observee
	void
	observe_forward( Variable * v )
	{
		if ( v == this ) { // Flag as self-observer
			self_observer_ = true;
		} else {
			observees_.push_back( v );
		}
	}

	// Add Observee and its Observer
	void
	observe( Variable * v )
	{
		if ( v == this ) { // Flag as self-observer
			self_observer_ = true;
		} else {
			observees_.push_back( v );
			v->observers_.add( this );
		}
	}

	// Add Back Observers
	void
	add_back_observers()
	{
		if ( ! observees_.empty() ) {
			for ( Variable * observee : observees_ ) {
				observee->observers_.add( this );
			}
		}
	}

	// Add Drill-Through Observees to Non-State Variables
	void
	add_drill_through_observees()
	{
		assert( not_state() );
		if ( ! observees_.empty() ) {
			for ( Variable * vo : observees_ ) {
				for ( Variable * voo : vo->observees_ ) {
					voo->observers_.add( this ); // Only need back-observer to force updates when observee has observer update
				}
			}
		}
	}

	// Initialize Observers Collection
	void
	init_observers()
	{
		observers_.init();
		observed_ = observers_.have();
		connected_output_observer = observers_.connected_output_observer();
	}

	// Initialize Observees Collection
	void
	init_observees()
	{
		observes_ = ( ! observees_.empty() );
		if ( observes_ ) { // Remove duplicates and discrete variables
			observees_.erase( std::remove_if( observees_.begin(), observees_.end(), []( Variable * v ){ return v->is_Discrete(); } ), observees_.end() ); // Remove discrete variables: Don't need them after ZC drill-thru observees set up
			uniquify( observees_, true ); // Sort by address and remove duplicates and recover unused memory
			observes_ = ( ! observees_.empty() ); // In case all were discrete
		}
	}

	// Connect
	void
	connect()
	{
		connected_ = true;
	}

	// Time Initialization
	void
	init_time( Time const t )
	{
		tQ = tX = tE = t;
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

	// Initialization: Stage 2.1
	virtual
	void
	init_2_1()
	{}

	// Initialization: Stage 3
	virtual
	void
	init_3()
	{}

	// Initialization: Stage Final
	virtual
	void
	init_F()
	{}

	// Discrete Add Event
	void
	add_discrete( Time const t )
	{
		event_ = eventq_->add_discrete( t, this );
	}

	// Discrete Shift Event to Time t
	void
	shift_discrete( Time const t )
	{
		event_ = eventq_->shift_discrete( t, event_ );
	}

	// Discrete Advance
	virtual
	void
	advance_discrete()
	{
		assert( false );
	}

	// Discrete Advance: Simultaneous
	virtual
	void
	advance_discrete_s()
	{
		assert( false );
	}

	// QSS Add Event
	void
	add_QSS( Time const t )
	{
		event_ = eventq_->add_QSS( t, this );
	}

	// QSS Shift Event to Time t
	void
	shift_QSS( Time const t )
	{
		event_ = eventq_->shift_QSS( t, event_ );
	}

	// QSS ZC Add Event
	void
	add_QSS_ZC( Time const t )
	{
		event_ = eventq_->add_QSS_ZC( t, this );
	}

	// QSS ZC Shift Event to Time t
	void
	shift_QSS_ZC( Time const t )
	{
		event_ = eventq_->shift_QSS_ZC( t, event_ );
	}

	// QSS Input Add Event
	void
	add_QSS_Inp( Time const t )
	{
		event_ = eventq_->add_QSS_Inp( t, this );
	}

	// QSS Input Shift Event to Time t
	void
	shift_QSS_Inp( Time const t )
	{
		event_ = eventq_->shift_QSS_Inp( t, event_ );
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

	// QSS Advance: Stage 0
	virtual
	void
	advance_QSS_0( Real const )
	{
		assert( false );
	}

	// QSS Advance: Stage 1
	virtual
	void
	advance_QSS_1( Real const )
	{
		assert( false );
	}

	// QSS Advance: Stage 1
	virtual
	void
	advance_QSS_1( Real const, Real const )
	{
		assert( false );
	}

	// QSS Advance: Stage 2
	virtual
	void
	advance_QSS_2()
	{
		assert( false );
	}

	// QSS Advance: Stage 2
	virtual
	void
	advance_QSS_2( Real const )
	{
		assert( false );
	}

	// QSS Advance: Stage 2.1
	virtual
	void
	advance_QSS_2_1()
	{}

	// QSS Advance: Stage 3
	virtual
	void
	advance_QSS_3( Real const )
	{
		assert( false );
	}

	// QSS Advance: Stage Final
	virtual
	void
	advance_QSS_F()
	{
		assert( false );
	}

	// Zero-Crossing Add Event
	void
	add_ZC( Time const t )
	{
		event_ = eventq_->add_ZC( t, this );
	}

	// Zero-Crossing Shift Event to Time t
	void
	shift_ZC( Time const t )
	{
		event_ = eventq_->shift_ZC( t, event_ );
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
		event_ = eventq_->add_handler( this );
	}

	// Handler Shift Event to Time t
	void
	shift_handler( Time const t )
	{
		event_ = eventq_->shift_handler( t, event_ );
	}

	// Handler Shift Event to Time Infinity
	void
	shift_handler()
	{
		event_ = eventq_->shift_handler( event_ );
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

	// Handler Advance: Stage 2.1
	virtual
	void
	advance_handler_2_1()
	{}

	// Handler Advance: Stage 3
	virtual
	void
	advance_handler_3()
	{
		assert( false ); // Not a QSS variable
	}

	// Handler Advance: Stage Final
	virtual
	void
	advance_handler_F()
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

	// Advance Connections
	void
	advance_connections();

	// Advance Connections for Observer Update
	void
	advance_connections_observer();

	// Advance Observers
	void
	advance_observers()
	{
		observers_.advance( tQ );
	}

	// Observer Advance
	virtual
	void
	advance_observer( Time const )
	{
		assert( false );
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

	// Observer Advance: Stage 1
	virtual
	void
	advance_observer_1_parallel( Time const, Real const )
	{
		assert( false );
	}

	// Observer Advance: Stage 1
	virtual
	void
	advance_observer_1( Time const, Real const, Real const )
	{
		assert( false );
	}

	// Observer Advance: Stage 1
	virtual
	void
	advance_observer_1( Time const, Real const, Real const, Real const )
	{
		assert( false );
	}

	// Observer Advance: Stage 2
	virtual
	void
	advance_observer_2()
	{
		assert( false );
	}

	// Observer Advance: Stage 2
	virtual
	void
	advance_observer_2( Real const )
	{
		assert( false );
	}

	// Observer Advance: Stage 2 Parallel
	virtual
	void
	advance_observer_2_parallel( Real const )
	{
		assert( false );
	}

	// Observer Advance: Stage 3
	virtual
	void
	advance_observer_3( Real const )
	{
		assert( false );
	}

	// Observer Advance: Stage 3 Parallel
	virtual
	void
	advance_observer_3_parallel( Real const )
	{
		assert( false );
	}

	// Observer Advance: Stage Final Parallel
	virtual
	void
	advance_observer_F_parallel()
	{
		assert( false );
	}

	// Observer Advance: Stage Final Serial
	virtual
	void
	advance_observer_F_serial()
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

public: // Methods: Output

	// Output Off
	void
	out_off()
	{
		out_on_ = false;
	}

	// Initialize Outputs
	void
	init_out( std::string const & dir )
	{
		if ( out_on_ ) {
			if ( options::output::X ) out_x_.init( dir, name(), 'x' );
			if ( options::output::Q ) out_q_.init( dir, name(), 'q' );
		}
	}

	// Output at Time t
	void
	out( Time const t )
	{
		if ( out_on_ ) {
			if ( options::output::X ) out_x_.append( t, x( t ) );
			if ( options::output::Q ) out_q_.append( t, q( t ) );
		}
		if ( connected_ ) connections_out( t );
	}

	// Output Quantized at Time t
	void
	out_q( Time const t )
	{
		if ( out_on_ ) {
			if ( options::output::Q ) out_q_.append( t, q( t ) );
		}
		if ( connected_ ) connections_out_q( t );
	}

	// Pre-Event Observer Output at Time t
	void
	observer_out_pre( Time const t )
	{
		if ( out_on_ ) {
			if ( options::output::X && ( ! is_BIDR() ) ) out_x_.append( t, x( t ) );
			if ( options::output::Q && is_ZC() ) out_q_.append( t, q( t ) );
		}
		if ( connected_ ) connections_observer_out_pre( t );
	}

	// Post-Event Observer Output at Time t
	void
	observer_out_post( Time const t )
	{
		if ( is_ZC() || is_BIDR() ) {
			if ( out_on_ ) {
				if ( options::output::X ) out_x_.append( t, x( t ) );
				if ( options::output::Q ) out_q_.append( t, q( t ) );
			}
			if ( connected_ ) connections_observer_out_post( t );
		}
	}

	// Pre-Event Observers Output at Time t
	void
	observers_out_pre( Time const t )
	{
		if ( options::output::O ) {
			for ( Variable * observer : observers_ ) {
				observer->observer_out_pre( t );
			}
		}
	}

	// Post-Event Observers Output at Time t
	void
	observers_out_post( Time const t )
	{
		if ( options::output::O ) {
			for ( Variable * observer : observers_ ) {
				observer->observer_out_post( t );
			}
		}
	}

	// Connections Output at Time t
	void
	connections_out( Time const t );

	// Connections Output at Time t
	void
	connections_out_q( Time const t );

	// Connections Pre-Event Observer Output at Time t
	void
	connections_observer_out_pre( Time const t );

	// Connections Post-Event Observer Output at Time t
	void
	connections_observer_out_post( Time const t );

	// Flush Outputs
	void
	flush_out()
	{
		if ( out_on_ ) {
			if ( options::output::X ) out_x_.flush();
			if ( options::output::Q ) out_q_.flush();
		}
	}

public: // Methods: FMU

	// Get FMU Time
	Time
	fmu_get_time() const
	{
		assert( fmu_me_ != nullptr );
		return fmu_me_->get_time();
	}

	// Set FMU Time
	void
	fmu_set_time( Time const t ) const
	{
		assert( fmu_me_ != nullptr );
		fmu_me_->set_time( t );
	}

	// Get FMU Real Variable Value
	Real
	fmu_get_real() const
	{
		assert( fmu_me_ != nullptr );
		return fmu_me_->get_real( var_.ref );
	}

	// Set FMU Real Variable to a Value
	void
	fmu_set_real( Real const v ) const
	{
		assert( fmu_me_ != nullptr );
		fmu_me_->set_real( var_.ref, v );
	}

	// Get FMU Real Variable Derivative
	Real
	fmu_get_derivative() const
	{
		assert( fmu_me_ != nullptr );
		return fmu_me_->get_real( der_.ref );
	}

	// Get FMU Integer Variable Value
	Integer
	fmu_get_integer() const
	{
		assert( fmu_me_ != nullptr );
		return fmu_me_->get_integer( var_.ref );
	}

	// Set FMU Integer Variable to a Value
	void
	fmu_set_integer( Integer const v ) const
	{
		assert( fmu_me_ != nullptr );
		fmu_me_->set_integer( var_.ref, v );
	}

	// Get FMU Boolean Variable Value
	bool
	fmu_get_boolean() const
	{
		assert( fmu_me_ != nullptr );
		return fmu_me_->get_boolean( var_.ref );
	}

	// Set FMU Boolean Variable to a Value
	void
	fmu_set_boolean( bool const v ) const
	{
		assert( fmu_me_ != nullptr );
		fmu_me_->set_boolean( var_.ref, v );
	}

	// Get FMU Variable Value as Real
	Real
	fmu_get_as_real() const
	{
		assert( fmu_me_ != nullptr );
		return fmu_me_->get_as_real( var_ );
	}

	// Set FMU Variable to Continuous Value at Time t
	virtual
	void
	fmu_set_x( Time const t ) const
	{
		assert( fmu_me_ != nullptr );
		fmu_me_->set_real( var_.ref, x( t ) );
	}

	// Set FMU Variable to Quantized Value at Time t
	virtual
	void
	fmu_set_q( Time const t ) const
	{
		assert( fmu_me_ != nullptr );
		fmu_me_->set_real( var_.ref, q( t ) );
	}

protected: // Methods: FMU

	// Set All Observee FMU Variables to Continuous Value at Time t
	void
	fmu_set_observees_x( Time const t ) const
	{
		for ( auto observee : observees_ ) {
			observee->fmu_set_x( t );
		}
	}

	// Set All Observee FMU Variables to Continuous Value at Time t Except for Specified Variable
	void
	fmu_set_observees_x( Time const t, Variable const * const var ) const
	{
		for ( auto observee : observees_ ) {
			if ( ( observee != var ) || ( var->fmu_get_as_real() == var->x_0_bump ) ) observee->fmu_set_x( t );
		}
	}

	// Set All Observee FMU Variables to Continuous Value at Time t Except for Specified Variables
	void
	fmu_set_observees_x( Time const t, Variables const & vars ) const
	{
		for ( auto observee : observees_ ) {
			Variables::const_iterator const i( std::find( vars.begin(), vars.end(), observee ) );
			if ( ( i == vars.end() ) || ( (*i)->fmu_get_as_real() == (*i)->x_0_bump ) ) observee->fmu_set_x( t );
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

	// Coefficient 0 from FMU: Observees Set
	Boolean
	bp_0() const
	{
		assert( fmu_me_ != nullptr );
		return fmu_me_->get_boolean( var_.ref );
	}

	// Coefficient 0 from FMU at Time t: X-Based
	Boolean
	bz_0( Time const t ) const
	{
		fmu_set_observees_x( t );
		return bp_0();
	}

	// Coefficient 0 from FMU: Observees Set
	Integer
	ip_0() const
	{
		assert( fmu_me_ != nullptr );
		return fmu_me_->get_integer( var_.ref );
	}

	// Coefficient 0 from FMU at Time t: X-Based
	Integer
	iz_0( Time const t ) const
	{
		fmu_set_observees_x( t );
		return ip_0();
	}

	// Coefficient 0 from FMU: Observees Set
	Real
	p_0() const
	{
		assert( fmu_me_ != nullptr );
		return fmu_me_->get_real( var_.ref );
	}

	// Coefficient 0 from FMU at Time tQ: X-Based
	Real
	z_0() const
	{
		assert( is_ZC() ); // Zero-crossing variable
		fmu_set_observees_x( tQ );
		return p_0();
	}

	// Coefficient 0 from FMU at Time t: X-Based
	Real
	z_0( Time const t ) const
	{
		fmu_set_observees_x( t );
		return p_0();
	}

	// Zero Coefficient 0: X-Based Setup
	Real
	z_x() const
	{
		assert( is_ZCe() ); // Explicit zero-crossing variable
		fmu_set_observees_x( tQ );
		return 0.0;
	}

	// Coefficient 1 from FMU: Observees Set
	Real
	p_1() const
	{
		assert( fmu_me_ != nullptr );
		return fmu_me_->get_real( der_.ref );
	}

	// Coefficient 1 from FMU at Time t: QSS
	Real
	c_1( Time const t ) const
	{
		fmu_set_observees_q( t );
		if ( self_observer_ ) fmu_set_q( t );
		return p_1();
	}

	// Coefficient 1 from FMU at Time tQ: QSS
	Real
	c_1( Time const t, Real const q_0 ) const
	{
		assert( t == tQ );
		fmu_set_observees_q( t );
		if ( self_observer_ ) fmu_set_real( q_0 );
		return p_1();
	}

	// Coefficient 1 from FMU at Time tQ: No Self-Observer Assignment
	Real
	h_1() const
	{
		fmu_set_observees_q( tQ );
		return p_1();
	}

	// Coefficient 1 from FMU at Time t: No Self-Observer Assignment
	Real
	h_1( Time const t ) const
	{
		fmu_set_observees_q( t );
		return p_1();
	}

	// Coefficient 1 from FMU at Time t: X-Based
	Real
	z_1( Time const t ) const
	{
		assert( is_ZCe() ); // Explicit zero-crossing variable
		fmu_set_observees_x( t );
		return p_1();
	}

	// Coefficient 1 from FMU at Time t: X-Based with ND First Derivative
	Real
	Z_1( Time const t, Real const x_0 ) const
	{
		assert( is_ZC() && not_ZCe() ); // For event-indicator based zero-crossing variables only
		Time const tN( t + options::dtND );
		fmu_set_time( tN );
		Real const x_1( options::one_over_dtND * ( z_0( tN ) - x_0 ) ); //ND Forward Euler
		fmu_set_time( t );
		return x_1;
	}

	// Coefficient 2 from FMU: Given Derivative
	Real
	p_2( Real const d, Real const x_1 ) const
	{
		return options::one_over_two_dtND * ( d - x_1 ); //ND Forward Euler
	}

	// Coefficient 2 from FMU at Time t
	Real
	c_2( Time const t, Real const x_1 ) const
	{
		Time const tN( t + options::dtND );
		fmu_set_time( tN );
		Real const x_2( options::one_over_two_dtND * ( c_1( tN ) - x_1 ) ); //ND Forward Euler
		fmu_set_time( t );
		return x_2;
	}

	// Coefficient 2 from FMU at Time t: No Self-Observer Check/Set
	Real
	h_2( Time const t, Real const x_1 ) const
	{
		Time const tN( t + options::dtND );
		fmu_set_time( tN );
		Real const x_2( options::one_over_two_dtND * ( h_1( tN ) - x_1 ) ); //ND Forward Euler
		fmu_set_time( t );
		return x_2;
	}

	// Coefficient 2 from FMU at Time tQ: X-Based
	Real
	z_2( Real const x_1 ) const
	{
		assert( is_ZCe() ); // Explicit zero-crossing variable
		Time const tN( tQ + options::dtND );
		fmu_set_time( tN );
		Real const x_2( options::one_over_two_dtND * ( z_1( tN ) - x_1 ) ); //ND Forward Euler
		fmu_set_time( tQ );
		return x_2;
	}

	// Coefficient 3 from FMU at Time t
	Real
	c_3( Time const t, Real const x_1 ) const
	{
		Time tN( t - options::dtND );
		fmu_set_time( tN );
		Real const x_1_m( c_1( tN ) );
		tN = t + options::dtND;
		fmu_set_time( tN );
		Real const x_1_p( c_1( tN ) );
		fmu_set_time( tQ );
		return options::one_over_six_dtND_squared * ( ( x_1_p - x_1 ) + ( x_1_m - x_1 ) ); //ND Centered difference
	}

protected: // Methods

	// Infinite Aligned Time Step Processing
	Time
	dt_infinity( Time const dt )
	{
		if ( is_time_ ) return dt;
		if ( dt_inf_ != infinity ) { // Deactivation control is enabled
			if ( dt == infinity ) { // Deactivation has occurred
				if ( dt_inf_rlx_ < half_infinity ) { // Relax and use deactivation time step
					return ( dt_inf_rlx_ *= 2.0 );
				} else {
					return dt;
				}
			} else { // Reset deactivation time step
				dt_inf_rlx_ = dt_inf_;
				return dt;
			}
		} else {
			return dt;
		}
	}

private: // Data

	int order_{ 0 }; // Method order
	bool is_time_{ false }; // Time variable?

public: // Data

	Real rTol{ 1.0e-4 }; // Relative tolerance
	Real aTol{ 1.0e-6 }; // Absolute tolerance
	Real qTol{ 1.0e-6 }; // Quantization tolerance
	Real xIni{ 0.0 }; // Initial value
	Time tQ{ 0.0 }; // Quantized time range begin
	Time tX{ 0.0 }; // Continuous time range begin
	Time tE{ 0.0 }; // Time range end: tQ <= tE and tX <= tE
	Time tD{ infinity }; // Discrete event time: tQ <= tD and tX <= tD
	Time dt_min{ 0.0 }; // Time step min
	Time dt_max{ infinity }; // Time step max
	Conditional< Variable > * conditional{ nullptr }; // Conditional (non-owning)
	Real x_0_bump{ 0.0 }; // Bumped value

private: // Data

	// Time steps
	Time dt_inf_{ infinity }; // Time step inf
	Time dt_inf_rlx_{ infinity }; // Relaxed time step inf

	// Observers
	Observers< Variable > observers_; // Variables dependent on this one
	bool observed_{ false }; // Has observers?
	bool self_observer_{ false }; // Appears in its function/derivative?

	// Observees
	Variables observees_; // Variables this one depends on
	bool observes_{ false }; // Has observees?

	// Connections
	Variable_Cons connections_; // Input connection variables this one outputs to
	bool connected_{ false }; // Have connection(s)?

	// FMU
	FMU_ME * fmu_me_{ nullptr }; // FMU-ME
	FMU_Variable var_; // FMU variables specs
	FMU_Variable der_; // FMU derivative specs
	EventQ * eventq_{ nullptr }; // FMU event queue

	// Outputs
	bool out_on_{ true }; // Output on?
	Output out_x_; // Continuous rep output
	Output out_q_; // Quantized rep output

}; // Variable

} // fmu
} // QSS

#endif
