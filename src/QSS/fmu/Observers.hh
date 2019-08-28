// FMU-Based Variable Observers
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2019 Objexx Engineering, Inc. All rights reserved.
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

public: // Predicates

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

	// Have Order 2+ Observer(s)?
	bool
	have2() const
	{
		return have2_;
	}

	// Have Order 2+ Non-Zero-Crossing Observer(s)?
	bool
	nz_have2() const
	{
		return nz_.have2_;
	}

	// Have Order 2+ Zero-Crossing Observer(s)?
	bool
	zc_have2() const
	{
		return zc_.have2_;
	}

	// Have Connected Output Observer(s)?
	bool
	connected_output_observer() const
	{
		return connected_output_observer_;
	}

public: // Properties

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
		have2_ = false;
		b_ = e_ = n_ = max_order_ = 0;
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

				nz_.max_order_ = ( nz_.b2_ < nz_.e_ ? 2 : 1 );
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

				zc_.max_order_ = ( zc_.b2_ < zc_.e_ ? 2 : 1 );
			}

			have2_ = ( nz_.have2_ || zc_.have2_ );
			max_order_ = std::max( nz_.max_order_, zc_.max_order_ );
		}

		// Non-zero-crossing observer observees set up
		std::unordered_set< Variable * > nz_oo2s; // Observees of observers of order 2+
		for ( size_type i = nz_.b2_, e = nz_.e_; i < e; ++i ) {
			auto observer( observers_[ i ] );
			assert( observer->order() >= 2 );
			if ( observer->self_observer ) nz_oo2s.insert( observer );
			for ( auto observee : observer->observees() ) {
				if ( ! observee->is_Discrete() ) nz_oo2s.insert( observee );
			}
		}
		std::unordered_set< Variable * > nz_oo1s; // Observees of observers of order <=1 not in nz_oo2s
		for ( size_type i = nz_.b_, e = nz_.b2_; i < e; ++i ) {
			auto observer( observers_[ i ] );
			assert( observer->order() <= 1 );
			if ( ( observer->self_observer ) && ( nz_oo2s.find( observer ) == nz_oo2s.end() ) ) nz_oo1s.insert( observer );
			for ( auto observee : observer->observees() ) {
				if ( ( ! observee->is_Discrete() ) && ( nz_oo2s.find( observee ) == nz_oo2s.end() ) ) nz_oo1s.insert( observee );
			}
		}
		nz_observees_.clear();
		nz_observees_.reserve( nz_oo1s.size() + nz_oo2s.size() );
		for ( auto observee : nz_oo1s ) {
			nz_observees_.push_back( observee );
		}
		for ( auto observee : nz_oo2s ) {
			nz_observees_.push_back( observee );
		}
		b2_nz_observees_ = nz_oo1s.size();

		// Zero-crossing observer observees set up
		std::unordered_set< Variable * > zc_oo2s; // Observees of observers of order 2+
		for ( size_type i = zc_.b2_, e = zc_.e_; i < e; ++i ) {
			auto observer( observers_[ i ] );
			assert( observer->order() >= 2 );
			if ( observer->self_observer ) zc_oo2s.insert( observer );
			for ( auto observee : observer->observees() ) {
				if ( ! observee->is_Discrete() ) zc_oo2s.insert( observee );
			}
		}
		std::unordered_set< Variable * > zc_oo1s; // Observees of observers of order <=1 not in zc_oo2s
		for ( size_type i = zc_.b_, e = zc_.b2_; i < e; ++i ) {
			auto observer( observers_[ i ] );
			assert( observer->order() <= 1 );
			if ( ( observer->self_observer ) && ( zc_oo2s.find( observer ) == zc_oo2s.end() ) ) zc_oo1s.insert( observer );
			for ( auto observee : observer->observees() ) {
				if ( ( ! observee->is_Discrete() ) && ( zc_oo2s.find( observee ) == zc_oo2s.end() ) ) zc_oo1s.insert( observee );
			}
		}
		zc_observees_.clear();
		zc_observees_.reserve( zc_oo1s.size() + zc_oo2s.size() );
		for ( auto observee : zc_oo1s ) {
			zc_observees_.push_back( observee );
		}
		for ( auto observee : zc_oo2s ) {
			zc_observees_.push_back( observee );
		}
		b2_zc_observees_ = zc_oo1s.size();
	}

	// Advance
	void
	advance( Time const t )
	{
		if ( nz_.have_ ) advance_NZ( t );
		if ( zc_.have_ ) {
			advance_ZC_1( t );
			if ( zc_.have2_ ) advance_ZC_2( t );
		}
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
		for ( size_type i = nz_.b_, e = nz_.e_; i < e; ++i ) {
			observers_[ i ]->advance_observer( t );
		}
	}

	// Advance Zero-Crossing Observers: Stage 1
	void
	advance_ZC_1( Time const t )
	{
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );
		for ( auto observee : zc_observees_ ) {
			observee->fmu_set_x( t );
		}
		for ( size_type i = zc_.b_, e = zc_.e_; i < e; ++i ) {
			observers_[ i ]->advance_observer_1( t );
		}
	}

	// Advance Zero-Crossing Observers: Stage 2
	void
	advance_ZC_2( Time const t )
	{
		assert( fmu_me_ != nullptr );
		Time const tN( t + options::dtNum ); // Set time to t + delta for numeric differentiation
		fmu_me_->set_time( tN );
		for ( size_type i = b2_zc_observees_, n = zc_observees_.size(); i < n; ++i ) {
			zc_observees_[ i ]->fmu_set_x( tN );
		}
		for ( size_type i = zc_.b2_, j = 0, e = zc_.e_; i < e; ++i, ++j ) { // Order 2+ observers
			observers_[ i ]->advance_observer_2( tN );
		}
		fmu_me_->set_time( t );
	}

	// Advance: Stage d
	void
	advance_d()
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

public: // Iterators

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
	bool have2_{ false }; // Order 2+ observers present?
	size_type b_{ 0 }; // Begin index
	size_type e_{ 0 }; // End index
	size_type n_{ 0 }; // Count
	int max_order_{ 0 }; // Max order

	struct {

		bool have_{ false }; // Observers of this type present?
		bool have2_{ false }; // Order 2+ observers of this type present?
		size_type b_{ 0 }; // Begin index
		size_type b2_{ 0 }; // Order 2+ begin index
		size_type e_{ 0 }; // End index
		size_type n_{ 0 }; // Count
		size_type n2_{ 0 }; // Order 2+ count
		int max_order_{ 0 }; // Max order

		void
		clear()
		{
			have_ = have2_ = false;
			b_ = b2_ = e_ = 0;
			n_ = n2_ = 0;
			max_order_ = 0;
		}

	} nz_, zc_;

	// FMU observer data
	bool connected_output_observer_{ false }; // Output connection observer to another FMU?

	// Non-zero-crossing observer observees
	Variables nz_observees_; // Non-zero-crossing observer observees (including self-observing observers)
	size_type b2_nz_observees_{ 0 }; // Begin index of observees of order 2+ non-zero-crossing observers

	// Zero-crossing observer observees
	Variables zc_observees_; // Zero-crossing observer observees (including self-observing observers)
	size_type b2_zc_observees_{ 0 }; // Begin index of observees of order 2+ zero-crossing observers

};

} // fmu
} // QSS

#endif
