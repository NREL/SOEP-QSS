// FMU-Based Variable Handlers
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2022 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_fmu_Handlers_hh_INCLUDED
#define QSS_fmu_Handlers_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/FMU_ME.hh>
#include <QSS/fmu/RefsDers.hh>
#include <QSS/container.hh>
#include <QSS/math.hh>
#include <QSS/options.hh>
#include <QSS/Range.hh>

// C++ Headers
#include <algorithm>
#include <cassert>

namespace QSS {
namespace fmu {

// FMU-Based Variable Handlers
template< typename V >
class Handlers final
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
	Handlers( FMU_ME * fmu_me = nullptr ) :
	 fmu_me_( fmu_me )
	{}

public: // Conversion

	// Handlers Conversion
	operator Variables const &() const
	{
		return handlers_;
	}

	// Handlers Conversion
	operator Variables &()
	{
		return handlers_;
	}

public: // Predicate

	// Empty?
	bool
	empty() const
	{
		return handlers_.empty();
	}

	// Have Observer(s)?
	bool
	have() const
	{
		return ( !handlers_.empty() );
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
		return handlers_.size();
	}

	// Handlers
	Variables const &
	handlers() const
	{
		return handlers_;
	}

	// Handlers
	Variables &
	handlers()
	{
		return handlers_;
	}

public: // Methods

	// Assign a Handlers Collection
	void
	assign( Variables & handlers )
	{
		handlers_ = handlers;

		if ( handlers_.empty() ) {
			reset_specs();
			return;
		}

		// Sort by type and order
		assert( is_unique( handlers_ ) ); // Precondition: No duplicates
		sort_by_order( handlers_ );

		// Set specs
		set_specs();

		// FMU pooled data set up
		hnd_ders_.clear(); hnd_ders_.reserve( hnd_.n() );
		for ( Variable * handler : handlers_ ) {
			assert( handler->not_Input() ); // Check valid handler types
			assert( handler->not_ZC() ); // Check valid handler types
			hnd_ders_.push_back( handler->der().ref );
		}

		// Observees set up
		hnd_observees_.clear();
		for ( Variable * handler : handlers_ ) {
			if ( handler->self_observer() ) hnd_observees_.push_back( handler );
			for ( auto observee : handler->observees() ) {
				if ( !observee->is_Discrete() ) hnd_observees_.push_back( observee );
			}
		}
		uniquify( hnd_observees_ );
		if ( !uni_order_ ) {
			assert( hnd1_.have() );
			hnd1_observees_.clear();
			for ( size_type i = hnd1_.b(), e = hnd_.e(); i < e; ++i ) { // Order 1+ handlers
				Variable * handler( handlers_[ i ] );
				if ( handler->self_observer() ) hnd1_observees_.push_back( handler );
				for ( auto observee : handler->observees() ) {
					if ( !observee->is_Discrete() ) hnd1_observees_.push_back( observee );
				}
			}
			uniquify( hnd1_observees_ );
			if ( hnd2_.have() ) {
				hnd2_observees_.clear();
				for ( size_type i = hnd2_.b(), e = hnd_.e(); i < e; ++i ) { // Order 2+ handlers
					Variable * handler( handlers_[ i ] );
					if ( handler->self_observer() ) hnd2_observees_.push_back( handler );
					for ( auto observee : handler->observees() ) {
						if ( !observee->is_Discrete() ) hnd2_observees_.push_back( observee );
					}
				}
				uniquify( hnd2_observees_ );
				if ( hnd3_.have() ) {
					hnd3_observees_.clear();
					for ( size_type i = hnd3_.b(), e = hnd_.e(); i < e; ++i ) { // Order 3+ handlers
						Variable * handler( handlers_[ i ] );
						if ( handler->self_observer() ) hnd3_observees_.push_back( handler );
						for ( auto observee : handler->observees() ) {
							if ( !observee->is_Discrete() ) hnd3_observees_.push_back( observee );
						}
					}
					uniquify( hnd3_observees_ );
				}
			}
		}
	}

	// Handler Advance
	void
	advance_handler( Time const t )
	{
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );

		for ( Variable * handler : handlers_ ) {
			assert( handler->not_Input() ); // Check valid handler types
			assert( handler->not_ZC() ); // Check valid handler types
			handler->advance_handler_0( t );
		}
		if ( hnd1_.have() ) {
			for ( Variable * observee : uni_order_ ? hnd_observees_ : hnd1_observees_ ) {
				observee->fmu_set_q( t );
			}
			assert( hnd_.n() == hnd_ders_.size() );
			size_type const hnd1_b( hnd1_.b() );
			fmu_me_->get_reals( hnd1_.n(), &hnd_ders_.refs[ hnd1_b ], &hnd_ders_.ders[ hnd1_b ] );
			for ( size_type i = hnd1_.b(), e = hnd_.e(); i < e; ++i ) { // Order 1+ handlers
				assert( handlers_[ i ]->is_QSS() );
				handlers_[ i ]->advance_handler_1( hnd_ders_.ders[ i ] );
			}
			if ( hnd3_.have() ) {
				Time tN( t - options::dtND );
				if ( fwd_time( tN ) ) { // Use centered ND formulas
					fmu_me_->set_time( tN );
					for ( Variable * observee : uni_order_ ? hnd_observees_ : hnd2_observees_ ) {
						observee->fmu_set_q( tN );
					}
					size_type const hnd2_b( hnd2_.b() );
					fmu_me_->get_reals( hnd2_.n(), &hnd_ders_.refs[ hnd2_b ], &hnd_ders_.ders_m[ hnd2_b ] );
					tN = t + options::dtND;
					fmu_me_->set_time( tN );
					for ( Variable * observee : uni_order_ ? hnd_observees_ : hnd2_observees_ ) {
						observee->fmu_set_q( tN );
					}
					fmu_me_->get_reals( hnd2_.n(), &hnd_ders_.refs[ hnd2_b ], &hnd_ders_.ders_p[ hnd2_b ] );
					for ( size_type i = hnd2_b, e = hnd_.e(); i < e; ++i ) { // Order 2+ handlers
						handlers_[ i ]->advance_handler_2( hnd_ders_.ders_m[ i ], hnd_ders_.ders_p[ i ] );
					}
					for ( size_type i = hnd2_b, e = hnd_.e(); i < e; ++i ) { // Order 2+ handlers
						handlers_[ i ]->advance_handler_2_1();
					}
					for ( size_type i = hnd3_.b(), e = hnd_.e(); i < e; ++i ) { // Order 3+ handlers
						handlers_[ i ]->advance_handler_3();
					}
				} else { // Use forward ND formulas
					tN = t + options::dtND;
					fmu_me_->set_time( tN );
					for ( Variable * observee : uni_order_ ? hnd_observees_ : hnd2_observees_ ) {
						observee->fmu_set_q( tN );
					}
					size_type const hnd2_b( hnd2_.b() );
					fmu_me_->get_reals( hnd2_.n(), &hnd_ders_.refs[ hnd2_b ], &hnd_ders_.ders_m[ hnd2_b ] );
					tN = t + options::two_dtND;
					fmu_me_->set_time( tN );
					for ( Variable * observee : uni_order_ ? hnd_observees_ : hnd2_observees_ ) {
						observee->fmu_set_q( tN );
					}
					fmu_me_->get_reals( hnd2_.n(), &hnd_ders_.refs[ hnd2_b ], &hnd_ders_.ders_p[ hnd2_b ] );
					for ( size_type i = hnd2_b, e = hnd_.e(); i < e; ++i ) { // Order 2+ handlers
						handlers_[ i ]->advance_handler_2_forward( hnd_ders_.ders_m[ i ], hnd_ders_.ders_p[ i ] );
					}
					for ( size_type i = hnd2_b, e = hnd_.e(); i < e; ++i ) { // Order 2+ handlers
						handlers_[ i ]->advance_handler_2_1();
					}
					for ( size_type i = hnd3_.b(), e = hnd_.e(); i < e; ++i ) { // Order 3+ handlers
						handlers_[ i ]->advance_handler_3_forward();
					}
				}
				fmu_me_->set_time( t );
			} else if ( hnd2_.have() ) {
				Time tN( t + options::dtND );
				fmu_me_->set_time( tN );
				for ( Variable * observee : uni_order_ ? hnd_observees_ : hnd2_observees_ ) {
					observee->fmu_set_q( tN );
				}
				size_type const hnd2_b( hnd2_.b() );
				fmu_me_->get_reals( hnd2_.n(), &hnd_ders_.refs[ hnd2_b ], &hnd_ders_.ders_p[ hnd2_b ] );
				for ( size_type i = hnd2_b, e = hnd_.e(); i < e; ++i ) { // Order 2+ handlers
					handlers_[ i ]->advance_handler_2( hnd_ders_.ders_p[ i ] );
				}
				for ( size_type i = hnd2_b, e = hnd_.e(); i < e; ++i ) { // Order 2+ handlers
					handlers_[ i ]->advance_handler_2_1();
				}
				fmu_me_->set_time( t );
			}
		}
		for ( Variable * handler : handlers_ ) {
			handler->advance_handler_F();
		}
	}

	// Clear
	void
	clear()
	{
		handlers_.clear();
		reset_specs();
	}

public: // Iterator

	// Begin Iterator
	const_iterator
	begin() const
	{
		return handlers_.begin();
	}

	// Begin Iterator
	iterator
	begin()
	{
		return handlers_.begin();
	}

	// End Iterator
	const_iterator
	end() const
	{
		return handlers_.end();
	}

	// End Iterator
	iterator
	end()
	{
		return handlers_.end();
	}

public: // Subscript

	// Handlers[ i ]
	Variable const *
	operator []( size_type const i ) const
	{
		return handlers_[ i ];
	}

	// Handlers[ i ]
	Variable *
	operator []( size_type const i )
	{
		return handlers_[ i ];
	}

private: // Methods

	// Reset Specs
	void
	reset_specs()
	{
		hnd_.reset();
		hnd2_.reset();
		hnd3_.reset();
	}

	// Set Specs
	void
	set_specs()
	{
		reset_specs();
		if ( handlers_.empty() ) return;

		hnd_.b() = 0u;
		hnd_.e() = hnd1_.b() = hnd1_.e() = hnd2_.b() = hnd2_.e() = hnd3_.b() = hnd3_.e() = handlers_.size();
		for ( size_type i = 0, e = hnd_.e(); i < e; ++i ) {
			int const order( handlers_[ i ]->order() );
			if ( order >= 1 ) {
				hnd1_.b() = std::min( hnd1_.b(), i );
				if ( order >= 2 ) {
					hnd2_.b() = std::min( hnd2_.b(), i );
					if ( order >= 3 ) {
						hnd3_.b() = std::min( hnd3_.b(), i );
						break;
					}
				}
			}
		}
		size_type const hnd_n( hnd_.n() );
		uni_order_ = (
		 ( hnd1_.empty() || hnd1_.n() == hnd_n ) &&
		 ( hnd2_.empty() || hnd2_.n() == hnd_n ) &&
		 ( hnd3_.empty() || hnd3_.n() == hnd_n )
		);
	}

private: // Data

	FMU_ME * fmu_me_{ nullptr }; // FMU-ME (non-owning) pointer

	Variables handlers_; // Handlers

	// Handler index specs
	bool uni_order_{ false }; // Handlers all the same order?
	Range hnd_; // Handlers
	Range hnd1_; // Handlers of order 1+
	Range hnd2_; // Handlers of order 2+
	Range hnd3_; // Handlers of order 3+

	// Observees (including self-observers)
	Variables hnd_observees_; // Handlers observees
	Variables hnd1_observees_; // Handlers of order 1+ observees
	Variables hnd2_observees_; // Handlers of order 2+ observees
	Variables hnd3_observees_; // Handlers of order 3+ observees

	RefsDers< Variable > hnd_ders_; // Handler derivative FMU pooled call data

}; // Handlers

} // fmu
} // QSS

#endif
