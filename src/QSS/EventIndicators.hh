// FMU-ME Event Indicator Support
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

#ifndef QSS_EventIndicators_hh_INCLUDED
#define QSS_EventIndicators_hh_INCLUDED

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <vector>

namespace QSS {

// Event Indicator XML Entry Specs
struct EventIndicator final
{
	// Types
	using size_type = std::size_t;
	using Index = size_type;

	// Less Than Comparison for Sorting
	friend
	bool
	operator <( EventIndicator const & a, EventIndicator const & b )
	{
		return a.index < b.index;
	}

	// Data
	Index index{ 0u }; // Index of variable

}; // EventIndicator

// FMU-ME EventIndicators Collection
struct FMU_EventIndicators final
{
	// Types
	using EventIndicators = std::vector< EventIndicator >;

	// Constructor
	explicit
	FMU_EventIndicators( void * context_ ) :
	 context( context_ )
	{}

	// Empty?
	bool
	empty() const
	{
		return event_indicators.empty();
	}

	// Has an EventIndicator for a Variable?
	bool
	has( EventIndicator::Index const idx ) const
	{
		assert( std::is_sorted( event_indicators.begin(), event_indicators.end() ) );
		EventIndicators::const_iterator i( std::lower_bound( event_indicators.begin(), event_indicators.end(), idx, []( EventIndicator const & ei, EventIndicator::Index const index ){ return ei.index < index; } ) );
		return ( ( i != event_indicators.end() ) && ( i->index == idx ) );
	}

	// Get EventIndicator for a Variable
	EventIndicator const *
	get( EventIndicator::Index const idx ) const
	{
		assert( std::is_sorted( event_indicators.begin(), event_indicators.end() ) );
		EventIndicators::const_iterator i( std::lower_bound( event_indicators.begin(), event_indicators.end(), idx, []( EventIndicator const & ei, EventIndicator::Index const index ){ return ei.index < index; } ) );
		return ( ( i != event_indicators.end() ) && ( i->index == idx ) ? &(*i) : nullptr );
	}

	// Get EventIndicator for a Variable
	EventIndicator *
	get( EventIndicator::Index const idx )
	{
		assert( std::is_sorted( event_indicators.begin(), event_indicators.end() ) );
		EventIndicators::iterator i( std::lower_bound( event_indicators.begin(), event_indicators.end(), idx, []( EventIndicator const & ei, EventIndicator::Index const index ){ return ei.index < index; } ) );
		return ( ( i != event_indicators.end() ) && ( i->index == idx ) ? &(*i) : nullptr );
	}

	// Add an Event Indicator
	void
	add( EventIndicator const & ei )
	{
		event_indicators.push_back( ei );
	}

	// Add an Event Indicator
	void
	add( EventIndicator && ei )
	{
		event_indicators.push_back( ei );
	}

	// Sort Event Indicators
	void
	sort()
	{
		std::sort( event_indicators.begin(), event_indicators.end() );
	}

	// Data
	EventIndicators event_indicators;
	bool inEventIndicators{ false };
	void * context{ nullptr }; // Context pointer to its FMU-ME

}; // FMU_EventIndicators

// EventIndicator Global Lookup by FMU-ME Context
namespace { // Pollution control
using All_EventIndicators = std::vector< FMU_EventIndicators >;
}
extern All_EventIndicators all_eventindicators;

} // QSS

#endif
