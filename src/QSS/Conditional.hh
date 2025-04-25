// Conditional
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2025 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_Conditional_hh_INCLUDED
#define QSS_Conditional_hh_INCLUDED

// QSS Headers
#include <QSS/Target.hh>
#include <QSS/Variable.hh>
#include <QSS/options.hh>

// C++ Headers
#include <algorithm>

namespace QSS {

// Conditional Class Template
template< typename Variable_ZC >
class Conditional final : public Target
{

public: // Types

	using Super = Target;
	using Variables = Variable::Variables;
	using VariablesSet = Variable::VariablesSet;
	using Time = Variable::Time;
	using Real = Variable::Real;
	using size_type = Variables::size_type;
	using EventQ = EventQueue< Target >;

public: // Creation

	// Default Constructor
	Conditional() = default;

	// Copy Constructor
	Conditional( Conditional const & ) = default;

	// Move Constructor
	Conditional( Conditional && ) noexcept = default;

	// Name + Variable_ZC + Event Queue Constructor
	Conditional(
	 std::string const & name,
	 Variable_ZC * var,
	 EventQ * eventq
	) :
	 Target( name ),
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

public: // Assignment

	// Copy Assignment
	Conditional &
	operator =( Conditional const & ) = default;

	// Move Assignment
	Conditional &
	operator =( Conditional && ) noexcept = default;

public: // Predicate

	// Empty?
	bool
	empty() const
	{
		return observers_.empty();
	}

	// Self Handler?
	bool
	self_handler() const
	{
		return self_handler_;
	}

public: // Property

	// Size
	size_type
	size() const
	{
		return observers_.size();
	}

	// Variable_ZC
	Variable_ZC const *
	var() const
	{
		return var_;
	}

	// Variable_ZC
	Variable_ZC * &
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

public: // Methods

	// Add an Observer Variable
	void
	add_observer( Variable * observer )
	{
		assert( observer != nullptr );
		if ( observer->is_Input() ) {
			std::cerr << "\nError: Input variable " << observer->name() << " is modified in conditional clause of " << var_->name() << std::endl;
			std::exit( EXIT_FAILURE );
		}
		observers_.push_back( observer );
	}

	// Initialize Observers Collection
	void
	init_observers()
	{
		uniquify( observers_ );
		bool const no_observers( observers_.empty() ); // No (active or passive) observers?
		short_circuit_passive_observers();
		assert( var_ != nullptr );
		if ( observers_.empty() ) { // No handlers => Passive EI
			std::cout << '\n' << var_->name() << " Conditional Computational Observers: None: Passive: " << ( no_observers ? " No Handlers" : "Passive Handler(s)" ) << std::endl;
			connected_output_observer = false;
		} else if ( ( observers_.size() == 1u ) && ( observers_[ 0 ]->is_ZC() ) ) { // Self handler => Active EI with passive observer(s)
			assert( static_cast< Variable_ZC * >( observers_[ 0 ] ) == var_ );
			std::cout << '\n' << var_->name() << " Conditional Computational Observers: None: Active" << std::endl;
			self_handler_ = true;
			connected_output_observer = false;
		} else { // Active EI with active observer(s)
			std::cout << '\n' << var_->name() << " Conditional Computational Observers:" << std::endl;
			for ( Variable const * observer : sorted_by_name( observers_ ) ) {
				std::cout << ' ' << observer->name() << std::endl;
			}
			connected_output_observer = std::any_of( observers_.begin(), observers_.end(), []( auto const observer ){ return observer->connected_output; } ); // Flag if output connection observers
		}
	}

	// Activity Notifier
	void
	activity( Time const t )
	{
		shift_conditional( t );
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
		if ( var_->is_tZ_last( st.t ) ) prep_handlers( st.t );
		shift_conditional();
	}

	// Run Handler
	void
	advance_conditional_join()
	{
		assert( var_ != nullptr );
		if ( var_->is_tZ_last( st.t ) ) prep_handlers_join( st.t );
		shift_conditional();
	}

	// Remove Associated Zero-Crossing Variable
	void
	rem_variable()
	{
		var_ = nullptr;
	}

private: // Methods

	// Short-Circuit Passive Observers
	void
	short_circuit_passive_observers()
	{
		assert( var_ != nullptr );
		if ( observers_.empty() ) { // No handler(s)
			if ( ( options::EI == 0 ) || ( options::EI == 2 ) ) { // Track EIs without handlers
				observers_.push_back( var_ ); // Make the ZC a self-handler to enable zero-crossing event processing
			}
		} else { // Handler(s) present (may be passive)
			VariablesSet observers_checked;
			VariablesSet observers_set;
			for ( Variable * observer : observers_ ) {
				if ( observer->is_ZC() ) { // ZC => Not a handler
					observers_checked.insert( observer );
				} else if ( observer->is_Active() ) { // Active => Computational
					observers_set.insert( observer );
					observers_checked.insert( observer );
				} else { // Passive: Short-circuit it
					assert( observer->is_Passive() );
					find_computational_observers( observer, observers_checked, observers_set );
				}
			}
			observers_.assign( observers_set.begin(), observers_set.end() ); // Swap in the computational observers
			if ( observers_.empty() ) { // Passive handler(s) only
				if ( options::EI < 2 ) { // Track EIs with only passive handler(s)
					observers_.push_back( var_ ); // Make the ZC a self-handler to enable zero-crossing event processing so (passive) handlers are updated at zero-crossing events
				}
			}
		}
	}

	// Find Short-Circuited Computational Observers
	void
	find_computational_observers(
	 Variable * observer,
	 VariablesSet & observers_checked,
	 VariablesSet & observers_set
	)
	{
#if ( __cplusplus >= 202002L ) // C++20+
		if ( !observers_checked.contains( observer ) ) { // Observer not already processed
#else
		if ( observers_checked.find( observer ) == observers_checked.end() ) { // Observer not already processed
#endif
			observers_checked.insert( observer );
			if ( observer->is_ZC() ) { // ZC => Not a handler
				// Done with this observer
			} else if ( observer->is_Active() ) { // Active => Computational
				observers_set.insert( observer );
			} else { // Passive: Short-circuit it
				for ( Variable * oo : observer->observers() ) { // Recurse: Traverse dependency sub-graph
					find_computational_observers( oo, observers_checked, observers_set );
				}
			}
		}
	}

	// Prepare Handlers: Set Observer FMU Value and Shift Handler Event
	void
	prep_handlers( Time const t )
	{
		for ( Variable * observer : observers_ ) {
			if ( observer->is_ZC() ) { // ZC "self-handler" is used to trigger (passive) handler events: Handler advance is not run on the ZC variable
				assert( static_cast< Variable_ZC * >( observer ) == var_ ); // The ZC variable is added as its own handler
				assert( observers_.size() == 1u ); // Only one ZC handler is added
			} else { // Set observers's observee FMU values so FMU event handler computes correct new observer value: Using continuous trajectories for best accuracy
// Setting all handler observee state now!
//				observer->fmu_set_observees_x( t ); //! Observer observees may overlap: Evaulate whether using a merged observers observees collection is typically faster
			}
// Setting all handler observee state now!
//			observer->fmu_set_x( t ); // Handler derivative, not value, may be set by the FMU event so we set the FMU value at the zero-crossing time here
			observer->shift_handler( t ); // Set observer's handler event
		}
	}

	// Prepare Handlers: Set Observer FMU Value and Shift Handler Event Joining Any Handler(s) at Front of Queue
	void
	prep_handlers_join( Time const t )
	{
		for ( Variable * observer : observers_ ) {
			if ( observer->is_ZC() ) { // ZC "self-handler" is used to trigger (passive) handler events: Handler advance is not run on the ZC variable
				assert( static_cast< Variable_ZC * >( observer ) == var_ ); // The ZC variable is added as its own handler
				assert( observers_.size() == 1u ); // Only one ZC handler is added
			} else { // Set observers's observee FMU values so FMU event handler computes correct new observer value: Using continuous trajectories for best accuracy
// Setting all handler observee state now!
//				observer->fmu_set_observees_x( t ); //! Observer observees may overlap: Evaulate whether using a merged observers observees collection is typically faster
			}
// Setting all handler observee state now!
//			observer->fmu_set_x( t ); // Handler derivative, not value, may be set by the FMU event so we set the FMU value at the zero-crossing time here
			observer->shift_handler_join( t ); // Set observer's handler event
		}
	}

private: // Data

	Variable_ZC * var_{ nullptr }; // Event indicator variable
	Variables observers_; // Variables dependent on this one (modified by handler)
	EventQ * eventq_{ nullptr }; // Event queue
	bool self_handler_{ false }; // Self handler?

}; // Conditional

} // QSS

#endif
