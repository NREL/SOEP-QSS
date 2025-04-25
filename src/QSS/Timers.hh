// QSS Performance Timers
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

#ifndef QSS_Timers_hh_INCLUDED
#define QSS_Timers_hh_INCLUDED

// Note: CPU timers are problematic: they may only have 15ms resolution and they may wrap.
//       So these timers are best used for timing intervals of 100 ms to a few minutes.

// QSS Headers
#include <QSS/cpu_time.hh>
#include <QSS/math.hh>

// C++ Headers
#include <cassert>

namespace QSS {
namespace timers {

// CPU Timer
class CPU final
{

public: // Types

	using Time = double;

public: // Creation

	// Default Constructor
	CPU() :
	 cb_( cpu_time() )
	{}

	// Deferring Constructor
	explicit
	CPU( bool const )
	{}

public: // Property

	// Elapsed Time
	Time
	operator()() const
	{
		return cpu_time() - cb_;
	}

	// Elapsed Time
	Time
	elapsed() const
	{
		return cpu_time() - cb_;
	}

public: // Method

	// Start
	void
	start()
	{
		cb_ = cpu_time();
	}

private: // Data

	Time cb_{ 0.0 }; // CPU begin time

}; // CPU

// Performance Metric
class Performance final
{

public: // Types

	using Time = double;
	using Velocity = double;

public: // Creation

	// Default Constructor
	Performance() :
	 cb_( cpu_time() )
	{}

	// Begin Time Constructor
	explicit
	Performance( Time const tb ) :
	 cb_( cpu_time() ),
	 tb_( tb )
	{}

	// Deferring Constructor
	explicit
	Performance( bool const )
	{}

public: // Property

	// Begin Time
	Time
	tb() const
	{
		return tb_;
	}

	// Elapsed Time
	Time
	operator()() const
	{
		return cpu_time() - cb_;
	}

	// Elapsed Time
	Time
	elapsed() const
	{
		return cpu_time() - cb_;
	}

	// Solution "Velocity"
	Velocity
	operator()( Time const tn ) const
	{
		assert( tb_ <= tn );
		Time const te( elapsed() );
		return te > 0.0 ? ( tn - tb_ ) / te : ( tb_ < tn ? infinity : 0.0 );
	}

	// Solution "Velocity" for a Given Elapsed Time
	Velocity
	operator()( Time const tn, Time const te ) const
	{
		assert( tb_ <= tn );
		return te > 0.0 ? ( tn - tb_ ) / te : ( tb_ < tn ? infinity : 0.0 );
	}

public: // Method

	// Start
	void
	start( Time const tb = Time( 0 ) )
	{
		cb_ = cpu_time();
		tb_ = tb;
	}

private: // Data

	Time cb_{ 0.0 }; // CPU begin time
	Time tb_{ 0.0 }; // Simulation begin time

}; // Performance

} // timers
} // QSS

#endif
