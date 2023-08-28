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
#include <QSS/RefsDirDers.hh>
#include <QSS/RefsVals.hh>
#include <QSS/RefsValsDers.hh>
#include <QSS/container.hh>
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
	using value_type = typename Variables::value_type;
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

		// Sort by type
		assert( is_unique( handlers_ ) ); // Precondition: No duplicates
		sort_by_type( handlers_ );

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
				for ( auto observee : handler->observees() ) {
					qss_observees_.push_back( observee );
				}
			}
			uniquify( qss_observees_ );
			n_qss_observees_ = qss_observees_.size();
		} else {
			n_qss_observees_ = 0u;
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
			n_r_observees_ = r_observees_.size();
		} else {
			n_r_observees_ = 0u;
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
		// 	n_ox_observees_ = ox_observees_.size();
		// } else {
		// 	n_ox_observees_ = 0u;
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
			n_zc_observees_ = zc_observees_.size();
		} else {
			n_zc_observees_ = 0u;
		}

		// Set up handlers observees pooled derivative data /////

		// QSS handlers
		if ( qss_.have() ) {
			qss_observees_v_ref_.clear(); qss_observees_v_ref_.reserve( n_qss_observees_ );
			qss_observees_v_.clear(); qss_observees_v_.resize( n_qss_observees_ );
			qss_observees_dv_.clear(); qss_observees_dv_.resize( n_qss_observees_ );
			for ( auto observee : qss_observees_ ) {
				qss_observees_v_ref_.push_back( observee->var().ref() );
			}
		}

		// Real handlers
		if ( r_.have() ) {
			r_observees_v_ref_.clear(); r_observees_v_ref_.reserve( n_r_observees_ );
			r_observees_v_.clear(); r_observees_v_.resize( n_r_observees_ );
			r_observees_dv_.clear(); r_observees_dv_.resize( n_r_observees_ );
			for ( auto observee : r_observees_ ) {
				r_observees_v_ref_.push_back( observee->var().ref() );
			}
		}

		// Zero-crossing handlers
		if ( zc_.have() ) {
			zc_observees_v_ref_.clear(); zc_observees_v_ref_.reserve( n_zc_observees_ );
			zc_observees_v_.clear(); zc_observees_v_.resize( n_zc_observees_ );
			zc_observees_dv_.clear(); zc_observees_dv_.resize( n_zc_observees_ );
			for ( auto observee : zc_observees_ ) {
				zc_observees_v_ref_.push_back( observee->var().ref() );
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
		if ( handlers_.empty() ) return;

		connected_output_handler_ = false;
		all_.b() = 0u;
		all_.e() = handlers_.size();
		size_type i( 0u );
		for ( Variable * handler : handlers_ ) {
			order_ = std::max( order_, handler->order() ); // Since ZC are sorted after B|I|D|Passive|Input we can't just look at the first variable (in case of strange model with ZC vars but no QSS vars)
		}

		// QSS state handlers
		while ( ( i < all_.e() ) && ( handlers_[ i ]->is_QSS() ) ) {
			Variable const * handler( handlers_[ i ] );
			assert( handler->order() == order_ );
			qss_.b() = std::min( qss_.b(), i );
			if ( handler->connected_output ) connected_output_handler_ = true;
			++i;
		}
		if ( qss_.began() ) {
			qss_.e() = i;
		}

		// Non-state handlers
		if ( i < all_.e() ) {
			ns_.b() = i;
			ns_.e() = all_.e();
		}

		// Real handlers
		while ( ( i < all_.e() ) && ( handlers_[ i ]->is_R() ) ) {
			Variable const * handler( handlers_[ i ] );
			assert( handler->order() == order_ );
			r_.b() = std::min( r_.b(), i );
			if ( handler->connected_output ) connected_output_handler_ = true;
			++i;
		}
		if ( r_.began() ) {
			r_.e() = i;
		}

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
			assert( handler->order() == order_ );
			zc_.b() = std::min( zc_.b(), i );
			if ( handler->connected_output ) connected_output_handler_ = true;
			++i;
		}
		if ( zc_.began() ) {
			zc_.e() = i;
		}
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

		fmu_me_->get_reals( qss_.n(), qss_vars_.refs.data(), qss_vars_.vals.data() );
		for ( size_type i = qss_.b(), e = qss_.e(), j = 0u; i < e; ++i, ++j ) { // Handler advance stage 0
			assert( handlers_[ i ]->is_QSS() );
			handlers_[ i ]->advance_handler_0( t, qss_vars_.vals[ j ] );
		}

		fmu_me_->get_reals( qss_.n(), qss_ders_.refs.data(), qss_ders_.ders.data() );
		for ( size_type i = qss_.b(), e = qss_.e(), j = 0u; i < e; ++i, ++j ) { // Handler advance stage 1
			handlers_[ i ]->advance_handler_1( qss_ders_.ders[ j ] );
		}

		if ( order_ >= 2 ) {
			set_qss_observees_dv( t );
			fmu_me_->get_directional_derivatives(
			 qss_observees_v_ref_.data(),
			 n_qss_observees_,
			 qss_ders_.refs.data(),
			 qss_.n(),
			 qss_observees_dv_.data(),
			 qss_ders_.ders.data()
			); // Get 2nd derivatives at t
			for ( size_type i = qss_.b(), e = qss_.e(), j = 0u; i < e; ++i, ++j ) { // Handler advance stage 2
				handlers_[ i ]->advance_handler_2_dd2( qss_ders_.ders[ j ] );
			}
			if ( order_ >= 3 ) {
				Time const tN( t + options::dtND );
				fmu_me_->set_time( tN );
				set_qss_observees_values( tN );
				set_qss_observees_dv( tN );
				fmu_me_->get_directional_derivatives(
				 qss_observees_v_ref_.data(),
				 n_qss_observees_,
				 qss_ders_.refs.data(),
				 qss_.n(),
				 qss_observees_dv_.data(),
				 qss_ders_.ders.data()
				); // Get 2nd derivatives at t + dtND
				for ( size_type i = qss_.b(), e = qss_.e(), j = 0u; i < e; ++i, ++j ) { // Handler advance stage 3
					handlers_[ i ]->advance_handler_3_dd2( qss_ders_.ders[ j ] );
				}
				fmu_me_->set_time( t );
			}
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

		fmu_me_->get_reals( r_.n(), r_vars_.refs.data(), r_vars_.vals.data() );
		for ( size_type i = r_.b(), e = r_.e(), j = 0u; i < e; ++i, ++j ) { // Handler advance stage 0
			assert( handlers_[ i ]->is_Active() );
			assert( handlers_[ i ]->is_R() );
			handlers_[ i ]->advance_handler_0( t, r_vars_.vals[ j ] );
		}

		set_r_observees_dv( t );
		fmu_me_->get_directional_derivatives(
		 r_observees_v_ref_.data(),
		 n_r_observees_,
		 r_vars_.refs.data(),
		 r_.n(),
		 r_observees_dv_.data(),
		 r_vars_.ders.data()
		); // Get derivatives at t
		for ( size_type i = r_.b(), e = r_.e(), j = 0u; i < e; ++i, ++j ) { // Handler advance stage 1
			handlers_[ i ]->advance_handler_1( r_vars_.ders[ j ] );
		}

		if ( order_ >= 3 ) {
			Time tN( t - options::dtND );
			if ( fwd_time( tN ) ) { // Use centered ND formulas
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
				for ( size_type i = r_.b(), e = r_.e(), j = 0u; i < e; ++i, ++j ) { // Handler advance stage 2
					handlers_[ i ]->advance_handler_2( r_vars_.ders[ j ], r_vars_.ders_p[ j ] );
				}
				for ( size_type i = r_.b(), e = r_.e(); i < e; ++i ) { // Handler advance stage 3
					handlers_[ i ]->advance_handler_3();
				}
			} else { // Use forward ND formulas
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
				for ( size_type i = r_.b(), e = r_.e(), j = 0u; i < e; ++i, ++j ) { // Handler advance stage 2
					handlers_[ i ]->advance_handler_2_forward( r_vars_.ders[ j ], r_vars_.ders_p[ j ] );
				}
				for ( size_type i = r_.b(), e = r_.e(); i < e; ++i ) { // Handler advance stage 3
					handlers_[ i ]->advance_handler_3_forward();
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
			for ( size_type i = r_.b(), e = r_.e(), j = 0u; i < e; ++i, ++j ) { // Handler advance stage 2
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

		fmu_me_->get_reals( ox_.n(), ox_vars_.refs.data(), ox_vars_.vals.data() );
		for ( size_type i = ox_.b(), e = ox_.e(), j = 0u; i < e; ++i, ++j ) {
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
		assert( fmu_me_->has_event_indicators );
		assert( zc_.n() == zc_vars_.size() );

		fmu_me_->get_reals( zc_.n(), zc_vars_.refs.data(), zc_vars_.vals.data() );
		for ( size_type i = zc_.b(), e = zc_.e(), j = 0u; i < e; ++i, ++j ) { // Handler advance stage 0
			handlers_[ i ]->advance_handler_0( t, zc_vars_.vals[ j ] );
		}

		set_zc_observees_dv( t );
		fmu_me_->get_directional_derivatives(
		 zc_observees_v_ref_.data(),
		 n_zc_observees_,
		 zc_vars_.refs.data(),
		 zc_.n(),
		 zc_observees_dv_.data(),
		 zc_vars_.ders.data()
		); // Get derivatives at t
		for ( size_type i = zc_.b(), e = zc_.e(), j = 0u; i < e; ++i, ++j ) { // Handler advance stage 1
			assert( handlers_[ i ]->is_ZC() );
			handlers_[ i ]->advance_handler_1( zc_vars_.ders[ j ] );
		}

		if ( order_ >= 3 ) {
			Time tN( t - options::dtND );
			if ( fwd_time( tN ) ) { // Use centered ND formulas
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
				for ( size_type i = zc_.b(), e = zc_.e(), j = 0u; i < e; ++i, ++j ) { // Handler advance stage 2
					handlers_[ i ]->advance_handler_2( zc_vars_.ders[ j ], zc_vars_.ders_p[ j ] );
				}
				for ( size_type i = zc_.b(), e = zc_.e(); i < e; ++i ) { // Handler advance stage 3
					handlers_[ i ]->advance_handler_3();
				}
			} else { // Use forward ND formulas
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
				for ( size_type i = zc_.b(), e = zc_.e(), j = 0u; i < e; ++i, ++j ) { // Handler advance stage 2
					handlers_[ i ]->advance_handler_2_forward( zc_vars_.ders[ j ], zc_vars_.ders_p[ j ] );
				}
				for ( size_type i = zc_.b(), e = zc_.e(); i < e; ++i ) { // Handler advance stage 3
					handlers_[ i ]->advance_handler_3_forward();
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
			for ( size_type i = zc_.b(), e = zc_.e(), j = 0u; i < e; ++i, ++j ) { // Handler advance stage 2
				handlers_[ i ]->advance_handler_2( zc_vars_.ders_p[ j ] );
			}
			fmu_me_->set_time( t );
		}
	}

	// Advance QSS State Handlers: Stage Final
	void
	advance_QSS_F( Time const t )
	{
		for ( size_type i = 0u, e = qss_.e(); i < e; ++i ) {
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

	// Set QSS Observees Derivative Vector at Time t
	void
	set_qss_observees_dv( Time const t )
	{
		for ( size_type i = 0u; i < n_qss_observees_; ++i ) {
#ifndef QSS_PROPAGATE_CONTINUOUS
			qss_observees_dv_[ i ] = qss_observees_[ i ]->q1( t ); // Quantized: Traditional QSS
#else
			qss_observees_dv_[ i ] = qss_observees_[ i ]->x1( t ); // Continuous: Modified QSS
#endif
		}
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

	// Set Real Observees Derivative Vector at Time t
	void
	set_r_observees_dv( Time const t )
	{
		for ( size_type i = 0u; i < n_r_observees_; ++i ) {
			r_observees_dv_[ i ] = r_observees_[ i ]->x1( t );
		}
	}

	// Set Zero-Crossing Observees FMU Values at Time t
	void
	set_zc_observees_values( Time const t )
	{
		for ( size_type i = 0u, e = n_zc_observees_; i < e; ++i ) { // Set observee value vector
			zc_observees_v_[ i ] = zc_observees_[ i ]->x( t );
		}
		fmu_me_->set_reals( zc_observees_.size(), zc_observees_v_ref_.data(), zc_observees_v_.data() ); // Set observees FMU values
	}

	// Set Zero-Crossing Observees Derivative Vector at Time t
	void
	set_zc_observees_dv( Time const t )
	{
		for ( size_type i = 0u, e = n_zc_observees_; i < e; ++i ) {
			zc_observees_dv_[ i ] = zc_observees_[ i ]->x1( t );
		}
	}

private: // Data

	FMU_ME * fmu_me_{ nullptr }; // FMU-ME (non-owning) pointer

	Variables handlers_; // Handlers

	bool connected_output_handler_{ false }; // Output connection handler to another FMU?

	// Handler order
	int order_{ 0 };

	// Handler index ranges
	Range all_; // All handlers
	Range qss_; // QSS state handlers
	Range ns_; // Non-state (X-based) handlers
	Range r_; // Real handlers
	Range ox_; // Other X-based handlers
	Range zc_; // Zero-crossing handlers

	// Handler FMU pooled call data
	RefsVals< Variable > qss_vars_; // QSS values
	RefsDirDers< Variable > qss_ders_; // QSS derivatives
	RefsValsDers< Variable > r_vars_; // Real non-state values and derivatives
	RefsVals< Variable > ox_vars_; // Other X-based values
	RefsValsDers< Variable > zc_vars_; // Zero-crossing values and derivatives

	// QSS state handlers observees
	size_type n_qss_observees_{ 0u }; // Number of QSS handlers observees
	Variables qss_observees_; // QSS handlers observees
	VariableRefs qss_observees_v_ref_; // QSS handlers observees value references
	Reals qss_observees_v_; // QSS handlers observees values
	Reals qss_observees_dv_; // QSS handlers observees derivatives

	// Real handlers observees
	size_type n_r_observees_{ 0u }; // Number of real handlers observees
	Variables r_observees_; // Real handlers observees
	VariableRefs r_observees_v_ref_; // Real handlers observees value references
	Reals r_observees_v_; // Real handlers observees values
	Reals r_observees_dv_; // Real handlers observees derivatives

	// Zero-crossing handlers observees
	size_type n_zc_observees_{ 0u }; // Number of zero-crossing handlers observees
	Variables zc_observees_; // Zero-crossing handlers observees
	VariableRefs zc_observees_v_ref_; // Zero-crossing handlers observees value references
	Reals zc_observees_v_; // Zero-crossing handlers observees values
	Reals zc_observees_dv_; // Zero-crossing handlers observees derivatives

}; // Handlers

} // QSS

#endif
