// FMU-ME Dependencies Support
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

#ifndef QSS_Dependencies_hh_INCLUDED
#define QSS_Dependencies_hh_INCLUDED

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <unordered_map>
#include <utility> // std::move
#include <vector>

namespace QSS {
namespace dep {

// FMU Variable Dependencies
struct Variable final
{
	// Types
	using size_type = std::size_t;
	using Index = std::size_t;
	using Observees = std::vector< Index >;

	// Default Constructor
	Variable() = default;

	// Index Constructor
	explicit
	Variable( Index const idx_ ) :
	 idx( idx_ )
	{}

	// Empty?
	bool
	empty() const
	{
		return observees.empty();
	}

	// Self Observer?
	bool
	self_observer() const
	{
		return has_observee( idx );
	}

	// Has Observee?
	bool
	has_observee( Index const idx_ ) const
	{
		assert( std::is_sorted( observees.begin(), observees.end() ) ); // Require sorted
		Observees::const_iterator const i( std::lower_bound( observees.begin(), observees.end(), idx_, []( Index const & observee_idx, Index const & index ){ return observee_idx < index; } ) );
		return ( ( i != observees.end() ) && ( *i == idx_ ) );
	}

	// Variable Index
	Index
	index() const
	{
		return idx;
	}

	// Set Variable Index
	void
	index( Index const idx_ )
	{
		idx = idx_;
	}

	// Add an Observee by Index
	void
	add_observee( Index const idx_ )
	{
		observees.push_back( idx_ );
	}

	// Sort Dependencies
	void
	sort()
	{
		std::sort( observees.begin(), observees.end() );
	}

	// Make Variables Collection Unique and Optionally Shrink-to-Fit
	void
	sort_and_uniquify()
	{
		std::sort( observees.begin(), observees.end() ); // Sort
		observees.erase( std::unique( observees.begin(), observees.end() ), observees.end() ); // Remove duplicates
	}

	// Less Than Comparison for Sorting
	friend
	bool
	operator <( Variable const & a, Variable const & b )
	{
		return a.idx < b.idx;
	}

	// Data
	Index idx{ 0u }; // FMU variable index
	Observees observees; // FMU variable's observees

}; // Variable

} // dep

// FMU Variable Dependencies Collection
struct FMU_Dependencies final
{
	// Types
	using Variable = dep::Variable;
	using Index = Variable::Index;
	using Variables = std::unordered_map< Index, Variable >;
	using Observees = Variable::Observees;
	using const_iterator = Variables::const_iterator;
	using iterator = Variables::iterator;
	using key_type = Variables::key_type;
	using value_type = Variables::value_type;

public: // Creation

	// Constructor
	explicit
	FMU_Dependencies( void * context_ ) :
	 context( context_ )
	{}

public: // Predicate

	// Empty?
	bool
	empty() const
	{
		return variables.empty();
	}

	// Has a Variable?
	bool
	has( Index const idx ) const
	{
		return ( variables.find( idx ) != variables.end() );
	}

public: // Property

	// Find a Variable
	const_iterator
	find( Index const idx ) const
	{
		return variables.find( idx );
	}

	// Find a Variable
	iterator
	find( Index const idx )
	{
		return variables.find( idx );
	}

	// Get a Variable
	Variable const &
	get( Index const idx ) const
	{
		assert( has( idx ) ); // Require has index
		return variables.at( idx );
	}

	// Get a Variable
	Variable &
	get( Index const idx )
	{
		assert( has( idx ) ); // Require has index
		return variables.at( idx );
	}

public: // Subscript

	// Get a Variable
	Variable const &
	operator []( Index const idx ) const
	{
		assert( has( idx ) ); // Require has index
		return variables.at( idx );
	}

	// Get a Variable
	Variable &
	operator []( Index const idx )
	{
		assert( has( idx ) ); // Require has index
		return variables.at( idx );
	}

public: // Iterator

	// Begin Iterator
	const_iterator
	begin() const
	{
		return variables.begin();
	}

	// Begin Iterator
	iterator
	begin()
	{
		return variables.begin();
	}

	// End Iterator
	const_iterator
	end() const
	{
		return variables.end();
	}

	// End Iterator
	iterator
	end()
	{
		return variables.end();
	}

public: // Methods

	// Add a Variable by Index
	void
	add( Index const idx )
	{
		variables.emplace( idx, idx );
	}

	// Add a Variable
	void
	add( Variable const & var )
	{
		variables.emplace( var.idx, var );
	}

	// Add a Variable
	void
	add( Variable && var )
	{
		variables.emplace( var.idx, std::move( var ) );
	}

	// Sort Dependencies by Index
	void
	sort()
	{
		for ( Variables::value_type & idx_variable : variables ) idx_variable.second.sort();
	}

	// Sort Dependencies by Index
	void
	sort_and_uniquify()
	{
		for ( Variables::value_type & idx_variable : variables ) idx_variable.second.sort_and_uniquify();
	}

	// Finalize
	void
	finalize()
	{
		// sort(); // Sort dependencies by index
		sort_and_uniquify(); // Sort dependencies by index and uniquify: Need uniquify for EI->EI dependency hack
	}

	// Data
	Variables variables;
	bool inDependencies{ false };
	void * context{ nullptr }; // Context pointer to its FMU-ME

}; // FMU_Dependencies

// Dependencies Global Lookup by FMU-ME Context
namespace { // Pollution control
using All_Dependencies = std::vector< FMU_Dependencies >;
}
extern All_Dependencies all_dependencies;

} // QSS

#endif
