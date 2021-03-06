// Variable Abstract Base Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2021 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_cod_Variable_hh_INCLUDED
#define QSS_cod_Variable_hh_INCLUDED

// QSS Headers
#include <QSS/cod/Variable.fwd.hh>
#include <QSS/Target.hh>
#include <QSS/cod/Conditional.hh>
#include <QSS/cod/events.hh>
#include <QSS/container.hh>
#include <QSS/globals.hh>
#include <QSS/math.hh>
#include <QSS/options.hh>
#include <QSS/Output.hh>
#include <QSS/SmoothToken.hh>

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
namespace cod {

// Variable Abstract Base Class
class Variable : public Target
{

public: // Types

	using Super = Target;

	using Boolean = bool;
	using Integer = std::int64_t;
	using Real = double;
	using Time = double;
	using Coefficient = double;
	using Reals = std::vector< Real >;
	using Variables = std::vector< Variable * >;
	using size_type = Variables::size_type;

	using If = Conditional_If< Variable >;
	using When = Conditional_When< Variable >;
	using If_Clauses = std::vector< If::Clause * >;
	using When_Clauses = std::vector< When::Clause * >;

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

	struct AdvanceSpecs_LIQSS1 final
	{
		Real l;
		Real u;
		Real z;
	};

	struct AdvanceSpecs_LIQSS2 final
	{
		Real l1;
		Real u1;
		Real l2;
		Real u2;
		Real z0;
		Real z1;
	};

	struct AdvanceSpecs_LIQSS3 final
	{
		Real l1;
		Real u1;
		Real l2;
		Real u2;
		Real l3;
		Real u3;
		Real z0;
		Real z1;
		Real z2;
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
	 Real const rTol,
	 Real const aTol,
	 Real const xIni = 0.0
	) :
	 Target( name ),
	 order_( order ),
	 is_time_( name == "time" ),
	 rTol( std::max( rTol, 0.0 ) ),
	 aTol( std::max( aTol, std::numeric_limits< Real >::min() ) ),
	 xIni( xIni ),
	 dt_min( is_time_ ? 0.0 : options::dtMin ),
	 dt_max( is_time_ ? infinity : options::dtMax ),
	 dt_inf_( is_time_ ? infinity : options::dtInf ),
	 dt_inf_rlx_( dt_inf_ == infinity ? infinity : 0.5 * dt_inf_ )
	{}

	// Name + Value Constructor
	Variable(
	 int const order,
	 std::string const & name,
	 Real const xIni = 0.0
	) :
	 Target( name ),
	 order_( order ),
	 is_time_( name == "time" ),
	 xIni( xIni ),
	 dt_min( is_time_ ? 0.0 : options::dtMin ),
	 dt_max( is_time_ ? infinity : options::dtMax ),
	 dt_inf_( is_time_ ? infinity : options::dtInf ),
	 dt_inf_rlx_( dt_inf_ == infinity ? infinity : 0.5 * dt_inf_ )
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

	// Input Variable?
	virtual
	bool
	is_Input() const
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

	// Non-QSS Variable?
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

	// Non-State Variable?
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
		return ( ! is_ZC() );
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

	// Unpredicted Crossing Detected?
	virtual
	bool
	detected_crossing() const
	{
		assert( false ); // Missing override
		return false;
	}

public: // Property

	// Order
	int
	order() const
	{
		return order_;
	}

	// State + Order Sorting Index
	int
	state_order() const
	{
		return order_ + ( is_state() ? 0 : max_rep_order );
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

	// Observers
	Variables const &
	observers() const
	{
		return observers_;
	}

	// Observers
	Variables &
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

	// Zero-Crossing Time
	virtual
	Time
	tZC() const
	{
		assert( false ); // Not a ZC variable
		return Time( 0.0 );
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
			self_observer_ = true;
		} else {
			observees_.push_back( v );
			v->observers_.push_back( this );
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
					voo->observers_.push_back( this ); // Only need back-observer to force updates when observee has observer update
				}
			}
		}
	}

	// Initialize Observers Collection
	void
	init_observers()
	{
		observed_ = ( ! observers_.empty() );
		if ( observed_ ) {
			// Remove duplicates
			uniquify( observers_, true ); // Sort by address and remove duplicates and recover unused memory

			// Put ZC variables at end for correct observer updates since they use observee x reps
			std::sort( observers_.begin(), observers_.end(), []( Variable const * v1, Variable const * v2 ){ return v1->not_ZC() && v2->is_ZC(); } );

			// Set index to first ZC observer
			if ( ! observers_.empty() ) {
				Variable const * front( observers_.front() );
				if ( front->is_ZC() ) {
					i_beg_ZC_observers_ = 0;
				} else {
					i_beg_ZC_observers_ = std::distance( observers_.begin(), std::upper_bound( observers_.begin(), observers_.end(), front, []( Variable const * v1, Variable const * v2 ){ return v1->not_ZC() && v2->is_ZC(); } ) );
				}
			} else { // No ZC observers
				i_beg_ZC_observers_ = observers_.size();
			}
		}
	}

	// Initialize Observees Collection
	void
	init_observees()
	{
		observes_ = ( ! observees_.empty() );
		if ( observes_ ) { // Remove duplicates and discrete variables
			observees_.erase( std::remove_if( observees_.begin(), observees_.end(), []( Variable * v ){ return v->is_Discrete(); } ), observees_.end() ); // Remove discrete variables: Don't need them after ZC drill-through observees set up
			uniquify( observees_, true ); // Sort by address and remove duplicates and recover unused memory

			// Put ZC variables at end
			std::sort( observees_.begin(), observees_.end(), []( Variable const * v1, Variable const * v2 ){ return v1->not_ZC() && v2->is_ZC(); } );

			observes_ = ( ! observees_.empty() ); // In case all were discrete
		}
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

	// Initialization: Stage 3
	virtual
	void
	init_3()
	{}

	// Initialization: Stage LIQSS
	virtual
	void
	init_LIQSS()
	{}

	// Discrete Add Event
	void
	add_discrete( Time const t )
	{
		event_ = events.add_discrete( t, this );
	}

	// Discrete Shift Event to Time t
	void
	shift_discrete( Time const t )
	{
		event_ = events.shift_discrete( t, event_ );
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
		event_ = events.add_QSS( t, this );
	}

	// QSS Shift Event to Time t
	void
	shift_QSS( Time const t )
	{
		event_ = events.shift_QSS( t, event_ );
	}

	// QSS ZC Add Event
	void
	add_QSS_ZC( Time const t )
	{
		event_ = events.add_QSS_ZC( t, this );
	}

	// QSS ZC Shift Event to Time t
	void
	shift_QSS_ZC( Time const t )
	{
		event_ = events.shift_QSS_ZC( t, event_ );
	}

	// QSS Input Add Event
	void
	add_QSS_Inp( Time const t )
	{
		event_ = events.add_QSS_Inp( t, this );
	}

	// QSS Input Shift Event to Time t
	void
	shift_QSS_Inp( Time const t )
	{
		event_ = events.shift_QSS_Inp( t, event_ );
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

	// QSS Advance: Stage Final
	virtual
	void
	advance_QSS_F()
	{}

	// Zero-Crossing Add Event
	void
	add_ZC( Time const t )
	{
		event_ = events.add_ZC( t, this );
	}

	// Zero-Crossing Shift Event to Time t
	void
	shift_ZC( Time const t )
	{
		event_ = events.shift_ZC( t, event_ );
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
		event_ = events.add_handler( this );
	}

	// Handler Shift Event to Time t
	void
	shift_handler( Time const t, Real const val )
	{
		event_ = events.shift_handler( t, val, event_ );
	}

	// Handler Shift Event to Time Infinity
	void
	shift_handler()
	{
		event_ = events.shift_handler( event_ );
	}

	// Handler Advance
	virtual
	void
	advance_handler( Time const, Real const )
	{
		assert( false ); // Not a QSS or Discrete variable
	}

	// Handler Advance: Stage 0
	virtual
	void
	advance_handler_0( Time const, Real const )
	{
		assert( false ); // Not a QSS or Discrete variable
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

	// Handler Advance: Stage 3
	virtual
	void
	advance_handler_3()
	{
		assert( false ); // Not a QSS variable
	}

	// Advance Observers
	void
	advance_observers()
	{
#ifdef _OPENMP
		static size_type const observer_crossover( 40u ); // Size when parallel starts to be faster
		size_type const nzo( i_beg_ZC_observers_ );
		size_type const zco( observers_.size() - nzo );
		if ( std::max( nzo, zco ) >= observer_crossover ) { // Parallel

			std::int64_t const bZC( static_cast< std::int64_t >( i_beg_ZC_observers_ ) );
			std::int64_t const n( static_cast< std::int64_t >( observers_.size() ) );
			bool const haveNZ( 0 < bZC );
			bool const haveZC( bZC < n );

			#pragma omp parallel
			{

			if ( haveNZ ) {
				if ( nzo >= observer_crossover ) {
					#pragma omp for schedule(guided)
					for ( std::int64_t i = 0; i < bZC; ++i ) { // Non-ZC
						assert( observers_[ i ]->not_ZC() );
						observers_[ i ]->advance_observer_parallel( tQ );
					}
				} else {
					#pragma omp single
					for ( std::int64_t i = 0; i < bZC; ++i ) { // Non-ZC
						assert( observers_[ i ]->not_ZC() );
						observers_[ i ]->advance_observer_parallel( tQ );
					}
				}
			}

			if ( haveZC ) {
				if ( zco >= observer_crossover ) {
					#pragma omp for schedule(guided)
					for ( std::int64_t i = bZC; i < n; ++i ) { // ZC
						assert( observers_[ i ]->is_ZC() );
						observers_[ i ]->advance_observer_parallel( tQ );
					}
				} else {
					#pragma omp single
					for ( std::int64_t i = bZC; i < n; ++i ) { // ZC
						assert( observers_[ i ]->is_ZC() );
						observers_[ i ]->advance_observer_parallel( tQ );
					}
				}
			}

			} // omp parallel

			if ( options::output::d ) {
				for ( Variable * observer : observers_ ) {
					observer->advance_observer_serial_d();
				}
			} else {
				for ( Variable * observer : observers_ ) {
					observer->advance_observer_serial();
				}
			}
			return;
		}
#endif
		for ( Variable * observer : observers_ ) {
			observer->advance_observer( tQ );
		}
	}

	// Advance Given Observers
	static
	void
	advance_observers( Variables & observers, Time const t )
	{
#ifdef _OPENMP
		static size_type const observer_crossover( 40u ); // Size when parallel starts to be faster
		std::int64_t bZC( ( ! observers.empty() ) && observers.front()->not_ZC() ?
		 std::distance( observers.begin(), std::upper_bound( observers.begin(), observers.end(), observers.front(), []( Variable const * v1, Variable const * v2 ){ return v1->not_ZC() && v2->is_ZC(); } ) ) :
		 observers.size() );
		size_type const nzo( bZC );
		size_type const zco( observers.size() - nzo );
		if ( std::max( nzo, zco ) >= observer_crossover ) { // Parallel

			std::int64_t const n( static_cast< std::int64_t >( observers.size() ) );
			bool const haveNZ( 0 < bZC );
			bool const haveZC( bZC < n );

			#pragma omp parallel
			{

			if ( haveNZ ) {
				if ( nzo >= observer_crossover ) {
					#pragma omp for schedule(guided)
					for ( std::int64_t i = 0; i < bZC; ++i ) { // Non-ZC
						assert( observers[ i ]->not_ZC() );
						observers[ i ]->advance_observer_parallel( t );
					}
				} else {
					#pragma omp single
					for ( std::int64_t i = 0; i < bZC; ++i ) { // Non-ZC
						assert( observers[ i ]->not_ZC() );
						observers[ i ]->advance_observer_parallel( t );
					}
				}
			}

			if ( haveZC ) {
				if ( zco >= observer_crossover ) {
					#pragma omp for schedule(guided)
					for ( std::int64_t i = bZC; i < n; ++i ) { // ZC
						assert( observers[ i ]->is_ZC() );
						observers[ i ]->advance_observer_parallel( t );
					}
				} else {
					#pragma omp single
					for ( std::int64_t i = bZC; i < n; ++i ) { // ZC
						assert( observers[ i ]->is_ZC() );
						observers[ i ]->advance_observer_parallel( t );
					}
				}
			}

			} // omp parallel

			if ( options::output::d ) {
				for ( Variable * observer : observers ) {
					observer->advance_observer_serial_d();
				}
			} else {
				for ( Variable * observer : observers ) {
					observer->advance_observer_serial();
				}
			}
			return;
		}
#endif
		for ( Variable * observer : observers ) {
			observer->advance_observer( t );
		}
	}

	// Observer Advance
	virtual
	void
	advance_observer( Time const )
	{
		assert( false ); // Not a QSS or ZC variable
	}

	// Observer Advance: Parallel
	virtual
	void
	advance_observer_parallel( Time const )
	{
		assert( false ); // Not a QSS or ZC variable
	}

	// Observer Advance: Serial + Diagnostics
	virtual
	void
	advance_observer_serial_d()
	{
		assert( false ); // Not a QSS or ZC variable
	}

	// Observer Advance: Serial
	virtual
	void
	advance_observer_serial()
	{
		shift_QSS( tE );
	}

public: // Methods: Output

	// Initialize Outputs
	void
	init_out( std::string const & dec = std::string() )
	{
		if ( options::output::X ) out_x_.init( name(), 'x', dec );
		if ( options::output::Q ) out_q_.init( name(), 'q', dec );
	}

	// Output at Time t
	void
	out( Time const t )
	{
		if ( options::output::X ) out_x_.append( t, x( t ) );
		if ( options::output::Q ) out_q_.append( t, q( t ) );
	}

	// Output Quantized at Time t
	void
	out_q( Time const t )
	{
		if ( options::output::Q ) out_q_.append( t, q( t ) );
	}

	// Pre-Event Observer Output at Time t
	void
	observer_out_pre( Time const t )
	{
		if ( options::output::X ) out_x_.append( t, x( t ) );
		if ( options::output::Q && is_ZC() ) out_q_.append( t, q( t ) );
	}

	// Post-Event Observer Output at Time t
	void
	observer_out_post( Time const t )
	{
		if ( is_ZC() ) {
			if ( options::output::X ) out_x_.append( t, x( t ) );
			if ( options::output::Q ) out_q_.append( t, q( t ) );
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

private: // Data

	int order_{ 0 }; // Order of method
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
	If_Clauses if_clauses; // Clauses in conditional if blocks
	When_Clauses when_clauses; // Clauses in conditional when blocks

private: // Data

	Time dt_inf_{ infinity }; // Time step inf
	mutable Time dt_inf_rlx_{ infinity }; // Relaxed time step inf

	Variables observers_; // Variables dependent on this one
	bool observed_{ false }; // Have any observers?
	bool self_observer_{ false }; // Appears in its function/derivative?
	size_type i_beg_ZC_observers_{ 0u }; // Index of first ZC observer

	Variables observees_; // Variables this one depends on
	bool observes_{ false }; // Has observees?

	// Outputs
	Output<> out_x_; // Continuous rep output
	Output<> out_q_; // Quantized rep output

}; // Variable

} // cod
} // QSS

#endif
