// Index Range Class
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

#ifndef QSS_Range_hh_INCLUDED
#define QSS_Range_hh_INCLUDED

// C++ Headers
#include <cstddef>
#include <limits>
#include <utility>

namespace QSS {

// Index Range
class Range final
{

public: // Types

	using size_type = std::size_t;

public: // Creation

	// Default Constructor
	Range() = default;

	// Indexes Constructor
	Range( size_type const b, size_type const e ) :
	 b_( b ),
	 e_( e )
	{}

public: // Predicate

	// Empty?
	bool
	empty() const
	{
		return b_ >= e_;
	}

	// Have?
	bool
	have() const
	{
		return b_ < e_;
	}

	// Began?
	bool
	began() const
	{
		return b_ < std::numeric_limits< size_type >::max();
	}

	// Intersects a Range?
	bool
	intersects( Range const & r ) const
	{
		if ( empty() || r.empty() ) {
			return false;
		} else {
			return ( b_ <= r.e_ ) && ( r.b_ <= e_ );
		}
	}

public: // Property

	// Size
	size_type
	size() const
	{
		return b_ < e_ ? e_ - b_ : 0u;
	}

	// Begin Index
	size_type
	b() const
	{
		return b_;
	}

	// Begin Index
	size_type &
	b()
	{
		return b_;
	}

	// End Index
	size_type
	e() const
	{
		return e_;
	}

	// End Index
	size_type &
	e()
	{
		return e_;
	}

	// Size
	size_type
	n() const
	{
		return b_ < e_ ? e_ - b_ : 0u;
	}

public: // Methods

	// Assign
	void
	assign( size_type const b, size_type const e )
	{
		b_ = b;
		e_ = e;
	}

	// Intersect with a Range
	void
	intersect( Range const & r )
	{
		b_ = std::max( b_, r.b_ );
		e_ = std::min( e_, r.e_ );
	}

	// Reset
	void
	reset()
	{
		b_ = std::numeric_limits< size_type >::max();
		e_ = 0u;
	}

	// Swap
	void
	swap( Range & r )
	{
		std::swap( b_, r.b_ );
		std::swap( e_, r.e_ );
	}

public: // Friend Functions

	// Swap
	friend
	void
	swap( Range & r1, Range & r2 )
	{
		r1.swap( r2 );
	}

private: // Data

	size_type b_{ std::numeric_limits< size_type >::max() }; // Begin index
	size_type e_{ 0u }; // End index (1 beyond last item)

}; // Range

} // QSS

#endif
