// QSS Solver Container Functions
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2020 Objexx Engineering, Inc. All rights reserved.
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
inline
bool
is_sorted_by_order( Variables & variables )
{
	using V = typename std::remove_pointer< typename Variables::value_type >::type;
	return std::is_sorted( variables.begin(), variables.end(), []( V const * v1, V const * v2 ){ return v1->order() < v2->order(); } );
}

// Variables Begin Index of Given Order or Greater
template< typename Variables >
inline
typename Variables::size_type
begin_order_index( Variables const & variables, int const order )
{
	using V = typename std::remove_pointer< typename Variables::value_type >::type;
	assert( is_sorted_by_order( variables ) ); // Require sorted by order
	return static_cast< typename Variables::size_type >( std::distance( variables.begin(), std::lower_bound( variables.begin(), variables.end(), order, []( V const * v, int const o ){ return v->order() < o; } ) ) );
}

// Sort Variables by Order
template< typename Variables >
inline
void
sort_by_order( Variables & variables )
{
	using V = typename std::remove_pointer< typename Variables::value_type >::type;
	// Stable sort to be deterministic given prior address sort without adding extra address condition to std::sort
	std::stable_sort( variables.begin(), variables.end(), []( V const * v1, V const * v2 ){ return v1->order() < v2->order(); } );
}

// Sort Variables by Type (State First) and Order
template< typename Variables >
inline
void
sort_by_type_and_order( Variables & variables )
{
	using V = typename std::remove_pointer< typename Variables::value_type >::type;
	// Stable sort to be deterministic given prior address sort without adding extra address condition to std::sort
	std::stable_sort( variables.begin(), variables.end(), []( V const * v1, V const * v2 ){ return v1->state_order() < v2->state_order(); } );
}

// Sort Variables by QSS State First
template< typename Variables >
inline
void
sort_by_QSS( Variables & variables )
{
	using V = typename std::remove_pointer< typename Variables::value_type >::type;
	// Stable sort to be deterministic given prior address sort without adding extra address condition to std::sort
	std::stable_sort( variables.begin(), variables.end(), []( V const * v1, V const * v2 ){ return v1->state_sort_index() < v2->state_sort_index(); } );
}

// Set up Non-Trigger Observers of Triggers and Sort Both by Order
template< typename Variables >
inline
void
variables_observers( Variables & triggers, Variables & observers )
{
	using V = typename std::remove_pointer< typename Variables::value_type >::type;
	using size_type = typename Variables::size_type;
	using iterator = typename Variables::iterator;

	observers.clear();

	// Collect all observers
	for ( V * trigger : triggers ) {
		for ( V * observer : trigger->observers() ) {
			observers.push_back( observer );
		}
	}

	// Remove duplicates and triggers from observers
	if ( ! observers.empty() ) {

		// Remove duplicates
		std::sort( observers.begin(), observers.end() );
		observers.erase( std::unique( observers.begin(), observers.end() ), observers.end() );

		// Remove triggers
		std::sort( triggers.begin(), triggers.end() );
		iterator it( triggers.begin() );
		iterator const et( triggers.end() );
		size_type no( observers.size() ); // Number of observers
		for ( V * & observer : observers ) {
			while ( ( it != et ) && ( *it < observer ) ) ++it;
			if ( it != et ) {
				if ( *it == observer ) {
					observer = ( V * )( std::numeric_limits< std::uintptr_t >::max() );
					--no;
				}
			} else {
				break;
			}
		}
		std::sort( observers.begin(), observers.end() );
		observers.resize( no );
		// Don't shrink observers: Meant for short-lived collections created for simultaneous variable event processing during simulation
		if ( ! observers.empty() ) sort_by_type_and_order( observers );
	}

	// Sort triggers by order
	sort_by_order( triggers );
}

// Remove Elements of a Container with a Value
template< typename T >
void
vector_remove_value( std::vector< T > & c, T const & t )
{
	typename std::vector< T >::iterator i( std::find( c.begin(), c.end(), t ) );
	if ( i != c.end() ) c.erase( i );
}

// Nullify Elements of a Container with a Value
template< typename T >
void
vector_nullify_value( std::vector< T > & c, T const & t )
{
	typename std::vector< T >::iterator i( std::find( c.begin(), c.end(), t ) );
	if ( i != c.end() ) *i = nullptr;
}

// Remove Elements of a Container with a Value
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
