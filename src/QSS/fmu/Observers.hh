// FMU-Based Variable Observers
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

#ifndef QSS_fmu_Observers_hh_INCLUDED
#define QSS_fmu_Observers_hh_INCLUDED

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

// FMU-Based Variable Observers
template< typename V >
class Observers final
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
	Observers( FMU_ME * fmu_me = nullptr ) :
	 fmu_me_( fmu_me )
	{}

public: // Conversion

	// Observers Conversion
	operator Variables const &() const
	{
		return observers_;
	}

	// Observers Conversion
	operator Variables &()
	{
		return observers_;
	}

public: // Predicate

	// Empty?
	bool
	empty() const
	{
		return observers_.empty();
	}

	// Have Observer(s)?
	bool
	have() const
	{
		return ( ! observers_.empty() );
	}

	// Have Connected Output Observer(s)?
	bool
	connected_output_observer() const
	{
		return connected_output_observer_;
	}

public: // Property

	// Size
	size_type
	size() const
	{
		return observers_.size();
	}

	// Observers Collection
	Variables const &
	observers() const
	{
		return observers_;
	}

	// Observers Collection
	Variables &
	observers()
	{
		return observers_;
	}

public: // Methods

	// Add Observer
	void
	add( Variable * v )
	{
		observers_.push_back( v );
	}

	// Delete Observer
	void
	del( Variable * const v )
	{
		typename Variables::iterator const i( std::find( observers_.begin(), observers_.end(), v ) );
		if ( i != observers_.end() ) observers_.erase( i );
	}

	// Initialize for Observers of a Single Variable
	void
	init()
	{
		set_up( true );
	}

	// Set up for Current Observers
	void
	set_up( bool const recover = false )
	{
		if ( observers_.empty() ) {
			reset_specs();
			return;
		}

		// Remove duplicates then sort by type and order
		uniquify( observers_, recover ); // Sort by address and remove duplicates and optionally recover unused memory
		sort_by_type_and_order( observers_ );

		// Set specs
		set_specs();

		// FMU pooled data set up
		if ( qss_.have() ) { // State variables
			qss_ders_.clear(); qss_ders_.reserve( qss_.n() );
			for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) {
				qss_ders_.push_back( observers_[ i ]->der().ref );
			}
		}
		if ( zc_.have() ) { // Zero-crossing variables
			size_type const zc_n( zc_.n() );
			if ( zc_type_ == ZC_Type::EventIndicator ) { // Event indicator variables
				ei_vars_.clear(); ei_vars_.reserve( zc_n );
				for ( size_type i = zc_.b(), e = zc_.e(); i < e; ++i ) {
					ei_vars_.push_back( observers_[ i ]->var().ref );
				}
			} else if ( zc_type_ == ZC_Type::EventIndicatorDD ) { // Event indicator directional derivative variables
				dd_vars_.clear(); dd_vars_.reserve( zc_n );
				for ( size_type i = zc_.b(), e = zc_.e(); i < e; ++i ) {
					dd_vars_.push_back( observers_[ i ]->var().ref );
				}
			} else { // Explicit zero-crossing variables
				assert( zc_type_ == ZC_Type::Explicit );
				zc_vars_.clear(); zc_vars_.reserve( zc_n );
				zc_ders_.clear(); zc_ders_.reserve( zc_n );
				for ( size_type i = zc_.b(), e = zc_.e(); i < e; ++i ) {
					zc_vars_.push_back( observers_[ i ]->var().ref );
					zc_ders_.push_back( observers_[ i ]->der().ref );
				}
			}
		}

		// Observees setup
		set_observees();
	}

	// Assign a Triggers Collection
	void
	assign( Variables & triggers )
	{
		// Combine all non-trigger observers
		observers_.clear();
		if ( triggers.size() < 16 ) { // Linear search
			for ( Variable * trigger : triggers ) {
				for ( Variable * observer : trigger->observers() ) {
					if ( std::find( triggers.begin(), triggers.end(), observer ) == triggers.end() ) observers_.push_back( observer );
				}
			}
		} else { // Binary search
			std::sort( triggers.begin(), triggers.end() ); //! Side effect!
			for ( Variable * trigger : triggers ) {
				for ( Variable * observer : trigger->observers() ) {
					if ( ! std::binary_search( triggers.begin(), triggers.end(), observer ) ) observers_.push_back( observer );
				}
			}
		}

		set_up();
	}

	// Advance
	void
	advance( Time const t )
	{
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );
		if ( qss_.have() ) advance_observers_QSS( t ); // QSS state variables
		if ( zc_.have() ) advance_observers_ZC( t ); // Zero-crossing variables
		if ( ox_.have() ) advance_observers_OX( t ); // Other variables
		if ( options::output::d ) advance_observers_d();
	}

	// Clear
	void
	clear()
	{
		observers_.clear();
		reset_specs();
	}

public: // Iterator

	// Begin Iterator
	const_iterator
	begin() const
	{
		return observers_.begin();
	}

	// Begin Iterator
	iterator
	begin()
	{
		return observers_.begin();
	}

	// End Iterator
	const_iterator
	end() const
	{
		return observers_.end();
	}

	// End Iterator
	iterator
	end()
	{
		return observers_.end();
	}

public: // Subscript

	// Observers[ i ]
	Variable const *
	operator []( size_type const i ) const
	{
		return observers_[ i ];
	}

	// Observers[ i ]
	Variable *
	operator []( size_type const i )
	{
		return observers_[ i ];
	}

private: // Methods

	// Reset Specs
	void
	reset_specs()
	{
		zc_type_ = ZC_Type::None;
		connected_output_observer_ = false;
		all_.reset();
		qss_.reset();
		qss2_.reset();
		qss3_.reset();
		zc_.reset();
		zc2_.reset();
		zc3_.reset();
		ox_.reset();
	}

	// Set Specs
	void
	set_specs()
	{
		reset_specs();
		if ( observers_.empty() ) return;

		assert( fmu_me_ != nullptr );
		zc_type_ = ( fmu_me_->has_event_indicators ? ( options::eidd ? ZC_Type::EventIndicatorDD : ZC_Type::EventIndicator ) : ZC_Type::Explicit );

		connected_output_observer_ = false;
		all_.b() = 0u;
		all_.e() = observers_.size();
		size_type i( 0u );

		// QSS state observers
		while ( ( i < all_.e() ) && ( observers_[ i ]->is_QSS() ) ) {
			Variable const * observer( observers_[ i ] );
			qss_.b() = std::min( qss_.b(), i );
			int const order( observer->order() );
			if ( order >= 2 ) {
				qss2_.b() = std::min( qss2_.b(), i );
				if ( order >= 3 ) {
					qss3_.b() = std::min( qss3_.b(), i );
				}
			}
			if ( observer->connected_output ) connected_output_observer_ = true;
			++i;
		}
		if ( qss_.began() ) {
			qss_.e() = i;
			if ( qss2_.began() ) {
				qss2_.e() = i;
				if ( qss3_.began() ) {
					qss3_.e() = i;
				}
			}
		}
		size_type const qss_n( qss_.n() );
		qss_uni_order_ = (
		 ( qss2_.empty() || qss2_.n() == qss_n ) &&
		 ( qss3_.empty() || qss3_.n() == qss_n )
		);

		// Zero crossing observers
		while ( ( i < all_.e() ) && ( observers_[ i ]->is_ZC() ) ) {
			Variable const * observer( observers_[ i ] );
			zc_.b() = std::min( zc_.b(), i );
			int const order( observer->order() );
			if ( order >= 2 ) {
				zc2_.b() = std::min( zc2_.b(), i );
				if ( order >= 3 ) {
					zc3_.b() = std::min( zc3_.b(), i );
				}
			}
			if ( observer->connected_output ) connected_output_observer_ = true;
			++i;
		}
		if ( zc_.began() ) {
			zc_.e() = i;
			if ( zc2_.began() ) {
				zc2_.e() = i;
				if ( zc3_.began() ) {
					zc3_.e() = i;
				}
			}
		}
		size_type const zc_n( zc_.n() );
		zc_uni_order_ = (
		 ( zc2_.empty() || zc2_.n() == zc_n ) &&
		 ( zc3_.empty() || zc3_.n() == zc_n )
		);

		// Other x-based observers
		while ( i < all_.e() ) {
			Variable const * observer( observers_[ i ] );
			ox_.b() = std::min( ox_.b(), i );
			ox_.e() = std::max( ox_.e(), i + 1 );
			assert( observer->order() == 0 ); // All of these are zero order output variables
			if ( observer->connected_output ) connected_output_observer_ = true;
			++i;
		}
	}

	// Assign Observers from a Triggers Collection
	void
	set_observees()
	{
		// QSS observer observees set up
		if ( qss_.have() ) {
			qss_observees_.clear();
			for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) {
				Variable * observer( observers_[ i ] );
				if ( observer->self_observer() ) qss_observees_.push_back( observer );
				for ( auto observee : observer->observees() ) {
					if ( ! observee->is_Discrete() ) qss_observees_.push_back( observee );
				}
			}
			uniquify( qss_observees_ );
			if ( ! qss_uni_order_ ) {
				assert( qss2_.have() );
				qss2_observees_.clear();
				for ( size_type i = qss2_.b(), e = qss_.e(); i < e; ++i ) { // Order 2+ observers
					Variable * observer( observers_[ i ] );
					if ( observer->self_observer() ) qss2_observees_.push_back( observer );
					for ( auto observee : observer->observees() ) {
						if ( ! observee->is_Discrete() ) qss2_observees_.push_back( observee );
					}
				}
				uniquify( qss2_observees_ );
				if ( qss3_.have() ) {
					qss3_observees_.clear();
					for ( size_type i = qss3_.b(), e = qss_.e(); i < e; ++i ) { // Order 3+ observers
						Variable * observer( observers_[ i ] );
						if ( observer->self_observer() ) qss3_observees_.push_back( observer );
						for ( auto observee : observer->observees() ) {
							if ( ! observee->is_Discrete() ) qss3_observees_.push_back( observee );
						}
					}
					uniquify( qss3_observees_ );
				}
			}
		}

		// Zero-crossing observer observees set up
		if ( zc_.have() ) {
			zc_observees_.clear();
			for ( size_type i = zc_.b(), e = zc_.e(); i < e; ++i ) {
				Variable * observer( observers_[ i ] );
				if ( observer->self_observer() ) zc_observees_.push_back( observer );
				for ( auto observee : observer->observees() ) {
					if ( ! observee->is_Discrete() ) zc_observees_.push_back( observee );
				}
			}
			uniquify( zc_observees_ );
			if ( ! zc_uni_order_ ) {
				assert( zc2_.have() );
				assert( fmu_me_ != nullptr );
				if ( ! fmu_me_->has_event_indicators ) { // Event indicators don't currently need the order 2+ observees
					zc2_observees_.clear();
					for ( size_type i = zc2_.b(), e = zc_.e(); i < e; ++i ) { // Order 2+ observers
						Variable * observer( observers_[ i ] );
						if ( observer->self_observer() ) zc2_observees_.push_back( observer );
						for ( auto observee : observer->observees() ) {
							if ( ! observee->is_Discrete() ) zc2_observees_.push_back( observee );
						}
					}
					uniquify( zc2_observees_ );
				}
				if ( zc3_.have() ) {
					zc3_observees_.clear();
					for ( size_type i = zc3_.b(), e = zc_.e(); i < e; ++i ) { // Order 3+ observers
						Variable * observer( observers_[ i ] );
						if ( observer->self_observer() ) zc3_observees_.push_back( observer );
						for ( auto observee : observer->observees() ) {
							if ( ! observee->is_Discrete() ) zc3_observees_.push_back( observee );
						}
					}
					uniquify( zc3_observees_ );
				}
			}

			// Observee directional derivative seed array set up
			if ( zc_type_ == ZC_Type::EventIndicatorDD ) { // Event indicator directional derivative variables
				zc_observees_v_ref_.clear();
				zc_observees_dv_.clear();
				for ( auto observee : zc_observees_ ) {
					zc_observees_v_ref_.push_back( observee->var().ref );
					zc_observees_dv_.push_back( 0.0 );
				}
			}
		}
	}

	// Advance QSS State Observers
	void
	advance_observers_QSS( Time const t )
	{
		assert( qss_.have() );
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );

		for ( Variable * observee : qss_observees_ ) {
			observee->fmu_set_q( t );
		}
		size_type const qss_n( qss_.n() );
		assert( qss_n == qss_ders_.size() );
		fmu_me_->get_reals( qss_n, &qss_ders_.refs[ 0 ], &qss_ders_.ders[ 0 ] );
#ifdef _OPENMP
		size_type const max_threads( static_cast< size_type >( omp_get_max_threads() ) );
		if ( ( max_threads > 1u ) && ( qss_n >= max_threads * 32u ) ) { // Parallel
			std::int64_t const qss_b( qss_.b() );
			std::int64_t const qss_e( qss_.e() );
			std::int64_t const qss_chunk_size( static_cast< std::int64_t >( ( qss_n + max_threads - 1u ) / max_threads ) );
			#pragma omp parallel
			{
			#pragma omp for schedule(static)
			for ( std::int64_t i = qss_b; i < qss_e; i += qss_chunk_size ) {
				for ( std::int64_t k = i, ke = std::min( i + qss_chunk_size, qss_e ); k < ke; ++k ) { // Chunk
					observers_[ k ]->advance_observer_1_parallel( t, qss_ders_.ders[ k ] );
				}
			}
			#pragma omp single
			{

			if ( qss3_.have() ) {
				Time tN( t - options::dtND );
				fmu_me_->set_time( tN );
				for ( Variable * observee : qss_uni_order_ ? qss_observees_ : qss3_observees_ ) {
					observee->fmu_set_q( tN );
				}
				size_type const qss2_b( qss2_.b() );
				fmu_me_->get_reals( qss2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.ders_m[ qss2_b ] );
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : qss_uni_order_ ? qss_observees_ : qss3_observees_ ) {
					observee->fmu_set_q( tN );
				}
				fmu_me_->get_reals( qss2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.ders_p[ qss2_b ] );
				for ( size_type i = qss2_b, e = qss_.e(); i < e; ++i ) { // Order 2+ observers
					observers_[ i ]->advance_observer_2_parallel( qss_ders_.ders_m[ i ], qss_ders_.ders_p[ i ] );
				}
				for ( size_type i = qss3_.b(), e = qss_.e(); i < e; ++i ) { // Order 3+ observers
					observers_[ i ]->advance_observer_3_parallel();
				}
				fmu_me_->set_time( t );
			} else if ( qss2_.have() ) {
				Time tN( t + options::dtND );
				fmu_me_->set_time( tN );
				for ( Variable * observee : qss_uni_order_ ? qss_observees_ : qss2_observees_ ) {
					observee->fmu_set_q( tN );
				}
				size_type const qss2_b( qss2_.b() );
				fmu_me_->get_reals( qss2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.ders[ qss2_b ] );
				for ( size_type i = qss2_b, e = qss_.e(); i < e; ++i ) { // Order 2+ observers
					observers_[ i ]->advance_observer_2_parallel( qss_ders_.ders[ i ] );
				}
				fmu_me_->set_time( t );
			}
			} // omp single
			#pragma omp for schedule(static)
			for ( std::int64_t i = qss_b; i < qss_e; i += qss_chunk_size ) {
				for ( std::int64_t k = i, ke = std::min( i + qss_chunk_size, qss_e ); k < ke; ++k ) { // Chunk
					observers_[ k ]->advance_observer_F_parallel();
				}
			}
			} // omp parallel
			for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) {
				observers_[ i ]->advance_observer_F_serial();
			}
		} else { // Serial
#endif
		for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) {
			observers_[ i ]->advance_observer_1( t, qss_ders_.ders[ i ] );
		}
		if ( qss3_.have() ) {
			Time tN( t - options::dtND );
			fmu_me_->set_time( tN );
			for ( Variable * observee : qss_uni_order_ ? qss_observees_ : qss3_observees_ ) {
				observee->fmu_set_q( tN );
			}
			size_type const qss2_b( qss2_.b() );
			fmu_me_->get_reals( qss2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.ders_m[ qss2_b ] );
			tN = t + options::dtND;
			fmu_me_->set_time( tN );
			for ( Variable * observee : qss_uni_order_ ? qss_observees_ : qss3_observees_ ) {
				observee->fmu_set_q( tN );
			}
			fmu_me_->get_reals( qss2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.ders_p[ qss2_b ] );
			for ( size_type i = qss2_b, e = qss_.e(); i < e; ++i ) { // Order 2+ observers
				observers_[ i ]->advance_observer_2( qss_ders_.ders_m[ i ], qss_ders_.ders_p[ i ] );
			}
			for ( size_type i = qss3_.b(), e = qss_.e(); i < e; ++i ) { // Order 3+ observers
				observers_[ i ]->advance_observer_3();
			}
			fmu_me_->set_time( t );
		} else if ( qss2_.have() ) {
			Time tN( t + options::dtND );
			fmu_me_->set_time( tN );
			for ( Variable * observee : qss_uni_order_ ? qss_observees_ : qss2_observees_ ) {
				observee->fmu_set_q( tN );
			}
			size_type const qss2_b( qss2_.b() );
			fmu_me_->get_reals( qss2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.ders[ qss2_b ] );
			for ( size_type i = qss2_b, e = qss_.e(); i < e; ++i ) { // Order 2+ observers
				observers_[ i ]->advance_observer_2( qss_ders_.ders[ i ] );
			}
			fmu_me_->set_time( t );
		}
#ifdef _OPENMP
		}
#endif
	}

	// Advance Zero-Crossing Observers
	void
	advance_observers_ZC( Time const t )
	{
		assert( zc_.have() );
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );

		for ( Variable * observee : zc_observees_ ) {
			observee->fmu_set_x( t );
		}
		size_type const zc_n( zc_.n() );
		if ( zc_type_ == ZC_Type::EventIndicator ) { // Event indicator variables
			assert( fmu_me_->has_event_indicators );
			assert( zc_n == ei_vars_.size() );
			fmu_me_->get_reals( zc_n, &ei_vars_.refs[ 0 ], &ei_vars_.vals[ 0 ] );
			if ( zc2_.have() ) {
				Time tN( t - options::dtND );
				fmu_me_->set_time( tN );
				for ( Variable * observee : zc_observees_ ) {
					observee->fmu_set_x( tN );
				}
				fmu_me_->get_reals( zc_n, &ei_vars_.refs[ 0 ], &ei_vars_.vals_m[ 0 ] );
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : zc_observees_ ) {
					observee->fmu_set_x( tN );
				}
				fmu_me_->get_reals( zc_n, &ei_vars_.refs[ 0 ], &ei_vars_.vals_p[ 0 ] );
				for ( size_type i = zc_.b(), j = 0, e = zc_.e(); i < e; ++i, ++j ) {
					observers_[ i ]->advance_observer_1( t, ei_vars_.vals[ j ], ei_vars_.vals_m[ j ], ei_vars_.vals_p[ j ] );
				}
				for ( size_type i = zc2_.b(), e = zc_.e(); i < e; ++i ) { // Order 2+ observers
					observers_[ i ]->advance_observer_2();
				}
				if ( zc3_.have() ) {
					tN = t + options::two_dtND;
					fmu_me_->set_time( tN );
					for ( Variable * observee : zc_uni_order_ ? zc_observees_ : zc3_observees_ ) {
						observee->fmu_set_x( tN );
					}
					size_type const zc3_bo( zc3_.b() - zc_.b() );
					fmu_me_->get_reals( zc3_.n(), &ei_vars_.refs[ zc3_bo ], &ei_vars_.vals_p[ zc3_bo ] );
					for ( size_type i = zc3_.b(), j = zc3_bo, e = zc_.e(); i < e; ++i, ++j ) { // Order 3+ observers
						observers_[ i ]->advance_observer_3( ei_vars_.vals_p[ j ] );
					}
				}
			} else { // Order 1 triggers only
				Time tN( t + options::dtND );
				fmu_me_->set_time( tN );
				for ( Variable * observee : zc_observees_ ) {
					observee->fmu_set_x( tN );
				}
				fmu_me_->get_reals( zc_n, &ei_vars_.refs[ 0 ], &ei_vars_.vals_p[ 0 ] );

				for ( size_type i = zc_.b(), j = 0, e = zc_.e(); i < e; ++i, ++j ) {
					observers_[ i ]->advance_observer_1( t, ei_vars_.vals[ j ], ei_vars_.vals_p[ j ] );
				}
			}
			fmu_me_->set_time( t );
		} else if ( zc_type_ == ZC_Type::EventIndicatorDD ) { // Event indicator directional derivative variables
			assert( fmu_me_->has_event_indicators );
			assert( zc_n == dd_vars_.size() );
			fmu_me_->get_reals( zc_n, &dd_vars_.refs[ 0 ], &dd_vars_.vals[ 0 ] );
			for ( size_type i = 0, e = zc_observees_.size(); i < e; ++i ) {
				zc_observees_dv_[ i ] = zc_observees_[ i ]->x1( t );
			}
			fmu_me_->get_directional_derivatives(
			 zc_observees_v_ref_.data(),
			 zc_observees_v_ref_.size(),
			 dd_vars_.refs.data(),
			 dd_vars_.refs.size(),
			 zc_observees_dv_.data(),
			 dd_vars_.ders.data()
			);
			for ( size_type i = zc_.b(), j = 0, e = zc_.e(); i < e; ++i, ++j ) {
				observers_[ i ]->advance_observer_1( t, dd_vars_.vals[ j ], dd_vars_.ders[ j ] );
			}
			if ( zc3_.have() ) {
				Time tN( t - options::dtND );
				fmu_me_->set_time( tN );
				for ( Variable * observee : zc_uni_order_ ? zc_observees_ : zc2_observees_ ) {
					observee->fmu_set_x( tN );
				}
				for ( size_type i = 0, e = zc_observees_.size(); i < e; ++i ) {
					zc_observees_dv_[ i ] = zc_observees_[ i ]->x1( tN );
				}
				fmu_me_->get_directional_derivatives(
				 zc_observees_v_ref_.data(),
				 zc_observees_v_ref_.size(),
				 dd_vars_.refs.data(),
				 dd_vars_.refs.size(),
				 zc_observees_dv_.data(),
				 dd_vars_.ders_m.data()
				);
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : zc_uni_order_ ? zc_observees_ : zc2_observees_ ) {
					observee->fmu_set_x( tN );
				}
				for ( size_type i = 0, e = zc_observees_.size(); i < e; ++i ) {
					zc_observees_dv_[ i ] = zc_observees_[ i ]->x1( tN );
				}
				fmu_me_->get_directional_derivatives(
				 zc_observees_v_ref_.data(),
				 zc_observees_v_ref_.size(),
				 dd_vars_.refs.data(),
				 dd_vars_.refs.size(),
				 zc_observees_dv_.data(),
				 dd_vars_.ders_p.data()
				);
				size_type const zc2_bo( zc2_.b() - zc_.b() );
				for ( size_type i = zc2_.b(), j = zc2_bo, e = zc_.e(); i < e; ++i, ++j ) { // Order 2+ observers
					observers_[ i ]->advance_observer_2( dd_vars_.ders_m[ j ], dd_vars_.ders_p[ j ] );
				}
				for ( size_type i = zc3_.b(), e = zc_.e(); i < e; ++i ) { // Order 3+ observers
					observers_[ i ]->advance_observer_3();
				}
				fmu_me_->set_time( t );
			} else if ( zc2_.have() ) {
				Time tN( t + options::dtND );
				fmu_me_->set_time( tN );
				for ( Variable * observee : zc_uni_order_ ? zc_observees_ : zc2_observees_ ) {
					observee->fmu_set_x( tN );
				}
				for ( size_type i = 0, e = zc_observees_.size(); i < e; ++i ) {
					zc_observees_dv_[ i ] = zc_observees_[ i ]->x1( tN );
				}
				fmu_me_->get_directional_derivatives(
				 zc_observees_v_ref_.data(),
				 zc_observees_v_ref_.size(),
				 dd_vars_.refs.data(),
				 dd_vars_.refs.size(),
				 zc_observees_dv_.data(),
				 dd_vars_.ders_p.data()
				);
				size_type const zc2_bo( zc2_.b() - zc_.b() );
				for ( size_type i = zc2_.b(), j = zc2_bo, e = zc_.e(); i < e; ++i, ++j ) { // Order 2+ observers
					observers_[ i ]->advance_observer_2( dd_vars_.ders_p[ j ] );
				}
				fmu_me_->set_time( t );
			}
		} else { // Explicit zero-crossing variables
			assert( zc_type_ == ZC_Type::Explicit );
			assert( fmu_me_->has_explicit_ZCs );
			assert( zc_n == zc_vars_.size() );
			assert( zc_n == zc_ders_.size() );
			fmu_me_->get_reals( zc_n, &zc_vars_.refs[ 0 ], &zc_vars_.vals[ 0 ] );
			fmu_me_->get_reals( zc_n, &zc_ders_.refs[ 0 ], &zc_ders_.ders[ 0 ] );
			for ( size_type i = zc_.b(), j = 0, e = zc_.e(); i < e; ++i, ++j ) {
				observers_[ i ]->advance_observer_1( t, zc_vars_.vals[ j ], zc_ders_.ders[ j ] );
			}
			if ( zc3_.have() ) {
				Time tN( t - options::dtND );
				fmu_me_->set_time( tN );
				for ( Variable * observee : zc_uni_order_ ? zc_observees_ : zc2_observees_ ) {
					observee->fmu_set_x( tN );
				}
				size_type const zc2_bo( zc2_.b() - zc_.b() );
				fmu_me_->get_reals( zc2_.n(), &zc_ders_.refs[ zc2_bo ], &zc_ders_.ders_m[ zc2_bo ] );
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : zc_uni_order_ ? zc_observees_ : zc2_observees_ ) {
					observee->fmu_set_x( tN );
				}
				fmu_me_->get_reals( zc2_.n(), &zc_ders_.refs[ zc2_bo ], &zc_ders_.ders_p[ zc2_bo ] );
				for ( size_type i = zc2_.b(), j = zc2_bo, e = zc_.e(); i < e; ++i, ++j ) { // Order 2+ observers
					observers_[ i ]->advance_observer_2( zc_ders_.ders_m[ j ], zc_ders_.ders_p[ j ] );
				}
				for ( size_type i = zc3_.b(), e = zc_.e(); i < e; ++i ) { // Order 3+ observers
					observers_[ i ]->advance_observer_3();
				}
				fmu_me_->set_time( t );
			} else if ( zc2_.have() ) {
				Time tN( t + options::dtND );
				fmu_me_->set_time( tN );
				for ( Variable * observee : zc_uni_order_ ? zc_observees_ : zc2_observees_ ) {
					observee->fmu_set_x( tN );
				}
				size_type const zc2_bo( zc2_.b() - zc_.b() );
				fmu_me_->get_reals( zc2_.n(), &zc_ders_.refs[ zc2_bo ], &zc_ders_.ders_p[ zc2_bo ] );
				for ( size_type i = zc2_.b(), j = zc2_bo, e = zc_.e(); i < e; ++i, ++j ) { // Order 2+ observers
					observers_[ i ]->advance_observer_2( zc_ders_.ders_p[ j ] );
				}
				fmu_me_->set_time( t );
			}
		}
	}

	// Advance Other X-Based Observers
	void
	advance_observers_OX( Time const t )
	{
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );
		for ( size_type i = ox_.b(), e = ox_.e(); i < e; ++i ) {
			observers_[ i ]->advance_observer( t );
		}
	}

	// Advance: Stage d
	void
	advance_observers_d() const
	{
		assert( options::output::d );
		for ( Variable const * observer : observers_ ) {
			observer->advance_observer_d();
		}
	}

private: // Data

	FMU_ME * fmu_me_{ nullptr }; // FMU-ME (non-owning) pointer

	ZC_Type zc_type_{ ZC_Type::None }; // Zero-crossing variable type

	Variables observers_; // Observers

	bool connected_output_observer_{ false }; // Output connection observer to another FMU?

	Range all_; // All observers
	Range qss_; // QSS state observers
	Range qss2_; // QSS state observers of order 2+
	Range qss3_; // QSS state observers of order 3+
	Range zc_; // Zero-crossing observers
	Range zc2_; // Zero-crossing observers of order 2+
	Range zc3_; // Zero-crossing observers of order 3+
	Range ox_; // Other x-based observers

	bool qss_uni_order_{ false }; // QSS observers all the same order?
	bool zc_uni_order_{ false }; // ZC observers all the same order?

	// Observer FMU pooled call data
	RefsDers< Variable > qss_ders_; // QSS state derivatives
	RefsValsEI< Variable > ei_vars_; // Event indicator variables
	RefsValsEIDD< Variable > dd_vars_; // Event indicator directional derivative variables
	RefsVals< Variable > zc_vars_; // Explicit zero-crossing variables
	RefsDers< Variable > zc_ders_; // Explicit zero-crossing derivatives

	// QSS state observers observees (including self-observers)
	Variables qss_observees_; // Observers observees
	Variables qss2_observees_; // Observers of order 2+ observees
	Variables qss3_observees_; // Observers of order 3+ observees

	// Zero-crossing observers observees
	Variables zc_observees_; // Observers observees
	Variables zc2_observees_; // Observers of order 2+ observees
	Variables zc3_observees_; // Observers of order 3+ observees

	// Observee directional derivative seed data
	VariableRefs zc_observees_v_ref_; // Observee value references for FMU directional derivative
	Reals zc_observees_dv_; // Observee seed derivatives for FMU directional derivative lookup

}; // Observers

} // fmu
} // QSS

#endif
