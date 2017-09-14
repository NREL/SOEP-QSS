// FMU-Based Integer Input Variable
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

#ifndef QSS_fmu_Variable_InpI_hh_INCLUDED
#define QSS_fmu_Variable_InpI_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable_Inp.hh>

// C++ Headers
#include <cstdint>

namespace QSS {
namespace fmu {

// FMU-Based Integer Input Variable
class Variable_InpI final : public Variable_Inp
{

public: // Types

	using Super = Variable_Inp;
	using Integer = std::int64_t;

public: // Creation

	// Constructor
	explicit
	Variable_InpI(
	 std::string const & name,
	 FMU_Variable const var = FMU_Variable(),
	 Function f = Function()
	) :
	 Super( name, var, f )
	{}

public: // Predicate

	// Discrete Variable?
	bool
	is_Discrete() const
	{
		return true;
	}

public: // Properties

	// Order of Method
	int
	order() const
	{
		return 0;
	}

	// Value
	Integer
	i() const
	{
		return x_;
	}

	// Continuous Value at Time t
	Integer
	i( Time const ) const
	{
		return x_;
	}

	// Value
	Value
	x() const
	{
		return x_;
	}

	// Continuous Value at Time t
	Value
	x( Time const ) const
	{
		return x_;
	}

	// Continuous First Derivative at Time t
	Value
	x1( Time const ) const
	{
		return 0.0;
	}

	// Quantized Value
	Value
	q() const
	{
		return x_;
	}

	// Quantized Value at Time t
	Value
	q( Time const ) const
	{
		return x_;
	}

	// Simultaneous Value at Time t
	Value
	s( Time const ) const
	{
		return x_;
	}

	// Simultaneous Numeric Differentiation Value at Time t
	Value
	sn( Time const ) const
	{
		return x_;
	}

public: // Methods

	// Initialization
	void
	init()
	{
		init_0();
	}

	// Initialization: Stage 0
	void
	init_0()
	{
		assert( observees_.empty() );
		init_observers();
		x_ = static_cast< Integer >( f_( tQ ).x_0 );
		tD = f_( tQ ).tD;
		event( events.add_discrete( tD, this ) );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << x_ << "   tD=" << tD << '\n';
	}

	// Discrete Advance
	void
	advance_discrete()
	{
		x_ = static_cast< Integer >( f_( tX = tQ = tD ).x_0 );
		advance_observers_1();
		if ( observers_max_order_ >= 2 ) {
			fmu::set_time( tN = tQ + options::dtNum );
			advance_observers_2();
		}
		tD = f_( tD ).tD;
		event( events.shift_discrete( tD, event() ) );
		if ( options::output::d ) {
			std::cout << "* " << name << '(' << tQ << ')' << " = " << x_ << "   tD=" << tD << '\n';
			advance_observers_d();
		}
	}

	// Discrete Advance: Stages 0 and 1
	void
	advance_discrete_0_1()
	{
		x_ = static_cast< Integer >( f_( tX = tQ = tD ).x_0 );
		tD = f_( tD ).tD;
		event( events.shift_discrete( tD, event() ) );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << x_ << "   tD=" << tD << '\n';
	}

private: // Data

	Integer x_; // Value

};

} // fmu
} // QSS

#endif
