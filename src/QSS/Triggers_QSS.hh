// QSS Variable Triggers
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

#ifndef QSS_Triggers_QSS_hh_INCLUDED
#define QSS_Triggers_QSS_hh_INCLUDED

// QSS Headers
#include <QSS/FMU_ME.hh>
#include <QSS/RefsDers.hh>
#include <QSS/RefsDirDers.hh> //n2d
#include <QSS/container.hh>
#include <QSS/options.hh>
#include <QSS/SuperdenseTime.hh>

// C++ Headers
#include <algorithm>
#include <cassert>

namespace QSS {

// QSS Variable Triggers
template< typename V >
class Triggers_QSS final
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
	Triggers_QSS( FMU_ME * fmu_me = nullptr ) :
	 fmu_me_( fmu_me ),
	 advance_ptr( options::d2d ? &Triggers_QSS::advance_d2d : &Triggers_QSS::advance_n2d )
	{}

public: // Predicate

	// Forward Time?
	bool
	fwd_time( Time const t ) const
	{
		assert( fmu_me_ != nullptr );
		return t >= fmu_me_->t0;
	}

public: // Methods

	// QSS Advance Triggers
	void
	advance( Variables & triggers, Time const t, SuperdenseTime const & s )
	{
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );

		if ( triggers.empty() ) {
			clear();
			return;
		}

		assert( is_unique( triggers ) ); // Precondition: No duplicates
		assert( all_same_order( triggers ) ); // Precondition

		n_triggers_ = triggers.size();
		order_ = triggers[ 0 ]->order();

		// FMU pooled data set up
		if ( options::d2d ) {
			qss_ders_.clear(); qss_ders_.reserve( n_triggers_ );
			for ( Variable * trigger : triggers ) {
				assert( trigger->is_QSS() );
				qss_ders_.push_back( trigger->der().ref() );
			}
		} else {
			assert( options::n2d );
			qss_dn2d_.clear(); qss_dn2d_.reserve( n_triggers_ );
			for ( Variable * trigger : triggers ) {
				assert( trigger->is_QSS() );
				qss_dn2d_.push_back( trigger->der().ref() );
			}
		}

		// Observees set up
		observees_.clear();
		for ( Variable * trigger : triggers ) {
			for ( Variable * observee : trigger->observees() ) {
				observees_.push_back( observee );
			}
		}
		uniquify( observees_ );
		n_observees_ = observees_.size();
		observees_v_ref_.clear(); observees_v_ref_.reserve( n_observees_ );
		observees_v_.clear(); observees_v_.resize( n_observees_ );
		if ( options::d2d ) { observees_dv_.clear(); observees_dv_.resize( n_observees_ ); }
		for ( Variable const * observee : observees_ ) {
			observees_v_ref_.push_back( observee->var().ref() );
		}

		(this->*advance_ptr)( triggers, t, s );
	}

	// QSS Advance Triggers: Directional Second Derivatives
	void
	advance_d2d( Variables & triggers, Time const t, SuperdenseTime const & s )
	{
		assert( !triggers.empty() );
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );
		assert( qss_ders_.size() == n_triggers_ );

		for ( Variable * trigger : triggers ) { // Requantization stage 0
			assert( trigger->tE >= t ); // Bin variables tE can be > t
			trigger->tE = t; // Bin variables tE can be > t
			trigger->st = s; // Set trigger superdense time
			trigger->advance_QSS_0();
		}

		set_observees_values( t );
		fmu_me_->get_reals( n_triggers_, qss_ders_.refs.data(), qss_ders_.ders.data() );
		for ( size_type i = 0u; i < n_triggers_; ++i ) { // Requantization stage 1
			triggers[ i ]->advance_QSS_1( qss_ders_.ders[ i ] );
		}

		if ( order_ >= 2 ) {
			get_second_derivatives( t );
			for ( size_type i = 0u; i < n_triggers_; ++i ) { // Requantization stage 2
				triggers[ i ]->advance_QSS_2_dd2( qss_ders_.ders[ i ] );
			}
			if ( order_ >= 3 ) {
				Time const tN( t + options::dtND );
				fmu_me_->set_time( tN );
				set_observees_values( tN );
				get_second_derivatives( tN );
				for ( size_type i = 0u; i < n_triggers_; ++i ) { // Requantization stage 3
					triggers[ i ]->advance_QSS_3_dd2( qss_ders_.ders[ i ] );
				}
				fmu_me_->set_time( t );
			}
		}
		for ( Variable * trigger : triggers ) { // Requantization stage final
			trigger->advance_QSS_F();
		}
	}

	// QSS Advance Triggers: Numerical Second Derivatives
	void
	advance_n2d( Variables & triggers, Time const t, SuperdenseTime const & s )
	{
		assert( !triggers.empty() );
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );
		assert( qss_dn2d_.size() == n_triggers_ );

		for ( Variable * trigger : triggers ) { // Requantization stage 0
			assert( trigger->tE >= t ); // Bin variables tE can be > t
			trigger->tE = t; // Bin variables tE can be > t
			trigger->st = s; // Set trigger superdense time
			trigger->advance_QSS_0();
		}

		set_observees_values( t );
		fmu_me_->get_reals( n_triggers_, qss_dn2d_.refs.data(), qss_dn2d_.ders.data() );
		for ( size_type i = 0u; i < n_triggers_; ++i ) { // Requantization stage 1
			triggers[ i ]->advance_QSS_1( qss_dn2d_.ders[ i ] );
		}
		if ( order_ >= 3 ) {
			Time tN( t - options::dtND );
			if ( fwd_time( tN ) ) { // Centered ND
				fmu_me_->set_time( tN );
				set_observees_values( tN );
				fmu_me_->get_reals( n_triggers_, qss_dn2d_.refs.data(), qss_dn2d_.ders.data() );
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				set_observees_values( tN );
				fmu_me_->get_reals( n_triggers_, qss_dn2d_.refs.data(), qss_dn2d_.ders_p.data() );
				for ( size_type i = 0u; i < n_triggers_; ++i ) { // Requantization stage 2
					triggers[ i ]->advance_QSS_2( qss_dn2d_.ders[ i ], qss_dn2d_.ders_p[ i ] );
				}
				for ( size_type i = 0u; i < n_triggers_; ++i ) { // Requantization stage 3
					triggers[ i ]->advance_QSS_3();
				}
			} else { // Forward ND
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				set_observees_values( tN );
				fmu_me_->get_reals( n_triggers_, qss_dn2d_.refs.data(), qss_dn2d_.ders.data() );
				tN = t + options::two_dtND;
				fmu_me_->set_time( tN );
				set_observees_values( tN );
				fmu_me_->get_reals( n_triggers_, qss_dn2d_.refs.data(), qss_dn2d_.ders_p.data() );
				for ( size_type i = 0u; i < n_triggers_; ++i ) { // Requantization stage 2
					triggers[ i ]->advance_QSS_2_forward( qss_dn2d_.ders[ i ], qss_dn2d_.ders_p[ i ] );
				}
				for ( Variable * trigger : triggers ) { // Requantization stage 3
					trigger->advance_QSS_3_forward();
				}
			}
			fmu_me_->set_time( t );
		} else if ( order_ >= 2 ) {
			Time const tN( t + options::dtND );
			fmu_me_->set_time( tN );
			set_observees_values( tN );
			fmu_me_->get_reals( n_triggers_, qss_dn2d_.refs.data(), qss_dn2d_.ders_p.data() );
			for ( size_type i = 0u; i < n_triggers_; ++i ) { // Requantization stage 2
				triggers[ i ]->advance_QSS_2( qss_dn2d_.ders_p[ i ] );
			}
			fmu_me_->set_time( t );
		}
		for ( Variable * trigger : triggers ) { // Requantization stage final
			trigger->advance_QSS_F();
		}
	}

	// Clear
	void
	clear()
	{
		n_triggers_ = 0u;
	}

private: // Methods

	// Set Observees FMU Values at Time t
	void
	set_observees_values( Time const t )
	{
		for ( size_type i = 0; i < n_observees_; ++i ) {
#ifndef QSS_PROPAGATE_CONTINUOUS
			observees_v_[ i ] = observees_[ i ]->q( t ); // Quantized: Traditional QSS
#else
			observees_v_[ i ] = observees_[ i ]->x( t ); // Continuous: Modified QSS
#endif
		}
		fmu_me_->set_reals( n_observees_, observees_v_ref_.data(), observees_v_.data() ); // Set observees FMU values
	}

	// Get Second Derivatives at Time t
	void
	get_second_derivatives( Time const t )
	{
		assert( options::d2d );
		for ( size_type i = 0; i < n_observees_; ++i ) { // Set directional derivative seed vector
#ifndef QSS_PROPAGATE_CONTINUOUS
			observees_dv_[ i ] = observees_[ i ]->q1( t ); // Quantized: Traditional QSS
#else
			observees_dv_[ i ] = observees_[ i ]->x1( t ); // Continuous: Modified QSS
#endif
		}
		fmu_me_->get_directional_derivatives(
		 observees_v_ref_.data(),
		 n_observees_,
		 qss_ders_.refs.data(),
		 n_triggers_,
		 observees_dv_.data(),
		 qss_ders_.ders.data()
		); // Get 2nd derivatives
	}

private: // Data

	FMU_ME * fmu_me_{ nullptr }; // FMU-ME (non-owning) pointer

	// Triggers
	size_type n_triggers_{ 0u }; // Number of triggers
	int order_{ 0 }; // Order of triggers

	// Observees
	size_type n_observees_{ 0u }; // Number of triggers observees
	Variables observees_; // Triggers observees
	VariableRefs observees_v_ref_; // Triggers observees value references
	Reals observees_v_; // Triggers observees values
	Reals observees_dv_; // Triggers observees derivatives

	// Trigger FMU pooled call data
	RefsDirDers< Variable > qss_ders_; // Triggers derivatives
	RefsDers< Variable > qss_dn2d_; //n2d Triggers derivatives

	// QSS advance method pointer
	void (Triggers_QSS::*advance_ptr)( Variables & triggers, Time const t, SuperdenseTime const & s ){ nullptr };

}; // Triggers_QSS

} // QSS

#endif
