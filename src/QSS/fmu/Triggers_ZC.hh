// FMU-Based Zero-Crossing Variable Triggers
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

#ifndef QSS_fmu_Triggers_ZC_hh_INCLUDED
#define QSS_fmu_Triggers_ZC_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/FMU_ME.hh>
#include <QSS/fmu/RefsDers.hh>
#include <QSS/fmu/RefsVals.hh>
#include <QSS/fmu/RefsValsEI.hh>
#include <QSS/fmu/RefsValsEIDD.hh>
#include <QSS/container.hh>
#include <QSS/math.hh>
#include <QSS/options.hh>
#include <QSS/Range.hh>
#include <QSS/SuperdenseTime.hh>

// OpenMP Headers
#ifdef _OPENMP
#include <omp.h>
#endif

// C++ Headers
#include <algorithm>
#include <cassert>
#ifdef _OPENMP
#include <cstdint>
#endif

namespace QSS {
namespace fmu {

// FMU-Based Zero-Crossing Variable Triggers
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

	enum class ZC_Type { None, EventIndicator, EventIndicatorDD, Explicit };

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
		return ( ! triggers_.empty() );
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

		// Sort by type and order
		assert( is_unique( triggers_ ) ); // Precondition: No duplicates
		sort_by_order( triggers_ );

		// Set specs
		set_specs();

		// FMU pooled data set up
		size_type const qss_n( qss_.n() );
		if ( zc_type_ == ZC_Type::EventIndicator ) { // Event indicator variables
			ei_vars_.clear(); ei_vars_.reserve( qss_n );
			for ( Variable * trigger : triggers_ ) {
				assert( trigger->is_ZC() && trigger->not_ZCd() && trigger->not_ZCe() );
				ei_vars_.push_back( trigger->var().ref );
			}
		} else if ( zc_type_ == ZC_Type::EventIndicatorDD ) { // Event indicator directional derivative variables
			dd_vars_.clear(); dd_vars_.reserve( qss_n );
			for ( Variable * trigger : triggers_ ) {
				assert( trigger->is_ZCd() );
				dd_vars_.push_back( trigger->var().ref );
			}
		} else { // Explicit zero-crossing variables
			assert( zc_type_ == ZC_Type::Explicit );
			zc_vars_.clear(); zc_vars_.reserve( qss_n );
			zc_ders_.clear(); zc_ders_.reserve( qss_n );
			for ( Variable * trigger : triggers_ ) {
				assert( trigger->is_ZCe() );
				zc_vars_.push_back( trigger->var().ref );
				zc_ders_.push_back( trigger->der().ref );
			}
		}

		// Observees set up
		qss_observees_.clear();
		for ( Variable * trigger : triggers_ ) {
			for ( auto observee : trigger->observees() ) {
				if ( ! observee->is_Discrete() ) qss_observees_.push_back( observee );
			}
		}
		uniquify( qss_observees_ );
		if ( ! uni_order_ ) {
			assert( qss2_.have() );
			qss2_observees_.clear();
			for ( size_type i = qss2_.b(), e = qss_.e(); i < e; ++i ) { // Order 2+ triggers
				Variable * trigger( triggers_[ i ] );
				for ( auto observee : trigger->observees() ) {
					if ( ! observee->is_Discrete() ) qss2_observees_.push_back( observee );
				}
			}
			uniquify( qss2_observees_ );
			if ( qss3_.have() ) {
				qss3_observees_.clear();
				for ( size_type i = qss3_.b(), e = qss_.e(); i < e; ++i ) { // Order 3+ triggers
					Variable * trigger( triggers_[ i ] );
					for ( auto observee : trigger->observees() ) {
						if ( ! observee->is_Discrete() ) qss3_observees_.push_back( observee );
					}
				}
				uniquify( qss3_observees_ );
			}
		}

		// Observee directional derivative seed array set up
		if ( zc_type_ == ZC_Type::EventIndicatorDD ) { // Event indicator directional derivative variables
			qss_observees_v_ref_.clear();
			qss_observees_dv_.clear();
			for ( auto observee : qss_observees_ ) {
				qss_observees_v_ref_.push_back( observee->var().ref );
				qss_observees_dv_.push_back( 0.0 );
			}
		}
	}

	// QSS Advance
	void
	advance_QSS( Time const t, SuperdenseTime const & s ) //Do More OpenMP loops
	{
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );

		for ( Variable * observee : qss_observees_ ) {
			observee->fmu_set_x( t );
		}
		size_type const qss_n( qss_.n() );
		if ( zc_type_ == ZC_Type::EventIndicator ) { // Event indicator variables
			assert( fmu_me_->has_event_indicators );
			assert( qss_n == ei_vars_.size() );
			fmu_me_->get_reals( qss_n, &ei_vars_.refs[ 0 ], &ei_vars_.vals[ 0 ] );
			for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) {
				Variable * trigger( triggers_[ i ] );
				assert( trigger->is_ZC() && trigger->not_ZCd() && trigger->not_ZCe() ); // Event indicator trigger
				assert( trigger->tE >= t ); // Bin variables tE can be > t
				trigger->tE = t; // Bin variables tE can be > t
				trigger->st = s; // Set trigger superdense time
				trigger->advance_QSS_0( ei_vars_.vals[ i ] );
			}
			if ( qss2_.have() ) {
				Time tN( t - options::dtND );
				fmu_me_->set_time( tN );
				for ( Variable * observee : qss_observees_ ) {
					observee->fmu_set_x( tN );
				}
				fmu_me_->get_reals( qss_n, &ei_vars_.refs[ 0 ], &ei_vars_.vals_m[ 0 ] );
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : qss_observees_ ) {
					observee->fmu_set_x( tN );
				}
				fmu_me_->get_reals( qss_n, &ei_vars_.refs[ 0 ], &ei_vars_.vals_p[ 0 ] );
				for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) {
					triggers_[ i ]->advance_QSS_1( ei_vars_.vals_m[ i ], ei_vars_.vals_p[ i ] );
				}
				for ( size_type i = qss2_.b(), e = qss_.e(); i < e; ++i ) { // Order 2+ triggers
					triggers_[ i ]->advance_QSS_2();
				}
				if ( qss3_.have() ) {
					tN = t + options::two_dtND;
					fmu_me_->set_time( tN );
					for ( Variable * observee : uni_order_ ? qss_observees_ : qss3_observees_ ) {
						observee->fmu_set_x( tN );
					}
					size_type const qss3_b( qss3_.b() );
					fmu_me_->get_reals( qss3_.n(), &ei_vars_.refs[ qss3_b ], &ei_vars_.vals_p[ qss3_b ] );
					for ( size_type i = qss3_b, e = qss_.e(); i < e; ++i ) { // Order 3+ triggers
						triggers_[ i ]->advance_QSS_3( ei_vars_.vals_p[ i ] );
					}
				}
			} else { // Order 1 triggers only
				Time tN( t + options::dtND );
				fmu_me_->set_time( tN );
				for ( Variable * observee : qss_observees_ ) {
					observee->fmu_set_x( tN );
				}
				fmu_me_->get_reals( qss_n, &ei_vars_.refs[ 0 ], &ei_vars_.vals_p[ 0 ] );
				for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) { // Order 1 triggers
					triggers_[ i ]->advance_QSS_1( ei_vars_.vals_p[ i ] );
				}
			}
			fmu_me_->set_time( t );
		} else if ( zc_type_ == ZC_Type::EventIndicatorDD ) { // Event indicator directional derivative variables
			assert( fmu_me_->has_event_indicators );
			assert( qss_n == dd_vars_.size() );
			fmu_me_->get_reals( qss_n, &dd_vars_.refs[ 0 ], &dd_vars_.vals[ 0 ] );
			for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) {
				Variable * trigger( triggers_[ i ] );
				assert( trigger->is_ZCd() ); // Event indicator using directional derivative trigger
				assert( trigger->tE >= t ); // Bin variables tE can be > t
				trigger->tE = t; // Bin variables tE can be > t
				trigger->st = s; // Set trigger superdense time
				trigger->advance_QSS_0( dd_vars_.vals[ i ] );
			}
			for ( size_type i = 0, e = qss_observees_.size(); i < e; ++i ) {
				qss_observees_dv_[ i ] = qss_observees_[ i ]->x1( t );
			}
			fmu_me_->get_directional_derivatives(
			 qss_observees_v_ref_.data(),
			 qss_observees_v_ref_.size(),
			 dd_vars_.refs.data(),
			 dd_vars_.refs.size(),
			 qss_observees_dv_.data(),
			 dd_vars_.ders.data()
			);
			for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) {
				triggers_[ i ]->advance_QSS_1( dd_vars_.ders[ i ] );
			}
			if ( qss3_.have() ) {
				Time tN( t - options::dtND );
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
				 dd_vars_.refs.data(),
				 dd_vars_.refs.size(),
				 qss_observees_dv_.data(),
				 dd_vars_.ders_m.data()
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
				 dd_vars_.refs.data(),
				 dd_vars_.refs.size(),
				 qss_observees_dv_.data(),
				 dd_vars_.ders_p.data()
				);
				for ( size_type i = qss2_.b(), e = qss_.e(); i < e; ++i ) { // Order 2+ triggers
					triggers_[ i ]->advance_QSS_2( dd_vars_.ders_m[ i ], dd_vars_.ders_p[ i ] );
				}
				for ( size_type i = qss3_.b(), e = qss_.e(); i < e; ++i ) { // Order 3+ triggers
					triggers_[ i ]->advance_QSS_3();
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
				 dd_vars_.refs.data(),
				 dd_vars_.refs.size(),
				 qss_observees_dv_.data(),
				 dd_vars_.ders_p.data()
				);
				for ( size_type i = qss2_.b(), e = qss_.e(); i < e; ++i ) { // Order 2+ triggers
					triggers_[ i ]->advance_QSS_2( dd_vars_.ders_p[ i ] );
				}
				fmu_me_->set_time( t );
			}
		} else { // Explicit zero-crossing variables
			assert( zc_type_ == ZC_Type::Explicit );
			assert( fmu_me_->has_explicit_ZCs );
			assert( qss_n == zc_vars_.size() );
			assert( qss_n == zc_ders_.size() );
			fmu_me_->get_reals( qss_n, &zc_vars_.refs[ 0 ], &zc_vars_.vals[ 0 ] );
			for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) {
				Variable * trigger( triggers_[ i ] );
				assert( trigger->is_ZCe() ); // Explicit zero-crossing trigger
				assert( trigger->tE >= t ); // Bin variables tE can be > t
				trigger->tE = t; // Bin variables tE can be > t
				trigger->st = s; // Set trigger superdense time
				trigger->advance_QSS_0( zc_vars_.vals[ i ] );
			}
			fmu_me_->get_reals( qss_n, &zc_ders_.refs[ 0 ], &zc_ders_.ders[ 0 ] );
#ifdef _OPENMP
			size_type const max_threads( static_cast< size_type >( omp_get_max_threads() ) );
			if ( ( max_threads > 1u ) && ( qss_n >= max_threads * 128u ) ) {
				std::int64_t const qss_b( qss_.b() );
				std::int64_t const qss_e( qss_.e() );
				std::int64_t const qss_chunk_size( static_cast< std::int64_t >( ( qss_n + max_threads - 1u ) / max_threads ) );
				#pragma omp parallel for schedule(static)
				for ( std::int64_t i = qss_b; i < qss_e; i += qss_chunk_size ) {
					for ( std::int64_t k = i, ke = std::min( i + qss_chunk_size, qss_e ); k < ke; ++k ) { // Chunk
						triggers_[ k ]->advance_QSS_1( zc_ders_.ders[ k ] );
					}
				}
			} else {
#endif
			for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) {
				triggers_[ i ]->advance_QSS_1( zc_ders_.ders[ i ] );
			}
#ifdef _OPENMP
			}
#endif
			if ( qss3_.have() ) {
				Time tN( t - options::dtND );
				fmu_me_->set_time( tN );
				for ( Variable * observee : uni_order_ ? qss_observees_ : qss2_observees_ ) {
					observee->fmu_set_x( tN );
				}
				size_type const qss2_b( qss2_.b() );
				fmu_me_->get_reals( qss2_.n(), &zc_ders_.refs[ qss2_b ], &zc_ders_.ders_m[ qss2_b ] );
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : uni_order_ ? qss_observees_ : qss2_observees_ ) {
					observee->fmu_set_x( tN );
				}
				fmu_me_->get_reals( qss2_.n(), &zc_ders_.refs[ qss2_b ], &zc_ders_.ders_p[ qss2_b ] );
				for ( size_type i = qss2_b, e = qss_.e(); i < e; ++i ) { // Order 2+ triggers
					triggers_[ i ]->advance_QSS_2( zc_ders_.ders_m[ i ], zc_ders_.ders_p[ i ] );
				}
				for ( size_type i = qss3_.b(), e = qss_.e(); i < e; ++i ) { // Order 3+ triggers
					triggers_[ i ]->advance_QSS_3();
				}
				fmu_me_->set_time( t );
			} else if ( qss2_.have() ) {
				Time tN( t + options::dtND );
				fmu_me_->set_time( tN );
				for ( Variable * observee : uni_order_ ? qss_observees_ : qss2_observees_ ) {
					observee->fmu_set_x( tN );
				}
				size_type const qss2_b( qss2_.b() );
				fmu_me_->get_reals( qss2_.n(), &zc_ders_.refs[ qss2_b ], &zc_ders_.ders_p[ qss2_b ] );
				for ( size_type i = qss2_b, e = qss_.e(); i < e; ++i ) { // Order 2+ triggers
					triggers_[ i ]->advance_QSS_2( zc_ders_.ders_p[ i ] );
				}
				fmu_me_->set_time( t );
			}
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

	// Reset Specs
	void
	reset_specs()
	{
		zc_type_ = ZC_Type::None;
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

		assert( fmu_me_ != nullptr );
		zc_type_ = ( fmu_me_->has_event_indicators ? ( options::eidd ? ZC_Type::EventIndicatorDD : ZC_Type::EventIndicator ) : ZC_Type::Explicit );

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

	ZC_Type zc_type_{ ZC_Type::None }; // Zero-crossing variable type

	Variables triggers_; // Triggers

	Range qss_; // Triggers
	Range qss2_; // Triggers of order 2+
	Range qss3_; // Triggers of order 3+

	bool uni_order_{ false }; // Triggers all the same order?

	// Trigger FMU pooled call data
	RefsValsEI< Variable > ei_vars_; // Event indicator variables
	RefsValsEIDD< Variable > dd_vars_; // Event indicator directional derivative variables
	RefsVals< Variable > zc_vars_; // Explict zero-crossing variables
	RefsDers< Variable > zc_ders_; // Explict zero-crossing derivatives

	// Observees
	Variables qss_observees_; // Triggers observees
	Variables qss2_observees_; // Triggers of order 2+ observees
	Variables qss3_observees_; // Triggers of order 3+ observees

	// Observee directional derivative seed data
	VariableRefs qss_observees_v_ref_; // Observee value references for FMU directional derivative
	Reals qss_observees_dv_; // Observee seed derivatives for FMU directional derivative lookup

}; // Triggers_ZC

} // fmu
} // QSS

#endif
