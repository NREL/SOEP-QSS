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
#include <QSS/fmu/FMU.hh>
#include <QSS/fmu/FMU_Variable.hh>
#include <QSS/fmu/Observers.hh>
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
#include <unordered_set>
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
	using Variables = std::vector< Variable * >;
	using VariableRefs = std::vector< fmi2_value_reference_t >;
	using size_type = Variables::size_type;
	using Indexes = std::vector< size_type >;

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

protected: // Creation

	// Name + Tolerance + Value Constructor
	Variable(
	 std::string const & name,
	 Real const rTol,
	 Real const aTol,
	 Real const xIni,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Target( name ),
	 rTol( std::max( rTol, 0.0 ) ),
	 aTol( std::max( aTol, std::numeric_limits< Real >::min() ) ),
	 xIni( xIni ),
	 dt_min( options::dtMin ),
	 dt_max( options::dtMax ),
	 dt_inf( options::dtInf ),
	 dt_inf_rlx( options::dtInf == infinity ? infinity : 0.5 * options::dtInf ),
	 var( var ),
	 der( der )
	{}

	// Name + Tolerance Constructor
	Variable(
	 std::string const & name,
	 Real const rTol,
	 Real const aTol,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Target( name ),
	 rTol( std::max( rTol, 0.0 ) ),
	 aTol( std::max( aTol, std::numeric_limits< Real >::min() ) ),
	 xIni( 0.0 ),
	 dt_min( options::dtMin ),
	 dt_max( options::dtMax ),
	 dt_inf( options::dtInf ),
	 dt_inf_rlx( options::dtInf == infinity ? infinity : 0.5 * options::dtInf ),
	 var( var ),
	 der( der )
	{}

	// Name + Value Constructor
	Variable(
	 std::string const & name,
	 Real const xIni,
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
	 der( der )
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
	 der( der )
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

public: // Properties

	// Order of Method
	virtual
	int
	order() const = 0;

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

	// Discrete Advance: Stage 0
	virtual
	void
	advance_discrete_0()
	{
		assert( false );
	}

	// Discrete Advance: Stage 1
	virtual
	void
	advance_discrete_1()
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

	// Handler Advance: Stage 3
	virtual
	void
	advance_handler_3()
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

	// Observer Advance
	virtual
	void
	advance_observer( Time const )
	{
		assert( false );
	}

	// Observer Advance: Simultaneous
	virtual
	void
	advance_observer_s( Time const )
	{
		assert( false );
	}

public: // Methods: FMU

	// Get FMU Real Variable Value
	Real
	fmu_get_real() const
	{
		return fmu::get_real( var.ref );
	}

	// Set FMU Real Variable to a Value
	void
	fmu_set_real( Real const v ) const
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

	// Get FMU Integer Variable Value
	Integer
	fmu_get_integer() const
	{
		return fmu::get_integer( var.ref );
	}

	// Set FMU Integer Variable to a Value
	void
	fmu_set_integer( Integer const v ) const
	{
		fmu::set_integer( var.ref, v );
	}

	// Get FMU Boolean Variable Value
	bool
	fmu_get_boolean() const
	{
		return fmu::get_boolean( var.ref );
	}

	// Set FMU Boolean Variable to a Value
	void
	fmu_set_boolean( bool const v ) const
	{
		fmu::set_boolean( var.ref, v );
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

	// Get FMU Variable Derivative
	Real
	fmu_get_deriv() const
	{
		return fmu::get_real( der.ref );
	}

	// Get Polynomial Trajectory Term 1 from FMU
	Real
	fmu_get_poly_1() const
	{
		return fmu::get_real( der.ref );
	}

// The fmu_get_poly function implementations below are placeholders until new FMI API is available
// They set observee x (continuous) state to do numeric differentiation but context may want x or s state
// This also does not produce correct results for variables with order 2+ terms in simultaenous QSS
//  events if the trigger and observee sets overlap because we can't correctly set the FMU variable values
//  at time > t when order 2+ terms are not yet set

	// Get Polynomial Trajectory Term 2 from FMU
	Real
	fmu_get_poly_2() const
	{ // Numeric differentiation placeholder pending FMI API for higher derivatives
		Time const t( fmu::get_time() ); // Assume FMU time is already set
		Real const x_1( fmu::get_real( der.ref ) );
		Time const tND( t + options::dtNum );
		fmu::set_time( tND ); // Advance FMU time for numeric differentiation
		fmu_set_observees_q( tND );
		if ( self_observer ) fmu_set_q( tND );
		Real const x_2( options::one_half_over_dtNum * ( fmu::get_real( der.ref ) - x_1 ) );
		fmu::set_time( t ); // Restore FMU time
		fmu_set_observees_q( t );
		if ( self_observer ) fmu_set_q( t );
		return x_2;
	}

	// Get Polynomial Trajectory Term 3 from FMU
	Real
	fmu_get_poly_3() const
	{ // Numeric differentiation placeholder pending FMI API for higher derivatives
		Time const t( fmu::get_time() ); // Assume FMU time is already set
		Real const d_1( fmu::get_real( der.ref ) );
		Time tND( t + options::dtNum );
		fmu::set_time( tND ); // Advance FMU time for numeric differentiation
		fmu_set_observees_q( tND );
		if ( self_observer ) fmu_set_q( tND );
		Real const d_2( fmu::get_real( der.ref ) );
		tND += options::dtNum;
		fmu::set_time( tND ); // Advance FMU time for numeric differentiation
		fmu_set_observees_q( tND );
		if ( self_observer ) fmu_set_q( tND );
		Real const d_3( fmu::get_real( der.ref ) );
		Real const x_3( options::one_sixth_over_dtNum_squared * ( d_1 - ( 2.0 * d_2 ) + d_3 ) );
		fmu::set_time( t ); // Restore FMU time
		fmu_set_observees_q( t );
		if ( self_observer ) fmu_set_q( t );
		return x_3;
	}

// Eliminate these variants when FMI API for higher derivatives is available

	// Get Polynomial Trajectory Term 2 from FMU
	Real
	fmu_get_poly_2_x() const
	{ // Numeric differentiation placeholder pending FMI API for higher derivatives
		Time const t( fmu::get_time() ); // Assume FMU time is already set
		Real const x_1( fmu::get_real( der.ref ) );
		Time const tND( t + options::dtNum );
		fmu::set_time( tND ); // Advance FMU time for numeric differentiation
		fmu_set_observees_x( tND );
		if ( self_observer ) fmu_set_x( tND );
		Real const x_2( options::one_half_over_dtNum * ( fmu::get_real( der.ref ) - x_1 ) );
		fmu::set_time( t ); // Restore FMU time
		fmu_set_observees_x( t );
		if ( self_observer ) fmu_set_x( t );
		return x_2;
	}

	// Get Polynomial Trajectory Term 3 from FMU
	Real
	fmu_get_poly_3_x() const
	{ // Numeric differentiation placeholder pending FMI API for higher derivatives
		Time const t( fmu::get_time() ); // Assume FMU time is already set
		Real const d_1( fmu::get_real( der.ref ) );
		Time tND( t + options::dtNum );
		fmu::set_time( tND ); // Advance FMU time for numeric differentiation
		fmu_set_observees_x( tND );
		if ( self_observer ) fmu_set_x( tND );
		Real const d_2( fmu::get_real( der.ref ) );
		tND += options::dtNum;
		fmu::set_time( tND ); // Advance FMU time for numeric differentiation
		fmu_set_observees_x( tND );
		if ( self_observer ) fmu_set_x( tND );
		Real const d_3( fmu::get_real( der.ref ) );
		Real const x_3( options::one_sixth_over_dtNum_squared * ( d_1 - ( 2.0 * d_2 ) + d_3 ) );
		fmu::set_time( t ); // Restore FMU time
		fmu_set_observees_x( t );
		if ( self_observer ) fmu_set_x( t );
		return x_3;
	}

//	// Get Polynomial Trajectory Terms 1,2 from FMU
//	void
//	fmu_get_poly_1_2( Real & x_1, Real & x_2 ) const
//	{ // Numeric differentiation placeholder pending FMI API for higher derivatives
//		Time const t( fmu::get_time() ); // Assume FMU time is already set
//		x_1 = fmu::get_real( der.ref );
//		Time const tND( t + options::dtNum );
//		fmu::set_time( tND ); // Advance FMU time for numeric differentiation
//		fmu_set_observees_q( tND );
//		if ( self_observer ) fmu_set_q( tND );
//		x_2 = options::one_half_over_dtNum * ( fmu::get_real( der.ref ) - x_1 );
//		fmu::set_time( t ); // Restore FMU time
//	}
//
//	// Get Polynomial Trajectory Terms 1,2,3 from FMU
//	void
//	fmu_get_poly_1_2_3( Real & x_1, Real & x_2, Real & x_3 ) const
//	{ // Numeric differentiation placeholder pending FMI API for higher derivatives
//		Time const t( fmu::get_time() ); // Assume FMU time is already set
//		x_1 = fmu::get_real( der.ref );
//		Time tND( t + options::dtNum );
//		fmu::set_time( tND ); // Advance FMU time for numeric differentiation
//		fmu_set_observees_q( tND );
//		if ( self_observer ) fmu_set_q( tND );
//		Real const d_2( fmu::get_real( der.ref ) );
//		x_2 = options::one_half_over_dtNum * ( d_2 - x_1 );
//		tND += options::dtNum;
//		fmu::set_time( tND ); // Advance FMU time for numeric differentiation
//		fmu_set_observees_q( tND );
//		if ( self_observer ) fmu_set_q( tND );
//		Real const d_3( fmu::get_real( der.ref ) );
//		x_3 = options::one_sixth_over_dtNum_squared * ( x_1 - ( 2.0 * d_2 ) + d_3 );
//		fmu::set_time( t ); // Restore FMU time
//	}

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
	bool self_observer{ false }; // Variable appears in its function/derivative?
	If_Clauses if_clauses; // Clauses in conditional if blocks
	When_Clauses when_clauses; // Clauses in conditional when blocks
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

};

} // fmu
} // QSS

#endif
