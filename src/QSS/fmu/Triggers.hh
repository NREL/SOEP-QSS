// FMU-Based Variable Triggers
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

#ifndef QSS_fmu_Triggers_hh_INCLUDED
#define QSS_fmu_Triggers_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/FMU_ME.hh>
#include <QSS/container.hh>
#include <QSS/math.hh>
#include <QSS/options.hh>
#include <QSS/Range.hh>
#include <QSS/RefsVals.hh>
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

// FMU-Based Variable Triggers
template< typename V >
class Triggers final
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
	Triggers( FMU_ME * fmu_me = nullptr ) :
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

	// Triggers Collection
	Variables const &
	triggers() const
	{
		return triggers_;
	}

	// Triggers Collection
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
		qss_ders_.clear(); qss_ders_.reserve( qss_.n());
		for ( Variable * trigger : triggers_ ) {
			assert( trigger->is_QSS() );
			qss_ders_.push_back( trigger->der().ref );
		}

		// Observees set up
		qss_observees_.clear();
		for ( Variable * trigger : triggers_ ) {
			if ( trigger->self_observer() ) qss_observees_.push_back( trigger );
			for ( auto observee : trigger->observees() ) {
				if ( ! observee->is_Discrete() ) qss_observees_.push_back( observee );
			}
		}
		uniquify( qss_observees_ );
		if ( ! qss_same_order_ ) {
			assert( qss2_.have() );
			qss2_observees_.clear();
			for ( size_type i = qss2_.b(), e = qss_.e(); i < e; ++i ) { // Order 2+ triggers
				Variable * trigger( triggers_[ i ] );
				if ( trigger->self_observer() ) qss2_observees_.push_back( trigger );
				for ( auto observee : trigger->observees() ) {
					if ( ! observee->is_Discrete() ) qss2_observees_.push_back( observee );
				}
			}
			uniquify( qss2_observees_ );
			if ( qss3_.have() ) {
				qss3_observees_.clear();
				for ( size_type i = qss3_.b(), e = qss_.e(); i < e; ++i ) { // Order 3+ triggers
					Variable * trigger( triggers_[ i ] );
					if ( trigger->self_observer() ) qss3_observees_.push_back( trigger );
					for ( auto observee : trigger->observees() ) {
						if ( ! observee->is_Discrete() ) qss3_observees_.push_back( observee );
					}
				}
				uniquify( qss3_observees_ );
			}
		}
	}

	// QSS Advance
	void
	advance_QSS( Time const t, SuperdenseTime const & s )
	{
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );

		for ( Variable * trigger : triggers_ ) {
			assert( trigger->is_QSS() ); // QSS triggers
			assert( trigger->tE >= t ); // Bin variables tE can be > t
			trigger->tE = t; // Bin variables tE can be > t
			trigger->st = s; // Set trigger superdense time
			trigger->advance_QSS_0();
		}
		for ( Variable * observee : qss_observees_ ) {
			observee->fmu_set_q( t );
		}
		fmu_me_->get_reals( qss_.n(), &qss_ders_.refs[ 0 ], &qss_ders_.vals[ 0 ] );
#ifdef _OPENMP
		size_type const max_threads( static_cast< size_type >( omp_get_max_threads() ) );
		if ( ( max_threads > 1u ) && ( qss_.n() >= max_threads * 128u ) ) {
			std::int64_t const qss_b( qss_.b() );
			std::int64_t const qss_e( qss_.e() );
			std::int64_t const qss_n( qss_.n() );
			std::int64_t const qss_chunk_size( static_cast< std::int64_t >( ( qss_n + max_threads - 1u ) / max_threads ) );
			#pragma omp parallel for schedule(static)
			for ( std::int64_t i = qss_b; i < qss_e; i += qss_chunk_size ) {
				for ( std::int64_t k = i, ke = std::min( i + qss_chunk_size, qss_e ); k < ke; ++k ) { // Chunk
					triggers_[ k ]->advance_QSS_1( qss_ders_.vals[ k ] );
				}
			}
		} else {
#endif
		for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) {
			triggers_[ i ]->advance_QSS_1( qss_ders_.vals[ i ] );
		}
#ifdef _OPENMP
		}
#endif
		if ( qss2_.have() ) {
			Time tN( t + options::dtND );
			fmu_me_->set_time( tN );
			for ( Variable * observee : qss_same_order_ ? qss_observees_ : qss2_observees_ ) {
				observee->fmu_set_q( tN );
			}
			size_type const qss2_b( qss2_.b() );
			fmu_me_->get_reals( qss2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.vals[ qss2_b ] );
			for ( size_type i = qss2_b, e = qss_.e(); i < e; ++i ) { // Order 2+ triggers
				triggers_[ i ]->advance_QSS_2( qss_ders_.vals[ i ] );
			}
			for ( size_type i = qss2_b, e = qss_.e(); i < e; ++i ) { // Order 2+ triggers
				triggers_[ i ]->advance_QSS_2_1();
			}
			if ( qss3_.have() ) {
				tN = t - options::dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : qss_same_order_ ? qss_observees_ : qss3_observees_ ) {
					observee->fmu_set_q( tN );
				}
				size_type const qss3_b( qss3_.b() );
				fmu_me_->get_reals( qss3_.n(), &qss_ders_.refs[ qss3_b ], &qss_ders_.vals[ qss3_b ] );
				for ( size_type i = qss3_b, e = qss_.e(); i < e; ++i ) { // Order 3+ triggers
					triggers_[ i ]->advance_QSS_3( qss_ders_.vals[ i ] );
				}
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

	// Triggers[ i ]
	Variable const *
	operator []( size_type const i ) const
	{
		return triggers_[ i ];
	}

	// Triggers[ i ]
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
		qss_.e() = triggers_.size();
		size_type i( 0u );
		while ( i < qss_.e() ) {
			Variable const * trigger( triggers_[ i ] );
			int const order( trigger->order() );
			if ( order >= 2 ) {
				qss2_.b() = std::min( qss2_.b(), i );
				if ( order >= 3 ) {
					qss3_.b() = std::min( qss3_.b(), i );
				}
			}
			++i;
		}
		if ( qss2_.began() ) {
			qss2_.e() = i;
			if ( qss3_.began() ) {
				qss3_.e() = i;
			}
		}
		size_type const qss_n( qss_.n() );
		qss_same_order_ = (
		 ( qss2_.empty() || qss2_.n() == qss_n ) &&
		 ( qss3_.empty() || qss3_.n() == qss_n )
		);
	}

private: // Data

	FMU_ME * fmu_me_{ nullptr }; // FMU-ME (non-owning) pointer

	Variables triggers_; // Triggers

	Range qss_; // Triggers
	Range qss2_; // Triggers of order 2+
	Range qss3_; // Triggers of order 3+

	bool qss_same_order_{ false }; // QSS triggers all the same order?

	RefsVals< Variable > qss_ders_; // Trigger derivative FMU pooled call data

	// Observees (including self-observers)
	Variables qss_observees_; // Triggers observees
	Variables qss2_observees_; // Triggers of order 2+ observees
	Variables qss3_observees_; // Triggers of order 3+ observees

}; // Triggers

} // fmu
} // QSS

#endif