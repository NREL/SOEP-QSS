// FMU-Based QSS Solver Container Functions
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2018 Objexx Engineerinc, Inc. All rights reserved.
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

#ifndef QSS_fmu_container_hh_INCLUDED
#define QSS_fmu_container_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <limits>

namespace QSS {
namespace fmu {

// Sort Variables by Order
inline
void
sort_by_order( Variable::Variables & variables )
{
	// Stable sort to be deterministic given prior address sort without adding extra address condition to std::sort
	std::stable_sort( variables.begin(), variables.end(), []( Variable const * v1, Variable const * v2 ){ return v1->order() < v2->order(); } );
}

// Sort Variables by Type (Zero-Crossing at the End) and Order
inline
void
sort_by_ZC( Variable::Variables & variables )
{
	// Stable sort to be deterministic given prior address sort without adding extra address condition to std::sort
	std::stable_sort( variables.begin(), variables.end(), []( Variable const * v1, Variable const * v2 ){ return ( v1->is_ZC() ? max_rep_order << 1 : 0 ) + v1->order() < ( v2->is_ZC() ? max_rep_order << 1 : 0 ) + v2->order(); } );
}

// Variables Begin Index of Given Order
inline
Variable::Variables::size_type
begin_order_index( Variable::Variables const & variables, int const order )
{
	assert( std::is_sorted( variables.begin(), variables.end(), []( Variable const * v1, Variable const * v2 ){ return v1->order() < v2->order(); } ) ); // Require sorted by order
	return static_cast< Variable::Variables::size_type >( std::distance( variables.begin(), std::lower_bound( variables.begin(), variables.end(), order, []( Variable const * v, int const o ){ return v->order() < o; } ) ) );
}

// Set up Non-Trigger Observers of Triggers and Sort Both by Order
inline
void
variables_observers( Variable::Variables & triggers, Variable::Variables & observers )
{
	using Variables = Variable::Variables;
	using size_type = Variables::size_type;

	observers.clear();

	// Collect all observers
	for ( Variable * trigger : triggers ) {
		for ( Variable * observer : trigger->observers() ) {
			observers.push_back( observer );
		}
	}

	// Remove duplicates and triggers from observers
	if ( ! observers.empty() ) {
		std::sort( triggers.begin(), triggers.end() );
		std::sort( observers.begin(), observers.end() );

		// Remove duplicates
		observers.resize( std::distance( observers.begin(), std::unique( observers.begin(), observers.end() ) ) );

		// Remove triggers
		Variables::iterator it( triggers.begin() );
		Variables::iterator const et( triggers.end() );
		size_type no( observers.size() ); // Number of observers
		for ( Variable * & observer : observers ) {
			while ( ( it != et ) && ( *it < observer ) ) ++it;
			if ( it != et ) {
				if ( *it == observer ) {
					observer = ( Variable * )( std::numeric_limits< std::uintptr_t >::max() );
					--no;
				}
			} else {
				break;
			}
		}
		std::sort( observers.begin(), observers.end() );
		observers.resize( no );
		// Don't shrink observers: Meant for short-lived collections created for simultaneous variable event processing during simulation
		if ( ! observers.empty() ) sort_by_ZC( observers );
	}

	// Sort triggers by order
	sort_by_order( triggers );
}

} // fmu
} // QSS

#endif
