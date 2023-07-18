// Variable Handlers
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

#ifndef QSS_Handlers_hh_INCLUDED
#define QSS_Handlers_hh_INCLUDED

// QSS Headers
#include <QSS/FMU_ME.hh>
#include <QSS/RefsDers.hh>
#include <QSS/container.hh>
#include <QSS/math.hh>
#include <QSS/options.hh>
#include <QSS/Range.hh>

// C++ Headers
#include <algorithm>
#include <cassert>

namespace QSS {

// Variable Handlers
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

	// Have Handler(s)?
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
	assign( Variables const & handlers )
	{
		handlers_ = handlers;

		if ( handlers_.empty() ) {
			reset_specs();
			return;
		}

		// Sort by type and order
		assert( is_unique( handlers_ ) ); // Precondition: No duplicates
		sort_by_type_and_order( handlers_ );

		// Set specs
		set_specs();

		// FMU pooled data set up
		if ( qss_.have() ) { // State variables
			qss_vars_.clear(); qss_vars_.reserve( qss_.n() );
			qss_ders_.clear(); qss_ders_.reserve( qss_.n() );
			for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) {
				qss_vars_.push_back( handlers_[ i ]->var().ref() );
				qss_ders_.push_back( handlers_[ i ]->der().ref() );
			}
		}
		if ( r_.have() ) { // R variables
			r_vars_.clear(); r_vars_.reserve( r_.n() );
			for ( size_type i = r_.b(), e = r_.e(); i < e; ++i ) {
				r_vars_.push_back( handlers_[ i ]->var().ref() );
			}
		}
		if ( ox_.have() ) { // Other X-based variables
			ox_vars_.clear(); ox_vars_.reserve( ox_.n() );
			for ( size_type i = ox_.b(), e = ox_.e(); i < e; ++i ) {
				ox_vars_.push_back( handlers_[ i ]->var().ref() );
			}
		}
		if ( zc_.have() ) { // Zero-crossing variables
			zc_vars_.clear(); zc_vars_.reserve( zc_.n() );
			for ( size_type i = zc_.b(), e = zc_.e(); i < e; ++i ) {
				zc_vars_.push_back( handlers_[ i ]->var().ref() );
			}
		}

		// Observees setup /////

		// QSS handler observees set up
		if ( qss_.have() ) {
			qss_observees_.clear();
			for ( size_type i = qss_.b(), e = qss_.e(); i < e; ++i ) {
				Variable * handler( handlers_[ i ] );
				if ( handler->self_observee() ) qss_observees_.push_back( handler );
				for ( auto observee : handler->observees() ) {
					qss_observees_.push_back( observee );
				}
			}
			uniquify( qss_observees_ );
			if ( !qss_uni_order_ ) {
				assert( qss2_.have() );
				qss2_observees_.clear();
				for ( size_type i = qss2_.b(), e = qss_.e(); i < e; ++i ) { // Order 2+ handlers
					Variable * handler( handlers_[ i ] );
					if ( handler->self_observee() ) qss2_observees_.push_back( handler );
					for ( auto observee : handler->observees() ) {
						qss2_observees_.push_back( observee );
					}
				}
				uniquify( qss2_observees_ );
				if ( qss3_.have() ) {
					qss3_observees_.clear();
					for ( size_type i = qss3_.b(), e = qss_.e(); i < e; ++i ) { // Order 3+ handlers
						Variable * handler( handlers_[ i ] );
						if ( handler->self_observee() ) qss3_observees_.push_back( handler );
						for ( auto observee : handler->observees() ) {
							qss3_observees_.push_back( observee );
						}
					}
					uniquify( qss3_observees_ );
				}
			}
		}

		// Real handler observees set up
		if ( r_.have() ) {
			r_observees_.clear();
			for ( size_type i = r_.b(), e = r_.e(); i < e; ++i ) {
				Variable * handler( handlers_[ i ] );
				assert( !handler->self_observee() );
				for ( auto observee : handler->observees() ) {
					r_observees_.push_back( observee );
				}
			}
			uniquify( r_observees_ );
			if ( !r_uni_order_ ) {
				assert( r2_.have() );
				r2_observees_.clear();
				for ( size_type i = r2_.b(), e = r_.e(); i < e; ++i ) { // Order 2+ handlers
					Variable * handler( handlers_[ i ] );
					assert( !handler->self_observee() );
					for ( auto observee : handler->observees() ) {
						r2_observees_.push_back( observee );
					}
				}
				uniquify( r2_observees_ );
				if ( r3_.have() ) {
					r3_observees_.clear();
					for ( size_type i = r3_.b(), e = r_.e(); i < e; ++i ) { // Order 3+ handlers
						Variable * handler( handlers_[ i ] );
						assert( !handler->self_observee() );
						for ( auto observee : handler->observees() ) {
							r3_observees_.push_back( observee );
						}
					}
					uniquify( r3_observees_ );
				}
			}
		}

		// Other X-based handler observees set up //Do Add observee operation pooling for these handlers
		// if ( ox_.have() ) {
		// 	ox_observees_.clear();
		// 	for ( size_type i = ox_.b(), e = ox_.e(); i < e; ++i ) {
		// 		Variable * handler( handlers_[ i ] );
		//			assert( !handler->self_observee() );
		// 		for ( auto observee : handler->observees() ) {
		// 			ox_observees_.push_back( observee );
		// 		}
		// 	}
		// 	uniquify( ox_observees_ );
		// }

		// Zero-crossing handler observees set up
		if ( zc_.have() ) {
			zc_observees_.clear();
			for ( size_type i = zc_.b(), e = zc_.e(); i < e; ++i ) {
				Variable * handler( handlers_[ i ] );
				assert( !handler->self_observee() );
				for ( auto observee : handler->observees() ) {
					zc_observees_.push_back( observee );
				}
			}
			uniquify( zc_observees_ );
			if ( !zc_uni_order_ ) {
				assert( zc2_.have() );
				zc2_observees_.clear();
				for ( size_type i = zc2_.b(), e = zc_.e(); i < e; ++i ) { // Order 2+ handlers
					Variable * handler( handlers_[ i ] );
					assert( !handler->self_observee() );
					for ( auto observee : handler->observees() ) {
						zc2_observees_.push_back( observee );
					}
				}
				uniquify( zc2_observees_ );
				if ( zc3_.have() ) {
					zc3_observees_.clear();
					for ( size_type i = zc3_.b(), e = zc_.e(); i < e; ++i ) { // Order 3+ handlers
						Variable * handler( handlers_[ i ] );
						assert( !handler->self_observee() );
						for ( auto observee : handler->observees() ) {
							zc3_observees_.push_back( observee );
						}
					}
					uniquify( zc3_observees_ );
				}
			}
		}

		// Set up handlers observees pooled directional derivative seed data /////

		// Real handlers
		if ( r_.have() ) {
			r_observees_v_ref_.clear();
			r_observees_dv_.clear();
			for ( auto observee : r_observees_ ) {
				r_observees_v_ref_.push_back( observee->var().ref() );
				r_observees_dv_.push_back( 0.0 ); // Actual values assigned when getting directional derivatives
			}
		}

		// Zero-crossing handlers
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
		if ( qss_.have() ) advance_QSS_F( t );
		if ( r_.have() ) advance_R_F( t );
		if ( ox_.have() ) advance_OX_F( t );
		if ( zc_.have() ) { // Zero-crossing variables
			advance_ZC( t );
			advance_ZC_F( t );
		}
		// advance_F( t ); // Using this instead of the other advance_*_F calls above uses old observee values for the observing event indicators, which probably doesn't make sense
		// if ( options::output::d ) advance_d(); // Currently advance_handler_F calls do diagnostic output
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
		connected_output_handler_ = false;
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
		if ( handlers_.empty() ) return;

		connected_output_handler_ = false;
		all_.b() = 0u;
		all_.e() = handlers_.size();
		size_type i( 0u );

		// QSS state handlers
		while ( ( i < all_.e() ) && ( handlers_[ i ]->is_QSS() ) ) {
			Variable const * handler( handlers_[ i ] );
			qss_.b() = std::min( qss_.b(), i );
			int const order( handler->order() );
			if ( order >= 2 ) {
				qss2_.b() = std::min( qss2_.b(), i );
				if ( order >= 3 ) {
					qss3_.b() = std::min( qss3_.b(), i );
				}
			}
			if ( handler->connected_output ) connected_output_handler_ = true;
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

		// Non-state handlers
		if ( i < all_.e() ) {
			ns_.b() = i;
			ns_.e() = all_.e();
		}

		// Real handlers
		while ( ( i < all_.e() ) && ( handlers_[ i ]->is_R() ) ) {
			Variable const * handler( handlers_[ i ] );
			r_.b() = std::min( r_.b(), i );
			int const order( handler->order() );
			if ( order >= 2 ) {
				r2_.b() = std::min( r2_.b(), i );
				if ( order >= 3 ) {
					r3_.b() = std::min( r3_.b(), i );
				}
			}
			if ( handler->connected_output ) connected_output_handler_ = true;
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

		// Other X-based handlers
		while ( ( i < all_.e() ) && ( handlers_[ i ]->not_ZC() ) ) {
			Variable const * handler( handlers_[ i ] );
			assert( handler->order() == 0 );
			ox_.b() = std::min( ox_.b(), i );
			if ( handler->connected_output ) connected_output_handler_ = true;
			++i;
		}
		if ( ox_.began() ) {
			ox_.e() = i;
		}

		// Zero crossing handlers
		while ( ( i < all_.e() ) && ( handlers_[ i ]->is_ZC() ) ) {
			Variable const * handler( handlers_[ i ] );
			zc_.b() = std::min( zc_.b(), i );
			int const order( handler->order() );
			if ( order >= 2 ) {
				zc2_.b() = std::min( zc2_.b(), i );
				if ( order >= 3 ) {
					zc3_.b() = std::min( zc3_.b(), i );
				}
			}
			if ( handler->connected_output ) connected_output_handler_ = true;
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

	// Advance QSS State Handlers
	void
	advance_QSS( Time const t )
	{
		assert( qss_.have() );
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );
		assert( qss_.n() == qss_vars_.size() );
		assert( qss_.n() == qss_ders_.size() );

		fmu_me_->get_reals( qss_.n(), &qss_vars_.refs[ 0 ], &qss_vars_.vals[ 0 ] );
		for ( size_type i = 0, e = qss_.e(); i < e; ++i ) {
			handlers_[ i ]->advance_handler_0( t, qss_vars_.vals[ i ] );
		}

		fmu_me_->get_reals( qss_.n(), &qss_ders_.refs[ 0 ], &qss_ders_.ders[ 0 ] );
		for ( size_type i = 0, e = qss_.e(); i < e; ++i ) {
			assert( handlers_[ i ]->is_Active() );
			assert( handlers_[ i ]->is_QSS() );
			handlers_[ i ]->advance_handler_1( qss_ders_.ders[ i ] );
		}

		if ( qss3_.have() ) {
			Time tN( t - options::dtND );
			if ( fwd_time( tN ) ) { // Use centered ND formulas
				fmu_me_->set_time( tN );
				for ( Variable * observee : qss_uni_order_ ? qss_observees_ : qss2_observees_ ) {
					observee->fmu_set_s( tN );
				}
				size_type const qss2_b( qss2_.b() );
				fmu_me_->get_reals( qss2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.ders_m[ qss2_b ] );
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : qss_uni_order_ ? qss_observees_ : qss2_observees_ ) {
					observee->fmu_set_s( tN );
				}
				fmu_me_->get_reals( qss2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.ders_p[ qss2_b ] );
				for ( size_type i = qss2_b, e = qss_.e(); i < e; ++i ) { // Order 2+ handlers
					handlers_[ i ]->advance_handler_2( qss_ders_.ders_m[ i ], qss_ders_.ders_p[ i ] );
				}
				for ( size_type i = qss3_.b(), e = qss_.e(); i < e; ++i ) { // Order 3+ handlers
					handlers_[ i ]->advance_handler_3();
				}
			} else { // Use forward ND formulas
				tN = t + options::dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : qss_uni_order_ ? qss_observees_ : qss2_observees_ ) {
					observee->fmu_set_s( tN );
				}
				size_type const qss2_b( qss2_.b() );
				fmu_me_->get_reals( qss2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.ders_m[ qss2_b ] );
				tN = t + options::two_dtND;
				fmu_me_->set_time( tN );
				for ( Variable * observee : qss_uni_order_ ? qss_observees_ : qss2_observees_ ) {
					observee->fmu_set_s( tN );
				}
				fmu_me_->get_reals( qss2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.ders_p[ qss2_b ] );
				for ( size_type i = qss2_b, e = qss_.e(); i < e; ++i ) { // Order 2+ handlers
					handlers_[ i ]->advance_handler_2_forward( qss_ders_.ders_m[ i ], qss_ders_.ders_p[ i ] );
				}
				for ( size_type i = qss3_.b(), e = qss_.e(); i < e; ++i ) { // Order 3+ handlers
					handlers_[ i ]->advance_handler_3_forward();
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
			fmu_me_->get_reals( qss2_.n(), &qss_ders_.refs[ qss2_b ], &qss_ders_.ders_p[ qss2_b ] );
			for ( size_type i = qss2_b, e = qss_.e(); i < e; ++i ) { // Order 2+ handlers
				handlers_[ i ]->advance_handler_2( qss_ders_.ders_p[ i ] );
			}
			fmu_me_->set_time( t );
		}
	}

	// Advance Real Non-State Handlers
	void
	advance_R( Time const t )
	{
		assert( r_.have() );
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );
		assert( r_.n() == r_vars_.size() );

		fmu_me_->get_reals( r_.n(), &r_vars_.refs[ 0 ], &r_vars_.vals[ 0 ] );
		for ( size_type i = r_.b(), e = r_.e(), j = 0; i < e; ++i, ++j ) {
			handlers_[ i ]->advance_handler_0( t, r_vars_.vals[ j ] );
		}

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
			assert( handlers_[ i ]->is_Active() );
			assert( handlers_[ i ]->is_R() );
			handlers_[ i ]->advance_handler_1( r_vars_.ders[ j ] );
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
				for ( size_type i = r2_.b(), j = r2_bo, e = r_.e(); i < e; ++i, ++j ) { // Order 2+ handlers
					handlers_[ i ]->advance_handler_2( r_vars_.ders_m[ j ], r_vars_.ders_p[ j ] );
				}
				for ( size_type i = r3_.b(), e = r_.e(); i < e; ++i ) { // Order 3+ handlers
					handlers_[ i ]->advance_handler_3();
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
				for ( size_type i = r2_.b(), j = r2_bo, e = r_.e(); i < e; ++i, ++j ) { // Order 2+ handlers
					handlers_[ i ]->advance_handler_2_forward( r_vars_.ders_m[ j ], r_vars_.ders_p[ j ] );
				}
				for ( size_type i = r3_.b(), e = r_.e(); i < e; ++i ) { // Order 3+ handlers
					handlers_[ i ]->advance_handler_3_forward();
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
			for ( size_type i = r2_.b(), j = r2_bo, e = r_.e(); i < e; ++i, ++j ) { // Order 2+ handlers
				handlers_[ i ]->advance_handler_2( r_vars_.ders_p[ j ] );
			}
			fmu_me_->set_time( t );
		}
	}

	// Advance Other X-Based Handlers
	void
	advance_OX( Time const t )
	{
		assert( ox_.have() );
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );

		fmu_me_->get_reals( ox_.n(), &ox_vars_.refs[ 0 ], &ox_vars_.vals[ 0 ] );
		for ( size_type i = ox_.b(), e = ox_.e(), j = 0; i < e; ++i, ++j ) {
			handlers_[ i ]->advance_handler_0( t, ox_vars_.vals[ j ] );
		}
	}

	// Advance Zero-Crossing Handlers
	void
	advance_ZC( Time const t )
	{
		assert( zc_.have() );
		assert( fmu_me_ != nullptr );
		assert( fmu_me_->get_time() == t );
		assert( zc_.n() == zc_vars_.size() );

		fmu_me_->get_reals( zc_.n(), &zc_vars_.refs[ 0 ], &zc_vars_.vals[ 0 ] );
		for ( size_type i = zc_.b(), e = zc_.e(), j = 0; i < e; ++i, ++j ) {
			handlers_[ i ]->advance_handler_0( t, zc_vars_.vals[ j ] );
		}

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
			assert( handlers_[ i ]->is_ZC() );
			handlers_[ i ]->advance_handler_1( zc_vars_.ders[ j ] );
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
				for ( size_type i = zc2_.b(), j = zc2_bo, e = zc_.e(); i < e; ++i, ++j ) { // Order 2+ handlers
					handlers_[ i ]->advance_handler_2( zc_vars_.ders_m[ j ], zc_vars_.ders_p[ j ] );
				}
				for ( size_type i = zc3_.b(), e = zc_.e(); i < e; ++i ) { // Order 3+ handlers
					handlers_[ i ]->advance_handler_3();
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
				for ( size_type i = zc2_.b(), j = zc2_bo, e = zc_.e(); i < e; ++i, ++j ) { // Order 2+ handlers
					handlers_[ i ]->advance_handler_2_forward( zc_vars_.ders_m[ j ], zc_vars_.ders_p[ j ] );
				}
				for ( size_type i = zc3_.b(), e = zc_.e(); i < e; ++i ) { // Order 3+ handlers
					handlers_[ i ]->advance_handler_3_forward();
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
			for ( size_type i = zc2_.b(), j = zc2_bo, e = zc_.e(); i < e; ++i, ++j ) { // Order 2+ handlers
				handlers_[ i ]->advance_handler_2( zc_vars_.ders_p[ j ] );
			}
			fmu_me_->set_time( t );
		}
	}

	// Advance QSS State Handlers: Stage Final
	void
	advance_QSS_F( Time const t )
	{
		for ( size_type i = 0, e = qss_.e(); i < e; ++i ) {
			handlers_[ i ]->advance_handler_F();
		}
	}

	// Advance Real Non-State Handlers: Stage Final
	void
	advance_R_F( Time const t )
	{
		for ( size_type i = r_.b(), e = r_.e(); i < e; ++i ) {
			handlers_[ i ]->advance_handler_F();
		}
	}

	// Advance Other X-Based Handlers: Stage Final
	void
	advance_OX_F( Time const t )
	{
		for ( size_type i = ox_.b(), e = ox_.e(); i < e; ++i ) {
			handlers_[ i ]->advance_handler_F();
		}
	}

	// Advance Zero-Crossing Handlers: Stage Final
	void
	advance_ZC_F( Time const t )
	{
		for ( size_type i = zc_.b(), e = zc_.e(); i < e; ++i ) {
			handlers_[ i ]->advance_handler_F();
		}
	}

	// Advance: Stage Final
	void
	advance_F( Time const t )
	{
		for ( Variable * handler : handlers_ ) {
			handler->advance_handler_F();
		}
	}

private: // Data

	FMU_ME * fmu_me_{ nullptr }; // FMU-ME (non-owning) pointer

	Variables handlers_; // Handlers

	bool connected_output_handler_{ false }; // Output connection handler to another FMU?

	// Handler index ranges
	Range all_; // All handlers
	Range qss_; // QSS state handlers
	Range qss2_; // QSS state handlers of order 2+
	Range qss3_; // QSS state handlers of order 3+
	Range ns_; // Non-state (X-based) handlers
	Range r_; // Real handlers
	Range r2_; // Real handlers of order 2+
	Range r3_; // Real handlers of order 3+
	Range ox_; // Other X-based handlers
	Range zc_; // Zero-crossing handlers
	Range zc2_; // Zero-crossing handlers of order 2+
	Range zc3_; // Zero-crossing handlers of order 3+

	// Uniform order flags
	bool qss_uni_order_{ false }; // QSS handlers all the same order?
	bool zc_uni_order_{ false }; // ZC handlers all the same order?
	bool r_uni_order_{ false }; // R handlers all the same order?

	// Handler FMU pooled call data
	RefsVals< Variable > qss_vars_; // QSS values
	RefsDers< Variable > qss_ders_; // QSS derivatives
	RefsValsDers< Variable > r_vars_; // Real non-state values and derivatives
	RefsVals< Variable > ox_vars_; // Other X-based values
	RefsValsDers< Variable > zc_vars_; // Zero-crossing values and derivatives

	// QSS state handlers observees
	Variables qss_observees_; // Handlers observees
	Variables qss2_observees_; // Handlers of order 2+ observees
	Variables qss3_observees_; // Handlers of order 3+ observees

	// Real handlers observees
	Variables r_observees_; // Handlers observees
	Variables r2_observees_; // Handlers of order 2+ observees
	Variables r3_observees_; // Handlers of order 3+ observees
	VariableRefs r_observees_v_ref_; // Handlers observees value references
	Reals r_observees_dv_; // Handlers observees derivatives

	// Other X-based handlers observees
	// Variables ox_observees_; // Handlers observees

	// Zero-crossing handlers observees
	Variables zc_observees_; // Handlers observees
	Variables zc2_observees_; // Handlers of order 2+ observees
	Variables zc3_observees_; // Handlers of order 3+ observees
	VariableRefs zc_observees_v_ref_; // Handlers observees value references
	Reals zc_observees_dv_; // Handlers observees derivatives

}; // Handlers

} // QSS

#endif
