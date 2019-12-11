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
#include <QSS/Target.hh>
#include <QSS/fmu/Conditional.hh>
#include <QSS/fmu/FMU_ME.hh>
#include <QSS/fmu/FMU_Variable.hh>
#include <QSS/fmu/Observers.hh>
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
	 eventq_( fmu_me->eventq )
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
	 eventq_( fmu_me->eventq )
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
	 eventq_( fmu_me->eventq )
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
	 eventq_( fmu_me->eventq )
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
		for ( Variable * observer : observers_ ) {
			Variables & oos( observer->observees_ );
			Variables::iterator const i( std::find( oos.begin(), oos.end(), this ) );
			if ( i != oos.end() ) oos.erase( i );
		}
		for ( Variable * observee : observees_ ) {
			observee->observers_.del( this );
		}
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

	// Non-Connection Input Variable?
	bool
	not_connection() const
	{
		return ! is_connection();
	}

	// QSS Variable?
	virtual
	bool
	is_QSS() const
	{
		return false;
	}

	// LIQSS Variable?
	virtual
	bool
	is_LIQSS() const
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
	bool
	not_ZC() const
	{
		return ! is_ZC();
	}

	// In Conditional?
	virtual
	bool
	in_conditional() const
	{
		return conditional != nullptr;
	}

	// Observed?
	bool
	observed() const
	{
		return observed_;
	}

public: // Property

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

	// Add Drill-Through Observees to Zero-Crossing Variables
	void
	add_drill_through_observees()
	{
		assert( is_ZC() );
		if ( ! observees_.empty() ) {
			for ( Variable * vo : observees_ ) {
				for ( Variable * voo : vo->observees() ) {
					observe_ZC( voo ); // Only need back-observer to force observer updates when observees update since ZC variable value doesn't depend on these 2nd level observees
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
		if ( ! observees_.empty() ) { // Remove duplicates and discrete variables
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

	// QSS Advance: Stage 2.1
	virtual
	void
	advance_QSS_2_1()
	{}

	// QSS Advance: Stage 3
	virtual
	void
	advance_QSS_3()
	{}

	// QSS Advance: Stage Final
	virtual
	void
	advance_QSS_F()
	{}

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

	// Advance Observers
	void
	advance_observers()
	{
		observers_.advance( tQ );
	}

	// Advance Observers: Stage d
	void
	advance_observers_d() const
	{
		assert( options::output::d );
		observers_.advance_d();
	}

	// Observer Advance
	virtual
	void
	advance_observer( Time const t )
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
	advance_observer_1( Time const, Real const, Real const )
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

	// Observer Advance: Stage 3
	virtual
	void
	advance_observer_3( Real const )
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

	// Initialize Outputs
	void
	init_out( std::string const & dir )
	{
		if ( options::output::x ) out_x_.init( dir, name, 'x' );
		if ( options::output::q ) out_q_.init( dir, name, 'q' );
	}

	// Output at Time t
	void
	out( Time const t )
	{
		if ( options::output::x ) out_x_.append( t, x( t ) );
		if ( options::output::q ) out_q_.append( t, q( t ) );
		if ( have_connections ) connections_out( t );
	}

	// Output Quantized at Time t
	void
	out_q( Time const t )
	{
		if ( options::output::q ) out_q_.append( t, q( t ) );
		if ( have_connections ) connections_out_q( t );
	}

	// Pre-Event Observer Output at Time t
	void
	observer_out_pre( Time const t )
	{
		if ( options::output::x ) out_x_.append( t, x( t ) );
		if ( options::output::q && is_ZC() ) out_q_.append( t, q( t ) );
		if ( have_connections ) connections_observer_out_pre( t );
	}

	// Post-Event Observer Output at Time t
	void
	observer_out_post( Time const t )
	{
		if ( is_ZC() ) {
			if ( options::output::x ) out_x_.append( t, x( t ) );
			if ( options::output::q ) out_q_.append( t, q( t ) );
			if ( have_connections ) connections_observer_out_post( t );
		}
	}

	// Pre-Event Observers Output at Time t
	void
	observers_out_pre( Time const t )
	{
		if ( options::output::o ) {
			for ( Variable * observer : observers_ ) {
				observer->observer_out_pre( t );
			}
		}
	}

	// Post-Event Observers Output at Time t
	void
	observers_out_post( Time const t )
	{
		if ( options::output::o ) {
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

public: // Methods: FMU

	// Get FMU Time
	Time
	fmu_get_time() const
	{
		assert( fmu_me != nullptr );
		return fmu_me->get_time();
	}

	// Set FMU Time
	void
	fmu_set_time( Time const t ) const
	{
		assert( fmu_me != nullptr );
		fmu_me->set_time( t );
	}

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

	// Set FMU Variable to Continuous Value at Time t
	virtual
	void
	fmu_set_x( Time const t ) const
	{
		assert( fmu_me != nullptr );
		fmu_me->set_real( var.ref, x( t ) );
	}

	// Set FMU Variable to Quantized Value at Time t
	virtual
	void
	fmu_set_q( Time const t ) const
	{
		assert( fmu_me != nullptr );
		fmu_me->set_real( var.ref, q( t ) );
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

	// Set All Observee FMU Variables to Quantized Value at Time t
	void
	fmu_set_observees_q( Time const t ) const
	{
		for ( auto observee : observees_ ) {
			observee->fmu_set_q( t );
		}
	}

	// Coefficient 0 from FMU: Observees Set
	Real
	p_0() const
	{
		assert( fmu_me != nullptr );
		return fmu_me->get_real( var.ref );
	}

	// Coefficient 0 from FMU at Time tQ: Zero-Crossing
	Real
	z_0() const
	{
		fmu_set_observees_x( tQ );
		return p_0();
	}

	// Coefficient 0 from FMU at Time t: Zero-Crossing
	Real
	z_0( Time const t ) const
	{
		fmu_set_observees_x( t );
		return p_0();
	}

	// Coefficient 1 from FMU: Observees Set
	Real
	p_1() const
	{
		assert( fmu_me != nullptr );
		return fmu_me->get_real( der.ref );
	}

	// Coefficient 1 from FMU at Time t: QSS
	Real
	c_1( Time const t ) const
	{
		fmu_set_observees_q( t );
		if ( self_observer ) fmu_set_q( t );
		return p_1();
	}

	// Coefficient 1 from FMU at Time tQ: QSS
	Real
	c_1( Time const t, Real const q_0 ) const
	{
		assert( t == tQ );
		fmu_set_observees_q( t );
		if ( self_observer ) fmu_set_real( q_0 );
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

	// Coefficient 1 from FMU at Time t: Zero-Crossing
	Real
	z_1( Time const t ) const
	{
		fmu_set_observees_x( t );
		return p_1();
	}

	// Coefficient 1 from FMU at Time t: Zero-Crossing with ND First Derivative
	Real
	Z_1( Time const t, Real const x_0 ) const
	{
		Time const tN( t + options::dtNum );
		fmu_set_time( tN );
		Real const x_1( options::one_over_dtNum * ( z_0( tN ) - x_0 ) ); //ND Forward Euler
		fmu_set_time( t );
		return x_1;
	}

	// Coefficient 2 from FMU: Given Derivative
	Real
	p_2( Real const d, Real const x_1 ) const
	{
		return options::one_over_two_dtNum * ( d - x_1 ); //ND Forward Euler
	}

	// Coefficient 2 from FMU at Time t
	Real
	c_2( Time const t, Real const x_1 ) const
	{
		Time const tN( t + options::dtNum );
		fmu_set_time( tN );
		Real const x_2( options::one_over_two_dtNum * ( c_1( tN ) - x_1 ) ); //ND Forward Euler
		fmu_set_time( t );
		return x_2;
	}

	// Coefficient 2 from FMU at Time t: No Self-Observer Check/Set
	Real
	h_2( Time const t, Real const x_1 ) const
	{
		Time const tN( t + options::dtNum );
		fmu_set_time( tN );
		Real const x_2( options::one_over_two_dtNum * ( h_1( tN ) - x_1 ) ); //ND Forward Euler
		fmu_set_time( t );
		return x_2;
	}

	// Coefficient 2 from FMU at Time t: Zero-Crossing
	Real
	z_2( Time const t, Real const x_1 ) const
	{
		Time const tN( t + options::dtNum );
		fmu_set_time( tN );
		Real const x_2( options::one_over_two_dtNum * ( z_1( tN ) - x_1 ) ); //ND Forward Euler
		fmu_set_time( t );
		return x_2;
	}

	// Coefficient 3 from FMU at Time t
	Real
	c_3( Time const t, Real const x_1 ) const
	{
		Time tN( t - options::dtNum );
		fmu_set_time( tN );
		Real const x_1_m( c_1( tN ) );
		tN = t + options::dtNum;
		fmu_set_time( tN );
		Real const x_1_p( c_1( tN ) );
		fmu_set_time( tQ );
		return options::one_over_six_dtNum_squared * ( x_1_p - ( two * x_1 ) + x_1_m ); //ND Centered difference
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
	Time tD{ infinity }; // Discrete event time: tQ <= tD and tX <= tD
	Time dt_min{ 0.0 }; // Time step min
	Time dt_max{ infinity }; // Time step max
	Time dt_inf{ infinity }; // Time step inf
	Time dt_inf_rlx{ infinity }; // Relaxed time step inf
	bool self_observer{ false }; // Appears in its function/derivative?
	Conditional< Variable > * conditional{ nullptr }; // Conditional (non-owning)
	FMU_ME * fmu_me{ nullptr }; // FMU-ME (non-owning) pointer
	FMU_Variable var; // FMU variables specs
	FMU_Variable der; // FMU derivative specs

	// Connections
	Variable_Cons connections; // Input connection variables this one outputs to
	bool have_connections{ false }; // Have connections?

protected: // Data

	// Observers
	Observers< Variable > observers_; // Variables dependent on this one
	bool observed_{ false }; // Has observers?

	// Observees
	Variables observees_; // Variables this one depends on

	// Event queue
	EventQ * eventq_{ nullptr };

private: // Data

	// Outputs
	Output out_x_; // Continuous rep output
	Output out_q_; // Quantized rep output

};

} // fmu
} // QSS

#endif
