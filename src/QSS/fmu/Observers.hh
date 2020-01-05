// FMU-Based Variable Observers
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

#ifndef QSS_fmu_Observers_hh_INCLUDED
#define QSS_fmu_Observers_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/FMU_ME.hh>
#include <QSS/container.hh>
#include <QSS/options.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <unordered_set>

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
	Observers( FMU_ME * fmu_me = nullptr ) :
	 fmu_me_( fmu_me )
	{}

public: // Conversion

	// Variables Conversion
	operator Variables const &() const
	{
		return observers_;
	}

	// Variables Conversion
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
		return have_;
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

	// Initialize
	void
	init()
	{
		// Remove duplicates and shrink
		std::sort( observers_.begin(), observers_.end() );
		observers_.erase( std::unique( observers_.begin(), observers_.end() ), observers_.end() ); // Remove duplicates
		observers_.shrink_to_fit();

		// Sort observers by NZ|ZC and then order
		sort_by_ZC_and_order( observers_ );

		// Flag if output connection observer(s)
		connected_output_observer_ = false;
		for ( auto const observer : observers_ ) {
			if ( observer->connected_output ) {
				connected_output_observer_ = true;
				break;
			}
		}

		// Observer specs
		have_ = ( ! observers_.empty() );
		b_ = e_ = n_ = 0;
		nz_.clear();
		zc_.clear();
		if ( have_ ) {
			e_ = n_ = observers_.size();

			if ( observers_[ 0 ]->not_ZC() ) { // Non-zero-crossing observers present
				nz_.e_ = static_cast< size_type >( std::distance( observers_.begin(), std::find_if( observers_.begin(), observers_.end(), []( Variable * v ){ return v->is_ZC(); } ) ) );
				nz_.n_ = ( nz_.b_ < nz_.e_ ? nz_.e_ - nz_.b_ : 0u );
				assert( nz_.n_ > 0u );
				nz_.have_ = true;

				nz_.b2_ = static_cast< size_type >( std::distance( observers_.begin(), std::find_if( observers_.begin(), observers_.end(), []( Variable * v ){ return v->not_ZC() && ( v->order() >= 2 ); } ) ) );
				nz_.n2_ = ( nz_.b2_ < nz_.e_ ? nz_.e_ - nz_.b2_ : 0u );
				nz_.have2_ = ( nz_.n2_ > 0u );

				nz_.b3_ = static_cast< size_type >( std::distance( observers_.begin(), std::find_if( observers_.begin(), observers_.end(), []( Variable * v ){ return v->not_ZC() && ( v->order() >= 3 ); } ) ) );
				nz_.n3_ = ( nz_.b3_ < nz_.e_ ? nz_.e_ - nz_.b3_ : 0u );
				nz_.have3_ = ( nz_.n3_ > 0u );
			}

			if ( observers_.back()->is_ZC() ) { // Zero-crossing observers present
				zc_.b_ = static_cast< size_type >( std::distance( observers_.begin(), std::find_if( observers_.begin(), observers_.end(), []( Variable * v ){ return v->is_ZC(); } ) ) );
				zc_.e_ = e_;
				zc_.n_ = ( zc_.b_ < zc_.e_ ? zc_.e_ - zc_.b_ : 0u );
				assert( zc_.n_ > 0u );
				zc_.have_ = true;

				zc_.b2_ = static_cast< size_type >( std::distance( observers_.begin(), std::find_if( observers_.begin(), observers_.end(), []( Variable * v ){ return v->is_ZC() && ( v->order() >= 2 ); } ) ) );
				zc_.n2_ = ( zc_.b2_ < zc_.e_ ? zc_.e_ - zc_.b2_ : 0u );
				zc_.have2_ = ( zc_.n2_ > 0u );

				zc_.b3_ = static_cast< size_type >( std::distance( observers_.begin(), std::find_if( observers_.begin(), observers_.end(), []( Variable * v ){ return v->is_ZC() && ( v->order() >= 3 ); } ) ) );
				zc_.n3_ = ( zc_.b3_ < zc_.e_ ? zc_.e_ - zc_.b3_ : 0u );
				zc_.have3_ = ( zc_.n3_ > 0u );
			}
		}

		// Non-zero-crossing observer FMU pooled data set up
		nz_der_refs_.clear();
		nz_der_vals_.clear();
		nz_der_refs_.reserve( nz_.n_ );
		nz_der_vals_.reserve( nz_.n_ );
		nz_2_der_refs_.clear();
		nz_2_der_vals_.clear();
		nz_2_der_refs_.reserve( nz_.n2_ );
		nz_2_der_vals_.reserve( nz_.n2_ );
		nz_3_der_refs_.clear();
		nz_3_der_vals_.clear();
		nz_3_der_refs_.reserve( nz_.n3_ );
		nz_3_der_vals_.reserve( nz_.n3_ );
		for ( size_type i = nz_.b_, e = nz_.e_; i < e; ++i ) {
			nz_der_refs_.push_back( observers_[ i ]->der().ref );
			nz_der_vals_.push_back( 0.0 );
		}
		for ( size_type i = nz_.b2_, e = nz_.e_; i < e; ++i ) {
			nz_2_der_refs_.push_back( observers_[ i ]->der().ref );
			nz_2_der_vals_.push_back( 0.0 );
		}
		for ( size_type i = nz_.b3_, e = nz_.e_; i < e; ++i ) {
			nz_3_der_refs_.push_back( observers_[ i ]->der().ref );
			nz_3_der_vals_.push_back( 0.0 );
		}

		// Zero-crossing observer FMU pooled data set up
		zc_der_refs_.clear();
		zc_der_vals_.clear();
		zc_der_refs_.reserve( zc_.n_ );
		zc_der_vals_.reserve( zc_.n_ );
		zc_2_der_refs_.clear();
		zc_2_der_vals_.clear();
		zc_2_der_refs_.reserve( zc_.n2_ );
		zc_2_der_vals_.reserve( zc_.n2_ );
		zc_3_der_refs_.clear();
		zc_3_der_vals_.clear();
		zc_3_der_refs_.reserve( zc_.n3_ );
		zc_3_der_vals_.reserve( zc_.n3_ );
		zc_refs_.clear();
		zc_vals_.clear();
		zc_vals_m_.clear();
		zc_vals_p_.clear();
		zc_refs_.reserve( zc_.n_ );
		zc_vals_.reserve( zc_.n_ );
		zc_vals_m_.reserve( zc_.n_ );
		zc_vals_p_.reserve( zc_.n_ );
		zc_2_refs_.clear();
		zc_2_vals_.clear();
		zc_2_refs_.reserve( zc_.n2_ );
		zc_2_vals_.reserve( zc_.n2_ );
		zc_3_refs_.clear();
		zc_3_vals_.clear();
		zc_3_refs_.reserve( zc_.n3_ );
		zc_3_vals_.reserve( zc_.n3_ );
		for ( size_type i = zc_.b_, e = zc_.e_; i < e; ++i ) {
			zc_der_refs_.push_back( observers_[ i ]->der().ref );
			zc_der_vals_.push_back( 0.0 );
			zc_refs_.push_back( observers_[ i ]->var().ref );
			zc_vals_.push_back( 0.0 );
			zc_vals_m_.push_back( 0.0 );
			zc_vals_p_.push_back( 0.0 );
		}
		for ( size_type i = zc_.b2_, e = zc_.e_; i < e; ++i ) {
			zc_2_der_refs_.push_back( observers_[ i ]->der().ref );
			zc_2_der_vals_.push_back( 0.0 );
			zc_2_refs_.push_back( observers_[ i ]->var().ref );
			zc_2_vals_.push_back( 0.0 );
		}
		for ( size_type i = zc_.b3_, e = zc_.e_; i < e; ++i ) {
			zc_3_der_refs_.push_back( observers_[ i ]->der().ref );
			zc_3_der_vals_.push_back( 0.0 );
			zc_3_refs_.push_back( observers_[ i ]->var().ref );
			zc_3_vals_.push_back( 0.0 );
		}

		// Non-zero-crossing observer observees set up
		std::unordered_set< Variable * > nz_oo3s; // Observees of observers of order 3+
		for ( size_type i = nz_.b3_, e = nz_.e_; i < e; ++i ) {
			auto observer( observers_[ i ] );
			assert( observer->order() >= 3 );
			if ( observer->self_observer() ) nz_oo3s.insert( observer );
			for ( auto observee : observer->observees() ) {
				if ( ! observee->is_Discrete() ) nz_oo3s.insert( observee );
			}
		}
		std::unordered_set< Variable * > nz_oo2s; // Observees of observers of order 2+ not in nz_oo3s
		for ( size_type i = nz_.b2_, e = nz_.b3_; i < e; ++i ) {
			auto observer( observers_[ i ] );
			assert( observer->order() >= 2 );
			if ( ( observer->self_observer() ) && ( nz_oo3s.find( observer ) == nz_oo3s.end() ) ) nz_oo2s.insert( observer );
			for ( auto observee : observer->observees() ) {
				if ( ( ! observee->is_Discrete() ) && ( nz_oo3s.find( observee ) == nz_oo3s.end() ) ) nz_oo2s.insert( observee );
			}
		}
		std::unordered_set< Variable * > nz_oo1s; // Observees of observers of order <=1 not in nz_oo2s
		for ( size_type i = nz_.b_, e = nz_.b2_; i < e; ++i ) {
			auto observer( observers_[ i ] );
			assert( observer->order() <= 1 );
			if ( ( observer->self_observer() ) && ( nz_oo2s.find( observer ) == nz_oo2s.end() ) && ( nz_oo3s.find( observer ) == nz_oo3s.end() ) ) nz_oo1s.insert( observer );
			for ( auto observee : observer->observees() ) {
				if ( ( ! observee->is_Discrete() ) && ( nz_oo2s.find( observee ) == nz_oo2s.end() ) && ( nz_oo3s.find( observee ) == nz_oo3s.end() ) ) nz_oo1s.insert( observee );
			}
		}
		nz_observees_.clear();
		nz_observees_.reserve( nz_oo1s.size() + nz_oo2s.size() + nz_oo3s.size() );
		for ( auto observee : nz_oo1s ) {
			nz_observees_.push_back( observee );
		}
		for ( auto observee : nz_oo2s ) {
			nz_observees_.push_back( observee );
		}
		for ( auto observee : nz_oo3s ) {
			nz_observees_.push_back( observee );
		}
		b2_nz_observees_ = nz_oo1s.size();
		b3_nz_observees_ = nz_oo1s.size() + nz_oo2s.size();

		// Zero-crossing observer observees set up
		std::unordered_set< Variable * > zc_oo3s; // Observees of observers of order 3+
		for ( size_type i = zc_.b3_, e = zc_.e_; i < e; ++i ) {
			auto observer( observers_[ i ] );
			assert( observer->order() >= 3 );
			if ( observer->self_observer() ) zc_oo3s.insert( observer );
			for ( auto observee : observer->observees() ) {
				if ( ! observee->is_Discrete() ) zc_oo3s.insert( observee );
			}
		}
		std::unordered_set< Variable * > zc_oo2s; // Observees of observers of order 2+ not in zc_oo3s
		for ( size_type i = zc_.b2_, e = zc_.b3_; i < e; ++i ) {
			auto observer( observers_[ i ] );
			assert( observer->order() >= 2 );
			if ( ( observer->self_observer() ) && ( zc_oo3s.find( observer ) == zc_oo3s.end() ) ) zc_oo2s.insert( observer );
			for ( auto observee : observer->observees() ) {
				if ( ( ! observee->is_Discrete() ) && ( zc_oo3s.find( observee ) == zc_oo3s.end() ) ) zc_oo2s.insert( observee );
			}
		}
		std::unordered_set< Variable * > zc_oo1s; // Observees of observers of order <=1 not in zc_oo2s
		for ( size_type i = zc_.b_, e = zc_.b2_; i < e; ++i ) {
			auto observer( observers_[ i ] );
			assert( observer->order() <= 1 );
			if ( ( observer->self_observer() ) && ( zc_oo2s.find( observer ) == zc_oo2s.end() ) && ( zc_oo3s.find( observer ) == zc_oo3s.end() ) ) zc_oo1s.insert( observer );
			for ( auto observee : observer->observees() ) {
				if ( ( ! observee->is_Discrete() ) && ( zc_oo2s.find( observee ) == zc_oo2s.end() ) && ( zc_oo3s.find( observee ) == zc_oo3s.end() ) ) zc_oo1s.insert( observee );
			}
		}
		zc_observees_.clear();
		zc_observees_.reserve( zc_oo1s.size() + zc_oo2s.size() + zc_oo3s.size() );
		for ( auto observee : zc_oo1s ) {
			zc_observees_.push_back( observee );
		}
		for ( auto observee : zc_oo2s ) {
			zc_observees_.push_back( observee );
		}
		for ( auto observee : zc_oo3s ) {
			zc_observees_.push_back( observee );
		}
		b2_zc_observees_ = zc_oo1s.size();
		b3_zc_observees_ = zc_oo1s.size() + zc_oo2s.size();
	}

	// Advance
	void
	advance( Time const t )
	{
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );
		if ( nz_.have_ ) advance_NZ( t );
		if ( zc_.have_ ) advance_ZC( t );
		if ( options::output::d ) advance_d();
	}

	// Advance Non-Zero-Crossing Observers
	void
	advance_NZ( Time const t )
	{
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );
		for ( auto observee : nz_observees_ ) {
			observee->fmu_set_q( t );
		}
		fmu_me_->get_reals( nz_der_refs_.size(), &nz_der_refs_[ 0 ], &nz_der_vals_[ 0 ] );
		for ( size_type i = nz_.b_, e = nz_.e_; i < e; ++i ) {
			observers_[ i ]->advance_observer_1( t, nz_der_vals_[ i ] );
		}
		if ( nz_.have2_ ) {
			Time tN( t + options::dtNum );
			fmu_me_->set_time( tN );
			for ( size_type i = b2_nz_observees_, n = nz_observees_.size(); i < n; ++i ) {
				nz_observees_[ i ]->fmu_set_q( tN );
			}
			fmu_me_->get_reals( nz_2_der_refs_.size(), &nz_2_der_refs_[ 0 ], &nz_2_der_vals_[ 0 ] );
			for ( size_type i = nz_.b2_, j = 0, e = nz_.e_; i < e; ++i, ++j ) { // Order 2+ observers
				observers_[ i ]->advance_observer_2( nz_2_der_vals_[ j ] );
			}
			if ( nz_.have3_ ) {
				tN = t - options::dtNum;
				fmu_me_->set_time( tN );
				for ( size_type i = b3_nz_observees_, n = nz_observees_.size(); i < n; ++i ) {
					nz_observees_[ i ]->fmu_set_q( tN );
				}
				fmu_me_->get_reals( nz_3_der_refs_.size(), &nz_3_der_refs_[ 0 ], &nz_3_der_vals_[ 0 ] );
				for ( size_type i = nz_.b3_, j = 0, e = nz_.e_; i < e; ++i, ++j ) { // Order 3+ observers
					observers_[ i ]->advance_observer_3( nz_3_der_vals_[ j ] );
				}
			}
			fmu_me_->set_time( t );
		}
	}

	// Advance Zero-Crossing Observers
	void
	advance_ZC( Time const t )
	{
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );
		for ( auto observee : zc_observees_ ) {
			observee->fmu_set_x( t );
		}
		fmu_me_->get_reals( zc_refs_.size(), &zc_refs_[ 0 ], &zc_vals_[ 0 ] );
		if ( fmu_me_->has_event_indicators ) { // Event indicators
			Time tN( t - options::dtNum );
			fmu_me_->set_time( tN );
			for ( auto observee : zc_observees_ ) {
				observee->fmu_set_x( tN );
			}
			fmu_me_->get_reals( zc_refs_.size(), &zc_refs_[ 0 ], &zc_vals_m_[ 0 ] );
			tN = t + options::dtNum;
			fmu_me_->set_time( tN );
			for ( auto observee : zc_observees_ ) {
				observee->fmu_set_x( tN );
			}
			fmu_me_->get_reals( zc_refs_.size(), &zc_refs_[ 0 ], &zc_vals_p_[ 0 ] );
			for ( size_type i = zc_.b_, j = 0, e = zc_.e_; i < e; ++i, ++j ) {
				observers_[ i ]->advance_observer_1( t, zc_vals_[ j ], zc_vals_m_[ j ], zc_vals_p_[ j ] );
			}
			if ( zc_.have2_ ) {
				for ( size_type i = zc_.b2_, j = 0, e = zc_.e_; i < e; ++i, ++j ) { // Order 2+ observers
					observers_[ i ]->advance_observer_2();
				}
				if ( zc_.have3_ ) {
					tN = t + ( two * options::dtNum );
					fmu_me_->set_time( tN );
					for ( size_type i = b3_zc_observees_, n = zc_observees_.size(); i < n; ++i ) {
						zc_observees_[ i ]->fmu_set_x( tN );
					}
					fmu_me_->get_reals( zc_3_refs_.size(), &zc_3_refs_[ 0 ], &zc_3_vals_[ 0 ] );
					for ( size_type i = zc_.b3_, j = 0, e = zc_.e_; i < e; ++i, ++j ) { // Order 3+ observers
						observers_[ i ]->advance_observer_3( zc_3_vals_[ j ] );
					}
				}
			}
			fmu_me_->set_time( t );
		} else { // Explicit zero-crossing variables
			assert( fmu_me_->has_explicit_ZCs );
			fmu_me_->get_reals( zc_der_refs_.size(), &zc_der_refs_[ 0 ], &zc_der_vals_[ 0 ] );
			for ( size_type i = zc_.b_, j = 0, e = zc_.e_; i < e; ++i, ++j ) {
				observers_[ i ]->advance_observer_1( t, zc_der_vals_[ j ], zc_vals_[ j ] );
			}
			if ( zc_.have2_ ) {
				Time tN( t + options::dtNum );
				fmu_me_->set_time( tN );
				for ( size_type i = b2_zc_observees_, n = zc_observees_.size(); i < n; ++i ) {
					zc_observees_[ i ]->fmu_set_x( tN );
				}
				fmu_me_->get_reals( zc_2_der_refs_.size(), &zc_2_der_refs_[ 0 ], &zc_2_der_vals_[ 0 ] );
				for ( size_type i = zc_.b2_, j = 0, e = zc_.e_; i < e; ++i, ++j ) { // Order 2+ observers
					observers_[ i ]->advance_observer_2( zc_2_der_vals_[ j ] );
				}
				if ( zc_.have3_ ) {
					tN = t - options::dtNum;
					fmu_me_->set_time( tN );
					for ( size_type i = b3_zc_observees_, n = zc_observees_.size(); i < n; ++i ) {
						zc_observees_[ i ]->fmu_set_x( tN );
					}
					fmu_me_->get_reals( zc_3_der_refs_.size(), &zc_3_der_refs_[ 0 ], &zc_3_der_vals_[ 0 ] );
					for ( size_type i = zc_.b3_, j = 0, e = zc_.e_; i < e; ++i, ++j ) { // Order 3+ observers
						observers_[ i ]->advance_observer_3( zc_3_der_vals_[ j ] );
					}
				}
				fmu_me_->set_time( t );
			}
		}
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

public: // Indexing

	// Index-Based Lookup
	Variable const *
	operator []( size_type const i ) const
	{
		assert( i < observers_.size() );
		return observers_[ i ];
	}

	// Index-Based Lookup
	Variable *
	operator []( size_type const i )
	{
		assert( i < observers_.size() );
		return observers_[ i ];
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

private: // Data

	FMU_ME * fmu_me_{ nullptr }; // FMU-ME (non-owning) pointer

	Variables observers_; // Observers of a variable

	bool have_{ false }; // Observers present?
	size_type b_{ 0u }; // Begin index
	size_type e_{ 0u }; // End index
	size_type n_{ 0u }; // Count

	struct {

		bool have_{ false }; // Observers of this type present?
		bool have2_{ false }; // Order 2+ observers of this type present?
		bool have3_{ false }; // Order 3+ observers of this type present?
		size_type b_{ 0u }; // Begin index
		size_type b2_{ 0u }; // Order 2+ begin index
		size_type b3_{ 0u }; // Order 3+ begin index
		size_type e_{ 0u }; // End index
		size_type n_{ 0u }; // Count
		size_type n2_{ 0u }; // Order 2+ count
		size_type n3_{ 0u }; // Order 3+ count

		void
		clear()
		{
			have_ = have2_ = have3_ = false;
			b_ = b2_ = b3_ = e_ = 0u;
			n_ = n2_ = n3_ = 0u;
		}

	} nz_, zc_;

	// FMU non-zero-crossing observer data
	VariableRefs nz_der_refs_; // Non-zero-crossing observer FMU derivative refs
	VariableRefs nz_2_der_refs_; // Non-zero-crossing observer FMU derivative refs
	VariableRefs nz_3_der_refs_; // Non-zero-crossing observer FMU derivative refs
	Reals nz_der_vals_; // Non-zero-crossing observer FMU derivative values
	Reals nz_2_der_vals_; // Non-zero-crossing observer FMU derivative values
	Reals nz_3_der_vals_; // Non-zero-crossing observer FMU derivative values

	// FMU zero-crossing observer data
	VariableRefs zc_der_refs_; // Zero-crossing observer FMU derivative refs
	VariableRefs zc_2_der_refs_; // Zero-crossing observer FMU derivative refs
	VariableRefs zc_3_der_refs_; // Zero-crossing observer FMU derivative refs
	Reals zc_der_vals_; // Zero-crossing observer FMU derivative values
	Reals zc_2_der_vals_; // Zero-crossing observer FMU derivative values
	Reals zc_3_der_vals_; // Zero-crossing observer FMU derivative values
	VariableRefs zc_refs_; // Zero-crossing observer FMU variable refs
	VariableRefs zc_2_refs_; // Zero-crossing observer FMU variable refs
	VariableRefs zc_3_refs_; // Zero-crossing observer FMU variable refs
	Reals zc_vals_; //  Zero-crossing observer FMU variable values
	Reals zc_vals_m_; //  Zero-crossing observer FMU variable values
	Reals zc_vals_p_; //  Zero-crossing observer FMU variable values
	Reals zc_2_vals_; //  Zero-crossing observer FMU variable values
	Reals zc_3_vals_; //  Zero-crossing observer FMU variable values

	// Non-zero-crossing observer observees
	Variables nz_observees_; // Non-zero-crossing observer observees (including self-observing observers)
	size_type b2_nz_observees_{ 0u }; // Begin index of observees of order 2+ non-zero-crossing observers
	size_type b3_nz_observees_{ 0u }; // Begin index of observees of order 3+ non-zero-crossing observers

	// Zero-crossing observer observees
	Variables zc_observees_; // Zero-crossing observer observees (including self-observing observers)
	size_type b2_zc_observees_{ 0u }; // Begin index of observees of order 2+ zero-crossing observers
	size_type b3_zc_observees_{ 0u }; // Begin index of observees of order 3+ zero-crossing observers

	bool connected_output_observer_{ false }; // Output connection observer to another FMU?

}; // Observers

} // fmu
} // QSS

#endif
