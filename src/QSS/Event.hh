// QSS Event Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017 Objexx Engineerinc, Inc. All rights reserved.
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
#include <cstddef>

namespace QSS {

// QSS Event Class
template< typename V >
class Event final
{

public: // Types

	using Variable = V;
	using Value = double;

	// Event Type
	enum Type : std::size_t {
	 Discrete, // Discrete
	 Handler, // Handler
	 ZC, // Zero-crossing
	 QSS // Requantization
	};

public: // Creation

	// Constructor
	Event(
	 Type const typ,
	 Variable * var = nullptr,
	 Value const val = 0.0
	) :
	 typ_( typ ),
	 var_( var ),
	 val_( val )
	{}

public: // Properties

	// Discrete Event?
	bool
	is_discrete() const
	{
		return ( typ_ == Discrete );
	}

	// Handler Event?
	bool
	is_handler() const
	{
		return ( typ_ == Handler );
	}

	// Zero-Crossing Event?
	bool
	is_ZC() const
	{
		return ( typ_ == ZC );
	}

	// QSS Event?
	bool
	is_QSS() const
	{
		return ( typ_ == QSS );
	}

	// Event Type
	Type
	type() const
	{
		return typ_;
	}

	// Variable
	Variable const *
	var() const
	{
		return var_;
	}

	// Variable
	Variable *
	var()
	{
		return var_;
	}

	// Value
	Value
	val() const
	{
		return val_;
	}

	// Value
	Value &
	val()
	{
		return val_;
	}

public: // Comparison: SuperdenseTime [?] SuperdenseTime

	// Event == Event
	friend
	bool
	operator ==( Event const & e1, Event const & e2 )
	{
		return ( e1.typ_ == e2.typ_ ) && ( e1.var_ == e2.var_ ) && ( ( e1.typ_ != Handler ) || ( e1.val_ == e2.val_ ) );
	}

	// Event != Event
	friend
	bool
	operator !=( Event const & e1, Event const & e2 )
	{
		return ( e1.typ_ != e2.typ_ ) || ( e1.var_ != e2.var_ ) || ( ( e1.typ_ == Handler ) && ( e1.val_ != e2.val_ ) );
	}

private: // Data

	Type typ_;
	Variable * var_{ nullptr };
	Value val_{ 0.0 };

};

} // QSS

#endif
