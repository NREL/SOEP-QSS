// Variable Observers
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2024 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_Observers_hh_INCLUDED
#define QSS_Observers_hh_INCLUDED

// QSS Headers
#include <QSS/FMU_ME.hh>
#include <QSS/RefsDers.hh> //n2d
#include <QSS/RefsDirDers.hh>
#include <QSS/RefsValsDers.hh>
#include <QSS/container.hh>
#include <QSS/options.hh>
#include <QSS/Range.hh>

// OpenMP Headers
#ifdef _OPENMP
#include <omp.h>
#endif // _OPENMP

// C++ Headers
#include <algorithm>
#include <cassert>
#include <utility>

namespace QSS {

// Variable Observers
template< typename V >
class Observers final
{

public: // Types

	using Variable = V;
	using Time = typename Variable::Time;
	using Real = typename Variable::Real;
	using Reals = typename Variable::Reals;
	using Variables = typename Variable::Variables;
	using VariablesSet = typename Variable::VariablesSet;
	using VariableRef = typename Variable::VariableRef;
	using VariableRefs = typename Variable::VariableRefs;
	using value_type = typename Variables::value_type;
	using size_type = typename Variables::size_type;
	using const_iterator = typename Variables::const_iterator;
	using iterator = typename Variables::iterator;
	using const_pointer = typename Variables::const_pointer;
	using pointer = typename Variables::pointer;
	using const_reference = typename Variables::const_reference;
	using reference = typename Variables::reference;

public: // Creation

	// FMU-ME Constructor
	explicit
	Observers( FMU_ME * fmu_me ) :
	 fmu_me_( fmu_me ),
	 advance_QSS_ptr( options::d2d ? &Observers::advance_QSS_d2d : &Observers::advance_QSS_n2d )
#ifdef _OPENMP
	 , max_threads_( omp_get_max_threads() )
#endif // _OPENMP
	{}

	// FMU-ME + Trigger Constructor
	Observers(
	 FMU_ME * fmu_me,
	 Variable * trigger
	) :
	 fmu_me_( fmu_me ),
	 trigger_( trigger ),
	 advance_QSS_ptr( options::d2d ? &Observers::advance_QSS_d2d : &Observers::advance_QSS_n2d )
#ifdef _OPENMP
	 , max_threads_( omp_get_max_threads() )
#endif // _OPENMP
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
		return !observers_.empty();
	}

	// Have Connected Output Observer(s)?
	bool
	connected_output_observer() const
	{
		return connected_output_observer_;
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

	// Add Observer
	void
	push_back( Variable * v )
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

	// Set Computational Observers
	void
	set_computational_observers()
	{
		assert( trigger_ != nullptr );
		computational_observers_.clear();
		if ( observers_.empty() ) return; // Nothing to do
		VariablesSet observers_checked;
		VariablesSet observers_set;
		find_computational_observers( observers_, observers_checked, observers_set );
		computational_observers_.assign( observers_set.begin(), observers_set.end() ); // Swap in the computational observers
	}

	// Assign Computational Observers
	void
	assign_computational_observers()
	{
		observers_ = std::move( computational_observers_ );
		computational_observers_.clear();
		computational_observers_.shrink_to_fit(); // Recover memory
	}

	// Initialize for Observers of a Single Variable
	void
	init()
	{
		set_up( true );
	}

	// Assign a Triggers Collection
	void
	assign( Variables const & triggers )
	{
		// Combine all non-trigger observers
		observers_.clear();
		if ( triggers.size() < 20u ) { // Linear search
			for ( Variable * trigger : triggers ) {
				for ( Variable * observer : trigger->observers() ) {
#if ( __cplusplus >= 202302L ) // C++23+
					if ( !std::ranges::contains( triggers, observer ) ) {
#else
					if ( std::find( triggers.begin(), triggers.end(), observer ) == triggers.end() ) {
#endif
						observers_.push_back( observer );
					}
				}
			}
		} else { // Binary search
			Variables sorted_triggers( triggers ); // Copy triggers to avoid sorting side effect causing non-deterministic results
			std::sort( sorted_triggers.begin(), sorted_triggers.end() );
			for ( Variable * trigger : sorted_triggers ) {
				for ( Variable * observer : trigger->observers() ) {
					if ( !std::binary_search( sorted_triggers.begin(), sorted_triggers.end(), observer ) ) {
						observers_.push_back( observer );
					}
				}
			}
		}

		set_up();
	}

	// Infinite Time Step Control Reset
	void
	dt_infinity_reset()
	{
		assert( options::dtInfReset );
		for ( Variable * observer : observers_ ) { // Reset dtInf relaxation state
			observer->dt_infinity_reset();
		}
	}

	// Advance
	void
	advance( Time const t )
	{
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );
		if ( qss_.have() ) advance_QSS( t ); // QSS state variables
		if ( r_.have() ) advance_R( t ); // Real variables
		if ( ox_.have() ) advance_OX( t ); // Other X-based variables
		if ( zc_.have() ) advance_ZC( t ); // Zero-crossing variables
		advance_F();
		if ( options::output::d ) advance_d();
	}

	// Output
	void
	out( Time const t )
	{
		assert( options::output::O );
		for ( Variable * observer : observers_ ) {
			observer->out( t );
		}
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

	// Find Extended Computational Observers
	void
	find_computational_observers(
	 Variables & observers,
	 VariablesSet & observers_checked,
	 VariablesSet & observers_set
	)
	{
		assert( trigger_ != nullptr );
		for ( Variable * observer : observers ) {
#if ( __cplusplus >= 202002L ) // C++20+
			if ( !observers_checked.contains( observer ) ) { // Observer not already processed
#else
			if ( observers_checked.find( observer ) == observers_checked.end() ) { // Observer not already processed
#endif
				observers_checked.insert( observer );
				if ( observer == trigger_ ) continue; // Trigger isn't a computational observer: Don't need to signal it when it updates
				if ( observer->is_Active() ) observers_set.insert( observer ); // Active => Computational
				if ( observer->is_QSS() ) { // Extend with its X-based observers
					find_computational_X_observers( observer->observers(), observers_checked, observers_set );
				} else if ( observer->not_ZC() ) { // Extend with its observers
					find_computational_observers( observer->observers(), observers_checked, observers_set ); // Recurse
				}
			}
		}
	}

	// Find Extended X-Based Computational Observers
	void
	find_computational_X_observers(
	 Variables & observers,
	 VariablesSet & observers_checked,
	 VariablesSet & observers_set
	)
	{
		assert( trigger_ != nullptr );
		for ( Variable * observer : observers ) {
			if ( observer->not_State() ) { // X-based
#if ( __cplusplus >= 202002L ) // C++20+
				if ( !observers_checked.contains( observer ) ) { // Observer not already processed
#else
				if ( observers_checked.find( observer ) == observers_checked.end() ) { // Observer not already processed
#endif
					observers_checked.insert( observer );
					if ( observer == trigger_ ) continue; // Trigger isn't a computational observer: Don't need to signal it when it updates
					if ( observer->is_Active() ) observers_set.insert( observer ); // Active => Computational
					find_computational_X_observers( observer->observers(), observers_checked, observers_set ); // Recurse
				}
			}
		}
	}

	// Reset Specs
	void
	reset_specs()
	{
		connected_output_observer_ = false;
		all_.reset();
		qss_.reset();
		ns_.reset();
		r_.reset();
		ox_.reset();
		zc_.reset();
		order_ = 0;
	}

	// Set Specs
	void
	set_specs()
	{
		reset_specs();
		if ( observers_.empty() ) return;

		connected_output_observer_ = false;
		all_.b() = 0u;
		all_.e() = observers_.size();
		size_type i( 0u );
		order_ = 0;
		for ( Variable * observer : observers_ ) {
			order_ = std::max( order_, observer->order() ); // Since ZC are sorted after B|I|D|Passive|Input we can't just look at the first variable (in case of strange model with ZC vars but no QSS vars)
		}

		// QSS state observers
		while ( ( i < all_.e() ) && ( observers_[ i ]->is_QSS() ) ) {
			Variable const * observer( observers_[ i ] );
			assert( observer->order() == order_ );
			qss_.b() = std::min( qss_.b(), i );
			if ( observer->connected_output ) connected_output_observer_ = true;
			++i;
		}
		if ( qss_.began() ) {
			qss_.e() = i;
		}

		// Non-state observers
		if ( i < all_.e() ) {
			ns_.b() = i;
			ns_.e() = all_.e();
		}

		// Real observers
		while ( ( i < all_.e() ) && ( observers_[ i ]->is_R() ) ) {
			Variable const * observer( observers_[ i ] );
			assert( observer->order() == order_ );
			r_.b() = std::min( r_.b(), i );
			if ( observer->connected_output ) connected_output_observer_ = true;
			++i;
		}
		if ( r_.began() ) {
			r_.e() = i;
		}

		// Other X-based observers
		while ( ( i < all_.e() ) && ( observers_[ i ]->not_ZC() ) ) {
			Variable const * observer( observers_[ i ] );
			assert( observer->order() == 0 );
			ox_.b() = std::min( ox_.b(), i );
			if ( observer->connected_output ) connected_output_observer_ = true;
			++i;
		}
		if ( ox_.began() ) {
			ox_.e() = i;
		}

		// Zero crossing observers
		while ( ( i < all_.e() ) && ( observers_[ i ]->is_ZC() ) ) {
			Variable const * observer( observers_[ i ] );
			assert( observer->order() == order_ );
			zc_.b() = std::min( zc_.b(), i );
			if ( observer->connected_output ) connected_output_observer_ = true;
			++i;
		}
		if ( zc_.began() ) {
			zc_.e() = i;
		}
	}

	// Set up for Current Observers
	void
	set_up( bool const recover = false )
	{
		if ( observers_.empty() ) {
			reset_specs();
			return;
		}

		// Remove duplicates then sort by type
		uniquify( observers_, recover ); // Sort by address and remove duplicates and optionally recover unused memory
		sort_by_type( observers_ );

		// Set specs
		set_specs();

		// FMU pooled data set up
		if ( qss_.have() ) { // State variables
			if ( options::d2d ) {
				qss_ders_.clear_and_reserve( qss_.n() );
				for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) {
					assert( observers_[ i ]->is_QSS() );
					qss_ders_.push_back( observers_[ i ]->der().ref() );
				}
			} else {
				assert( options::n2d );
				qss_dn2d_.clear_and_reserve( qss_.n() );
				for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) {
					assert( observers_[ i ]->is_QSS() );
					qss_dn2d_.push_back( observers_[ i ]->der().ref() );
				}
			}
		}
		if ( r_.have() ) { // R variables
			r_vars_.clear_and_reserve( r_.n() );
			for ( size_type i = r_.b(), e = r_.e(); i < e; ++i ) {
				assert( observers_[ i ]->is_R() );
				r_vars_.push_back( observers_[ i ]->var().ref() );
			}
		}
		if ( zc_.have() ) { // Zero-crossing variables
			zc_vars_.clear_and_reserve( zc_.n() );
			for ( size_type i = zc_.b(), e = zc_.e(); i < e; ++i ) {
				assert( observers_[ i ]->is_ZC() );
				zc_vars_.push_back( observers_[ i ]->var().ref() );
			}
		}

		// Observees setup /////

		// QSS observer observees set up
		if ( qss_.have() ) {
			qss_observees_.clear();
			for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) {
				Variable * observer( observers_[ i ] );
				for ( auto observee : observer->observees() ) {
					qss_observees_.push_back( observee );
				}
			}
			uniquify( qss_observees_ );
			n_qss_observees_ = qss_observees_.size();
		} else {
			n_qss_observees_ = 0u;
		}

		// Real observer observees set up
		if ( r_.have() ) {
			r_observees_.clear();
			for ( size_type i = r_.b(), e = r_.e(); i < e; ++i ) {
				Variable * observer( observers_[ i ] );
				assert( !observer->self_observee() );
				for ( auto observee : observer->observees() ) {
					r_observees_.push_back( observee );
				}
			}
			uniquify( r_observees_ );
			n_r_observees_ = r_observees_.size();
		} else {
			n_r_observees_ = 0u;
		}

		// Other X-based observer observees set up //Do Add observee operation pooling for these observers
		// if ( ox_.have() ) {
		// 	ox_observees_.clear();
		// 	for ( size_type i = ox_.b(), e = ox_.e(); i < e; ++i ) {
		// 		Variable * observer( observers_[ i ] );
		//			assert( !observer->self_observee() );
		// 		for ( auto observee : observer->observees() ) {
		// 			ox_observees_.push_back( observee );
		// 		}
		// 	}
		// 	uniquify( ox_observees_ );
		// 	n_ox_observees_ = ox_observees_.size();
		// } else {
		// 	n_ox_observees_ = 0u;
		// }

		// Zero-crossing observer observees set up
		if ( zc_.have() ) {
			zc_observees_.clear();
			for ( size_type i = zc_.b(), e = zc_.e(); i < e; ++i ) {
				Variable * observer( observers_[ i ] );
				assert( !observer->self_observee() );
				for ( auto observee : observer->observees() ) {
					zc_observees_.push_back( observee );
				}
			}
			uniquify( zc_observees_ );
			n_zc_observees_ = zc_observees_.size();
		} else {
			n_zc_observees_ = 0u;
		}

		// Set up observers observees pooled derivative data /////

		// QSS observers
		if ( qss_.have() ) {
			qss_observees_v_ref_.clear(); qss_observees_v_ref_.reserve( n_qss_observees_ );
			qss_observees_v_.clear(); qss_observees_v_.resize( n_qss_observees_ );
			if ( options::d2d ) { qss_observees_dv_.clear(); qss_observees_dv_.resize( n_qss_observees_ ); }
			for ( auto observee : qss_observees_ ) {
				qss_observees_v_ref_.push_back( observee->var().ref() );
			}
		}

		// Real observers
		if ( r_.have() ) {
			r_observees_v_ref_.clear(); r_observees_v_ref_.reserve( n_r_observees_ );
			r_observees_v_.clear(); r_observees_v_.resize( n_r_observees_ );
			r_observees_dv_.clear(); r_observees_dv_.resize( n_r_observees_ );
			for ( auto observee : r_observees_ ) {
				r_observees_v_ref_.push_back( observee->var().ref() );
			}
		}

		// Zero-crossing observers
		if ( zc_.have() ) {
			zc_observees_v_ref_.clear(); zc_observees_v_ref_.reserve( n_zc_observees_ );
			zc_observees_v_.clear(); zc_observees_v_.resize( n_zc_observees_ );
			zc_observees_dv_.clear(); zc_observees_dv_.resize( n_zc_observees_ );
			for ( auto observee : zc_observees_ ) {
				zc_observees_v_ref_.push_back( observee->var().ref() );
			}
		}
	}

	// Advance QSS State Observers
	void
	advance_QSS( Time const t )
	{
		(this->*advance_QSS_ptr)( t );
	}

	// Advance QSS State Observers: Directional Second Derivatives
	void
	advance_QSS_d2d( Time const t )
	{
		assert( options::d2d );
		assert( qss_.have() );
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );
		assert( qss_.n() == qss_ders_.size() );

#ifdef _OPENMP
		if ( ( max_threads_ > 1u ) && ( qss_.n() >= max_threads_ * 64u ) ) { // Parallel

		size_type const qss_b( qss_.b() );
		size_type const qss_e( qss_.e() );
		set_qss_observees_values_parallel( t );
		fmu_me_->get_reals( qss_.n(), qss_ders_.refs.data(), qss_ders_.ders.data() );
		#pragma omp parallel for schedule(static)
		for ( size_type i = qss_b; i < qss_e; ++i ) { // Observer advance stage 1
			assert( observers_[ i ]->is_QSS() );
			observers_[ i ]->advance_observer_1( t, qss_ders_.ders[ i - qss_b ] );
		}
		if ( order_ >= 2 ) {
			get_qss_second_derivatives_parallel( t );
			#pragma omp parallel for schedule(static)
			for ( size_type i = qss_b; i < qss_e; ++i ) { // Observer advance stage 2
				observers_[ i ]->advance_observer_2_dd2( qss_ders_.ders[ i - qss_b ] );
			}
			if ( order_ >= 3 ) {
				Time const tN( t + options::dtND );
				fmu_me_->set_time( tN );
				set_qss_observees_values_parallel( tN );
				get_qss_second_derivatives_parallel( tN );
				#pragma omp parallel for schedule(static)
				for ( size_type i = qss_b; i < qss_e; ++i ) { // Observer advance stage 3
					observers_[ i ]->advance_observer_3_dd2( qss_ders_.ders[ i - qss_b ] );
				}
				fmu_me_->set_time( t );
			}
		}

		} else { // Serial
#endif // _OPENMP

		set_qss_observees_values( t );
		fmu_me_->get_reals( qss_.n(), qss_ders_.refs.data(), qss_ders_.ders.data() );
		for ( size_type i = qss_.b(), e = qss_.e(), j = 0u; i < e; ++i, ++j ) { // Observer advance stage 1
			assert( observers_[ i ]->is_QSS() );
			observers_[ i ]->advance_observer_1( t, qss_ders_.ders[ j ] );
		}
		if ( order_ >= 2 ) {
			get_qss_second_derivatives( t );
			for ( size_type i = qss_.b(), e = qss_.e(), j = 0u; i < e; ++i, ++j ) { // Observer advance stage 2
				observers_[ i ]->advance_observer_2_dd2( qss_ders_.ders[ j ] );
			}
			if ( order_ >= 3 ) {
				Time const tN( t + options::dtND );
				fmu_me_->set_time( tN );
				set_qss_observees_values( tN );
				get_qss_second_derivatives( tN );
				for ( size_type i = qss_.b(), e = qss_.e(), j = 0u; i < e; ++i, ++j ) { // Observer advance stage 3
					observers_[ i ]->advance_observer_3_dd2( qss_ders_.ders[ j ] );
				}
				fmu_me_->set_time( t );
			}
		}

#ifdef _OPENMP
		}
#endif // _OPENMP

	}

	// Advance QSS State Observers: Numerical Second Derivatives
	void
	advance_QSS_n2d( Time const t )
	{
		assert( options::n2d );
		assert( qss_.have() );
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );
		assert( qss_.n() == qss_dn2d_.size() );

#ifdef _OPENMP
		if ( ( max_threads_ > 1u ) && ( qss_.n() >= max_threads_ * 64u ) ) { // Parallel

		size_type const qss_b( qss_.b() );
		size_type const qss_e( qss_.e() );
		set_qss_observees_values_parallel( t );
		fmu_me_->get_reals( qss_.n(), qss_dn2d_.refs.data(), qss_dn2d_.ders.data() );
		#pragma omp parallel for schedule(static)
		for ( size_type i = qss_b; i < qss_e; ++i ) { // Observer advance stage 1
			assert( observers_[ i ]->is_QSS() );
			observers_[ i ]->advance_observer_1( t, qss_dn2d_.ders[ i - qss_b ] );
		}
		if ( order_ >= 3 ) {
			Time tN( t - options::dtND );
			if ( fwd_time( tN ) ) { // Centered ND
				fmu_me_->set_time( tN );
				set_qss_observees_values_parallel( tN );
				fmu_me_->get_reals( qss_.n(), qss_dn2d_.refs.data(), qss_dn2d_.ders.data() );
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				set_qss_observees_values_parallel( tN );
				fmu_me_->get_reals( qss_.n(), qss_dn2d_.refs.data(), qss_dn2d_.ders_p.data() );
				#pragma omp parallel for schedule(static)
				for ( size_type i = qss_b; i < qss_e; ++i ) { // Observer advance stage 2
					observers_[ i ]->advance_observer_2( qss_dn2d_.ders[ i - qss_b ], qss_dn2d_.ders_p[ i - qss_b ] );
				}
				#pragma omp parallel for schedule(static)
				for ( size_type i = qss_b; i < qss_e; ++i ) { // Observer advance stage 3
					observers_[ i ]->advance_observer_3();
				}
			} else { // Forward ND
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				set_qss_observees_values_parallel( tN );
				fmu_me_->get_reals( qss_.n(), qss_dn2d_.refs.data(), qss_dn2d_.ders.data() );
				tN = t + options::two_dtND;
				fmu_me_->set_time( tN );
				set_qss_observees_values_parallel( tN );
				fmu_me_->get_reals( qss_.n(), qss_dn2d_.refs.data(), qss_dn2d_.ders_p.data() );
				#pragma omp parallel for schedule(static)
				for ( size_type i = qss_b; i < qss_e; ++i ) { // Observer advance stage 2
					observers_[ i ]->advance_observer_2_forward( qss_dn2d_.ders[ i - qss_b ], qss_dn2d_.ders_p[ i - qss_b ] );
				}
				#pragma omp parallel for schedule(static)
				for ( size_type i = qss_b; i < qss_e; ++i ) { // Observer advance stage 3
					observers_[ i ]->advance_observer_3_forward();
				}
			}
			fmu_me_->set_time( t );
		} else if ( order_ >= 2 ) {
			Time const tN( t + options::dtND );
			fmu_me_->set_time( tN );
			set_qss_observees_values_parallel( tN );
			fmu_me_->get_reals( qss_.n(), qss_dn2d_.refs.data(), qss_dn2d_.ders_p.data() );
			#pragma omp parallel for schedule(static)
			for ( size_type i = qss_b; i < qss_e; ++i ) { // Observer advance stage 2
				observers_[ i ]->advance_observer_2( qss_dn2d_.ders_p[ i - qss_b ] );
			}
			fmu_me_->set_time( t );
		}

		} else { // Serial
#endif // _OPENMP

		set_qss_observees_values( t );
		fmu_me_->get_reals( qss_.n(), qss_dn2d_.refs.data(), qss_dn2d_.ders.data() );
		for ( size_type i = qss_.b(), e = qss_.e(), j = 0u; i < e; ++i, ++j ) { // Observer advance stage 1
			assert( observers_[ i ]->is_QSS() );
			observers_[ i ]->advance_observer_1( t, qss_dn2d_.ders[ j ] );
		}
		if ( order_ >= 3 ) {
			Time tN( t - options::dtND );
			if ( fwd_time( tN ) ) { // Centered ND
				fmu_me_->set_time( tN );
				set_qss_observees_values( tN );
				fmu_me_->get_reals( qss_.n(), qss_dn2d_.refs.data(), qss_dn2d_.ders.data() );
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				set_qss_observees_values( tN );
				fmu_me_->get_reals( qss_.n(), qss_dn2d_.refs.data(), qss_dn2d_.ders_p.data() );
				for ( size_type i = qss_.b(), e = qss_.e(), j = 0u; i < e; ++i, ++j ) { // Observer advance stage 2
					observers_[ i ]->advance_observer_2( qss_dn2d_.ders[ j ], qss_dn2d_.ders_p[ j ] );
				}
				for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) { // Observer advance stage 3
					observers_[ i ]->advance_observer_3();
				}
			} else { // Forward ND
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				set_qss_observees_values( tN );
				fmu_me_->get_reals( qss_.n(), qss_dn2d_.refs.data(), qss_dn2d_.ders.data() );
				tN = t + options::two_dtND;
				fmu_me_->set_time( tN );
				set_qss_observees_values( tN );
				fmu_me_->get_reals( qss_.n(), qss_dn2d_.refs.data(), qss_dn2d_.ders_p.data() );
				for ( size_type i = qss_.b(), e = qss_.e(), j = 0u; i < e; ++i, ++j ) { // Observer advance stage 2
					observers_[ i ]->advance_observer_2_forward( qss_dn2d_.ders[ j ], qss_dn2d_.ders_p[ j ] );
				}
				for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) { // Observer advance stage 3
					observers_[ i ]->advance_observer_3_forward();
				}
			}
			fmu_me_->set_time( t );
		} else if ( order_ >= 2 ) {
			Time const tN( t + options::dtND );
			fmu_me_->set_time( tN );
			set_qss_observees_values( tN );
			fmu_me_->get_reals( qss_.n(), qss_dn2d_.refs.data(), qss_dn2d_.ders_p.data() );
			for ( size_type i = qss_.b(), e = qss_.e(), j = 0u; i < e; ++i, ++j ) { // Observer advance stage 2
				observers_[ i ]->advance_observer_2( qss_dn2d_.ders_p[ j ] );
			}
			fmu_me_->set_time( t );
		}

#ifdef _OPENMP
		}
#endif // _OPENMP

	}

	// Advance Real Non-State Observers
	void
	advance_R( Time const t )
	{
		assert( r_.have() );
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );
		assert( r_.n() == r_vars_.size() );

#ifdef _OPENMP
		if ( ( max_threads_ > 1u ) && ( r_.n() >= max_threads_ * 64u ) ) { // Parallel

		size_type const r_b( r_.b() );
		size_type const r_e( r_.e() );
		set_r_observees_values_parallel( t );
		fmu_me_->get_reals( r_.n(), r_vars_.refs.data(), r_vars_.vals.data() );
		set_r_observees_dv_parallel( t );
		fmu_me_->get_directional_derivatives(
		 r_observees_v_ref_.data(),
		 n_r_observees_,
		 r_vars_.refs.data(),
		 r_.n(),
		 r_observees_dv_.data(),
		 r_vars_.ders.data()
		); // Get derivatives at t
		#pragma omp parallel for schedule(static)
		for ( size_type i = r_b; i < r_e; ++i ) { // Observer advance stage 1
			assert( observers_[ i ]->is_Active() );
			assert( observers_[ i ]->is_R() );
			observers_[ i ]->advance_observer_1( t, r_vars_.vals[ i - r_b ], r_vars_.ders[ i - r_b ] );
		}
		if ( order_ >= 3 ) {
			Time tN( t - options::dtND );
			if ( fwd_time( tN ) ) { // Centered ND
				fmu_me_->set_time( tN );
				set_r_observees_values_parallel( tN );
				set_r_observees_dv_parallel( tN );
				fmu_me_->get_directional_derivatives(
				 r_observees_v_ref_.data(),
				 n_r_observees_,
				 r_vars_.refs.data(),
				 r_.n(),
				 r_observees_dv_.data(),
				 r_vars_.ders.data()
				); // Get derivatives at t - dtND
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				set_r_observees_values_parallel( tN );
				set_r_observees_dv_parallel( tN );
				fmu_me_->get_directional_derivatives(
				 r_observees_v_ref_.data(),
				 n_r_observees_,
				 r_vars_.refs.data(),
				 r_.n(),
				 r_observees_dv_.data(),
				 r_vars_.ders_p.data()
				); // Get derivatives at t + dtND
				#pragma omp parallel for schedule(static)
				for ( size_type i = r_b; i < r_e; ++i ) { // Observer advance stage 2
					observers_[ i ]->advance_observer_2( r_vars_.ders[ i - r_b ], r_vars_.ders_p[ i - r_b ] );
				}
				#pragma omp parallel for schedule(static)
				for ( size_type i = r_b; i < r_e; ++i ) { // Observer advance stage 3
					observers_[ i ]->advance_observer_3();
				}
			} else { // Forward ND
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				set_r_observees_values_parallel( tN );
				set_r_observees_dv_parallel( tN );
				fmu_me_->get_directional_derivatives(
				 r_observees_v_ref_.data(),
				 n_r_observees_,
				 r_vars_.refs.data(),
				 r_.n(),
				 r_observees_dv_.data(),
				 r_vars_.ders.data()
				); // Get derivatives at t + dtND
				tN = t + options::two_dtND;
				fmu_me_->set_time( tN );
				set_r_observees_values_parallel( tN );
				set_r_observees_dv_parallel( tN );
				fmu_me_->get_directional_derivatives(
				 r_observees_v_ref_.data(),
				 n_r_observees_,
				 r_vars_.refs.data(),
				 r_.n(),
				 r_observees_dv_.data(),
				 r_vars_.ders_p.data()
				); // Get derivatives at t + 2*dtND
				#pragma omp parallel for schedule(static)
				for ( size_type i = r_b; i < r_e; ++i ) { // Observer advance stage 2
					observers_[ i ]->advance_observer_2_forward( r_vars_.ders[ i - r_b ], r_vars_.ders_p[ i - r_b ] );
				}
				#pragma omp parallel for schedule(static)
				for ( size_type i = r_b; i < r_e; ++i ) { // Observer advance stage 3
					observers_[ i ]->advance_observer_3_forward();
				}
			}
			fmu_me_->set_time( t );
		} else if ( order_ >= 2 ) {
			Time const tN( t + options::dtND );
			fmu_me_->set_time( tN );
			set_r_observees_values_parallel( tN );
			set_r_observees_dv_parallel( tN );
			fmu_me_->get_directional_derivatives(
			 r_observees_v_ref_.data(),
			 n_r_observees_,
			 r_vars_.refs.data(),
			 r_.n(),
			 r_observees_dv_.data(),
			 r_vars_.ders_p.data()
			); // Get derivatives at t + dtND
			#pragma omp parallel for schedule(static)
			for ( size_type i = r_b; i < r_e; ++i ) { // Observer advance stage 2
				observers_[ i ]->advance_observer_2( r_vars_.ders_p[ i - r_b ] );
			}
			fmu_me_->set_time( t );
		}

		} else { // Serial
#endif // _OPENMP

		set_r_observees_values( t );
		fmu_me_->get_reals( r_.n(), r_vars_.refs.data(), r_vars_.vals.data() );
		set_r_observees_dv( t );
		fmu_me_->get_directional_derivatives(
		 r_observees_v_ref_.data(),
		 n_r_observees_,
		 r_vars_.refs.data(),
		 r_.n(),
		 r_observees_dv_.data(),
		 r_vars_.ders.data()
		); // Get derivatives at t
		for ( size_type i = r_.b(), e = r_.e(), j = 0u; i < e; ++i, ++j ) { // Observer advance stage 1
			assert( observers_[ i ]->is_Active() );
			assert( observers_[ i ]->is_R() );
			observers_[ i ]->advance_observer_1( t, r_vars_.vals[ j ], r_vars_.ders[ j ] );
		}
		if ( order_ >= 3 ) {
			Time tN( t - options::dtND );
			if ( fwd_time( tN ) ) { // Centered ND
				fmu_me_->set_time( tN );
				set_r_observees_values( tN );
				set_r_observees_dv( tN );
				fmu_me_->get_directional_derivatives(
				 r_observees_v_ref_.data(),
				 n_r_observees_,
				 r_vars_.refs.data(),
				 r_.n(),
				 r_observees_dv_.data(),
				 r_vars_.ders.data()
				); // Get derivatives at t - dtND
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				set_r_observees_values( tN );
				set_r_observees_dv( tN );
				fmu_me_->get_directional_derivatives(
				 r_observees_v_ref_.data(),
				 n_r_observees_,
				 r_vars_.refs.data(),
				 r_.n(),
				 r_observees_dv_.data(),
				 r_vars_.ders_p.data()
				); // Get derivatives at t + dtND
				for ( size_type i = r_.b(), j = 0u, e = r_.e(); i < e; ++i, ++j ) { // Observer advance stage 2
					observers_[ i ]->advance_observer_2( r_vars_.ders[ j ], r_vars_.ders_p[ j ] );
				}
				for ( size_type i = r_.b(), e = r_.e(); i < e; ++i ) { // Observer advance stage 3
					observers_[ i ]->advance_observer_3();
				}
			} else { // Forward ND
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				set_r_observees_values( tN );
				set_r_observees_dv( tN );
				fmu_me_->get_directional_derivatives(
				 r_observees_v_ref_.data(),
				 n_r_observees_,
				 r_vars_.refs.data(),
				 r_.n(),
				 r_observees_dv_.data(),
				 r_vars_.ders.data()
				); // Get derivatives at t + dtND
				tN = t + options::two_dtND;
				fmu_me_->set_time( tN );
				set_r_observees_values( tN );
				set_r_observees_dv( tN );
				fmu_me_->get_directional_derivatives(
				 r_observees_v_ref_.data(),
				 n_r_observees_,
				 r_vars_.refs.data(),
				 r_.n(),
				 r_observees_dv_.data(),
				 r_vars_.ders_p.data()
				); // Get derivatives at t + 2*dtND
				for ( size_type i = r_.b(), j = 0u, e = r_.e(); i < e; ++i, ++j ) { // Observer advance stage 2
					observers_[ i ]->advance_observer_2_forward( r_vars_.ders[ j ], r_vars_.ders_p[ j ] );
				}
				for ( size_type i = r_.b(), e = r_.e(); i < e; ++i ) { // Observer advance stage 3
					observers_[ i ]->advance_observer_3_forward();
				}
			}
			fmu_me_->set_time( t );
		} else if ( order_ >= 2 ) {
			Time const tN( t + options::dtND );
			fmu_me_->set_time( tN );
			set_r_observees_values( tN );
			set_r_observees_dv( tN );
			fmu_me_->get_directional_derivatives(
			 r_observees_v_ref_.data(),
			 n_r_observees_,
			 r_vars_.refs.data(),
			 r_.n(),
			 r_observees_dv_.data(),
			 r_vars_.ders_p.data()
			); // Get derivatives at t + dtND
			for ( size_type i = r_.b(), j = 0u, e = r_.e(); i < e; ++i, ++j ) { // Observer advance stage 2
				observers_[ i ]->advance_observer_2( r_vars_.ders_p[ j ] );
			}
			fmu_me_->set_time( t );
		}

#ifdef _OPENMP
		}
#endif // _OPENMP

	}

	// Advance Other X-Based Observers
	void
	advance_OX( Time const t )
	{
		assert( ox_.have() );
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );

#ifdef _OPENMP
		if ( ( max_threads_ > 1u ) && ( ox_.n() >= max_threads_ * 64u ) ) { // Parallel

		size_type const ox_b( ox_.b() );
		size_type const ox_e( ox_.e() );
		#pragma omp parallel for schedule(static)
		for ( size_type i = ox_b; i < ox_e; ++i ) {
			assert( observers_[ i ]->is_BIDR() && !( observers_[ i ]->is_R() && observers_[ i ]->is_Active() ) );
			observers_[ i ]->advance_observer_1( t );
		}

		} else { // Serial
#endif // _OPENMP

		for ( size_type i = ox_.b(), e = ox_.e(); i < e; ++i ) {
			assert( observers_[ i ]->is_BIDR() && !( observers_[ i ]->is_R() && observers_[ i ]->is_Active() ) );
			observers_[ i ]->advance_observer_1( t );
		}

#ifdef _OPENMP
		}
#endif // _OPENMP

	}

	// Advance Zero-Crossing Observers
	void
	advance_ZC( Time const t )
	{
		assert( zc_.have() );
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );
		assert( fmu_me_->has_event_indicators );
		assert( zc_.n() == zc_vars_.size() );

#ifdef _OPENMP
		if ( ( max_threads_ > 1u ) && ( zc_.n() >= max_threads_ * 64u ) ) { // Parallel

		size_type const zc_b( zc_.b() );
		size_type const zc_e( zc_.e() );
		set_zc_observees_values_parallel( t );
		fmu_me_->get_reals( zc_.n(), zc_vars_.refs.data(), zc_vars_.vals.data() );
		set_zc_observees_dv_parallel( t );
		fmu_me_->get_directional_derivatives(
		 zc_observees_v_ref_.data(),
		 n_zc_observees_,
		 zc_vars_.refs.data(),
		 zc_.n(),
		 zc_observees_dv_.data(),
		 zc_vars_.ders.data()
		); // Get derivatives at t
		#pragma omp parallel for schedule(static)
		for ( size_type i = zc_b; i < zc_e; ++i ) { // Observer advance stage 1
			assert( observers_[ i ]->is_ZC() );
			observers_[ i ]->advance_observer_1( t, zc_vars_.vals[ i - zc_b ], zc_vars_.ders[ i - zc_b ] );
		}
		if ( order_ >= 3 ) {
			Time tN( t - options::dtND );
			if ( fwd_time( tN ) ) { // Centered ND
				fmu_me_->set_time( tN );
				set_zc_observees_values_parallel( tN );
				set_zc_observees_dv_parallel( tN );
				fmu_me_->get_directional_derivatives(
				 zc_observees_v_ref_.data(),
				 n_zc_observees_,
				 zc_vars_.refs.data(),
				 zc_.n(),
				 zc_observees_dv_.data(),
				 zc_vars_.ders.data()
				); // Get derivatives at t - dtND
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				set_zc_observees_values_parallel( tN );
				set_zc_observees_dv_parallel( tN );
				fmu_me_->get_directional_derivatives(
				 zc_observees_v_ref_.data(),
				 n_zc_observees_,
				 zc_vars_.refs.data(),
				 zc_.n(),
				 zc_observees_dv_.data(),
				 zc_vars_.ders_p.data()
				); // Get derivatives at t + dtND
				#pragma omp parallel for schedule(static)
				for ( size_type i = zc_b; i < zc_e; ++i ) { // Observer advance stage 2
					observers_[ i ]->advance_observer_2( zc_vars_.ders[ i - zc_b ], zc_vars_.ders_p[ i - zc_b ] );
				}
				#pragma omp parallel for schedule(static)
				for ( size_type i = zc_b; i < zc_e; ++i ) { // Observer advance stage 3
					observers_[ i ]->advance_observer_3();
				}
			} else { // Forward ND
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				set_zc_observees_values_parallel( tN );
				set_zc_observees_dv_parallel( tN );
				fmu_me_->get_directional_derivatives(
				 zc_observees_v_ref_.data(),
				 n_zc_observees_,
				 zc_vars_.refs.data(),
				 zc_.n(),
				 zc_observees_dv_.data(),
				 zc_vars_.ders.data()
				); // Get derivatives at t + dtND
				tN = t + options::two_dtND;
				fmu_me_->set_time( tN );
				set_zc_observees_values_parallel( tN );
				set_zc_observees_dv_parallel( tN );
				fmu_me_->get_directional_derivatives(
				 zc_observees_v_ref_.data(),
				 n_zc_observees_,
				 zc_vars_.refs.data(),
				 zc_.n(),
				 zc_observees_dv_.data(),
				 zc_vars_.ders_p.data()
				); // Get derivatives at t + 2*dtND
				#pragma omp parallel for schedule(static)
				for ( size_type i = zc_b; i < zc_e; ++i ) { // Observer advance stage 2
					observers_[ i ]->advance_observer_2_forward( zc_vars_.ders[ i - zc_b ], zc_vars_.ders_p[ i - zc_b ] );
				}
				#pragma omp parallel for schedule(static)
				for ( size_type i = zc_b; i < zc_e; ++i ) { // Observer advance stage 3
					observers_[ i ]->advance_observer_3_forward();
				}
			}
			fmu_me_->set_time( t );
		} else if ( order_ >= 2 ) {
			Time const tN( t + options::dtND );
			fmu_me_->set_time( tN );
			set_zc_observees_values_parallel( tN );
			set_zc_observees_dv_parallel( tN );
			fmu_me_->get_directional_derivatives(
			 zc_observees_v_ref_.data(),
			 n_zc_observees_,
			 zc_vars_.refs.data(),
			 zc_.n(),
			 zc_observees_dv_.data(),
			 zc_vars_.ders_p.data()
			); // Get derivatives at t + dtND
			#pragma omp parallel for schedule(static)
			for ( size_type i = zc_b; i < zc_e; ++i ) { // Observer advance stage 2
				observers_[ i ]->advance_observer_2( zc_vars_.ders_p[ i - zc_b ] );
			}
			fmu_me_->set_time( t );
		}

		} else { // Serial
#endif // _OPENMP

		set_zc_observees_values( t );
		fmu_me_->get_reals( zc_.n(), zc_vars_.refs.data(), zc_vars_.vals.data() );
		set_zc_observees_dv( t );
		fmu_me_->get_directional_derivatives(
		 zc_observees_v_ref_.data(),
		 n_zc_observees_,
		 zc_vars_.refs.data(),
		 zc_.n(),
		 zc_observees_dv_.data(),
		 zc_vars_.ders.data()
		); // Get derivatives at t
		for ( size_type i = zc_.b(), e = zc_.e(), j = 0u; i < e; ++i, ++j ) { // Observer advance stage 1
			assert( observers_[ i ]->is_ZC() );
			observers_[ i ]->advance_observer_1( t, zc_vars_.vals[ j ], zc_vars_.ders[ j ] );
		}
		if ( order_ >= 3 ) {
			Time tN( t - options::dtND );
			if ( fwd_time( tN ) ) { // Centered ND
				fmu_me_->set_time( tN );
				set_zc_observees_values( tN );
				set_zc_observees_dv( tN );
				fmu_me_->get_directional_derivatives(
				 zc_observees_v_ref_.data(),
				 n_zc_observees_,
				 zc_vars_.refs.data(),
				 zc_.n(),
				 zc_observees_dv_.data(),
				 zc_vars_.ders.data()
				); // Get derivatives at t - dtND
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				set_zc_observees_values( tN );
				set_zc_observees_dv( tN );
				fmu_me_->get_directional_derivatives(
				 zc_observees_v_ref_.data(),
				 n_zc_observees_,
				 zc_vars_.refs.data(),
				 zc_.n(),
				 zc_observees_dv_.data(),
				 zc_vars_.ders_p.data()
				); // Get derivatives at t + dtND
				for ( size_type i = zc_.b(), j = 0u, e = zc_.e(); i < e; ++i, ++j ) { // Observer advance stage 2
					observers_[ i ]->advance_observer_2( zc_vars_.ders[ j ], zc_vars_.ders_p[ j ] );
				}
				for ( size_type i = zc_.b(), e = zc_.e(); i < e; ++i ) { // Observer advance stage 3
					observers_[ i ]->advance_observer_3();
				}
			} else { // Forward ND
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				set_zc_observees_values( tN );
				set_zc_observees_dv( tN );
				fmu_me_->get_directional_derivatives(
				 zc_observees_v_ref_.data(),
				 n_zc_observees_,
				 zc_vars_.refs.data(),
				 zc_.n(),
				 zc_observees_dv_.data(),
				 zc_vars_.ders.data()
				); // Get derivatives at t + dtND
				tN = t + options::two_dtND;
				fmu_me_->set_time( tN );
				set_zc_observees_values( tN );
				set_zc_observees_dv( tN );
				fmu_me_->get_directional_derivatives(
				 zc_observees_v_ref_.data(),
				 n_zc_observees_,
				 zc_vars_.refs.data(),
				 zc_.n(),
				 zc_observees_dv_.data(),
				 zc_vars_.ders_p.data()
				); // Get derivatives at t + 2*dtND
				for ( size_type i = zc_.b(), j = 0u, e = zc_.e(); i < e; ++i, ++j ) { // Observer advance stage 2
					observers_[ i ]->advance_observer_2_forward( zc_vars_.ders[ j ], zc_vars_.ders_p[ j ] );
				}
				for ( size_type i = zc_.b(), e = zc_.e(); i < e; ++i ) { // Observer advance stage 3
					observers_[ i ]->advance_observer_3_forward();
				}
			}
			fmu_me_->set_time( t );
		} else if ( order_ >= 2 ) {
			Time const tN( t + options::dtND );
			fmu_me_->set_time( tN );
			set_zc_observees_values( tN );
			set_zc_observees_dv( tN );
			fmu_me_->get_directional_derivatives(
			 zc_observees_v_ref_.data(),
			 n_zc_observees_,
			 zc_vars_.refs.data(),
			 zc_.n(),
			 zc_observees_dv_.data(),
			 zc_vars_.ders_p.data()
			); // Get derivatives at t + dtND
			for ( size_type i = zc_.b(), j = 0u, e = zc_.e(); i < e; ++i, ++j ) { // Observer advance stage 2
				observers_[ i ]->advance_observer_2( zc_vars_.ders_p[ j ] );
			}
			fmu_me_->set_time( t );
		}

#ifdef _OPENMP
		}
#endif // _OPENMP

	}

	// Advance: Stage Final
	void
	advance_F()
	{
#ifdef _OPENMP
		if ( ( max_threads_ > 1u ) && ( observers_.size() >= max_threads_ * 64u ) ) { // Parallel

		#pragma omp parallel for schedule(static)
		for ( typename Variables::iterator observer = observers_.begin(); observer != observers_.end(); ++observer ) { // OpenMP 3.0+ supports this form
		// for ( Variable * observer : observers_ ) { // OpenMP 5.0+ supports this form
			(*observer)->advance_observer_F_parallel();
		}

		for ( Variable * observer : observers_ ) {
			observer->advance_observer_F_serial();
		}

		} else { // Serial
#endif // _OPENMP
		for ( Variable * observer : observers_ ) {
			observer->advance_observer_F();
		}
#ifdef _OPENMP
		}
#endif // _OPENMP
	}

	// Advance: Stage d
	void
	advance_d() const
	{
		assert( options::output::d );
		for ( Variable const * observer : observers_ ) {
			observer->advance_observer_d();
		}
	}

	// Set QSS Observees FMU Values at Time t
	void
	set_qss_observees_values( Time const t )
	{
		for ( size_type i = 0u; i < n_qss_observees_; ++i ) { // Set observee value vector
#ifndef QSS_PROPAGATE_CONTINUOUS
			qss_observees_v_[ i ] = qss_observees_[ i ]->q( t ); // Quantized: Traditional QSS
#else
			qss_observees_v_[ i ] = qss_observees_[ i ]->x( t ); // Continuous: Modified QSS
#endif
		}
		fmu_me_->set_reals( qss_observees_.size(), qss_observees_v_ref_.data(), qss_observees_v_.data() ); // Set observees FMU values
	}

	// Set QSS Observees FMU Values at Time t
	void
	set_qss_observees_values_parallel( Time const t )
	{
		#pragma omp parallel for schedule(static)
		for ( size_type i = 0u; i < n_qss_observees_; ++i ) { // Set observee value vector
#ifndef QSS_PROPAGATE_CONTINUOUS
			qss_observees_v_[ i ] = qss_observees_[ i ]->q( t ); // Quantized: Traditional QSS
#else
			qss_observees_v_[ i ] = qss_observees_[ i ]->x( t ); // Continuous: Modified QSS
#endif
		}
		fmu_me_->set_reals( qss_observees_.size(), qss_observees_v_ref_.data(), qss_observees_v_.data() ); // Set observees FMU values
	}

	// Get QSS Second Derivatives at Time t
	void
	get_qss_second_derivatives( Time const t )
	{
		assert( options::d2d );

		for ( size_type i = 0u; i < n_qss_observees_; ++i ) {
#ifndef QSS_PROPAGATE_CONTINUOUS
			qss_observees_dv_[ i ] = qss_observees_[ i ]->q1( t ); // Quantized: Traditional QSS
#else
			qss_observees_dv_[ i ] = qss_observees_[ i ]->x1( t ); // Continuous: Modified QSS
#endif
		}
		fmu_me_->get_directional_derivatives(
		 qss_observees_v_ref_.data(),
		 n_qss_observees_,
		 qss_ders_.refs.data(),
		 qss_.n(),
		 qss_observees_dv_.data(),
		 qss_ders_.ders.data()
		); // Get 2nd derivatives at t
	}

	// Get QSS Second Derivatives at Time t
	void
	get_qss_second_derivatives_parallel( Time const t )
	{
		assert( options::d2d );

		#pragma omp parallel for schedule(static)
		for ( size_type i = 0u; i < n_qss_observees_; ++i ) {
#ifndef QSS_PROPAGATE_CONTINUOUS
			qss_observees_dv_[ i ] = qss_observees_[ i ]->q1( t ); // Quantized: Traditional QSS
#else
			qss_observees_dv_[ i ] = qss_observees_[ i ]->x1( t ); // Continuous: Modified QSS
#endif
		}
		fmu_me_->get_directional_derivatives(
		 qss_observees_v_ref_.data(),
		 n_qss_observees_,
		 qss_ders_.refs.data(),
		 qss_.n(),
		 qss_observees_dv_.data(),
		 qss_ders_.ders.data()
		); // Get 2nd derivatives at t
	}

	// Set Real Observees FMU Values at Time t
	void
	set_r_observees_values( Time const t )
	{
		for ( size_type i = 0u; i < n_r_observees_; ++i ) { // Set observee value vector
			r_observees_v_[ i ] = r_observees_[ i ]->x( t );
		}
		fmu_me_->set_reals( r_observees_.size(), r_observees_v_ref_.data(), r_observees_v_.data() ); // Set observees FMU values
	}

	// Set Real Observees FMU Values at Time t
	void
	set_r_observees_values_parallel( Time const t )
	{
		#pragma omp parallel for schedule(static)
		for ( size_type i = 0u; i < n_r_observees_; ++i ) { // Set observee value vector
			r_observees_v_[ i ] = r_observees_[ i ]->x( t );
		}
		fmu_me_->set_reals( r_observees_.size(), r_observees_v_ref_.data(), r_observees_v_.data() ); // Set observees FMU values
	}

	// Set Real Observees Derivative Vector at Time t
	void
	set_r_observees_dv( Time const t )
	{
		for ( size_type i = 0u; i < n_r_observees_; ++i ) {
			r_observees_dv_[ i ] = r_observees_[ i ]->x1( t );
		}
	}

	// Set Real Observees Derivative Vector at Time t
	void
	set_r_observees_dv_parallel( Time const t )
	{
		#pragma omp parallel for schedule(static)
		for ( size_type i = 0u; i < n_r_observees_; ++i ) {
			r_observees_dv_[ i ] = r_observees_[ i ]->x1( t );
		}
	}

	// Set Zero-Crossing Observees FMU Values at Time t
	void
	set_zc_observees_values( Time const t )
	{
		for ( size_type i = 0u; i < n_zc_observees_; ++i ) { // Set observee value vector
			zc_observees_v_[ i ] = zc_observees_[ i ]->x( t );
		}
		fmu_me_->set_reals( zc_observees_.size(), zc_observees_v_ref_.data(), zc_observees_v_.data() ); // Set observees FMU values
	}

	// Set Zero-Crossing Observees FMU Values at Time t
	void
	set_zc_observees_values_parallel( Time const t )
	{
		#pragma omp parallel for schedule(static)
		for ( size_type i = 0u; i < n_zc_observees_; ++i ) { // Set observee value vector
			zc_observees_v_[ i ] = zc_observees_[ i ]->x( t );
		}
		fmu_me_->set_reals( zc_observees_.size(), zc_observees_v_ref_.data(), zc_observees_v_.data() ); // Set observees FMU values
	}

	// Set Zero-Crossing Observees Derivative Vector at Time t
	void
	set_zc_observees_dv( Time const t )
	{
		for ( size_type i = 0u; i < n_zc_observees_; ++i ) {
			zc_observees_dv_[ i ] = zc_observees_[ i ]->x1( t );
		}
	}

	// Set Zero-Crossing Observees Derivative Vector at Time t
	void
	set_zc_observees_dv_parallel( Time const t )
	{
		#pragma omp parallel for schedule(static)
		for ( size_type i = 0u; i < n_zc_observees_; ++i ) {
			zc_observees_dv_[ i ] = zc_observees_[ i ]->x1( t );
		}
	}

private: // Data

	FMU_ME * fmu_me_{ nullptr }; // FMU-ME (non-owning) pointer

	Variable * trigger_{ nullptr }; // Trigger variable

	Variables observers_; // Observers
	Variables computational_observers_; // Computational observers

	bool connected_output_observer_{ false }; // Output connection observer to another FMU?

	// Observer order
	int order_{ 0 };

	// Observer index ranges
	Range all_; // All observers
	Range qss_; // QSS state observers
	Range ns_; // Non-state (X-based) observers
	Range r_; // Real observers
	Range ox_; // Other X-based observers
	Range zc_; // Zero-crossing observers

	// Observer FMU pooled call data
	RefsDirDers< Variable > qss_ders_; // QSS derivatives
	RefsDers< Variable > qss_dn2d_; //n2d QSS derivatives
	RefsValsDers< Variable > r_vars_; // Real non-state values and derivatives
	RefsValsDers< Variable > zc_vars_; // Zero-crossing values and derivatives

	// QSS state observers observees
	size_type n_qss_observees_{ 0u }; // Number of QSS observers observees
	Variables qss_observees_; // QSS observers observees
	VariableRefs qss_observees_v_ref_; // QSS observers observees value references
	Reals qss_observees_v_; // QSS handlers observees values
	Reals qss_observees_dv_; // QSS observers observees derivatives

	// Real observers observees
	size_type n_r_observees_{ 0u }; // Number of Real observers observees
	Variables r_observees_; // Real observers observees
	VariableRefs r_observees_v_ref_; // Real observers observees value references
	Reals r_observees_v_; // Real handlers observees values
	Reals r_observees_dv_; // Real observers observees derivatives

	// Zero-crossing observers observees
	size_type n_zc_observees_{ 0u }; // Number of Real observers observees
	Variables zc_observees_; // Zero-crossing observers observees
	VariableRefs zc_observees_v_ref_; // Zero-crossing observers observees value references
	Reals zc_observees_v_; // Zero-crossing handlers observees values
	Reals zc_observees_dv_; // Zero-crossing observers observees derivatives

	// QSS advance method pointer
	void (Observers::*advance_QSS_ptr)( Time const t ){ nullptr };

	// Parallel
#ifdef _OPENMP
	size_type max_threads_{ 0u };
#endif // _OPENMP

}; // Observers

} // QSS

#endif
