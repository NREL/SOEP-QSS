// Zero-Crossing Variable Triggers
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

#ifndef QSS_Triggers_ZC_hh_INCLUDED
#define QSS_Triggers_ZC_hh_INCLUDED

// QSS Headers
#include <QSS/FMU_ME.hh>
#include <QSS/RefsValsDers.hh>
#include <QSS/container.hh>
#include <QSS/options.hh>
#include <QSS/SuperdenseTime.hh>

// C++ Headers
#include <algorithm>
#include <cassert>

namespace QSS {

// Zero-Crossing Variable Triggers
template< typename V >
class Triggers_ZC final
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
	Triggers_ZC( FMU_ME * fmu_me = nullptr ) :
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
		if ( triggers.empty() ) {
			clear();
			return;
		}

		assert( is_unique( triggers ) ); // Precondition: No duplicates
		assert( all_same_order( triggers ) ); // Precondition

		triggers_ = triggers;
		n_triggers_ = triggers_.size();
		order_ = triggers_[ 0 ]->order();

		// FMU pooled data set up
		vars_.clear(); vars_.reserve( n_triggers_ );
		for ( Variable * trigger : triggers_ ) {
			assert( trigger->is_ZC() );
			vars_.push_back( trigger->var().ref() );
		}

		// Observees set up
		observees_.clear();
		for ( Variable * trigger : triggers_ ) {
			for ( auto observee : trigger->observees() ) {
				observees_.push_back( observee );
			}
		}
		uniquify( observees_ );
		n_observees_ = observees_.size();
		observees_v_ref_.clear(); observees_v_ref_.reserve( n_observees_ );
		observees_v_.clear(); observees_v_.resize( n_observees_ );
		observees_dv_.clear(); observees_dv_.resize( n_observees_ );
		for ( Variable const * observee : observees_ ) {
			observees_v_ref_.push_back( observee->var().ref() );
		}
	}

	// QSS Advance Triggers
	void
	advance_QSS( Time const t, SuperdenseTime const & s )
	{
		assert( !triggers_.empty() );
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );
		assert( vars_.size() == n_triggers_ );

		set_observees_values( t );
		fmu_me_->get_reals( n_triggers_, vars_.refs.data(), vars_.vals.data() );
		for ( size_type i = 0u; i < n_triggers_; ++i ) { // Requantization stage 0
			Variable * trigger( triggers_[ i ] );
			assert( trigger->tE >= t ); // Bin variables tE can be > t
			trigger->tE = t; // Bin variables tE can be > t
			trigger->st = s; // Set trigger superdense time
			trigger->advance_QSS_0( vars_.vals[ i ] );
		}
		set_observees_dv( t );
		fmu_me_->get_directional_derivatives(
		 observees_v_ref_.data(),
		 n_observees_,
		 vars_.refs.data(),
		 n_triggers_,
		 observees_dv_.data(),
		 vars_.ders.data()
		);
		for ( size_type i = 0u; i < n_triggers_; ++i ) { // Requantization stage 1
			triggers_[ i ]->advance_QSS_1( vars_.ders[ i ] );
		}

		if ( order_ >= 3 ) {
			Time tN( t - options::dtND );
			if ( fwd_time( tN ) ) { // Use centered ND formulas
				fmu_me_->set_time( tN );
				set_observees_values( tN );
				set_observees_dv( tN );
				fmu_me_->get_directional_derivatives(
				 observees_v_ref_.data(),
				 n_observees_,
				 vars_.refs.data(),
				 n_triggers_,
				 observees_dv_.data(),
				 vars_.ders.data()
				);
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				set_observees_values( tN );
				set_observees_dv( tN );
				fmu_me_->get_directional_derivatives(
				 observees_v_ref_.data(),
				 n_observees_,
				 vars_.refs.data(),
				 n_triggers_,
				 observees_dv_.data(),
				 vars_.ders_p.data()
				);
				for ( size_type i = 0u; i < n_triggers_; ++i ) { // Requantization stage 2
					triggers_[ i ]->advance_QSS_2( vars_.ders[ i ], vars_.ders_p[ i ] );
				}
				for ( Variable * trigger : triggers_ ) { // Requantization stage 3
					trigger->advance_QSS_3();
				}
			} else { // Use forward ND formulas
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				set_observees_values( tN );
				set_observees_dv( tN );
				fmu_me_->get_directional_derivatives(
				 observees_v_ref_.data(),
				 n_observees_,
				 vars_.refs.data(),
				 n_triggers_,
				 observees_dv_.data(),
				 vars_.ders.data()
				);
				tN = t + options::two_dtND;
				fmu_me_->set_time( tN );
				set_observees_values( tN );
				set_observees_dv( tN );
				fmu_me_->get_directional_derivatives(
				 observees_v_ref_.data(),
				 n_observees_,
				 vars_.refs.data(),
				 n_triggers_,
				 observees_dv_.data(),
				 vars_.ders_p.data()
				);
				for ( size_type i = 0u; i < n_triggers_; ++i ) { // Requantization stage 2
					triggers_[ i ]->advance_QSS_2_forward( vars_.ders[ i ], vars_.ders_p[ i ] );
				}
				for ( Variable * trigger : triggers_ ) { // Requantization stage 3
					trigger->advance_QSS_3_forward();
				}
			}
			fmu_me_->set_time( t );
		} else if ( order_ >= 2 ) {
			Time const tN( t + options::dtND );
			fmu_me_->set_time( tN );
			set_observees_values( tN );
			set_observees_dv( tN );
			fmu_me_->get_directional_derivatives(
			 observees_v_ref_.data(),
			 n_observees_,
			 vars_.refs.data(),
			 n_triggers_,
			 observees_dv_.data(),
			 vars_.ders_p.data()
			);
			for ( size_type i = 0u; i < n_triggers_; ++i ) { // Requantization stage 2
				triggers_[ i ]->advance_QSS_2( vars_.ders_p[ i ] );
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
		n_triggers_ = 0u;
		triggers_.clear();
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

	// Triggers_ZC[ i ]
	Variable const *
	operator []( size_type const i ) const
	{
		return triggers_[ i ];
	}

	// Triggers_ZC[ i ]
	Variable *
	operator []( size_type const i )
	{
		return triggers_[ i ];
	}

private: // Methods

	// Set Observees FMU Values at Time t
	void
	set_observees_values( Time const t )
	{
		for ( size_type i = 0; i < n_observees_; ++i ) {
			observees_v_[ i ] = observees_[ i ]->x( t );
		}
		fmu_me_->set_reals( n_observees_, observees_v_ref_.data(), observees_v_.data() ); // Set observees FMU values
	}

	// Set Observees Derivative Vector at Time t
	void
	set_observees_dv( Time const t )
	{
		for ( size_type i = 0u; i < n_observees_; ++i ) {
			observees_dv_[ i ] = observees_[ i ]->x1( t );
		}
	}

private: // Data

	FMU_ME * fmu_me_{ nullptr }; // FMU-ME (non-owning) pointer

	// Triggers
	size_type n_triggers_{ 0u }; // Number of triggers
	int order_{ 0 }; // Order of triggers
	Variables triggers_; // Triggers

	// Observees
	size_type n_observees_{ 0u }; // Number of triggers observees
	Variables observees_; // Triggers observees
	VariableRefs observees_v_ref_; // Triggers observees value references
	Reals observees_v_; // Triggers observees values
	Reals observees_dv_; // Triggers observees derivatives

	// Trigger FMU pooled call data
	RefsValsDers< Variable > vars_; // Values and derivatives

}; // Triggers_ZC

} // QSS

#endif
