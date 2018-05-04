// FMU-Based Variable Abstract Base Class
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

#ifndef QSS_fmu_Variable_hh_INCLUDED
#define QSS_fmu_Variable_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable.fwd.hh>
#include <QSS/fmu/Conditional.hh>
#include <QSS/fmu/FMI.hh>
#include <QSS/fmu/FMU_Variable.hh>
#include <QSS/globals.hh>
#include <QSS/math.hh>
#include <QSS/options.hh>
#include <QSS/Target.hh>
#include <QSS/VectorPair.hh>

// C++ Headers
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <unordered_set>
#include <vector>

namespace QSS {
namespace fmu {

// FMU-Based Variable Abstract Base Class
class Variable : public Target
{

public: // Types

	using Super = Target;
	using Time = double;
	using Value = double;
	using Variables = std::vector< Variable * >;
	using VariableRefs = std::vector< fmi2_value_reference_t >;
	using Values = std::vector< Value >;
	using size_type = Variables::size_type;
	using Indexes = std::vector< size_type >;
	using VariablesPair = VectorPair< Variable * >;

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

protected: // Creation

	// Name + Tolerance + Value Constructor
	Variable(
	 std::string const & name,
	 Value const rTol,
	 Value const aTol,
	 Value const xIni,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Target( name ),
	 rTol( std::max( rTol, 0.0 ) ),
	 aTol( std::max( aTol, std::numeric_limits< Value >::min() ) ),
	 xIni( xIni ),
	 dt_min( options::dtMin ),
	 dt_max( options::dtMax ),
	 dt_inf( options::dtInf ),
	 dt_inf_rlx( options::dtInf == infinity ? infinity : 0.5 * options::dtInf ),
	 var( var ),
	 der( der ),
	 observers_( observers_NZ_, observers_ZC_ )
	{}

	// Name + Tolerance Constructor
	Variable(
	 std::string const & name,
	 Value const rTol,
	 Value const aTol,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Target( name ),
	 rTol( std::max( rTol, 0.0 ) ),
	 aTol( std::max( aTol, std::numeric_limits< Value >::min() ) ),
	 xIni( 0.0 ),
	 dt_min( options::dtMin ),
	 dt_max( options::dtMax ),
	 dt_inf( options::dtInf ),
	 dt_inf_rlx( options::dtInf == infinity ? infinity : 0.5 * options::dtInf ),
	 var( var ),
	 der( der ),
	 observers_( observers_NZ_, observers_ZC_ )
	{}

	// Name + Value Constructor
	Variable(
	 std::string const & name,
	 Value const xIni,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Target( name ),
	 xIni( xIni ),
	 dt_min( options::dtMin ),
	 dt_max( options::dtMax ),
	 dt_inf( options::dtInf ),
	 dt_inf_rlx( options::dtInf == infinity ? infinity : 0.5 * options::dtInf ),
	 var( var ),
	 der( der ),
	 observers_( observers_NZ_, observers_ZC_ )
	{}

	// Name Constructor
	explicit
	Variable(
	 std::string const & name,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Target( name ),
	 xIni( 0.0 ),
	 dt_min( options::dtMin ),
	 dt_max( options::dtMax ),
	 dt_inf( options::dtInf ),
	 dt_inf_rlx( options::dtInf == infinity ? infinity : 0.5 * options::dtInf ),
	 var( var ),
	 der( der ),
	 observers_( observers_NZ_, observers_ZC_ )
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

	// Max Non-Zero-Crossing Observer Order
	int
	observers_NZ_max_order() const
	{
		return observers_NZ_max_order_;
	}

	// Max Zero-Crossing Observer Order
	int
	observers_ZC_max_order() const
	{
		return observers_ZC_max_order_;
	}

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

	// Non-Zero-Crossing Observers
	Variables const &
	observers_NZ() const
	{
		return observers_NZ_;
	}

	// Non-Zero-Crossing Observers
	Variables &
	observers_NZ()
	{
		return observers_NZ_;
	}

	// Zero-Crossing Observers
	Variables const &
	observers_ZC() const
	{
		return observers_ZC_;
	}

	// Zero-Crossing Observers
	Variables &
	observers_ZC()
	{
		return observers_ZC_;
	}

	// Observers
	VariablesPair const &
	observers() const
	{
		return observers_;
	}

	// Observers
	VariablesPair &
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
			not_ZC() ? v->observers_NZ_.push_back( this ) : v->observers_ZC_.push_back( this );
		}
	}

	// Add Zero-Crossing Variable as an Observer
	void
	observe_ZC( Variable * v )
	{
		assert( is_ZC() );
		assert( v != this );
		v->observers_ZC_.push_back( this );
	}

	// Sort Observers Collections by Order
	void
	sort_observers()
	{
		// Remove duplicates: Add special case for sufficiently many observers using unordered_set if needed
		std::sort( observers_NZ_.begin(), observers_NZ_.end() );
		observers_NZ_.resize( std::distance( observers_NZ_.begin(), std::unique( observers_NZ_.begin(), observers_NZ_.end() ) ) );
		observers_NZ_.shrink_to_fit();
		std::sort( observers_ZC_.begin(), observers_ZC_.end() );
		observers_ZC_.resize( std::distance( observers_ZC_.begin(), std::unique( observers_ZC_.begin(), observers_ZC_.end() ) ) );
		observers_ZC_.shrink_to_fit();

		// Sort by order
		std::sort( observers_NZ_.begin(), observers_NZ_.end(), []( Variable const * v1, Variable const * v2 ){ return v1->order() < v2->order(); } );
		std::sort( observers_ZC_.begin(), observers_ZC_.end(), []( Variable const * v1, Variable const * v2 ){ return v1->order() < v2->order(); } );

		// Save useful specs
		have_observers_NZ_ = ( ! observers_NZ_.empty() );
		have_observers_ZC_ = ( ! observers_ZC_.empty() );
		have_observers_ = ( have_observers_NZ_ || have_observers_ZC_ );
		iBeg_observers_NZ_2_ = static_cast< size_type >( std::distance( observers_NZ_.begin(), std::find_if( observers_NZ_.begin(), observers_NZ_.end(), []( Variable * v ){ return v->order() >= 2; } ) ) );
		iBeg_observers_ZC_2_ = static_cast< size_type >( std::distance( observers_ZC_.begin(), std::find_if( observers_ZC_.begin(), observers_ZC_.end(), []( Variable * v ){ return v->order() >= 2; } ) ) );
		observers_NZ_max_order_ = ( have_observers_NZ_ ? observers_NZ_.back()->order() : 0 );
		observers_ZC_max_order_ = ( have_observers_ZC_ ? observers_ZC_.back()->order() : 0 );
	}

	// Shrink Observees Collection
	void
	shrink_observees()
	{
		// Remove duplicates: Add special case for sufficiently many observers using unordered_set if needed
		std::sort( observees_.begin(), observees_.end() );
		observees_.resize( std::distance( observees_.begin(), std::unique( observees_.begin(), observees_.end() ) ) );
		observees_.shrink_to_fit();
	}

	// Initialization: Observers
	void
	init_observers()
	{
		sort_observers();
		shrink_observees();
		size_type const n_observers_NZ( observers_NZ_.size() );
		size_type const n_observers_ZC( observers_ZC_.size() );

		// Observer derivatives
		observers_NZ_der_refs_.clear();
		observers_NZ_der_vals_.clear();
		observers_NZ_der_refs_.reserve( n_observers_NZ );
		observers_NZ_der_vals_.reserve( n_observers_NZ );
		for ( Variable const * observer : observers_NZ_ ) {
			observers_NZ_der_refs_.push_back( observer->der.ref );
			observers_NZ_der_vals_.push_back( 0.0 );
		}
		observers_ZC_der_refs_.clear();
		observers_ZC_der_vals_.clear();
		observers_ZC_der_refs_.reserve( n_observers_ZC );
		observers_ZC_der_vals_.reserve( n_observers_ZC );
		for ( Variable const * observer : observers_ZC_ ) {
			observers_ZC_der_refs_.push_back( observer->der.ref );
			observers_ZC_der_vals_.push_back( 0.0 );
		}

		// Zero-crossing observer values
		observers_ZC_refs_.clear();
		observers_ZC_vals_.clear();
		for ( Variable const * observer : observers_ZC_ ) {
			observers_ZC_refs_.push_back( observer->var.ref );
			observers_ZC_vals_.push_back( 0.0 );
		}
		observers_ZC_refs_.shrink_to_fit();
		observers_ZC_vals_.shrink_to_fit();

		{ // Non-zero-crossing observers observees setup
			std::unordered_set< Variable * > oo2s; // Observees of observers of order 2+
			for ( Variable * observer : observers_NZ_ ) {
				if ( observer->order() >= 2 ) {
					if ( observer->self_observer ) oo2s.insert( observer );
					for ( auto observee : observer->observees_ ) {
						oo2s.insert( observee );
					}
				}
			}
			std::unordered_set< Variable * > oo1s; // Observees of observers of order <=1 not in oo2s
			for ( Variable * observer : observers_NZ_ ) {
				if ( observer->order() <= 1 ) {
					if ( ( observer->self_observer ) && ( oo2s.find( observer ) == oo2s.end() ) ) oo1s.insert( observer );
					for ( auto observee : observer->observees_ ) {
						if ( oo2s.find( observee ) == oo2s.end() ) oo1s.insert( observee );
					}
				}
			}
			observers_NZ_observees_.clear();
			observers_NZ_observees_.reserve( oo1s.size() + oo2s.size() );
			for ( auto observee : oo1s ) {
				observers_NZ_observees_.push_back( observee );
			}
			for ( auto observee : oo2s ) {
				observers_NZ_observees_.push_back( observee );
			}
			iBeg_observers_NZ_2_observees_ = oo1s.size();
		}

		{ // Zero-crossing observers observees setup
			std::unordered_set< Variable * > oo2s; // Observees of observers of order 2+
			for ( Variable * observer : observers_ZC_ ) {
				if ( observer->order() >= 2 ) {
					if ( observer->self_observer ) oo2s.insert( observer );
					for ( auto observee : observer->observees_ ) {
						oo2s.insert( observee );
					}
				}
			}
			std::unordered_set< Variable * > oo1s; // Observees of observers of order <=1 not in oo2s
			for ( Variable * observer : observers_ZC_ ) {
				if ( observer->order() <= 1 ) {
					if ( ( observer->self_observer ) && ( oo2s.find( observer ) == oo2s.end() ) ) oo1s.insert( observer );
					for ( auto observee : observer->observees_ ) {
						if ( oo2s.find( observee ) == oo2s.end() ) oo1s.insert( observee );
					}
				}
			}
			observers_ZC_observees_.clear();
			observers_ZC_observees_.reserve( oo1s.size() + oo2s.size() );
			for ( auto observee : oo1s ) {
				observers_ZC_observees_.push_back( observee );
			}
			for ( auto observee : oo2s ) {
				observers_ZC_observees_.push_back( observee );
			}
			iBeg_observers_ZC_2_observees_ = oo1s.size();
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
	init( Value const )
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
	shift_handler( Time const t )
	{
		event_ = events.shift_handler( t, event_ );
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

	// Advance Observers
	void
	advance_observers()
	{
		if ( have_observers_NZ_ ) {
			assert( fmu::get_time() == tQ );
			advance_observers_NZ_1();
			if ( observers_NZ_max_order_ >= 2 ) {
				fmu::set_time( tN = tQ + options::dtNum );
				advance_observers_NZ_2();
			}
		}
		if ( have_observers_ZC_ ) {
			if ( observers_NZ_max_order_ >= 2 ) fmu::set_time( tQ );
			advance_observers_ZC_1();
			if ( observers_ZC_max_order_ >= 2 ) {
				fmu::set_time( tN = tQ + options::dtNum );
				advance_observers_ZC_2();
			}
		}
		if ( options::output::d ) {
			for ( Variable const * observer : observers_ ) {
				observer->advance_observer_d();
			}
		}
	}

	// Advance Given Observers
	static
	void
	advance_observers( Variables & observers, Time const t )
	{
		assert( std::is_sorted( observers.begin(), observers.end(), []( Variable const * v1, Variable const * v2 ){ return ( v1->is_ZC() ? max_rep_order << 1 : 0 ) + v1->order() < ( v2->is_ZC() ? max_rep_order << 1 : 0 ) + v2->order(); } ) ); // Require sorted by NZ,ZC type and order
		size_type const no( observers.size() );
		Variables::iterator const end_NZ( std::upper_bound( observers.begin(), observers.end(), false, []( bool const, Variable const * v ){ return v->is_ZC(); } ) );
		Variables::iterator const beg_ZC( std::lower_bound( observers.begin(), observers.end(), true, []( Variable const * v, bool const ){ return v->not_ZC(); } ) );
		size_type const i_end_NZ( static_cast< Variable::Variables::size_type >( std::distance( observers.begin(), end_NZ ) ) );
		size_type const i_beg_ZC( static_cast< Variable::Variables::size_type >( std::distance( observers.begin(), beg_ZC ) ) );
		size_type const i_beg_NZ_2( static_cast< Variable::Variables::size_type >( std::distance( observers.begin(), std::lower_bound( observers.begin(), end_NZ, 2, []( Variable const * v, int const o ){ return v->order() < o; } ) ) ) );
		size_type const i_beg_ZC_2( static_cast< Variable::Variables::size_type >( std::distance( observers.begin(), std::lower_bound( beg_ZC, observers.end(), 2, []( Variable const * v, int const o ){ return v->order() < o; } ) ) ) );
		bool have_NZ( false ), have_ZC( false );
		int order_max_NZ( 0 ), order_max_ZC( 0 );
		if ( end_NZ == observers.begin() ) { // No non-zero-crossing observers
			have_NZ = false;
			order_max_NZ = 0;
		} else { // Have non-zero-crossing observers
			have_NZ = true;
			order_max_NZ = (*( end_NZ - 1 ))->order();
		}
		if ( end_NZ == observers.end() ) { // No zero-crossing observers
			have_ZC = false;
			order_max_ZC = 0;
		} else { // Have zero-crossing observers
			have_ZC = true;
			order_max_ZC = observers.back()->order();
		}

		if ( have_NZ ) {
			for ( size_type i = 0; i < i_end_NZ; ++i ) {
				observers[ i ]->advance_observer_1( t );
			}
			if ( order_max_NZ >= 2 ) { // 2nd order pass
				Time const tN( t + options::dtNum ); // Set time to t + delta for numeric differentiation
				fmu::set_time( tN );
				for ( size_type i = i_beg_NZ_2; i < i_end_NZ; ++i ) {
					observers[ i ]->advance_observer_2( tN );
				}
			}
		}
		if ( have_ZC ) {
			if ( order_max_NZ >= 2 ) fmu::set_time( t );
			for ( size_type i = i_beg_ZC; i < no; ++i ) {
				observers[ i ]->advance_observer_1( t );
			}
			if ( order_max_ZC >= 2 ) { // 2nd order pass
				Time const tN( t + options::dtNum ); // Set time to t + delta for numeric differentiation
				fmu::set_time( tN );
				for ( size_type i = i_beg_ZC_2; i < no; ++i ) {
					observers[ i ]->advance_observer_2( tN );
				}
			}
		}
		if ( options::output::d ) {
			for ( Variable const * observer : observers ) {
				observer->advance_observer_d();
			}
		}
	}

	// Advance Observers: Set FMU Time to tQ First
	void
	advance_observers_tQ()
	{
		fmu::set_time( tQ );
		advance_observers();
	}

	// Advance Non-Zero-Crossing Observers: Stage 1
	void
	advance_observers_NZ_1()
	{
		fmu_set_observers_NZ_observees_q( tQ );
		fmu_get_observers_NZ_derivs();
		for ( size_type i = 0, n = observers_NZ_.size(); i < n; ++i ) {
			observers_NZ_[ i ]->advance_observer_1( tQ, observers_NZ_der_vals_[ i ] );
		}
	}

	// Advance Zero-Crossing Observers: Stage 1
	void
	advance_observers_ZC_1()
	{
		fmu_set_observers_ZC_observees_x( tQ );
		fmu_get_observers_ZC_derivs();
		fmu_get_observers_ZC_values();
		for ( size_type i = 0, n = observers_ZC_.size(); i < n; ++i ) {
			 observers_ZC_[ i ]->advance_observer_ZC_1( tQ, observers_ZC_der_vals_[ i ], observers_ZC_vals_[ i ] );
		}
	}

	// Advance Non-Zero-Crossing Observers: Stage 2
	void
	advance_observers_NZ_2()
	{
		assert( tN == tQ + options::dtNum );
		fmu_set_observers_NZ_2_observees_q( tN );
		fmu_get_observers_NZ_derivs();
		for ( size_type i = iBeg_observers_NZ_2_, n = observers_NZ_.size(); i < n; ++i ) { // Order 2+ observers
			observers_NZ_[ i ]->advance_observer_2( tN, observers_NZ_der_vals_[ i ] );
		}
	}

	// Advance Zero-Crossing Observers: Stage 2
	void
	advance_observers_ZC_2()
	{
		assert( tN == tQ + options::dtNum );
		fmu_set_observers_ZC_2_observees_x( tN );
		fmu_get_observers_ZC_derivs();
		for ( size_type i = iBeg_observers_ZC_2_, n = observers_ZC_.size(); i < n; ++i ) { // Order 2+ observers
			observers_ZC_[ i ]->advance_observer_2( tN, observers_ZC_der_vals_[ i ] );
		}
	}

	// Observer Advance
	virtual
	void
	advance_observer( Time const )
	{
		assert( false ); // Not a QSS or ZC variable
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
	advance_observer_1( Time const, Value const )
	{
		assert( false );
	}

	// Zero-Crossing Observer Advance: Stage 1
	virtual
	void
	advance_observer_ZC_1( Time const, Value const, Value const )
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
	advance_observer_2( Time const, Value const )
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

	// Get FMU Variable Value
	Value
	fmu_get_value() const
	{
		return fmu::get_real( var.ref );
	}

	// Get FMU Variable Derivative
	Value
	fmu_get_deriv() const
	{
		return fmu::get_real( der.ref );
	}

	// Get Non-Zero-Crossing Observers FMU Derivatives
	void
	fmu_get_observers_NZ_derivs()
	{
		if ( ! observers_NZ_der_refs_.empty() ) {
			fmu::get_reals( observers_NZ_der_refs_.size(), &observers_NZ_der_refs_[ 0 ], &observers_NZ_der_vals_[ 0 ] );
		}
	}

	// Get Zero-Crossing Observers FMU Derivatives
	void
	fmu_get_observers_ZC_derivs()
	{
		if ( ! observers_ZC_der_refs_.empty() ) {
			fmu::get_reals( observers_ZC_der_refs_.size(), &observers_ZC_der_refs_[ 0 ], &observers_ZC_der_vals_[ 0 ] );
		}
	}

	// Get FMU Variable Zero-Crossing Observer Values
	void
	fmu_get_observers_ZC_values()
	{
		if ( ! observers_ZC_refs_.empty() ) {
			fmu::get_reals( observers_ZC_refs_.size(), &observers_ZC_refs_[ 0 ], &observers_ZC_vals_[ 0 ] );
		}
	}

	// Set FMU Variable to a Value
	void
	fmu_set_value( Value const v ) const
	{
		fmu::set_real( var.ref, v );
	}

	// Set FMU Variable to Continuous Value at Time t
	void
	fmu_set_x( Time const t ) const
	{
		fmu::set_real( var.ref, x( t ) );
	}

	// Set FMU Variable to Quantized Value at Time t
	void
	fmu_set_q( Time const t ) const
	{
		fmu::set_real( var.ref, q( t ) );
	}

	// Set FMU Variable to Simultaneous Value at Time t
	void
	fmu_set_s( Time const t ) const
	{
		fmu::set_real( var.ref, s( t ) );
	}

	// Set FMU Variable to Simultaneous Numeric Differentiation Value at Time t
	void
	fmu_set_sn( Time const t ) const
	{
		fmu::set_real( var.ref, sn( t ) );
	}

	// Get FMU Integer Variable Value
	Integer
	fmu_get_integer_value() const
	{
		return fmu::get_integer( var.ref );
	}

	// Set FMU Integer Variable to a Value
	void
	fmu_set_integer_value( Integer const v ) const
	{
		fmu::set_integer( var.ref, v );
	}

	// Get FMU Boolean Variable Value
	bool
	fmu_get_boolean_value() const
	{
		return fmu::get_boolean( var.ref );
	}

	// Set FMU Boolean Variable to a Value
	void
	fmu_set_boolean_value( bool const v ) const
	{
		fmu::set_boolean( var.ref, v );
	}

	// Set All Observee FMU Variables to Continuous Value at Time t
	void
	fmu_set_observees_x( Time const t ) const
	{
		for ( auto observee : observees_ ) {
			if ( ! observee->is_Discrete() ) observee->fmu_set_x( t );
		}
	}

	// Set All Observee FMU Variables to Quantized Value at Time t
	void
	fmu_set_observees_q( Time const t ) const
	{
		for ( auto observee : observees_ ) {
			if ( ! observee->is_Discrete() ) observee->fmu_set_q( t );
		}
	}

	// Set All Observee FMU Variables to Simultaneous Value at Time t
	void
	fmu_set_observees_s( Time const t ) const
	{
		for ( auto observee : observees_ ) {
			if ( ! observee->is_Discrete() ) observee->fmu_set_s( t );
		}
	}

	// Set All Observee FMU Variables to Simultaneous Numeric Differentiation Value at Time t
	void
	fmu_set_observees_sn( Time const t ) const
	{
		for ( auto observee : observees_ ) {
			if ( ! observee->is_Discrete() ) observee->fmu_set_sn( t );
		}
	}

	// Set Non-Zero-Crossing Observers Observee FMU Variables to Quantized Value at Time t
	void
	fmu_set_observers_NZ_observees_q( Time const t ) const
	{
		for ( auto observee : observers_NZ_observees_ ) {
			if ( ! observee->is_Discrete() ) observee->fmu_set_q( t );
		}
	}

	// Set Zero-Crossing Observers Observee FMU Variables to Continuous Value at Time t
	void
	fmu_set_observers_ZC_observees_x( Time const t ) const
	{
		for ( auto observee : observers_ZC_observees_ ) {
			if ( ! observee->is_Discrete() ) observee->fmu_set_x( t );
		}
	}

	// Set All Order 2+ Non-Zero-Crossing Observers Observee FMU Variables to Quantized Value at Time t
	void
	fmu_set_observers_NZ_2_observees_q( Time const t ) const
	{
		for ( size_type i = iBeg_observers_NZ_2_observees_, n = observers_NZ_observees_.size(); i < n; ++i ) {
			auto & observee( observers_NZ_observees_[ i ] );
			if ( ! observee->is_Discrete() ) observee->fmu_set_q( t );
		}
	}

	// Set All Order 2+ Zero-Crossing Observers Observee FMU Variables to Continuous Value at Time t
	void
	fmu_set_observers_ZC_2_observees_x( Time const t ) const
	{
		for ( size_type i = iBeg_observers_ZC_2_observees_, n = observers_ZC_observees_.size(); i < n; ++i ) {
			auto & observee( observers_ZC_observees_[ i ] );
			if ( ! observee->is_Discrete() ) observee->fmu_set_x( t );
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

public: // Data

	Value rTol{ 1.0e-4 }; // Relative tolerance
	Value aTol{ 1.0e-6 }; // Absolute tolerance
	Value qTol{ 1.0e-6 }; // Quantization tolerance
	Value xIni{ 0.0 }; // Initial value
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
	If_Clauses if_clauses; // Clauses in conditional if blocks
	When_Clauses when_clauses; // Clauses in conditional when blocks
	FMU_Variable var; // FMU variables specs
	FMU_Variable der; // FMU derivative specs

protected: // Data

	// Observers
	Variables observers_NZ_; // Non-zero-crossing variables dependent on this one
	Variables observers_ZC_; // Zero-crossing variables dependent on this one
	VariablesPair observers_; // Variables dependent on this one (iterable view)
	bool have_observers_NZ_{ false }; // Have non-zero-crossing observers?
	bool have_observers_ZC_{ false }; // Have zero-crossing observers?
	bool have_observers_{ false }; // Have observers?
	int observers_NZ_max_order_{ 0 }; // Max QSS order of non-zero-crossing observers
	int observers_ZC_max_order_{ 0 }; // Max QSS order of zero-crossing observers
	size_type iBeg_observers_NZ_2_{ 0 }; // Index of first observer of order 2+
	size_type iBeg_observers_ZC_2_{ 0 }; // Index of first observer of order 2+

	// FMU Observers
	VariableRefs observers_NZ_der_refs_; // Non-zero-crossing observer FMU derivative refs
	VariableRefs observers_ZC_der_refs_; // Zero-crossing observer FMU derivative refs
	Values observers_NZ_der_vals_; // Non-zero-crossing observer FMU derivative values
	Values observers_ZC_der_vals_; // Zero-crossing observer FMU derivative values
	VariableRefs observers_ZC_refs_; // Zero-crossing observer FMU variable refs
	Values observers_ZC_vals_; //  Zero-crossing observer FMU variable values

	// Observees
	Variables observees_; // Variables this one depends on

	// Observers Observees
	Variables observers_NZ_observees_; // Non-zero-crossing observer observees (including self-observing observers)
	Variables observers_ZC_observees_; // Zero-crossing observer observees (including self-observing observers)
	size_type iBeg_observers_NZ_2_observees_{ 0 }; // Index of first observee of a non-zero-crossing observer of order 2+
	size_type iBeg_observers_ZC_2_observees_{ 0 }; // Index of first observee of a zero-crossing observer of order 2+

};

} // fmu
} // QSS

#endif
