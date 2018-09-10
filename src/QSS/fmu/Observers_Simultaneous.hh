// FMU-Based Variable Observers for Simultaneous Events
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

#ifndef QSS_fmu_Observers_Simultaneous_hh_INCLUDED
#define QSS_fmu_Observers_Simultaneous_hh_INCLUDED

// QSS Headers
#include <QSS/container.hh>
#include <QSS/options.hh>
#include <QSS/fmu/FMU.hh>

// C++ Headers
#include <algorithm>
#include <cassert>

namespace QSS {
namespace fmu {

// FMU-Based Variable Observers for Simultaneous Events
template< typename V >
class Observers_Simultaneous final
{

public: // Types

	using Variable = V;
	using Time = typename Variable::Time;
	using Real = typename Variable::Real;
	using Reals = typename Variable::Reals;
	using Variables = typename Variable::Variables;
	using size_type = typename Variables::size_type;

	using const_iterator = typename Variables::const_iterator;
	using iterator = typename Variables::iterator;
	using const_pointer = typename Variables::const_pointer;
	using pointer = typename Variables::pointer;
	using const_reference = typename Variables::const_reference;
	using reference = typename Variables::reference;

public: // Creation

	// Triggers Constructor
	Observers_Simultaneous( Variables & triggers )
	{
		// Collect all observers
		for ( Variable * trigger : triggers ) {
			for ( Variable * observer : trigger->observers() ) {
				observers_.push_back( observer );
			}
		}

		// Remove duplicates and triggers from observers
		if ( ! observers_.empty() ) {

			// Remove duplicate observers
			std::sort( observers_.begin(), observers_.end() );
			observers_.erase( std::unique( observers_.begin(), observers_.end() ), observers_.end() );

			// Remove triggers from observers
			std::sort( triggers.begin(), triggers.end() ); // Side effect!
			auto it( triggers.begin() );
			auto const et( triggers.end() );
			size_type no( observers_.size() ); // Number of observers
			for ( Variable * & observer : observers_ ) {
				while ( ( it != et ) && ( *it < observer ) ) ++it;
				if ( it != et ) {
					if ( *it == observer ) {
						observer = ( Variable * )( std::numeric_limits< std::uintptr_t >::max() );
						--no;
					}
				} else {
					break;
				}
			}
			std::sort( observers_.begin(), observers_.end() ); // Puts elements to remove at the end
			observers_.resize( no );
			// Don't shrink observers: Simultaneous event => short-lived on-the-fly collection
		}

		if ( ! observers_.empty() ) {

			// Sort observers by NZ|ZC and then order
			sort_by_ZC_and_order( observers_ );

			// Observer specs ///

			have_ = true;
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
	}

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

	// Have Observers?
	bool
	have() const
	{
		return have_;
	}

	// Have Order 2+ Observers?
	bool
	have2() const
	{
		return have2_;
	}

	// Have Order 2+ Non-Zero-Crossing Observers?
	bool
	nz_have2() const
	{
		return nz_.have2_;
	}

	// Have Order 2+ Zero-Crossing Observers?
	bool
	zc_have2() const
	{
		return zc_.have2_;
	}

public: // Properties

	// Size
	size_type
	size() const
	{
		return observers_.size();
	}

public: // Methods

	// Advance
	void
	advance( Time const t )
	{
		if ( nz_.have_ ) advance_NZ( t );
		if ( zc_.have_ ) advance_ZC( t );
		if ( options::output::d ) {
			for ( Variable const * observer : observers_ ) {
				observer->advance_observer_d();
			}
		}
	}

	// Advance Non-Zero-Crossing Observers
	void
	advance_NZ( Time const t )
	{
		for ( size_type i = nz_.b_, e = nz_.e_; i < e; ++i ) {
			observers_[ i ]->advance_observer_1( t );
		}
		if ( nz_.max_order_ >= 2 ) { // 2nd order pass
			Time const tN( t + options::dtNum ); // Set time to t + delta for numeric differentiation
			fmu::set_time( tN );
			for ( size_type i = nz_.b2_, e = nz_.e_; i < e; ++i ) {
				observers_[ i ]->advance_observer_2( tN );
			}
			fmu::set_time( t );
		}
	}

	// Advance Zero-Crossing Observers
	void
	advance_ZC( Time const t )
	{
		for ( size_type i = zc_.b_, e = zc_.e_; i < e; ++i ) {
			observers_[ i ]->advance_observer_1( t );
		}
		if ( zc_.max_order_ >= 2 ) { // 2nd order pass
			Time const tN( t + options::dtNum ); // Set time to t + delta for numeric differentiation
			fmu::set_time( tN );
			for ( size_type i = zc_.b2_, e = zc_.e_; i < e; ++i ) {
				observers_[ i ]->advance_observer_2( tN );
			}
			fmu::set_time( t );
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

};

} // fmu
} // QSS

#endif