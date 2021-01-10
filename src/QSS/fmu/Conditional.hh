// FMU-Based Conditional
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2021 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_fmu_Conditional_hh_INCLUDED
#define QSS_fmu_Conditional_hh_INCLUDED

// QSS Headers
#include <QSS/Conditional.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <vector>

namespace QSS {
namespace fmu {

// Conditional Template
template< typename V >
class Conditional final : public QSS::Conditional
{

public: // Types

	using Super = QSS::Conditional;
	using Variable = V;
	using Variables = typename Variable::Variables;
	using Time = typename Variable::Time;
	using Real = typename Variable::Real;
	using size_type = typename Variables::size_type;
	using EventQ = EventQueue< Target >;

public: // Creation

	// Variable + EventQ Constructor
	Conditional(
	 Variable * var,
	 EventQ * eventq
	) :
	 var_( var ),
	 eventq_( eventq )
	{
		assert( var_ != nullptr );
		assert( eventq_ != nullptr );
		var_->conditional = this;
		add_conditional();
	}

	// Name + Variable + Event Queue Constructor
	Conditional(
	 std::string const & name,
	 Variable * var,
	 EventQ * eventq
	) :
	 QSS::Conditional( name ),
	 var_( var ),
	 eventq_( eventq )
	{
		assert( var_ != nullptr );
		assert( eventq_ != nullptr );
		var_->conditional = this;
		add_conditional();
	}

	// Destructor
	~Conditional()
	{
		if ( var_ != nullptr ) var_->conditional = nullptr;
	}

public: // Predicate

	// Empty?
	bool
	empty() const
	{
		return false;
	}

	// Valid?
	bool
	valid() const
	{
		return true;
	}

public: // Property

	// Size
	size_type
	size() const
	{
		return 1u;
	}

	// Variable
	Variable const *
	var() const
	{
		return var_;
	}

	// Variable
	Variable * &
	var()
	{
		return var_;
	}

	// Handler-Modified (Observer) Variables
	Variables const &
	observers() const
	{
		return observers_;
	}

	// Handler-Modified (Observer) Variables
	Variables &
	observers()
	{
		return observers_;
	}

	// Boolean Value at SuperdenseTime s
	bool
	b( SuperdenseTime const & s ) const
	{
		assert( var_ != nullptr );
		return var_->b( s.t );
	}

	// Boolean Value at Time t
	bool
	b( Time const t ) const
	{
		assert( var_ != nullptr );
		return var_->b( t );
	}

public: // Methods

	// Add an Observer Variable
	void
	add_observer( Variable * v )
	{
		observers_.push_back( v );
	}

	// Initialize Observers Collection
	void
	init_observers()
	{
		// Flag if output connection observers
		connected_output_observer = false;
		for ( auto const observer : observers_ ) {
			if ( observer->connected_output ) {
				connected_output_observer = true;
				break;
			}
		}
	}

	// Variable Activity Notifier
	void
	activity( Time const t )
	{
		shift_conditional( t );
	}

	// Set Observer FMU Value and Shift Handler Event
	void
	handler( Time const t )
	{
		for ( Variable * observer : observers_ ) {
			observer->fmu_set_x( t ); // Set FMU value for handler derivative dependencies
			observer->shift_handler( t );
		}
	}

	// Add Event at Time Infinity
	void
	add_conditional()
	{
		assert( eventq_ != nullptr );
		event_ = eventq_->add_conditional( this );
	}

	// Shift Event to Time Infinity
	void
	shift_conditional()
	{
		assert( eventq_ != nullptr );
		event_ = eventq_->shift_conditional( event_ );
	}

	// Shift Event to Time t
	void
	shift_conditional( Time const t )
	{
		assert( eventq_ != nullptr );
		event_ = eventq_->shift_conditional( t, event_ );
	}

	// Run Handler
	void
	advance_conditional()
	{
		assert( var_ != nullptr );
		if ( var_->b( st.t ) ) handler( st.t );
		shift_conditional();
	}

private: // Data

	Variable * var_{ nullptr }; // Event variable
	Variables observers_; // Variables dependent on this one (modified by handler)
	EventQ * eventq_{ nullptr }; // Event queue

}; // Conditional

} // fmu
} // QSS

#endif
