// QSS Real Passive Variable
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

#ifndef QSS_Variable_RP_hh_INCLUDED
#define QSS_Variable_RP_hh_INCLUDED

// QSS Headers
#include <QSS/Variable.hh>

namespace QSS {

// QSS Real Passive Variable
class Variable_RP final : public Variable
{

public: // Types

	using Super = Variable;

public: // Creation

	// Name + Value Constructor
	Variable_RP(
	 FMU_ME * fmu_me,
	 std::string const & name,
	 Real const xIni_ = 0.0,
	 FMU_Variable const & var = FMU_Variable()
	) :
	 Super( fmu_me, 0, name, xIni_, var )
	{}

public: // Predicate

	// Real Variable?
	bool
	is_Real() const override
	{
		return true;
	}

	// B|I|D|R Variable?
	bool
	is_BIDR() const override
	{
		return true;
	}

	// R Variable?
	bool
	is_R() const override
	{
		return true;
	}

	// Active Variable?
	bool
	is_Active() const override
	{
		return false;
	}

public: // Property

	// Real Value at Time t
	Real
	r( Time const t ) const override
	{
		return r_0( t );
	}

	// Continuous Value at Time t
	Real
	x( Time const t ) const override
	{
		return r_0( t );
	}

	// Quantized Value at Time t
	Real
	q( Time const t ) const override
	{
		return r_0( t );
	}

public: // Methods

	// Initialization
	void
	init() override
	{
		init_0();
		init_observers();
		init_F();
	}

	// Initialization: Stage 0
	void
	init_0() override
	{
		init_observees();
		assert( fmu_get_real() == xIni );
	}

	// Initialization: Stage Final
	void
	init_F() override
	{
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << r_0( tQ ) << std::noshowpos << std::endl;
	}

}; // Variable_RP

} // QSS

#endif
