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

#ifndef QSS_Triggers_hh_INCLUDED
#define QSS_Triggers_hh_INCLUDED

// QSS Headers
#include <QSS/FMU_ME.hh>
#include <QSS/RefsDers.hh>
#include <QSS/container.hh>
#include <QSS/math.hh>
#include <QSS/options.hh>
#include <QSS/Range.hh>
#include <QSS/SuperdenseTime.hh>

// C++ Headers
#include <algorithm>
#include <cassert>

namespace QSS {

// QSS Variable Triggers
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
		qss_ders_.clear(); qss_ders_.reserve( range_.n() );
		for ( Variable * trigger : triggers_ ) {
			assert( trigger->is_QSS() );
			qss_ders_.push_back( trigger->der().ref() );
		}

		// Observees set up
		observees_.clear();
		for ( Variable * trigger : triggers_ ) {
			for ( auto observee : trigger->observees() ) {
				observees_.push_back( observee );
			}
		}
		uniquify( observees_ );
		if ( !uni_order_ ) {
			assert( range2_.have() );
			observees2_.clear();
			for ( size_type i = range2_.b(), e = range_.e(); i < e; ++i ) { // Order 2+ triggers
				Variable * trigger( triggers_[ i ] );
				for ( auto observee : trigger->observees() ) {
					observees2_.push_back( observee );
				}
			}
			uniquify( observees2_ );
			if ( range3_.have() ) {
				observees3_.clear();
				for ( size_type i = range3_.b(), e = range_.e(); i < e; ++i ) { // Order 3+ triggers
					Variable * trigger( triggers_[ i ] );
					for ( auto observee : trigger->observees() ) {
						observees3_.push_back( observee );
					}
				}
				uniquify( observees3_ );
			}
		}
	}

	// QSS Advance Triggers
	void
	advance_QSS( Time const t, SuperdenseTime const & s )
	{
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );
		assert( range_.n() == qss_ders_.size() );

		for ( Variable * trigger : triggers_ ) {
			assert( trigger->is_QSS() ); // QSS triggers
			assert( trigger->tE >= t ); // Bin variables tE can be > t
			trigger->tE = t; // Bin variables tE can be > t
			trigger->st = s; // Set trigger superdense time
			trigger->advance_QSS_0();
		}
		for ( Variable * observee : observees_ ) {
			observee->fmu_set_s( t );
		}
		fmu_me_->get_reals( range_.n(), &qss_ders_.refs[ 0 ], &qss_ders_.ders[ 0 ] );
		for ( size_type i = range_.b(), e = range_.e(); i < e; ++i ) {
			triggers_[ i ]->advance_QSS_1( qss_ders_.ders[ i ] );
		}
		if ( range3_.have() ) {
			Time tN( t - options::dtND );
			if ( fwd_time( tN ) ) { // Use centered ND formulas
				fmu_me_->set_time( tN );
				for ( Variable * observee : uni_order_ ? observees_ : observees2_ ) {
					observee->fmu_set_s( tN );
				}
				size_type const qss2_b( range2_.b() );
				fmu_me_->get_reals( range2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.ders_m[ qss2_b ] );
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : uni_order_ ? observees_ : observees2_ ) {
					observee->fmu_set_s( tN );
				}
				fmu_me_->get_reals( range2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.ders_p[ qss2_b ] );
				for ( size_type i = qss2_b, e = range_.e(); i < e; ++i ) { // Order 2+ triggers
					triggers_[ i ]->advance_QSS_2( qss_ders_.ders_m[ i ], qss_ders_.ders_p[ i ] );
				}
				for ( size_type i = range3_.b(), e = range_.e(); i < e; ++i ) { // Order 3+ triggers
					triggers_[ i ]->advance_QSS_3();
				}
			} else { // Use forward ND formulas
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : uni_order_ ? observees_ : observees2_ ) {
					observee->fmu_set_s( tN );
				}
				size_type const qss2_b( range2_.b() );
				fmu_me_->get_reals( range2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.ders_m[ qss2_b ] );
				tN = t + options::two_dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : uni_order_ ? observees_ : observees2_ ) {
					observee->fmu_set_s( tN );
				}
				fmu_me_->get_reals( range2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.ders_p[ qss2_b ] );
				for ( size_type i = qss2_b, e = range_.e(); i < e; ++i ) { // Order 2+ triggers
					triggers_[ i ]->advance_QSS_2_forward( qss_ders_.ders_m[ i ], qss_ders_.ders_p[ i ] );
				}
				for ( size_type i = range3_.b(), e = range_.e(); i < e; ++i ) { // Order 3+ triggers
					triggers_[ i ]->advance_QSS_3_forward();
				}
			}
			fmu_me_->set_time( t );
		} else if ( range2_.have() ) {
			Time const tN( t + options::dtND );
			fmu_me_->set_time( tN );
			for ( Variable * observee : uni_order_ ? observees_ : observees2_ ) {
				observee->fmu_set_s( tN );
			}
			size_type const qss2_b( range2_.b() );
			fmu_me_->get_reals( range2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.ders_p[ qss2_b ] );
			for ( size_type i = qss2_b, e = range_.e(); i < e; ++i ) { // Order 2+ triggers
				triggers_[ i ]->advance_QSS_2( qss_ders_.ders_p[ i ] );
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
		range_.reset();
		range2_.reset();
		range3_.reset();
	}

	// Set Specs
	void
	set_specs()
	{
		reset_specs();
		if ( triggers_.empty() ) return;

		range_.b() = 0u;
		range_.e() = range2_.b() = range2_.e() = range3_.b() = range3_.e() = triggers_.size();
		for ( size_type i = 0, e = range_.e(); i < e; ++i ) {
			int const order( triggers_[ i ]->order() );
			if ( order >= 2 ) {
				range2_.b() = std::min( range2_.b(), i );
				if ( order >= 3 ) {
					range3_.b() = std::min( range3_.b(), i );
					break;
				}
			}
		}
		size_type const range_n( range_.n() );
		uni_order_ = (
		 ( range2_.empty() || range2_.n() == range_n ) &&
		 ( range3_.empty() || range3_.n() == range_n )
		);
	}

private: // Data

	FMU_ME * fmu_me_{ nullptr }; // FMU-ME (non-owning) pointer

	Variables triggers_; // Triggers

	// Trigger index specs
	bool uni_order_{ false }; // Triggers all the same order?
	Range range_; // Triggers range
	Range range2_; // Triggers of order 2+ range
	Range range3_; // Triggers of order 3+ range

	// Observees
	Variables observees_; // Triggers observees
	Variables observees2_; // Triggers of order 2+ observees
	Variables observees3_; // Triggers of order 3+ observees

	// Trigger FMU pooled call data
	RefsDers< Variable > qss_ders_; // Derivatives

}; // Triggers

} // QSS

#endif
