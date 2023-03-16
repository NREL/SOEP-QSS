// Variable Observers
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

#ifndef QSS_Observers_hh_INCLUDED
#define QSS_Observers_hh_INCLUDED

// QSS Headers
#include <QSS/FMU_ME.hh>
#include <QSS/RefsDers.hh>
#include <QSS/RefsVals.hh>
#include <QSS/RefsValsDers.hh>
#include <QSS/container.hh>
#include <QSS/math.hh>
#include <QSS/options.hh>
#include <QSS/Range.hh>

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
	 fmu_me_( fmu_me )
	{}

	// FMU-ME + Trigger Constructor
	explicit
	Observers(
	 FMU_ME * fmu_me,
	 Variable * trigger
	) :
	 fmu_me_( fmu_me ),
	 trigger_( trigger )
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
		return ( !observers_.empty() );
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
		find_computational_observers( observers_, observers_checked, observers_set ); // Note: Looks at other variable observers that aren't necessarily uniquified yet but that is OK: Might be more efficient to make this a separate phase after all are uniquified
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
	assign( Variables & triggers )
	{
		// Combine all non-trigger observers (they are already be computational observers)
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
					if ( !std::binary_search( triggers.begin(), triggers.end(), observer ) ) observers_.push_back( observer );
				}
			}
		}

		set_up();
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
				qss_ders_.push_back( observers_[ i ]->der().ref() );
			}
		}
		if ( r_.have() ) { // R variables
			r_vars_.clear(); r_vars_.reserve( r_.n() );
			for ( size_type i = r_.b(), e = r_.e(); i < e; ++i ) {
				r_vars_.push_back( observers_[ i ]->var().ref() );
			}
		}
		if ( zc_.have() ) { // Zero-crossing variables
			zc_vars_.clear(); zc_vars_.reserve( zc_.n() );
			for ( size_type i = zc_.b(), e = zc_.e(); i < e; ++i ) {
				zc_vars_.push_back( observers_[ i ]->var().ref() );
			}
		}

		// Observees setup /////

		// QSS observer observees set up
		if ( qss_.have() ) {
			qss_observees_.clear();
			for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) {
				Variable * observer( observers_[ i ] );
				if ( observer->self_observee() ) qss_observees_.push_back( observer );
				for ( auto observee : observer->observees() ) {
					qss_observees_.push_back( observee );
				}
			}
			uniquify( qss_observees_ );
			if ( !qss_uni_order_ ) {
				assert( qss2_.have() );
				qss2_observees_.clear();
				for ( size_type i = qss2_.b(), e = qss_.e(); i < e; ++i ) { // Order 2+ observers
					Variable * observer( observers_[ i ] );
					assert( observer->order() >= 2 );
					if ( observer->self_observee() ) qss2_observees_.push_back( observer );
					for ( auto observee : observer->observees() ) {
						qss2_observees_.push_back( observee );
					}
				}
				uniquify( qss2_observees_ );
				if ( qss3_.have() ) {
					qss3_observees_.clear();
					for ( size_type i = qss3_.b(), e = qss_.e(); i < e; ++i ) { // Order 3+ observers
						Variable * observer( observers_[ i ] );
						assert( observer->order() >= 3 );
						if ( observer->self_observee() ) qss3_observees_.push_back( observer );
						for ( auto observee : observer->observees() ) {
							qss3_observees_.push_back( observee );
						}
					}
					uniquify( qss3_observees_ );
				}
			}
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
			if ( !r_uni_order_ ) {
				assert( r2_.have() );
				r2_observees_.clear();
				for ( size_type i = r2_.b(), e = r_.e(); i < e; ++i ) { // Order 2+ observers
					Variable * observer( observers_[ i ] );
					assert( observer->order() >= 2 );
					assert( !observer->self_observee() );
					for ( auto observee : observer->observees() ) {
						r2_observees_.push_back( observee );
					}
				}
				uniquify( r2_observees_ );
				if ( r3_.have() ) {
					r3_observees_.clear();
					for ( size_type i = r3_.b(), e = r_.e(); i < e; ++i ) { // Order 3+ observers
						Variable * observer( observers_[ i ] );
						assert( observer->order() >= 3 );
						assert( !observer->self_observee() );
						for ( auto observee : observer->observees() ) {
							r3_observees_.push_back( observee );
						}
					}
					uniquify( r3_observees_ );
				}
			}
		}

		// // Other X-based observer observees set up // We don't currently do observee operation pooling for these observers
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
			if ( !zc_uni_order_ ) {
				assert( zc2_.have() );
				zc2_observees_.clear();
				for ( size_type i = zc2_.b(), e = zc_.e(); i < e; ++i ) { // Order 2+ observers
					Variable * observer( observers_[ i ] );
					assert( observer->order() >= 2 );
					assert( !observer->self_observee() );
					for ( auto observee : observer->observees() ) {
						zc2_observees_.push_back( observee );
					}
				}
				uniquify( zc2_observees_ );
				if ( zc3_.have() ) {
					zc3_observees_.clear();
					for ( size_type i = zc3_.b(), e = zc_.e(); i < e; ++i ) { // Order 3+ observers
						Variable * observer( observers_[ i ] );
						assert( observer->order() >= 3 );
						assert( !observer->self_observee() );
						for ( auto observee : observer->observees() ) {
							zc3_observees_.push_back( observee );
						}
					}
					uniquify( zc3_observees_ );
				}
			}
		}

		// Set up observers observees pooled directional derivative seed data /////

		// Real observers
		if ( r_.have() ) {
			r_observees_v_ref_.clear();
			r_observees_dv_.clear();
			for ( auto observee : r_observees_ ) {
				r_observees_v_ref_.push_back( observee->var().ref() );
				r_observees_dv_.push_back( 0.0 ); // Actual values assigned when getting directional derivatives
			}
		}

		// Zero-crossing observers
		if ( zc_.have() ) {
			zc_observees_v_ref_.clear();
			zc_observees_dv_.clear();
			for ( auto observee : zc_observees_ ) {
				zc_observees_v_ref_.push_back( observee->var().ref() );
				zc_observees_dv_.push_back( 0.0 ); // Actual values assigned when getting directional derivatives
			}
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
			if ( observers_checked.find( observer ) == observers_checked.end() ) { // Observer not already processed
				observers_checked.insert( observer );
				if ( observer == trigger_ ) continue;
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
				if ( observers_checked.find( observer ) == observers_checked.end() ) { // Observer not already processed
					observers_checked.insert( observer );
					if ( observer == trigger_ ) continue;
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
		qss2_.reset();
		qss3_.reset();
		ns_.reset();
		zc_.reset();
		zc2_.reset();
		zc3_.reset();
		r_.reset();
		r2_.reset();
		r3_.reset();
		ox_.reset();
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

		// Non-state observers
		if ( i < all_.e() ) {
			ns_.b() = i;
			ns_.e() = all_.e();
		}

		// Real observers
		while ( ( i < all_.e() ) && ( observers_[ i ]->is_R() ) ) {
			Variable const * observer( observers_[ i ] );
			r_.b() = std::min( r_.b(), i );
			int const order( observer->order() );
			if ( order >= 2 ) {
				r2_.b() = std::min( r2_.b(), i );
				if ( order >= 3 ) {
					r3_.b() = std::min( r3_.b(), i );
				}
			}
			if ( observer->connected_output ) connected_output_observer_ = true;
			++i;
		}
		if ( r_.began() ) {
			r_.e() = i;
			if ( r2_.began() ) {
				r2_.e() = i;
				if ( r3_.began() ) {
					r3_.e() = i;
				}
			}
		}
		size_type const r_n( r_.n() );
		r_uni_order_ = (
		 ( r2_.empty() || r2_.n() == r_n ) &&
		 ( r3_.empty() || r3_.n() == r_n )
		);

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
	}

	// Advance QSS State Observers
	void
	advance_QSS( Time const t )
	{
		assert( qss_.have() );
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );
		assert( qss_.n() == qss_ders_.size() );

		for ( Variable * observee : qss_observees_ ) {
			observee->fmu_set_s( t );
		}
		fmu_me_->get_reals( qss_.n(), &qss_ders_.refs[ 0 ], &qss_ders_.ders[ 0 ] );
		for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) {
			assert( observers_[ i ]->is_QSS() );
			observers_[ i ]->advance_observer_1( t, qss_ders_.ders[ i ] );
		}
		if ( qss3_.have() ) {
			Time tN( t - options::dtND );
			if ( fwd_time( tN ) ) { // Use centered ND formulas
				fmu_me_->set_time( tN );
				for ( Variable * observee : qss_uni_order_ ? qss_observees_ : qss3_observees_ ) {
					observee->fmu_set_s( tN );
				}
				size_type const qss2_b( qss2_.b() );
				fmu_me_->get_reals( qss2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.ders_m[ qss2_b ] );
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : qss_uni_order_ ? qss_observees_ : qss3_observees_ ) {
					observee->fmu_set_s( tN );
				}
				fmu_me_->get_reals( qss2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.ders_p[ qss2_b ] );
				for ( size_type i = qss2_b, e = qss_.e(); i < e; ++i ) { // Order 2+ observers
					observers_[ i ]->advance_observer_2( qss_ders_.ders_m[ i ], qss_ders_.ders_p[ i ] );
				}
				for ( size_type i = qss3_.b(), e = qss_.e(); i < e; ++i ) { // Order 3+ observers
					observers_[ i ]->advance_observer_3();
				}
			} else { // Use forward ND formulas
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : qss_uni_order_ ? qss_observees_ : qss3_observees_ ) {
					observee->fmu_set_s( tN );
				}
				size_type const qss2_b( qss2_.b() );
				fmu_me_->get_reals( qss2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.ders_m[ qss2_b ] );
				tN = t + options::two_dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : qss_uni_order_ ? qss_observees_ : qss3_observees_ ) {
					observee->fmu_set_s( tN );
				}
				fmu_me_->get_reals( qss2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.ders_p[ qss2_b ] );
				for ( size_type i = qss2_b, e = qss_.e(); i < e; ++i ) { // Order 2+ observers
					observers_[ i ]->advance_observer_2_forward( qss_ders_.ders_m[ i ], qss_ders_.ders_p[ i ] );
				}
				for ( size_type i = qss3_.b(), e = qss_.e(); i < e; ++i ) { // Order 3+ observers
					observers_[ i ]->advance_observer_3_forward();
				}
			}
			fmu_me_->set_time( t );
		} else if ( qss2_.have() ) {
			Time const tN( t + options::dtND );
			fmu_me_->set_time( tN );
			for ( Variable * observee : qss_uni_order_ ? qss_observees_ : qss2_observees_ ) {
				observee->fmu_set_s( tN );
			}
			size_type const qss2_b( qss2_.b() );
			fmu_me_->get_reals( qss2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.ders[ qss2_b ] );
			for ( size_type i = qss2_b, e = qss_.e(); i < e; ++i ) { // Order 2+ observers
				observers_[ i ]->advance_observer_2( qss_ders_.ders[ i ] );
			}
			fmu_me_->set_time( t );
		}
	}

	// Advance Real Non-State Observers
	void
	advance_R( Time const t )
	{
		assert( r_.have() );
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );
		assert( r_.n() == r_vars_.size() );

		for ( Variable * observee : r_observees_ ) {
			observee->fmu_set_x( t );
		}
		fmu_me_->get_reals( r_.n(), &r_vars_.refs[ 0 ], &r_vars_.vals[ 0 ] );
		for ( size_type i = 0, e = r_observees_.size(); i < e; ++i ) {
			r_observees_dv_[ i ] = r_observees_[ i ]->x1( t );
		}
		fmu_me_->get_directional_derivatives(
		 r_observees_v_ref_.data(),
		 r_observees_v_ref_.size(),
		 r_vars_.refs.data(),
		 r_vars_.refs.size(),
		 r_observees_dv_.data(),
		 r_vars_.ders.data()
		);
		for ( size_type i = r_.b(), e = r_.e(), j = 0; i < e; ++i, ++j ) {
			assert( observers_[ i ]->is_R() );
			observers_[ i ]->advance_observer_1( t, r_vars_.vals[ j ], r_vars_.ders[ j ] );
		}
		if ( r3_.have() ) {
			Time tN( t - options::dtND );
			if ( fwd_time( tN ) ) { // Use centered ND formulas
				fmu_me_->set_time( tN );
				for ( Variable * observee : r_uni_order_ ? r_observees_ : r2_observees_ ) {
					observee->fmu_set_x( tN );
				}
				for ( size_type i = 0, e = r_observees_.size(); i < e; ++i ) {
					r_observees_dv_[ i ] = r_observees_[ i ]->x1( tN );
				}
				fmu_me_->get_directional_derivatives(
				 r_observees_v_ref_.data(),
				 r_observees_v_ref_.size(),
				 r_vars_.refs.data(),
				 r_vars_.refs.size(),
				 r_observees_dv_.data(),
				 r_vars_.ders_m.data()
				);
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : r_uni_order_ ? r_observees_ : r2_observees_ ) {
					observee->fmu_set_x( tN );
				}
				for ( size_type i = 0, e = r_observees_.size(); i < e; ++i ) {
					r_observees_dv_[ i ] = r_observees_[ i ]->x1( tN );
				}
				fmu_me_->get_directional_derivatives(
				 r_observees_v_ref_.data(),
				 r_observees_v_ref_.size(),
				 r_vars_.refs.data(),
				 r_vars_.refs.size(),
				 r_observees_dv_.data(),
				 r_vars_.ders_p.data()
				);
				size_type const r2_bo( r2_.b() - r_.b() );
				for ( size_type i = r2_.b(), j = r2_bo, e = r_.e(); i < e; ++i, ++j ) { // Order 2+ observers
					observers_[ i ]->advance_observer_2( r_vars_.ders_m[ j ], r_vars_.ders_p[ j ] );
				}
				for ( size_type i = r3_.b(), e = r_.e(); i < e; ++i ) { // Order 3+ observers
					observers_[ i ]->advance_observer_3();
				}
			} else { // Use forward ND formulas
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : r_uni_order_ ? r_observees_ : r2_observees_ ) {
					observee->fmu_set_x( tN );
				}
				for ( size_type i = 0, e = r_observees_.size(); i < e; ++i ) {
					r_observees_dv_[ i ] = r_observees_[ i ]->x1( tN );
				}
				fmu_me_->get_directional_derivatives(
				 r_observees_v_ref_.data(),
				 r_observees_v_ref_.size(),
				 r_vars_.refs.data(),
				 r_vars_.refs.size(),
				 r_observees_dv_.data(),
				 r_vars_.ders_m.data()
				);
				tN = t + options::two_dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : r_uni_order_ ? r_observees_ : r2_observees_ ) {
					observee->fmu_set_x( tN );
				}
				for ( size_type i = 0, e = r_observees_.size(); i < e; ++i ) {
					r_observees_dv_[ i ] = r_observees_[ i ]->x1( tN );
				}
				fmu_me_->get_directional_derivatives(
				 r_observees_v_ref_.data(),
				 r_observees_v_ref_.size(),
				 r_vars_.refs.data(),
				 r_vars_.refs.size(),
				 r_observees_dv_.data(),
				 r_vars_.ders_p.data()
				);
				size_type const r2_bo( r2_.b() - r_.b() );
				for ( size_type i = r2_.b(), j = r2_bo, e = r_.e(); i < e; ++i, ++j ) { // Order 2+ observers
					observers_[ i ]->advance_observer_2_forward( r_vars_.ders_m[ j ], r_vars_.ders_p[ j ] );
				}
				for ( size_type i = r3_.b(), e = r_.e(); i < e; ++i ) { // Order 3+ observers
					observers_[ i ]->advance_observer_3_forward();
				}
			}
			fmu_me_->set_time( t );
		} else if ( r2_.have() ) {
			Time const tN( t + options::dtND );
			fmu_me_->set_time( tN );
			for ( Variable * observee : r_uni_order_ ? r_observees_ : r2_observees_ ) {
				observee->fmu_set_x( tN );
			}
			for ( size_type i = 0, e = r_observees_.size(); i < e; ++i ) {
				r_observees_dv_[ i ] = r_observees_[ i ]->x1( tN );
			}
			fmu_me_->get_directional_derivatives(
			 r_observees_v_ref_.data(),
			 r_observees_v_ref_.size(),
			 r_vars_.refs.data(),
			 r_vars_.refs.size(),
			 r_observees_dv_.data(),
			 r_vars_.ders_p.data()
			);
			size_type const r2_bo( r2_.b() - r_.b() );
			for ( size_type i = r2_.b(), j = r2_bo, e = r_.e(); i < e; ++i, ++j ) { // Order 2+ observers
				observers_[ i ]->advance_observer_2( r_vars_.ders_p[ j ] );
			}
			fmu_me_->set_time( t );
		}
	}

	// Advance Other X-Based Observers
	void
	advance_OX( Time const t )
	{
		assert( ox_.have() );
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );

		for ( size_type i = ox_.b(), e = ox_.e(); i < e; ++i ) {
			assert( observers_[ i ]->is_BIDR() && !observers_[ i ]->is_R() );
			observers_[ i ]->advance_observer_1( t );
		}
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

		for ( Variable * observee : zc_observees_ ) {
			observee->fmu_set_x( t );
		}
		fmu_me_->get_reals( zc_.n(), &zc_vars_.refs[ 0 ], &zc_vars_.vals[ 0 ] );
		for ( size_type i = 0, e = zc_observees_.size(); i < e; ++i ) {
			zc_observees_dv_[ i ] = zc_observees_[ i ]->x1( t );
		}
		fmu_me_->get_directional_derivatives(
		 zc_observees_v_ref_.data(),
		 zc_observees_v_ref_.size(),
		 zc_vars_.refs.data(),
		 zc_vars_.refs.size(),
		 zc_observees_dv_.data(),
		 zc_vars_.ders.data()
		);
		for ( size_type i = zc_.b(), e = zc_.e(), j = 0; i < e; ++i, ++j ) {
			assert( observers_[ i ]->is_ZC() );
			observers_[ i ]->advance_observer_1( t, zc_vars_.vals[ j ], zc_vars_.ders[ j ] );
		}
		if ( zc3_.have() ) {
			Time tN( t - options::dtND );
			if ( fwd_time( tN ) ) { // Use centered ND formulas
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
				 zc_vars_.refs.data(),
				 zc_vars_.refs.size(),
				 zc_observees_dv_.data(),
				 zc_vars_.ders_m.data()
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
				 zc_vars_.refs.data(),
				 zc_vars_.refs.size(),
				 zc_observees_dv_.data(),
				 zc_vars_.ders_p.data()
				);
				size_type const zc2_bo( zc2_.b() - zc_.b() );
				for ( size_type i = zc2_.b(), j = zc2_bo, e = zc_.e(); i < e; ++i, ++j ) { // Order 2+ observers
					observers_[ i ]->advance_observer_2( zc_vars_.ders_m[ j ], zc_vars_.ders_p[ j ] );
				}
				for ( size_type i = zc3_.b(), e = zc_.e(); i < e; ++i ) { // Order 3+ observers
					observers_[ i ]->advance_observer_3();
				}
			} else { // Use forward ND formulas
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
				 zc_vars_.refs.data(),
				 zc_vars_.refs.size(),
				 zc_observees_dv_.data(),
				 zc_vars_.ders_m.data()
				);
				tN = t + options::two_dtND;
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
				 zc_vars_.refs.data(),
				 zc_vars_.refs.size(),
				 zc_observees_dv_.data(),
				 zc_vars_.ders_p.data()
				);
				size_type const zc2_bo( zc2_.b() - zc_.b() );
				for ( size_type i = zc2_.b(), j = zc2_bo, e = zc_.e(); i < e; ++i, ++j ) { // Order 2+ observers
					observers_[ i ]->advance_observer_2_forward( zc_vars_.ders_m[ j ], zc_vars_.ders_p[ j ] );
				}
				for ( size_type i = zc3_.b(), e = zc_.e(); i < e; ++i ) { // Order 3+ observers
					observers_[ i ]->advance_observer_3_forward();
				}
			}
			fmu_me_->set_time( t );
		} else if ( zc2_.have() ) {
			Time const tN( t + options::dtND );
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
			 zc_vars_.refs.data(),
			 zc_vars_.refs.size(),
			 zc_observees_dv_.data(),
			 zc_vars_.ders_p.data()
			);
			size_type const zc2_bo( zc2_.b() - zc_.b() );
			for ( size_type i = zc2_.b(), j = zc2_bo, e = zc_.e(); i < e; ++i, ++j ) { // Order 2+ observers
				observers_[ i ]->advance_observer_2( zc_vars_.ders_p[ j ] );
			}
			fmu_me_->set_time( t );
		}
	}

	// Advance: Stage Final
	void
	advance_F()
	{
		for ( Variable * observer : observers_ ) {
			observer->advance_observer_F();
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

private: // Data

	FMU_ME * fmu_me_{ nullptr }; // FMU-ME (non-owning) pointer

	Variable * trigger_{ nullptr }; // Trigger variable

	Variables observers_; // Observers
	Variables computational_observers_; // Computational observers

	bool connected_output_observer_{ false }; // Output connection observer to another FMU?

	// Observer index ranges
	Range all_; // All observers
	Range qss_; // QSS state observers
	Range qss2_; // QSS state observers of order 2+
	Range qss3_; // QSS state observers of order 3+
	Range ns_; // Non-state (X-based) observers
	Range r_; // Real observers
	Range r2_; // Real observers of order 2+
	Range r3_; // Real observers of order 3+
	Range ox_; // Other X-based observers
	Range zc_; // Zero-crossing observers
	Range zc2_; // Zero-crossing observers of order 2+
	Range zc3_; // Zero-crossing observers of order 3+

	// Uniform order flags
	bool qss_uni_order_{ false }; // QSS observers all the same order?
	bool zc_uni_order_{ false }; // ZC observers all the same order?
	bool r_uni_order_{ false }; // R observers all the same order?

	// Observer FMU pooled call data
	RefsDers< Variable > qss_ders_; // QSS state derivatives
	RefsValsDers< Variable > r_vars_; // Real non-state values and derivatives
	RefsValsDers< Variable > zc_vars_; // Zero-crossing values and derivatives

	// QSS state observers observees (including self-observers)
	Variables qss_observees_; // Observers observees
	Variables qss2_observees_; // Observers of order 2+ observees
	Variables qss3_observees_; // Observers of order 3+ observees

	// Real observers observees
	Variables r_observees_; // Observers observees
	Variables r2_observees_; // Observers of order 2+ observees
	Variables r3_observees_; // Observers of order 3+ observees
	VariableRefs r_observees_v_ref_; // Observers observees value references
	Reals r_observees_dv_; // Observers observees derivatives

	// Other X-based observers observees
	// Variables ox_observees_; // Observers observees

	// Zero-crossing observers observees
	Variables zc_observees_; // Observers observees
	Variables zc2_observees_; // Observers of order 2+ observees
	Variables zc3_observees_; // Observers of order 3+ observees
	VariableRefs zc_observees_v_ref_; // Observers observees value references
	Reals zc_observees_dv_; // Observers observees derivatives

}; // Observers

} // QSS

#endif
