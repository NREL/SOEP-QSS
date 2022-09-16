// Real Variable Triggers
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2022 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_Triggers_R_hh_INCLUDED
#define QSS_Triggers_R_hh_INCLUDED

// QSS Headers
#include <QSS/FMU_ME.hh>
#include <QSS/RefsDers.hh>
#include <QSS/RefsVals.hh>
#include <QSS/RefsValsDers.hh>
#include <QSS/container.hh>
#include <QSS/math.hh>
#include <QSS/options.hh>
#include <QSS/Range.hh>
#include <QSS/SuperdenseTime.hh>

// C++ Headers
#include <algorithm>
#include <cassert>

namespace QSS {

// Real Variable Triggers
template< typename V >
class Triggers_R final
{

public: // Types

	using Variable = V;
	using Time = typename Variable::Time;
	using Real = typename Variable::Real;
	using Reals = typename Variable::Reals;
	using Variables = typename Variable::Variables;
	using VariableRef = typename Variable::VariableRef;
	using VariableRefs = typename Variable::VariableRefs;
	using size_type = typename Variables::size_type;
	using const_iterator = typename Variables::const_iterator;
	using iterator = typename Variables::iterator;
	using const_pointer = typename Variables::const_pointer;
	using pointer = typename Variables::pointer;
	using const_reference = typename Variables::const_reference;
	using reference = typename Variables::reference;

public: // Creation

	// Constructor
	explicit
	Triggers_R( FMU_ME * fmu_me = nullptr ) :
	 fmu_me_( fmu_me )
	{}

public: // Conversion

	// Triggers Conversion
	operator Variables const &() const
	{
		return triggers_;
	}

	// Triggers Conversion
	operator Variables &()
	{
		return triggers_;
	}

public: // Predicate

	// Empty?
	bool
	empty() const
	{
		return triggers_.empty();
	}

	// Have Observer(s)?
	bool
	have() const
	{
		return ( !triggers_.empty() );
	}

	// Forward Time?
	bool
	fwd_time( Time const t ) const
	{
		assert( fmu_me_ != nullptr );
		return t >= fmu_me_->t0;
	}

public: // Property

	// Size
	size_type
	size() const
	{
		return triggers_.size();
	}

	// Triggers
	Variables const &
	triggers() const
	{
		return triggers_;
	}

	// Triggers
	Variables &
	triggers()
	{
		return triggers_;
	}

public: // Methods

	// Assign a Triggers Collection
	void
	assign( Variables & triggers )
	{
		triggers_ = triggers;

		if ( triggers_.empty() ) {
			reset_specs();
			return;
		}

		// Sort by order
		assert( is_unique( triggers_ ) ); // Precondition: No duplicates
		sort_by_order( triggers_ );

		// Set specs
		set_specs();

		// FMU pooled data set up
		r_vars_.clear(); r_vars_.reserve( qss_.n() );
		for ( Variable * trigger : triggers_ ) {
			assert( trigger->is_R() );
			r_vars_.push_back( trigger->var().ref() );
		}

		// Observees set up
		qss_observees_.clear();
		for ( Variable * trigger : triggers_ ) {
			if ( trigger->self_observer() ) qss_observees_.push_back( trigger );
			for ( auto observee : trigger->observees() ) {
				qss_observees_.push_back( observee );
			}
		}
		uniquify( qss_observees_ );
		if ( !uni_order_ ) {
			assert( qss2_.have() );
			qss2_observees_.clear();
			for ( size_type i = qss2_.b(), e = qss_.e(); i < e; ++i ) { // Order 2+ triggers
				Variable * trigger( triggers_[ i ] );
				if ( trigger->self_observer() ) qss2_observees_.push_back( trigger );
				for ( auto observee : trigger->observees() ) {
					qss2_observees_.push_back( observee );
				}
			}
			uniquify( qss2_observees_ );
			if ( qss3_.have() ) {
				qss3_observees_.clear();
				for ( size_type i = qss3_.b(), e = qss_.e(); i < e; ++i ) { // Order 3+ triggers
					Variable * trigger( triggers_[ i ] );
					if ( trigger->self_observer() ) qss3_observees_.push_back( trigger );
					for ( auto observee : trigger->observees() ) {
						qss3_observees_.push_back( observee );
					}
				}
				uniquify( qss3_observees_ );
			}
		}

		// Observee directional derivative seed array set up
		qss_observees_v_ref_.clear();
		qss_observees_dv_.clear();
		for ( auto observee : qss_observees_ ) {
			qss_observees_v_ref_.push_back( observee->var().ref() );
			qss_observees_dv_.push_back( 0.0 );
		}
	}

	// QSS Advance
	void
	advance_QSS( Time const t, SuperdenseTime const & s )
	{
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );
		assert( qss_.n() == r_vars_.size() );

		for ( Variable * observee : qss_observees_ ) {
			observee->fmu_set_x( t );
		}
		fmu_me_->get_reals( qss_.n(), &r_vars_.refs[ 0 ], &r_vars_.vals[ 0 ] );
		for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) {
			Variable * trigger( triggers_[ i ] );
			assert( trigger->is_R() ); // Real variable trigger
			assert( trigger->tE >= t ); // Bin variables tE can be > t
			trigger->tE = t; // Bin variables tE can be > t
			trigger->st = s; // Set trigger superdense time
			trigger->advance_QSS_0( r_vars_.vals[ i ] );
		}
		for ( size_type i = 0, e = qss_observees_.size(); i < e; ++i ) {
			qss_observees_dv_[ i ] = qss_observees_[ i ]->x1( t );
		}
		fmu_me_->get_directional_derivatives(
			qss_observees_v_ref_.data(),
			qss_observees_v_ref_.size(),
			r_vars_.refs.data(),
			r_vars_.refs.size(),
			qss_observees_dv_.data(),
			r_vars_.ders.data()
		);
		for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) {
			triggers_[ i ]->advance_QSS_1( r_vars_.ders[ i ] );
		}
		if ( qss3_.have() ) {
			Time tN( t - options::dtND );
			if ( fwd_time( tN ) ) { // Use centered ND formulas
				fmu_me_->set_time( tN );
				for ( Variable * observee : uni_order_ ? qss_observees_ : qss2_observees_ ) {
					observee->fmu_set_x( tN );
				}
				for ( size_type i = 0, e = qss_observees_.size(); i < e; ++i ) { //? Worth it to do this for observees of order 2+ triggers only?
					qss_observees_dv_[ i ] = qss_observees_[ i ]->x1( tN );
				}
					fmu_me_->get_directional_derivatives(
					qss_observees_v_ref_.data(),
					qss_observees_v_ref_.size(),
					r_vars_.refs.data(),
					r_vars_.refs.size(),
					qss_observees_dv_.data(),
					r_vars_.ders_m.data()
				);
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : uni_order_ ? qss_observees_ : qss2_observees_ ) {
					observee->fmu_set_x( tN );
				}
				for ( size_type i = 0, e = qss_observees_.size(); i < e; ++i ) { //? Worth it to do this for observees of order 2+ triggers only?
					qss_observees_dv_[ i ] = qss_observees_[ i ]->x1( tN );
				}
					fmu_me_->get_directional_derivatives(
					qss_observees_v_ref_.data(),
					qss_observees_v_ref_.size(),
					r_vars_.refs.data(),
					r_vars_.refs.size(),
					qss_observees_dv_.data(),
					r_vars_.ders_p.data()
				);
				for ( size_type i = qss2_.b(), e = qss_.e(); i < e; ++i ) { // Order 2+ triggers
					triggers_[ i ]->advance_QSS_2( r_vars_.ders_m[ i ], r_vars_.ders_p[ i ] );
				}
				for ( size_type i = qss3_.b(), e = qss_.e(); i < e; ++i ) { // Order 3+ triggers
					triggers_[ i ]->advance_QSS_3();
				}
			} else { // Use forward ND formulas
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : uni_order_ ? qss_observees_ : qss2_observees_ ) {
					observee->fmu_set_x( tN );
				}
				for ( size_type i = 0, e = qss_observees_.size(); i < e; ++i ) { //? Worth it to do this for observees of order 2+ triggers only?
					qss_observees_dv_[ i ] = qss_observees_[ i ]->x1( tN );
				}
					fmu_me_->get_directional_derivatives(
					qss_observees_v_ref_.data(),
					qss_observees_v_ref_.size(),
					r_vars_.refs.data(),
					r_vars_.refs.size(),
					qss_observees_dv_.data(),
					r_vars_.ders_m.data()
				);
				tN = t + options::two_dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : uni_order_ ? qss_observees_ : qss2_observees_ ) {
					observee->fmu_set_x( tN );
				}
				for ( size_type i = 0, e = qss_observees_.size(); i < e; ++i ) { //? Worth it to do this for observees of order 2+ triggers only?
					qss_observees_dv_[ i ] = qss_observees_[ i ]->x1( tN );
				}
					fmu_me_->get_directional_derivatives(
					qss_observees_v_ref_.data(),
					qss_observees_v_ref_.size(),
					r_vars_.refs.data(),
					r_vars_.refs.size(),
					qss_observees_dv_.data(),
					r_vars_.ders_p.data()
				);
				for ( size_type i = qss2_.b(), e = qss_.e(); i < e; ++i ) { // Order 2+ triggers
					triggers_[ i ]->advance_QSS_2_forward( r_vars_.ders_m[ i ], r_vars_.ders_p[ i ] );
				}
				for ( size_type i = qss3_.b(), e = qss_.e(); i < e; ++i ) { // Order 3+ triggers
					triggers_[ i ]->advance_QSS_3_forward();
				}
			}
			fmu_me_->set_time( t );
		} else if ( qss2_.have() ) {
			Time tN( t + options::dtND );
			fmu_me_->set_time( tN );
			for ( Variable * observee : uni_order_ ? qss_observees_ : qss2_observees_ ) {
				observee->fmu_set_x( tN );
			}
			for ( size_type i = 0, e = qss_observees_.size(); i < e; ++i ) { //? Worth it to do this for observees of order 2+ triggers only?
				qss_observees_dv_[ i ] = qss_observees_[ i ]->x1( tN );
			}
			fmu_me_->get_directional_derivatives(
				qss_observees_v_ref_.data(),
				qss_observees_v_ref_.size(),
				r_vars_.refs.data(),
				r_vars_.refs.size(),
				qss_observees_dv_.data(),
				r_vars_.ders_p.data()
			);
			for ( size_type i = qss2_.b(), e = qss_.e(); i < e; ++i ) { // Order 2+ triggers
				triggers_[ i ]->advance_QSS_2( r_vars_.ders_p[ i ] );
			}
			fmu_me_->set_time( t );
		}
		for ( Variable * trigger : triggers_ ) {
			trigger->advance_QSS_F();
		}
	}

	// Clear
	void
	clear()
	{
		triggers_.clear();
		reset_specs();
	}

public: // Iterator

	// Begin Iterator
	const_iterator
	begin() const
	{
		return triggers_.begin();
	}

	// Begin Iterator
	iterator
	begin()
	{
		return triggers_.begin();
	}

	// End Iterator
	const_iterator
	end() const
	{
		return triggers_.end();
	}

	// End Iterator
	iterator
	end()
	{
		return triggers_.end();
	}

public: // Subscript

	// Triggers_R[ i ]
	Variable const *
	operator []( size_type const i ) const
	{
		return triggers_[ i ];
	}

	// Triggers_R[ i ]
	Variable *
	operator []( size_type const i )
	{
		return triggers_[ i ];
	}

private: // Methods

	// Reset Specs
	void
	reset_specs()
	{
		qss_.reset();
		qss2_.reset();
		qss3_.reset();
	}

	// Set Specs
	void
	set_specs()
	{
		reset_specs();
		if ( triggers_.empty() ) return;

		qss_.b() = 0u;
		qss_.e() = qss2_.b() = qss2_.e() = qss3_.b() = qss3_.e() = triggers_.size();
		for ( size_type i = 0, e = qss_.e(); i < e; ++i ) {
			int const order( triggers_[ i ]->order() );
			if ( order >= 2 ) {
				qss2_.b() = std::min( qss2_.b(), i );
				if ( order >= 3 ) {
					qss3_.b() = std::min( qss3_.b(), i );
					break;
				}
			}
		}
		size_type const qss_n( qss_.n() );
		uni_order_ = (
		 ( qss2_.empty() || qss2_.n() == qss_n ) &&
		 ( qss3_.empty() || qss3_.n() == qss_n )
		);
	}

private: // Data

	FMU_ME * fmu_me_{ nullptr }; // FMU-ME (non-owning) pointer

	Variables triggers_; // Triggers

	// Trigger index specs
	bool uni_order_{ false }; // Triggers all the same order?
	Range qss_; // Triggers
	Range qss2_; // Triggers of order 2+
	Range qss3_; // Triggers of order 3+

	// Observees (including self-observers)
	Variables qss_observees_; // Triggers observees
	Variables qss2_observees_; // Triggers of order 2+ observees
	Variables qss3_observees_; // Triggers of order 3+ observees

	// Trigger FMU pooled call data
	RefsValsDers< Variable > r_vars_; // Real variables

	// Observee directional derivative lookup seed data
	VariableRefs qss_observees_v_ref_; // Observee value references
	Reals qss_observees_dv_; // Observee seed derivatives

}; // Triggers_R

} // QSS

#endif
