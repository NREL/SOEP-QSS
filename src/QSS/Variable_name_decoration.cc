// Variable Name Decoration for Case-Insentive File Systems
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2024 Objexx Engineering, Inc. All rights reserved.
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
#include <QSS/Variable_name_decoration.hh>
#include <QSS/Variable.hh>
#include <QSS/string.hh>

// C++ Headers
#include <algorithm>

namespace QSS {

// Types
using Variables = std::vector< Variable * >;

// Set Variable Output File Name Decoration for Case-Insensitive Collisions
void
name_decorate( Variables & vars )
{
	// Types
	using NameCounts = std::unordered_map< std::string, Variable::size_type >;
	using VariableCounts = std::unordered_map< Variable *, Variable::size_type >;

	// Create name and variable counter map for each case-insensitive name
	NameCounts name_counts;
	for ( Variable const * var : vars ) { // Initialize name count
		name_counts[ uppercased( var->name() ) ] = 0u;
	}
	VariableCounts var_counts;
	for ( Variable * var : vars ) { // Increment name count and set variable count // Assumes variables are in deterministic order from FMU
		var_counts[ var ] = ( name_counts[ uppercased( var->name() ) ] += 1u );
	}

	// Add decoration for variables with colliding names
	for ( Variable * var : vars ) { // Increment name count and set variable count
		if ( name_counts[ uppercased( var->name() ) ] > 1u ) { // Has collisions: decorate
			std::string dec( '.' + std::to_string( var_counts[ var ] ) );
			while ( std::find_if( vars.begin(), vars.end(), [ &var, &dec ]( Variable * v ){ return v->name() == var->name() + dec; } ) != vars.end() ) {
				dec += '_'; // In case the count-appended name conflicts
			}
			var->decorate_out( dec );
		}
	}
}

// Types
using Names = std::vector< std::string >;
using Decs = std::unordered_map< std::string, std::string >;

void
name_decorations( Names & names, Decs & decs )
{
	// Types
	using Name = std::string;
	using NameCounts = std::unordered_map< std::string, std::string::size_type >;
	using VariableCounts = std::unordered_map< std::string, Variable::size_type >;

	// Initialize colletions
	std::sort( names.begin(), names.end() ); // For deterministic counts
	decs.clear();

	// Create name and variable counter map for each case-insensitive name
	NameCounts name_counts;
	for ( Name const & name : names ) { // Initialize name count
		name_counts[ uppercased( name ) ] = 0u;
	}
	VariableCounts var_counts;
	for ( Name const & name : names ) { // Increment name count
		var_counts[ name ] = ( name_counts[ uppercased( name ) ] += 1u );
	}

	// Add decoration for variables with colliding names
	for ( Name const & name : names ) { // Increment name count
		std::string dec;
		if ( name_counts[ uppercased( name ) ] > 1u ) { // Has collisions: decorate
			dec = '.' + std::to_string( var_counts[ name ] );
			while ( std::find_if( names.begin(), names.end(), [ &name, &dec ]( Name const & nam ){ return nam == name + dec; } ) != names.end() ) {
				dec += '_'; // In case the count-appended name conflicts
			}
		}
		decs[ name ] = dec;
	}
}

} // QSS
