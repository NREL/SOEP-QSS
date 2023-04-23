// Variable Abstract Base Class
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

// QSS Headers
#include <QSS/Variable.hh>
#include <QSS/Variable_Con.hh>
#include <QSS/Variable_ZC.hh>

// C++ Headers
#include <cassert>

namespace QSS {

	// Add Observee and its Observer
	void
	Variable::
	observe( Variable * v )
	{
		assert( v != nullptr );
		if ( v->is_ZC() ) { // Observing zero-crossing variable means being an observer of (modified within) its conditional
			Variable_ZC * zc( static_cast< Variable_ZC * >( v ) );
			zc->add_observer( this ); // Conditional is not an observee of this variable
		} else if ( v == this ) { // Flag as self-observer
			self_observer_ = true;
		} else { // Bidirectional observer/observee relationship
			observees_.push_back( v );
			v->observers_.add( this );
		}
	}

	// Uniquify Observees
	void
	Variable::
	uniquify_observees()
	{
		observes_ = !observees_.empty();
		if ( observes_ ) uniquify( observees_ ); // Sort by address and remove duplicates
	}

	// Initialize Observees
	void
	Variable::
	init_observees()
	{
		// Observees setup
		VariablesSet observees_checked;
		VariablesSet observees_set;
		find_computational_observees( observees_, observees_checked, observees_set ); // Note: Looks at other variable observees that aren't necessarily uniquified yet but that is OK: Might be more efficient to make this a separate phase after all are uniquified
		observees_.clear();
		if ( !observees_set.empty() ) {
			for ( Variable * observee : observees_set ) {
				if ( observee == this ) {
					self_observer_ = true;
				} else {
					observees_.push_back( observee );
				}
			}
		}
		observes_ = !observees_.empty();
		if ( observes_ ) uniquify( observees_, true ); // Sort by address and remove duplicates and recover unused memory
		std::cout << '\n' << name() << " Computational Observees:" << std::endl;
		if ( self_observer_ ) std::cout << ' ' << name() << std::endl;
		for ( Variable const * observee : sorted_by_name( observees_ ) ) {
			std::cout << ' ' << observee->name() << std::endl;
		}

#ifndef NDEBUG
		// Check for duplicate value references
		std::vector< VariableRef > observees_refs;
		observees_refs.reserve( observees_.size() );
		for ( Variable * observee : observees_ ) {
			observees_refs.push_back( observee->var_.ref() );
		}
		std::sort( observees_refs.begin(), observees_refs.end() );
		assert( std::adjacent_find( observees_refs.begin(), observees_refs.end() ) == observees_refs.end() ); // No repeat value references
#endif

		// FMU directional derivative call argument initialization
		assert( observees_v_ref_.empty() ); // Initialization shouldn't be called twice for a variable
		assert( observees_dv_.empty() ); // Initialization shouldn't be called twice for a variable
		observees_nv_ = observees_.size();
		observees_v_ref_.reserve( observees_nv_ );
		observees_dv_.reserve( observees_nv_ );
		for ( Variable * observee : observees_ ) {
			observees_v_ref_.push_back( observee->var_.ref() );
			observees_dv_.push_back( 0.0 ); // Actual values assigned when getting directional derivatives
		}
	}

	// Initialize Observers
	void
	Variable::
	init_observers()
	{
		observers_.set_computational_observers();
	}

	// Finalize Observers
	void
	Variable::
	finalize_observers()
	{
		observers_.assign_computational_observers();
		if ( is_Active() ) { // Passive variable observers are only used for short-circuiting around them so we don't show them here
			std::cout << '\n' << name() << " Computational Observers:" << std::endl;
			for ( Variable const * observer : sorted_by_name( observers_ ) ) {
				std::cout << ' ' << observer->name() << std::endl;
			}
		}
		observers_.init();
		observed_ = observers_.have();
		connected_output_observer = observers_.connected_output_observer();
	}

	// Advance Connections
	void
	Variable::
	advance_connections()
	{
		for ( Variable_Con * connection : connections_ ) {
			connection->advance_connection( tQ );
		}
	}

	// Advance Connections for Observer Update
	void
	Variable::
	advance_connections_observer()
	{
		for ( Variable_Con * connection : connections_ ) {
			connection->advance_connection_observer();
		}
	}

	// Decorate Outputs
	void
	Variable::
	decorate_out( std::string const & dec )
	{
		dec_ = dec;
		if ( out_on_ ) {
			if ( options::output::X ) out_x_.decorate( dec );
			if ( is_Active() ) {
				if ( options::output::Q ) out_q_.decorate( dec );
				if ( options::output::T ) out_t_.decorate( dec );
			}
		}
	}

	// Initialize Outputs
	void
	Variable::
	init_out( std::string const & dir, std::string const & dec )
	{
		if ( out_on_ ) {
			if ( options::output::X ) out_x_.init( dir, name(), 'x', dec );
			if ( is_Active() ) {
				if ( options::output::Q ) out_q_.init( dir, name(), 'q', dec );
				if ( options::output::T ) out_t_.init( dir, name(), 't', dec );
			}
			if ( options::output::h ) {
				if ( var_.is_Real() ) {
					char const * var_type_char( fmi2_import_get_real_variable_quantity( var_.rvr ) );
					std::string const var_type( var_type_char == nullptr ? "" : var_type_char );
					fmi2_import_unit_t * const var_unit_ptr( fmi2_import_get_real_variable_unit( var_.rvr ) );
					std::string const var_unit( var_unit_ptr == nullptr ? "" : fmi2_import_get_unit_name( var_unit_ptr ) );
					if ( options::output::X ) out_x_.header( var_type, var_unit );
					if ( is_Active() ) {
						if ( options::output::Q ) out_q_.header( var_type, var_unit );
						if ( options::output::T ) out_t_.header( "Time", "s" );
					}
				} else if ( var_.is_Integer() ) { // Integer variables have no unit
					char const * var_type_char( fmi2_import_get_integer_variable_quantity( var_.ivr ) );
					std::string const var_type( var_type_char == nullptr ? "" : var_type_char );
					if ( options::output::X ) out_x_.header( var_type );
					if ( is_Active() ) {
						if ( options::output::Q ) out_q_.header( var_type );
					}
				} else { // Modelica Boolean variables can have a quantity but there is no FMIL API for getting it
					if ( options::output::X ) out_x_.header();
					if ( is_Active() ) {
						if ( options::output::Q ) out_q_.header();
					}
				}
			}
		}
	}

	// Connections Output at Time t
	void
	Variable::
	connections_out( Time const t )
	{
		for ( Variable_Con * connection : connections_ ) {
			connection->out( t );
		}
	}

	// Connections Output at Time t
	void
	Variable::
	connections_out_q( Time const t )
	{
		for ( Variable_Con * connection : connections_ ) {
			connection->out_q( t );
		}
	}

	// Connections Pre-Event Observer Output at Time t
	void
	Variable::
	connections_observer_out_pre( Time const t )
	{
		for ( Variable_Con * connection : connections_ ) {
			connection->observer_out_pre( t );
		}
	}

	// Connections Post-Event Observer Output at Time t
	void
	Variable::
	connections_observer_out_post( Time const t )
	{
		for ( Variable_Con * connection : connections_ ) {
			connection->observer_out_post( t );
		}
	}

} // QSS
