// QSS Event Target Abstract Base Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2019 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_Target_hh_INCLUDED
#define QSS_Target_hh_INCLUDED

// QSS Headers
#include <QSS/Target.fwd.hh>
#include <QSS/Event.hh>
#include <QSS/SuperdenseTime.hh>

// C++ Headers
#include <cassert>
#include <map>
#include <string>

namespace QSS {

// QSS Event Target Abstract Base Class
class Target
{

public: // Types

	using EventMap = std::multimap< SuperdenseTime, Event< Target > >;

protected: // Creation

	// Default Constructor
	Target() = default;

	// Name Constructor
	explicit
	Target( std::string const & name ) :
	 name( name )
	{}

	// Copy Constructor
	Target( Target const & ) = default;

	// Move Constructor
	Target( Target && ) noexcept = default;

public: // Creation

	// Destructor
	virtual
	~Target()
	{}

protected: // Assignment

	// Copy Assignment
	Target &
	operator =( Target const & ) = default;

	// Move Assignment
	Target &
	operator =( Target && ) noexcept = default;

public: // Properties

	// Event Queue Iterator
	EventMap::iterator &
	event()
	{
		return event_;
	}

	// Event Queue Iterator Assignment
	void
	event( EventMap::iterator const i )
	{
		event_ = i;
		assert( event_->second.tar() == this );
	}

public: // Data

	std::string name;
	SuperdenseTime st; // Superdense time of latest event

protected: // Data

	EventMap::iterator event_; // Iterator into event queue

};

} // QSS

#endif
