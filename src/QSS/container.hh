// QSS Solver Container Functions
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

// Variables All the Same Order?
template< typename Variables >
bool
all_same_order( Variables const & variables )
{
	using V = typename std::remove_pointer< typename Variables::value_type >::type;
	if ( variables.empty() ) {
		return true;
	} else {
		int order( variables[ 0 ]->order() );
		return std::all_of( variables.begin(), variables.end(), [ order ]( V const * v ){ return v->order() == order; } );
	}
}

// Variables Collection Unique?
template< typename Variables >
bool
is_unique( Variables const & variables )
{
	if ( variables.size() <= 1u ) return true;
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

// Make Variables Collection Unique and Sorted and Optionally Shrink-to-Fit
template< typename Variables >
void
uniquify( Variables & variables, bool const shrink = false )
{
	if ( variables.size() > 1u ) {
		std::sort( variables.begin(), variables.end() ); // Sort
		variables.erase( std::unique( variables.begin(), variables.end() ), variables.end() ); // Remove duplicates
	}
	if ( shrink ) variables.shrink_to_fit();
}

// Make Variables Collection Unique and Sorted and Optionally Shrink-to-Fit
template< typename Variables >
void
uniquify_and_sort_by_name( Variables & variables, bool const shrink = false )
{
	if ( variables.size() > 1u ) {
		std::sort( variables.begin(), variables.end() ); // Sort
		variables.erase( std::unique( variables.begin(), variables.end() ), variables.end() ); // Remove duplicates
		sort_by_name( variables ); // Determinism
	}
	if ( shrink ) variables.shrink_to_fit();
}

// Sort Variables by Type
template< typename Variables >
void
sort_by_type( Variables & variables )
{
	using V = typename std::remove_pointer< typename Variables::value_type >::type;
	std::sort( variables.begin(), variables.end(),
	 []( V const * v1, V const * v2 ){
		int const v1_i( v1->var_sort_index() );
		int const v2_i( v2->var_sort_index() );
//		return v1_i < v2_i; // Don't sort by name => Non-deterministic order-dependent results
//		return ( v1_i < v2_i ) || ( ( v1_i == v2_i ) && ( v1 < v2 ) ); // Also sort by address for deterministic order-dependent results: Faster than sorting by name
		return ( v1_i < v2_i ) || ( ( v1_i == v2_i ) && ( v1->name() < v2->name() ) ); // Also sort by name for deterministic output and order-dependent results
	} );
}

// Sort Variables by Name
template< typename Variables >
void
sort_by_name( Variables & variables )
{
	using V = typename std::remove_pointer< typename Variables::value_type >::type;
	std::sort( variables.begin(), variables.end(), []( V const * v1, V const * v2 ){ return v1->name() < v2->name(); } );
}

// Copy of Variables Sorted by Name
template< typename Variables >
Variables
sorted_by_name( Variables const & variables )
{
	using V = typename std::remove_pointer< typename Variables::value_type >::type;
	Variables sorted_variables( variables );
	std::sort( sorted_variables.begin(), sorted_variables.end(), []( V const * v1, V const * v2 ){ return v1->name() < v2->name(); } );
	return sorted_variables;
}

// Vector Has a Value?
template< typename T >
bool
vector_has( std::vector< T > const & c, T const & t )
{
#if ( __cplusplus >= 202302L ) // C++23+
	return std::ranges::contains( c, t );
#else
	return std::find( c.begin(), c.end(), t ) != c.end();
#endif
}

// Vector Has a Value?
template< typename T >
bool
vector_contains( std::vector< T > const & c, T const & t )
{
#if ( __cplusplus >= 202302L ) // C++23+
	return std::ranges::contains( c, t );
#else
	return std::find( c.begin(), c.end(), t ) != c.end();
#endif
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
