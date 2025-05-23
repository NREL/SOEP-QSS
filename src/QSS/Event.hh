// QSS Event
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

#ifndef QSS_Event_hh_INCLUDED
#define QSS_Event_hh_INCLUDED

// C++ Headers
#include <cassert>
#include <cstddef>

namespace QSS {

// QSS Event
template< typename T >
class Event final
{

public: // Types

	using Target = T;
	using Real = double;
	using Offset = std::size_t;

	// Event Type
	enum class Type : std::size_t {
	 Observer,
	 Discrete,
	 ZC,
	 Conditional,
	 Handler,
	 QSS,
	 QSS_R,
	 QSS_ZC,
	 QSS_Inp
	};

	// Event Type SuperdenseTime Offsets
	struct Off final {
		static Offset const Observer{ static_cast< Offset >( Type::Observer ) };
		static Offset const Discrete{ static_cast< Offset >( Type::Discrete ) };
		static Offset const ZC{ static_cast< Offset >( Type::ZC ) };
		static Offset const Conditional{ static_cast< Offset >( Type::Conditional ) };
		static Offset const Handler{ static_cast< Offset >( Type::Handler ) };
		static Offset const QSS{ static_cast< Offset >( Type::QSS ) };
		static Offset const QSS_R{ static_cast< Offset >( Type::QSS_R ) };
		static Offset const QSS_ZC{ static_cast< Offset >( Type::QSS_ZC ) };
		static Offset const QSS_Inp{ static_cast< Offset >( Type::QSS_Inp ) };
	};

public: // Creation

	// Constructor
	explicit
	Event(
	 Type const typ,
	 Target * tar = nullptr,
	 Real const val = 0.0
	) :
	 typ_( typ ),
	 tar_( tar ),
	 val_( val )
	{}

public: // Predicate

	// Discrete Event?
	bool
	is_discrete() const
	{
		return typ_ == Type::Discrete;
	}

	// Zero-Crossing Event?
	bool
	is_ZC() const
	{
		return typ_ == Type::ZC;
	}

	// Conditional Event?
	bool
	is_conditional() const
	{
		return typ_ == Type::Conditional;
	}

	// Handler Event?
	bool
	is_handler() const
	{
		return typ_ == Type::Handler;
	}

	// QSS Event?
	bool
	is_QSS() const
	{
		return typ_ == Type::QSS;
	}

	// QSS ZC Event?
	bool
	is_QSS_ZC() const
	{
		return typ_ == Type::QSS_ZC;
	}

	// QSS R Event?
	bool
	is_QSS_R() const
	{
		return typ_ == Type::QSS_R;
	}

	// QSS Input Event?
	bool
	is_QSS_Inp() const
	{
		return typ_ == Type::QSS_Inp;
	}

	// Observer Event?
	bool
	is_observer() const
	{
		return typ_ == Type::Observer;
	}

public: // Property

	// Event Type
	Type
	type() const
	{
		return typ_;
	}

	// Target
	Target const *
	tar() const
	{
		return tar_;
	}

	// Target
	Target *
	tar()
	{
		return tar_;
	}

	// Target
	Target const *
	target() const
	{
		return tar_;
	}

	// Target
	Target *
	target()
	{
		return tar_;
	}

	// Target Subtype
	template< typename S >
	S const *
	sub() const
	{
		assert( dynamic_cast< S const * >( tar_ ) != nullptr );
		return static_cast< S const * >( tar_ );
	}

	// Target Subtype
	template< typename S >
	S *
	sub()
	{
		assert( dynamic_cast< S * >( tar_ ) != nullptr );
		return static_cast< S * >( tar_ );
	}

	// Target Handler Subtype
	template< typename S >
	S *
	handler_sub()
	{
		assert( dynamic_cast< S * >( tar_ ) != nullptr );
		S * s( static_cast< S * >( tar_ ) );
		return s->not_ZC() ? s : nullptr;
	}

	// Value
	Real
	val() const
	{
		return val_;
	}

	// Value
	Real &
	val()
	{
		return val_;
	}

	// Value
	Real
	value() const
	{
		return val_;
	}

	// Value
	Real &
	value()
	{
		return val_;
	}

public: // Comparison

	// Event == Event
	friend
	bool
	operator ==( Event const & e1, Event const & e2 )
	{
		return ( e1.typ_ == e2.typ_ ) && ( e1.tar_ == e2.tar_ ) && ( ( e1.typ_ != Type::Handler ) || ( e1.val_ == e2.val_ ) );
	}

	// Event != Event
	friend
	bool
	operator !=( Event const & e1, Event const & e2 )
	{
		return ( e1.typ_ != e2.typ_ ) || ( e1.tar_ != e2.tar_ ) || ( ( e1.typ_ == Type::Handler ) && ( e1.val_ != e2.val_ ) );
	}

private: // Data

	Type typ_; // Event type
	Target * tar_{ nullptr }; // Event target
	Real val_{ 0.0 }; // Event target value

}; // Event

} // QSS

#endif
