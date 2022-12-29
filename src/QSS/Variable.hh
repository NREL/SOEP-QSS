// Variable Abstract Base Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2023 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_Variable_hh_INCLUDED
#define QSS_Variable_hh_INCLUDED

// QSS Headers
#include <QSS/Variable.fwd.hh>
#include <QSS/Target.hh>
#include <QSS/container.hh>
#include <QSS/FMU_ME.hh>
#include <QSS/FMU_Variable.hh>
#include <QSS/globals.hh>
#include <QSS/math.hh>
#include <QSS/Observers.hh>
#include <QSS/options.hh>
#include <QSS/Output.hh>
#include <QSS/SmoothToken.hh>
#include <QSS/string.hh>

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
#include <unordered_set>
#include <utility>
#include <vector>

namespace QSS {

// Forward
class Variable_Con;

// Variable Abstract Base Class
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
	using VariablesSet = std::unordered_set< Variable * >;
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
	 FMU_ME * fmu_me,
	 int const order,
	 std::string const & name,
	 Real const rTol_ = options::rTol,
	 Real const aTol_ = options::aTol,
	 Real const zTol_ = options::zTol,
	 Real const xIni_ = 0.0,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Target( name ),
	 order_( order ),
	 is_time_( name == "time" ),
	 rTol( std::max( rTol_, 0.0 ) ),
	 aTol( std::max( aTol_, std::numeric_limits< Real >::min() ) ),
	 zTol( std::max( zTol_, 0.0 ) ),
	 xIni( xIni_ ),
	 dt_min( is_time_ ? 0.0 : options::dtMin ),
	 dt_max( is_time_ ? infinity : options::dtMax ),
	 dt_inf_( is_time_ ? infinity : options::dtInf ),
	 dt_inf_rlx_( dt_inf_ == infinity ? infinity : 0.5 * dt_inf_ ),
	 observers_( fmu_me, this ),
	 fmu_me_( fmu_me ),
	 var_( var ),
	 der_( der ),
	 eventq_( fmu_me->eventq ),
	 out_x_( name, 'x', false ),
	 out_q_( name, 'q', false )
	{}

	// Name + Tolerance + Value Constructor
	Variable(
	 FMU_ME * fmu_me,
	 int const order,
	 std::string const & name,
	 Real const rTol_ = options::rTol,
	 Real const aTol_ = options::aTol,
	 Real const xIni_ = 0.0,
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
	 observers_( fmu_me, this ),
	 fmu_me_( fmu_me ),
	 var_( var ),
	 der_( der ),
	 eventq_( fmu_me->eventq ),
	 out_x_( name, 'x', false ),
	 out_q_( name, 'q', false )
	{}

	// Name + Value Constructor
	Variable(
	 FMU_ME * fmu_me,
	 int const order,
	 std::string const & name,
	 Real const xIni_ = 0.0,
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
	 observers_( fmu_me, this ),
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

	// Real Variable?
	virtual
	bool
	is_Real() const
	{
		return false;
	}

	// Integer Variable?
	virtual
	bool
	is_Integer() const
	{
		return false;
	}

	// Boolean Variable?
	virtual
	bool
	is_Boolean() const
	{
		return false;
	}

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
		return !is_Discrete();
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
		return !is_Input();
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
		return !is_connection();
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
		return !is_QSS();
	}

	// State Variable?
	bool
	is_State() const
	{
		return is_QSS();
	}

	// Not State Variable?
	bool
	not_State() const
	{
		return !is_QSS();
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
		return !is_QSS();
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
		return !is_LIQSS();
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
		return !is_ZC();
	}

	// B|I|D|R Variable?
	virtual
	bool
	is_BIDR() const
	{
		return false;
	}

	// D Variable?
	virtual
	bool
	is_D() const
	{
		return false;
	}

	// R Variable?
	virtual
	bool
	is_R() const
	{
		return false;
	}

	// Active Variable?
	virtual
	bool
	is_Active() const
	{
		return true;
	}

	// Passive Variable?
	bool
	is_Passive() const
	{
		return !is_Active();
	}

	// Time Variable?
	bool
	is_time() const
	{
		return is_time_;
	}

	// Self-Observer?
	bool
	self_observer() const
	{
		return self_observer_;
	}

	// Self-Observee?
	bool
	self_observee() const
	{
		return self_observer_; // Self-observer <=> self-observee for QSS purposes
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

	// Forward Time?
	bool
	fwd_time( Time const t ) const
	{
		return t >= t0();
	}

	// Forward Time With ND Back Step?
	bool
	fwd_time_ND( Time const t ) const
	{
		return t - options::dtND >= t0();
	}

	// Unpredicted Crossing Detected?
	virtual
	bool
	detected_crossing() const
	{
		assert( false );
		return false;
	}

public: // Property

	// Order
	int
	order() const
	{
		return order_;
	}

	// Variable Type + Order Sorting Index
	int
	var_sort_index() const
	{
		// Indexes with max_rep_order == 3:  State:1-3  R:4-6  Discrete:7  ZC:8-10
		return order_ + ( is_state() ? 0 : max_rep_order + ( is_R() ? 0 : max_rep_order + 1 ) ); // Order: State -> R -> Discrete -> ZC
	}

	// State Sorting Index
	int
	state_sort_index() const
	{
		return ( is_state() ? 0 : 1 );
	}

	// Output File Name Decoration
	std::string const &
	decoration() const
	{
		return dec_;
	}

	// Boolean Value
	virtual
	Boolean
	b() const
	{
		assert( false );
		return false;
	}

	// Boolean Value at Time t
	virtual
	Boolean
	b( Time const ) const
	{
		assert( false );
		return false;
	}

	// Integer Value
	virtual
	Integer
	i() const
	{
		assert( false );
		return 0;
	}

	// Integer Value at Time t
	virtual
	Integer
	i( Time const ) const
	{
		assert( false );
		return 0;
	}

	// Real Value
	virtual
	Real
	r() const
	{
		assert( false );
		return 0.0;
	}

	// Real Value at Time t
	virtual
	Real
	r( Time const ) const
	{
		assert( false );
		return 0.0;
	}

	// Continuous Value at Time t
	virtual
	Real
	x( Time const ) const
	{
		assert( false );
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
		assert( false );
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

	// Start Time
	Time
	t0() const
	{
		assert( fmu_me_ != nullptr );
		return fmu_me_->t0;
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

	// Self-Observe
	void
	self_observe_off()
	{
		self_observer_ = false;
	}

	// Add Observee and its Observer
	void
	observe( Variable * v );

	// Initialize Observees
	void
	init_observees();

	// Initialize Observers
	void
	init_observers();

	// Finalize Observers
	void
	finalize_observers();

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

	// Initialization: Stage 0
	virtual
	void
	init_0()
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
	advance_discrete_simultaneous()
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

	// QSS R Add Event
	void
	add_QSS_R( Time const t )
	{
		event_ = eventq_->add_QSS_R( t, this );
	}

	// QSS R Shift Event to Time t
	void
	shift_QSS_R( Time const t )
	{
		event_ = eventq_->shift_QSS_R( t, event_ );
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

	// QSS Advance: Stage 1
	virtual
	void
	advance_QSS_1_forward( Real const, Real const )
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

	// QSS Advance: Stage 2: Forward ND
	virtual
	void
	advance_QSS_2_forward()
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

	// QSS Advance: Stage 2
	virtual
	void
	advance_QSS_2( Real const, Real const )
	{
		assert( false );
	}

	// QSS Advance: Stage 2: Forward ND
	virtual
	void
	advance_QSS_2_forward( Real const, Real const )
	{
		assert( false );
	}

	// QSS Advance: Stage 3
	virtual
	void
	advance_QSS_3()
	{
		assert( false );
	}

	// QSS Advance: Stage 3: Forward ND
	virtual
	void
	advance_QSS_3_forward()
	{
		assert( false );
	}

	// QSS Advance: Stage 3
	virtual
	void
	advance_QSS_3( Real const )
	{
		assert( false );
	}

	// QSS Advance: Stage 3: Forward ND
	virtual
	void
	advance_QSS_3_forward( Real const )
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

	// QSS Advance LIQSS/QSS Step Ratio
	virtual
	Real
	advance_LIQSS_QSS_step_ratio()
	{
		return 1.0;
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
		assert( false );
	}

	// Handler Advance: Stage 0
	virtual
	void
	advance_handler_0( Time const, Real const )
	{
		assert( false );
	}

	// Handler Advance: Stage 1
	virtual
	void
	advance_handler_1( Real const )
	{
		assert( false );
	}

	// Handler Advance: Stage 2
	virtual
	void
	advance_handler_2( Real const )
	{
		assert( false );
	}

	// Handler Advance: Stage 2
	virtual
	void
	advance_handler_2( Real const, Real const )
	{
		assert( false );
	}

	// Handler Advance: Stage 2: Forward ND
	virtual
	void
	advance_handler_2_forward( Real const, Real const )
	{
		assert( false );
	}

	// Handler Advance: Stage 3
	virtual
	void
	advance_handler_3()
	{
		assert( false );
	}

	// Handler Advance: Stage 3: Forward ND
	virtual
	void
	advance_handler_3_forward()
	{
		assert( false );
	}

	// Handler Advance: Stage Final
	virtual
	void
	advance_handler_F()
	{
		assert( false );
	}

	// Handler No-Advance
	virtual
	void
	no_advance_handler()
	{
		assert( false );
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

	// Observer Advance: Stage 1
	virtual
	void
	advance_observer_1_forward( Time const, Real const, Real const, Real const )
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

	// Observer Advance: Stage 2: Forward ND
	virtual
	void
	advance_observer_2_forward()
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

	// Observer Advance: Stage 2
	virtual
	void
	advance_observer_2( Real const, Real const )
	{
		assert( false );
	}

	// Observer Advance: Stage 2
	virtual
	void
	advance_observer_2_forward( Real const, Real const )
	{
		assert( false );
	}

	// Observer Advance: Stage 3
	virtual
	void
	advance_observer_3()
	{
		assert( false );
	}

	// Observer Advance: Stage 3: Forward ND
	virtual
	void
	advance_observer_3_forward()
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

	// Observer Advance: Stage 3
	virtual
	void
	advance_observer_3_forward( Real const )
	{
		assert( false );
	}

	// Observer Advance: Stage Final
	virtual
	void
	advance_observer_F()
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

	// Output Turn Off
	void
	out_turn_off()
	{
		out_on_ = false;
	}

	// Output Turn On
	void
	out_turn_on()
	{
		out_on_ = true;
	}

	// Decorate Outputs
	void
	decorate_out( std::string const & dec = std::string() );

	// Initialize Outputs
	void
	init_out( std::string const & dir, std::string const & dec = std::string() );

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
			if ( options::output::X ) out_x_.append( t, x( t ) );
			if ( options::output::Q ) out_q_.append( t, q( t ) );
		}
		if ( connected_ ) connections_observer_out_pre( t );
	}

	// Post-Event Observer Output at Time t
	void
	observer_out_post( Time const t )
	{
		if ( not_state() ) { // State observers derivative may change but not value
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
		return fmu_me_->get_real( var_.ref() );
	}

	// Set FMU Real Variable to a Value
	void
	fmu_set_real( Real const v ) const
	{
		assert( fmu_me_ != nullptr );
		assert( is_QSS() || is_Input() );
		fmu_me_->set_real( var_.ref(), v );
	}

	// Get FMU Real Variable Derivative
	Real
	fmu_get_derivative() const
	{
		assert( fmu_me_ != nullptr );
		return fmu_me_->get_real( der_.ref() );
	}

	// Get FMU Integer Variable Value
	Integer
	fmu_get_integer() const
	{
		assert( fmu_me_ != nullptr );
		return fmu_me_->get_integer( var_.ref() );
	}

	// Set FMU Integer Variable to a Value
	void
	fmu_set_integer( Integer const v ) const
	{
		assert( fmu_me_ != nullptr );
		assert( is_Input() );
		fmu_me_->set_integer( var_.ref(), v );
	}

	// Get FMU Boolean Variable Value
	bool
	fmu_get_boolean() const
	{
		assert( fmu_me_ != nullptr );
		return fmu_me_->get_boolean( var_.ref() );
	}

	// Set FMU Boolean Variable to a Value
	void
	fmu_set_boolean( bool const v ) const
	{
		assert( fmu_me_ != nullptr );
		assert( is_Input() );
		fmu_me_->set_boolean( var_.ref(), v );
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
		fmu_me_->set_real( var_.ref(), x( t ) );
	}

	// Set FMU Variable to Quantized Value at Time t
	virtual
	void
	fmu_set_q( Time const t ) const
	{
		assert( fmu_me_ != nullptr );
		fmu_me_->set_real( var_.ref(), q( t ) );
	}

	// Set FMU Variable to Appropriate Value at Time t
	virtual
	void
	fmu_set_s( Time const t ) const
	{
		assert( fmu_me_ != nullptr );
#ifndef QSS_STATE_PROPAGATE_CONTINUOUS
		fmu_me_->set_real( var_.ref(), q( t ) ); // Quantized: Traditional QSS
#else
		fmu_me_->set_real( var_.ref(), x( t ) ); // Continuous: Modified QSS
#endif
	}

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

	// Set All Observee FMU Variables to Appropriate Value at Time t
	void
	fmu_set_observees_s( Time const t ) const
	{
		assert( is_QSS() );
		for ( auto observee : observees_ ) {
			observee->fmu_set_s( t );
		}
	}

protected: // Methods: FMU

	// Boolean Value from FMU at Time tQ: Don't Set Observees
	Boolean
	b_f() const
	{
		assert( is_Boolean() );
		assert( fmu_get_time() == tQ );
		return fmu_get_boolean();
	}

	// Boolean Value from FMU at Time tQ: X-Based
	Boolean
	b_0() const
	{
		assert( is_Boolean() );
		assert( fmu_get_time() == tQ );
		fmu_set_observees_x( tQ );
		return fmu_get_boolean();
	}

	// Boolean Value from FMU at Time t: X-Based
	Boolean
	b_0( Time const t ) const
	{
		assert( is_Boolean() );
		assert( fmu_get_time() == t );
		fmu_set_observees_x( t );
		return fmu_get_boolean();
	}

	// Integer Coefficient 0 from FMU at Time tQ: Don't Set Observees
	Integer
	i_f() const
	{
		assert( is_Integer() );
		assert( fmu_get_time() == tQ );
		return fmu_get_integer();
	}

	// Integer Coefficient 0 from FMU at Time tQ: X-Based
	Integer
	i_0() const
	{
		assert( is_Integer() );
		assert( fmu_get_time() == tQ );
		fmu_set_observees_x( tQ );
		return fmu_get_integer();
	}

	// Integer Value from FMU at Time t: X-Based
	Integer
	i_0( Time const t ) const
	{
		assert( is_Integer() );
		assert( fmu_get_time() == t );
		fmu_set_observees_x( t );
		return fmu_get_integer();
	}

	// Discrete Value from FMU at Time tQ: Don't Set Observees
	Real
	d_f() const
	{
		assert( is_D() );
		assert( fmu_get_time() == tQ );
		return fmu_get_real();
	}

	// Discrete Value from FMU at Time tQ: X-Based
	Real
	d_0() const
	{
		assert( is_D() );
		assert( fmu_get_time() == tQ );
		fmu_set_observees_x( tQ );
		return fmu_get_real();
	}

	// Discrete Value from FMU at Time t: X-Based
	Real
	d_0( Time const t ) const
	{
		assert( is_D() );
		assert( fmu_get_time() == t );
		fmu_set_observees_x( t );
		return fmu_get_real();
	}

	// Real Value from FMU at Time tQ: Don't Set Observees
	Real
	r_f() const
	{
		assert( is_R() );
		assert( fmu_get_time() == tQ );
		return fmu_get_real();
	}

	// Real Value from FMU at Time tQ: X-Based
	Real
	r_0() const
	{
		assert( is_R() );
		assert( fmu_get_time() == tQ );
		fmu_set_observees_x( tQ );
		return fmu_get_real();
	}

	// Real Value from FMU at Time t: X-Based
	Real
	r_0( Time const t ) const
	{
		assert( is_R() );
		assert( fmu_get_time() == t );
		fmu_set_observees_x( t );
		return fmu_get_real();
	}

	// Value from FMU: Don't Set Observees
	Real
	p_0() const
	{
		return fmu_get_real();
	}

	// Value from FMU at Time tQ: QSS
	Real
	c_0() const
	{
		assert( is_QSS() );
		assert( fmu_get_time() == tQ );
		fmu_set_observees_s( tQ );
		if ( self_observer_ ) fmu_set_s( tQ );
		return p_0();
	}

	// Value from FMU at Time t: QSS
	Real
	c_0( Time const t ) const
	{
		assert( is_QSS() );
		assert( fmu_get_time() == t );
		fmu_set_observees_s( t );
		if ( self_observer_ ) fmu_set_s( t );
		return p_0();
	}

	// Value from FMU at Time tQ: X-Based
	Real
	z_0() const
	{
		assert( is_ZC() );
		assert( fmu_get_time() == tQ );
		fmu_set_observees_x( tQ );
		return p_0();
	}

	// Value from FMU at Time t: X-Based
	Real
	z_0( Time const t ) const
	{
		assert( is_ZC() );
		assert( fmu_get_time() == t );
		fmu_set_observees_x( t );
		return p_0();
	}

	// Coefficient 1 from FMU: Observees Set
	Real
	p_1() const
	{
		assert( fmu_me_ != nullptr );
		return fmu_me_->get_real( der_.ref() );
	}

	// Coefficient 1 from FMU at Time t: QSS
	Real
	c_1( Time const t ) const
	{
		assert( is_QSS() );
		assert( fmu_get_time() == t );
		fmu_set_observees_s( t );
		if ( self_observer_ ) fmu_set_s( t );
		return p_1();
	}

	// Coefficient 1 from FMU at Time t: QSS
	Real
	c_1( Time const t, Real const x_0 ) const
	{
		assert( is_QSS() );
		assert( fmu_get_time() == t );
		fmu_set_observees_s( t );
		if ( self_observer_ ) fmu_set_real( x_0 );
		return p_1();
	}

	// Coefficient 1 from FMU at Time tQ: QSS: No Self-Observer Assignment
	Real
	h_1() const
	{
		assert( is_QSS() );
		assert( fmu_get_time() == tQ );
		fmu_set_observees_s( tQ );
		return p_1();
	}

	// Coefficient 1 from FMU at Time t: QSS: No Self-Observer Assignment
	Real
	h_1( Time const t ) const
	{
		assert( is_QSS() );
		assert( fmu_get_time() == t );
		fmu_set_observees_s( t );
		return p_1();
	}

	// Coefficient 1 from FMU at Time tQ: X-Based ZC with Directional First Derivative
	Real
	Z_1() const
	{
		assert( is_ZC() );
		assert( !self_observer_ );
		assert( !observed_ );
		assert( fmu_me_ != nullptr );
		assert( fmu_get_time() == tQ );
		assert( observees_nv_ == observees_v_ref_.size() );
		assert( observees_nv_ == observees_dv_.size() );
		assert( observees_nv_ == observees_.size() ); // ZC variables can't be self-observers
		fmu_set_observees_x( tQ ); // Modelon indicates that observee state matters for Jacobian computation
		for ( Variables::size_type i = 0, e = observees_.size(); i < e; ++i ) { // Get observee derivatives
			observees_dv_[ i ] = observees_[ i ]->x1( tQ );
		}
		return fmu_me_->get_directional_derivative( observees_v_ref_.data(), observees_nv_, var_.ref(), observees_dv_.data() );
	}

	// Coefficient 1 from FMU at Time t: X-Based ZC with Directional First Derivative
	Real
	Z_1( Time const t ) const
	{
		assert( is_ZC() );
		assert( !self_observer_ );
		assert( !observed_ );
		assert( fmu_me_ != nullptr );
		assert( fmu_get_time() == t );
		assert( observees_nv_ == observees_v_ref_.size() );
		assert( observees_nv_ == observees_dv_.size() );
		assert( observees_nv_ == observees_.size() ); // ZC variables can't be self-observers
		fmu_set_observees_x( t ); // Modelon indicates that observee state matters for Jacobian computation
		for ( Variables::size_type i = 0, e = observees_.size(); i < e; ++i ) { // Get observee derivatives
			observees_dv_[ i ] = observees_[ i ]->x1( t );
		}
		return fmu_me_->get_directional_derivative( observees_v_ref_.data(), observees_nv_, var_.ref(), observees_dv_.data() );
	}

	// Coefficient 1 from FMU at Time tQ: X-Based with Directional First Derivative
	Real
	R_1() const
	{
		assert( is_R() );
		assert( !self_observer_ );
		assert( fmu_me_ != nullptr );
		assert( fmu_get_time() == tQ );
		assert( observees_nv_ == observees_v_ref_.size() );
		assert( observees_nv_ == observees_dv_.size() );
		assert( observees_nv_ == observees_.size() ); // Self-observee not used with directional derivatives
		fmu_set_observees_x( tQ ); // Modelon indicates that observee state matters for Jacobian computation
		for ( Variables::size_type i = 0, e = observees_.size(); i < e; ++i ) { // Get observee derivatives
			observees_dv_[ i ] = observees_[ i ]->x1( tQ );
		}
		return fmu_me_->get_directional_derivative( observees_v_ref_.data(), observees_nv_, var_.ref(), observees_dv_.data() );
	}

	// Coefficient 1 from FMU at Time t: X-Based with Directional First Derivative
	Real
	R_1( Time const t ) const
	{
		assert( is_R() );
		assert( !self_observer_ );
		assert( fmu_me_ != nullptr );
		assert( fmu_get_time() == t );
		assert( observees_nv_ == observees_v_ref_.size() );
		assert( observees_nv_ == observees_dv_.size() );
		assert( observees_nv_ == observees_.size() ); // Self-observee not used with directional derivatives
		fmu_set_observees_x( t ); // Modelon indicates that observee state matters for Jacobian computation
		for ( Variables::size_type i = 0, e = observees_.size(); i < e; ++i ) { // Get observee derivatives
			observees_dv_[ i ] = observees_[ i ]->x1( t );
		}
		return fmu_me_->get_directional_derivative( observees_v_ref_.data(), observees_nv_, var_.ref(), observees_dv_.data() );
	}

	// Coefficient 2 from FMU: Given Derivative
	Real
	p_2( Real const x_1, Real const x_1_p ) const
	{
		return options::one_over_two_dtND * ( x_1_p - x_1 ); //ND Forward Euler
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

	// Coefficient 2 from FMU at Time tQ: X-Based ZC with Directional First Derivative
	Real
	Z_2( Real const x_1 ) const
	{
		assert( is_ZC() );
		Time const tN( tQ + options::dtND );
		fmu_set_time( tN );
		Real const x_2( options::one_over_two_dtND * ( Z_1( tN ) - x_1 ) ); //ND Forward Euler
		fmu_set_time( tQ );
		return x_2;
	}

	// Coefficient 2 from FMU at Time tQ: X-Based with Directional First Derivative
	Real
	R_2( Real const x_1 ) const
	{
		assert( is_R() );
		Time const tN( tQ + options::dtND );
		fmu_set_time( tN );
		Real const x_2( options::one_over_two_dtND * ( R_1( tN ) - x_1 ) ); //ND Forward Euler
		fmu_set_time( tQ );
		return x_2;
	}

	// Coefficient 3 from FMU at Time t
	Real
	f_3( Time const t, Real const x_1 ) const
	{
		Time tN( t + options::dtND );
		fmu_set_time( tN );
		Real const x_1_p( c_1( tN ) );
		tN = t + options::two_dtND;
		fmu_set_time( tN );
		Real const x_1_2p( c_1( tN ) );
		fmu_set_time( tQ );
		return options::one_over_six_dtND_squared * ( ( x_1_2p - x_1_p ) + ( x_1 - x_1_p ) ); //ND 3-point formula
	}

protected: // Methods

	// Infinite Aligned Time Step Processing
	Time
	dt_infinity( Time const dt ) const
	{
		if ( ( dt_inf_ == infinity ) || is_time_ ) return dt; // Deactivation control is not enabled
		if ( dt >= dt_inf_ ) { // Apply deactivation control
			return ( dt_inf_rlx_ < half_infinity ? std::min( dt_inf_rlx_ *= 2.0, dt ) : dt ); // Use min of relaxed deactivation time step and dt
		} else { // Reset relaxed deactivation time step
			dt_inf_rlx_ = dt_inf_;
			return dt;
		}
	}

private: // Methods

	// Find Short-Circuited Computational State and Input Observees
	void
	find_computational_observees_of(
	 Variables & observees,
	 VariablesSet & observees_checked,
	 VariablesSet & observees_set
	)
	{
		for ( Variable * observee : observees ) {
			if ( observees_checked.find( observee ) == observees_checked.end() ) { // Observee not already processed
				observees_checked.insert( observee );
				if ( observee->is_state() || observee->is_Input() ) { // State or input => Computational
					observees_set.insert( observee );
				} else { // Traverse dependency sub-graph
					find_computational_observees_of( observee->observees_, observees_checked, observees_set ); // Recurse
				}
			}
		}
	}

private: // Data

	int order_{ 0 }; // Method order
	bool is_time_{ false }; // Time variable?

public: // Data

	Real rTol{ 1.0e-4 }; // Relative tolerance
	Real aTol{ 1.0e-6 }; // Absolute tolerance
	Real zTol{ 0.0 }; // Zero-crossing/root tolerance
	Real qTol{ 1.0e-6 }; // Quantization tolerance
	Real xIni{ 0.0 }; // Initial value
	Time tQ{ 0.0 }; // Quantized time range begin
	Time tX{ 0.0 }; // Continuous time range begin
	Time tE{ 0.0 }; // Time range end: tQ <= tE and tX <= tE
	Time tD{ infinity }; // Discrete event time: tQ <= tD and tX <= tD
	Time dt_min{ 0.0 }; // Time step min
	Time dt_max{ infinity }; // Time step max
	Real x_0_bump{ 0.0 }; // Bumped value

private: // Data

	// Time steps
	Time dt_inf_{ infinity }; // Time step inf
	mutable Time dt_inf_rlx_{ infinity }; // Relaxed time step inf

	// Observers
	bool observed_{ false }; // Has observers?
	bool self_observer_{ false }; // Appears in its function/derivative?
	Observers< Variable > observers_; // Variables dependent on this one

	// Observees
	bool observes_{ false }; // Has observees?
	Variables observees_; // State and input variable downstream dependencies to set in FMU to get value and directional derivatives
	VariableRefs observees_v_ref_; // Observee value references for FMU directional derivative lookup
	mutable Reals observees_dv_; // Observee derivatives for FMU directional derivative lookup
	std::size_t observees_nv_{ 0u }; // Observee count for FMU directional derivative lookup

	// Connections
	bool connected_{ false }; // Have connection(s)?
	Variable_Cons connections_; // Input connection variables this one outputs to

	// FMU
	FMU_ME * fmu_me_{ nullptr }; // FMU-ME
	FMU_Variable var_; // FMU variables specs
	FMU_Variable der_; // FMU derivative specs
	EventQ * eventq_{ nullptr }; // FMU event queue

	// Outputs
	bool out_on_{ true }; // Output on?
	std::string dec_; // Output file name decoration
	Output<> out_x_; // Continuous trajectory output
	Output<> out_q_; // Quantized trajectory output

}; // Variable

} // QSS

#endif
