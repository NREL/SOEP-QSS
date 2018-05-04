// Variable Abstract Base Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2018 Objexx Engineerinc, Inc. All rights reserved.
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

#ifndef QSS_dfn_Variable_hh_INCLUDED
#define QSS_dfn_Variable_hh_INCLUDED

// QSS Headers
#include <QSS/dfn/Variable.fwd.hh>
#include <QSS/dfn/Conditional.hh>
#include <QSS/globals.hh>
#include <QSS/math.hh>
#include <QSS/options.hh>
#include <QSS/Target.hh>

// C++ Headers
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <vector>

namespace QSS {
namespace dfn {

// Variable Abstract Base Class
class Variable : public Target
{

public: // Types

	using Super = Target;
	using Time = double;
	using Value = double;
	using Variables = std::vector< Variable * >;
	using size_type = Variables::size_type;
	using Coefficient = double;

	using If = IfV< Variable >;
	using When = WhenV< Variable >;
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
		Value l;
		Value u;
		Value z;
	};

	struct AdvanceSpecs_LIQSS2 final
	{
		Value l1;
		Value u1;
		Value z1;
		Value l2;
		Value u2;
		Value z2;
	};

protected: // Creation

	// Name + Tolerance + Value Constructor
	Variable(
	 std::string const & name,
	 Value const rTol,
	 Value const aTol,
	 Value const xIni = 0.0
	) :
	 Target( name ),
	 rTol( std::max( rTol, 0.0 ) ),
	 aTol( std::max( aTol, std::numeric_limits< Value >::min() ) ),
	 xIni( xIni ),
	 dt_min( options::dtMin ),
	 dt_max( options::dtMax ),
	 dt_inf( options::dtInf ),
	 dt_inf_rlx( options::dtInf == infinity ? infinity : 0.5 * options::dtInf )
	{}

	// Name + Value Constructor
	explicit
	Variable(
	 std::string const & name,
	 Value const xIni = 0.0
	) :
	 Target( name ),
	 xIni( xIni ),
	 dt_min( options::dtMin ),
	 dt_max( options::dtMax ),
	 dt_inf( options::dtInf ),
	 dt_inf_rlx( options::dtInf == infinity ? infinity : 0.5 * options::dtInf )
	{}

	// Copy Constructor
	Variable( Variable const & ) = delete;

	// Move Constructor
	Variable( Variable && ) noexcept = default;

protected: // Assignment

	// Copy Assignment
	Variable &
	operator =( Variable const & ) = delete;

	// Move Assignment
	Variable &
	operator =( Variable && ) noexcept = default;

public: // Predicate

	// Discrete Variable?
	virtual
	bool
	is_Discrete() const
	{ // Default implementation
		return false;
	}

	// Input Variable?
	virtual
	bool
	is_Input() const
	{ // Default implementation
		return false;
	}

	// QSS Variable?
	virtual
	bool
	is_QSS() const
	{ // Default implementation
		return false;
	}

	// Zero-Crossing Variable?
	virtual
	bool
	is_ZC() const
	{ // Default implementation
		return false;
	}

	// Non-Zero-Crossing Variable?
	virtual
	bool
	not_ZC() const
	{ // Default implementation
		return true;
	}

public: // Properties

	// Order of Method
	virtual
	int
	order() const = 0;

	// Boolean Value at Time t
	virtual
	bool
	b( Time const ) const
	{
		assert( false ); // Missing override
		return false;
	}

	// Continuous Value at Time t
	virtual
	Value
	x( Time const t ) const = 0;

	// Continuous First Derivative at Time t
	virtual
	Value
	x1( Time const t ) const = 0;

	// Continuous Second Derivative at Time t
	virtual
	Value
	x2( Time const ) const
	{
		return 0.0;
	}

	// Continuous Third Derivative at Time t
	virtual
	Value
	x3( Time const ) const
	{
		return 0.0;
	}

	// Quantized Value at Time t
	virtual
	Value
	q( Time const t ) const = 0;

	// Quantized First Derivative at Time t
	virtual
	Value
	q1( Time const ) const
	{
		return 0.0;
	}

	// Quantized Second Derivative at Time t
	virtual
	Value
	q2( Time const ) const
	{
		return 0.0;
	}

	// Simultaneous Value at Time t
	virtual
	Value
	s( Time const ) const
	{
		assert( false ); // Missing override
		return 0.0;
	}

	// Simultaneous Numeric Differentiation Value at Time t
	virtual
	Value
	sn( Time const ) const
	{
		assert( false ); // Missing override
		return 0.0;
	}

	// Simultaneous First Derivative at Time t
	virtual
	Value
	s1( Time const ) const
	{
		return 0.0;
	}

	// Simultaneous Second Derivative at Time t
	virtual
	Value
	s2( Time const ) const
	{
		return 0.0;
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
		if ( v == this ) { // Don't need to self-observe
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

	// Shrink Observers Collection
	void
	shrink_observers()
	{
		// Remove duplicates
		std::sort( observers_.begin(), observers_.end() );
		observers_.resize( std::distance( observers_.begin(), std::unique( observers_.begin(), observers_.end() ) ) );
		observers_.shrink_to_fit();

		// Put ZC variables at end for correct observer updates since they use observee x reps
		std::sort( observers_.begin(), observers_.end(), []( Variable const * v1, Variable const * v2 ){ return !( v1->is_ZC() ) && ( v2->is_ZC() ); } );

		// Set index to first ZC observer
		if ( ! observers_.empty() ) {
			Variable const * front( observers_.front() );
			if ( front->is_ZC() ) {
				i_beg_ZC_observers_ = 0;
			} else {
				i_beg_ZC_observers_ = std::distance( observers_.begin(), std::upper_bound( observers_.begin(), observers_.end(), front, []( Variable const * v1, Variable const * v2 ){ return !( v1->is_ZC() ) && ( v2->is_ZC() ); } ) );
			}
		} else { // No ZC observers
			i_beg_ZC_observers_ = observers_.size();
		}
	}

	// Shrink Observees Collection
	void
	shrink_observees()
	{
		// Remove duplicates
		std::sort( observees_.begin(), observees_.end() );
		observees_.resize( std::distance( observees_.begin(), std::unique( observees_.begin(), observees_.end() ) ) );
		observees_.shrink_to_fit();

		// Put ZC variables at end
		std::sort( observees_.begin(), observees_.end(), []( Variable const * v1, Variable const * v2 ){ return !( v1->is_ZC() ) && ( v2->is_ZC() ); } );
	}

	// Initialization
	virtual
	void
	init()
	{}

	// Initialization to a Value
	virtual
	void
	init( Value const )
	{}

	// Initialization: Stage 0
	virtual
	void
	init_0()
	{}

	// Initialization to a Value: Stage 0
	virtual
	void
	init_0( Value const )
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

	// Discrete Advance: Stages 0 and 1
	virtual
	void
	advance_discrete_0_1()
	{
		assert( false );
	}

	// Discrete Advance: Stage 2
	virtual
	void
	advance_discrete_2()
	{}

	// Discrete Advance: Stage 3
	virtual
	void
	advance_discrete_3()
	{}

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
	shift_handler( Time const t, Value const val )
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
	advance_handler( Time const, Value const )
	{
		assert( false ); // Not a QSS or Discrete variable
	}

	// Handler Advance: Stage 0
	virtual
	void
	advance_handler_0( Time const, Value const )
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
		std::int64_t const n( static_cast< std::int64_t >( observers_.size() ) );
		if ( n >= 4u ) { // Tuned on 4-core/8-thread CPU: Should tune on 8+ core systems
			std::int64_t const iZC( static_cast< std::int64_t >( i_beg_ZC_observers_ ) );
			#pragma omp parallel for schedule(guided)
			for ( std::int64_t i = 0; i < iZC; ++i ) { // Non-ZC
				assert( ! observers[ i ]->is_ZC() );
				observers_[ i ]->advance_observer_parallel( tQ );
			}
			#pragma omp parallel for schedule(guided)
			for ( std::int64_t i = iZC; i < n; ++i ) { // ZC
				assert( observers[ i ]->is_ZC() );
				observers_[ i ]->advance_observer_parallel( tQ );
			}
			for ( Variable * observer : observers_ ) {
				observer->advance_observer_sequential();
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
		std::int64_t const n( static_cast< std::int64_t >( observers.size() ) );
		if ( n >= 4u ) { // Tuned on 4-core/8-thread CPU: Should tune on 8+ core systems
			std::int64_t iZC( 0 );
			if ( ! observers.empty() ) {
				Variable const * front( observers.front() );
				if ( ! front->is_ZC() ) { // Some non-ZC observers
					iZC = std::distance( observers.begin(), std::upper_bound( observers.begin(), observers.end(), front, []( Variable const * v1, Variable const * v2 ){ return !( v1->is_ZC() ) && ( v2->is_ZC() ); } ) );
				}
			} else { // No ZC observers
				iZC = observers.size();
			}
			#pragma omp parallel for schedule(guided)
			for ( std::int64_t i = 0; i < iZC; ++i ) { // Non-ZC
				assert( ! observers[ i ]->is_ZC() );
				observers[ i ]->advance_observer_parallel( t );
			}
			#pragma omp parallel for schedule(guided)
			for ( std::int64_t i = iZC; i < n; ++i ) { // ZC
				assert( observers[ i ]->is_ZC() );
				observers[ i ]->advance_observer_parallel( t );
			}
			for ( Variable * observer : observers ) {
				observer->advance_observer_sequential();
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
	advance_observer_parallel( Time const t )
	{
		assert( false ); // Not a QSS or ZC variable
	}

	// Observer Advance: Sequential
	virtual
	void
	advance_observer_sequential()
	{
		assert( false ); // Not a QSS or ZC variable
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

public: // Data

	Value rTol{ 1.0e-4 }; // Relative tolerance
	Value aTol{ 1.0e-6 }; // Absolute tolerance
	Value qTol{ 1.0e-6 }; // Quantization tolerance
	Value xIni{ 0.0 }; // Initial value
	Time tQ{ 0.0 }; // Quantized time range begin
	Time tX{ 0.0 }; // Continuous time range begin
	Time tE{ 0.0 }; // Time range end: tQ <= tE and tX <= tE
	Time tD{ infinity }; // Discrete event time: tQ <= tD and tX <= tD
	Time dt_min{ 0.0 }; // Time step min
	Time dt_max{ infinity }; // Time step max
	Time dt_inf{ infinity }; // Time step inf
	Time dt_inf_rlx{ infinity }; // Relaxed time step inf
	bool self_observer{ false }; // Variable appears in its function/derivative?
	If_Clauses if_clauses; // Clauses in conditional if blocks
	When_Clauses when_clauses; // Clauses in conditional when blocks

protected: // Data

	Variables observers_; // Variables dependent on this one
	Variables observees_; // Variables this one depends on
	size_type i_beg_ZC_observers_{ 0u }; // Index of first ZC observer

};

} // dfn
} // QSS

#endif
