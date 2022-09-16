// Optional Value Class Template
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2022 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_Optional_hh_INCLUDED
#define QSS_Optional_hh_INCLUDED

// C++ Headers
#include <cassert>

namespace QSS {

// Optional Value Class Template
template< typename Value >
class Optional final
{

private: // Friend

	template< typename > friend class Optional;

public: // Creation

	// Default Constructor
	Optional() = default;

	// Value Constructor
	Optional( Value const & val ) :
	 set_( true ),
	 val_( val )
	{}

	// Set + Value Constructor
	Optional(
	 bool const set,
	 Value const & val
	) :
	 set_( set ),
	 val_( set ? val : Value() )
	{}

	// Optional Constructor Template
	template< typename T >
	Optional( Optional< T > const & opt ) :
	 set_( opt.set_ ),
	 val_( Value( opt.val_ ) )
	{}

public: // Assignment

	// Value Assignment
	Optional &
	operator =( Value const & val )
	{
		set_ = true;
		val_ = val;
		return *this;
	}

public: // Conversion

	// Value Conversion
	operator Value const &() const
	{
		return val_;
	}

	// Value Conversion
	operator Value &()
	{
		return val_;
	}

public: // Operator

	// Value
	Value const &
	operator ()() const
	{
		return val_;
	}

	// Value
	Value &
	operator ()()
	{
		return val_;
	}

public: // Predicate

	// Set?
	bool
	set() const
	{
		return set_;
	}

public: // Property

	// Value
	Value const &
	val() const
	{
		return val_;
	}

public: // Methods

	// Clear
	void
	clear()
	{
		set_ = false;
		val_ = Value();
	}

	// Un-Set: Retain Value
	void
	un_set()
	{
		set_ = false;
	}

	// Re-Set: Retain Value
	void
	re_set()
	{
		set_ = true;
	}

	// Value Set
	void
	val( Value const & val )
	{
		set_ = true;
		val_ = val;
	}

public: // Comparison

	// Optional == Optional
	friend
	bool
	operator ==( Optional const & a, Optional const & b )
	{
		return ( ( a.set_ == b.set_ ) && ( a.val_ == b.val_ ) );
	}

	// Optional != Optional
	friend
	bool
	operator !=( Optional const & a, Optional const & b )
	{
		return !( a == b );
	}

	// Optional == Value
	friend
	bool
	operator ==( Optional const & a, Value const & b )
	{
		return ( a.set_ && ( a.val_ == b ) );
	}

	// Optional != Value
	friend
	bool
	operator !=( Optional const & a, Value const & b )
	{
		return !( a == b );
	}

	// Value == Optional
	friend
	bool
	operator ==( Value const & a, Optional const & b )
	{
		return ( b.set_ && ( a == b.val_ ) );
	}

	// Value != Optional
	friend
	bool
	operator !=( Value const & a, Optional const & b )
	{
		return !( a == b );
	}

private: // Data

	bool set_{ false }; // Value Set?
	Value val_{ Value() }; // Value

}; // Optional

} // QSS

#endif
