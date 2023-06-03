// QSS Solver Container Functions
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

#ifndef QSS_container_hh_INCLUDED
#define QSS_container_hh_INCLUDED

// QSS Headers
#include <QSS/options.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <limits>
#include <type_traits>
#include <vector>

namespace QSS {

// Variables Sorted by Order?
template< typename Variables >
bool
is_sorted_by_order( Variables & variables )
{
	using V = typename std::remove_pointer< typename Variables::value_type >::type;
	return std::is_sorted( variables.begin(), variables.end(), []( V const * v1, V const * v2 ){ return v1->order() < v2->order(); } );
}

// Variables Collection Unique?
template< typename Variables >
bool
is_unique( Variables const & variables )
{
	if ( ( variables.empty() ) || ( variables.size() == 1u ) ) return true;
	if ( std::is_sorted( variables.begin(), variables.end() ) ) { // Sorted
		for ( typename Variables::size_type i = 0, e = variables.size() - 1u; i < e; ++i ) {
			if ( variables[ i ] == variables[ i + 1 ] ) return false; // Duplicates
		}
		return true; // No duplicates
	} else { // Not sorted: Slow to avoid sorting side effect!
		Variables vars_sorted( variables );
		std::sort( vars_sorted.begin(), vars_sorted.end() ); // Sort by address
		for ( typename Variables::size_type i = 0, e = vars_sorted.size() - 1u; i < e; ++i ) {
			if ( vars_sorted[ i ] == vars_sorted[ i + 1 ] ) return false; // Duplicates
		}
		return true; // No duplicates
	}
}

// Variables Begin Index of Given Order or Greater
template< typename Variables >
typename Variables::size_type
begin_order_index( Variables const & variables, int const order )
{
	using V = typename std::remove_pointer< typename Variables::value_type >::type;
	assert( is_sorted_by_order( variables ) ); // Require sorted by order
	return static_cast< typename Variables::size_type >( std::distance( variables.begin(), std::lower_bound( variables.begin(), variables.end(), order, []( V const * v, int const o ){ return v->order() < o; } ) ) );
}

// Make Variables Collection Unique and Optionally Shrink-to-Fit
template< typename Variables >
void
uniquify( Variables & variables, bool const shrink = false )
{
	if ( !variables.empty() ) {
		std::sort( variables.begin(), variables.end() ); // Sort
		variables.erase( std::unique( variables.begin(), variables.end() ), variables.end() ); // Remove duplicates
		if ( shrink ) variables.shrink_to_fit();
	}
}

// Make Sorted Variables Collection Unique and Optionally Shrink-to-Fit
template< typename Variables >
void
uniquify_sorted( Variables & variables, bool const shrink = false )
{
	if ( !variables.empty() ) {
		assert( std::is_sorted( variables.begin(), variables.end() ) ); // Precondition: Sorted
		variables.erase( std::unique( variables.begin(), variables.end() ), variables.end() ); // Remove duplicates
		if ( shrink ) variables.shrink_to_fit();
	}
}

// Sort Variables by Order
template< typename Variables >
void
sort_by_order( Variables & variables )
{
	using V = typename std::remove_pointer< typename Variables::value_type >::type;
	// if ( options::output::d ) { // Also sort by name for deterministic diagnostic output and order-dependent results
		std::sort( variables.begin(), variables.end(), []( V const * v1, V const * v2 ){ return ( v1->order() < v2->order() ) || ( ( v1->order() == v2->order() ) && ( v1->name() < v2->name() ) ); } );
	// } else { // Just sort by order (faster)
	// 	std::sort( variables.begin(), variables.end(), []( V const * v1, V const * v2 ){ return v1->order() < v2->order(); } );
	// }
}

// Sort Variables by Type (State First) and Order
template< typename Variables >
void
sort_by_type_and_order( Variables & variables )
{
	using V = typename std::remove_pointer< typename Variables::value_type >::type;
	// if ( options::output::d ) { // Also sort by name for deterministic diagnostic output and order-dependent results
		std::sort( variables.begin(), variables.end(), []( V const * v1, V const * v2 ){ return ( v1->var_sort_index() < v2->var_sort_index() ) || ( ( v1->var_sort_index() == v2->var_sort_index() ) && ( v1->name() < v2->name() ) ); } );
	// } else { // Just sort by type and order (faster)
	// 	std::sort( variables.begin(), variables.end(), []( V const * v1, V const * v2 ){ return v1->var_sort_index() < v2->var_sort_index(); } );
	// }
}

// Sort Variables by Name
template< typename Variables >
Variables
sorted_by_name( Variables const & variables )
{
	using V = typename std::remove_pointer< typename Variables::value_type >::type;
	Variables sorted_variables( variables );
	std::sort( sorted_variables.begin(), sorted_variables.end(), []( V const * v1, V const * v2 ){ return v1->name() < v2->name(); } );
	return sorted_variables;
}

// Set up Non-Trigger Observers of Triggers and Sort Both by Order
template< typename Variables >
void
variables_observers( Variables & triggers, Variables & observers )
{
	using V = typename std::remove_pointer< typename Variables::value_type >::type;

	// Combine all non-trigger observers
	observers.clear();
	if ( triggers.size() < 16 ) { // Linear search
		for ( V * trigger : triggers ) {
			for ( V * observer : trigger->observers() ) {
				if ( std::find( triggers.begin(), triggers.end(), observer ) == triggers.end() ) observers.push_back( observer );
			}
		}
	} else { // Binary search
		std::sort( triggers.begin(), triggers.end() ); // Side effect!
		for ( V * trigger : triggers ) {
			for ( V * observer : trigger->observers() ) {
				if ( !std::binary_search( triggers.begin(), triggers.end(), observer ) ) observers.push_back( observer );
			}
		}
	}

	// Remove duplicates and sort by type and order
	if ( !observers.empty() ) {
		std::sort( observers.begin(), observers.end() ); // Sort by address
		observers.erase( std::unique( observers.begin(), observers.end() ), observers.end() ); // Remove duplicates
		//if ( shrink ) observers.shrink_to_fit(); // Since we always reuse these containers we don't need a shrink option
		if ( !observers.empty() ) sort_by_type_and_order( observers );
	}

	// Sort triggers by order
	sort_by_order( triggers );
}

// Vector Has a Value?
template< typename T >
bool
vector_has( std::vector< T > const & c, T const & t )
{
	return std::find( c.begin(), c.end(), t ) != c.end();
}

// Remove Elements of a Vector with a Value
template< typename T >
void
vector_remove_value( std::vector< T > & c, T const & t )
{
	typename std::vector< T >::iterator i( std::find( c.begin(), c.end(), t ) );
	if ( i != c.end() ) c.erase( i );
}

// Nullify Elements of a Vector with a Value
template< typename T >
void
vector_nullify_value( std::vector< T > & c, T const & t )
{
	typename std::vector< T >::iterator i( std::find( c.begin(), c.end(), t ) );
	if ( i != c.end() ) *i = nullptr;
}

// Remove Elements of a Vector with a Value
template< class C, typename T >
void
map_remove_value( C & c, T const & t )
{
	for ( typename C::iterator i = c.begin(); i != c.end(); ) {
		if ( i->second == t ) {
			i = c.erase( i );
		} else {
			++i;
		}
	}
}

} // QSS

#endif
