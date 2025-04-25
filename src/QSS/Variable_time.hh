// Time QSS Variable
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

#ifndef QSS_Variable_time_hh_INCLUDED
#define QSS_Variable_time_hh_INCLUDED

// QSS Headers
#include <QSS/Variable_QSS.hh>

namespace QSS {

// Time QSS Variable
class Variable_time final : public Variable_QSS
{

public: // Types

	using Super = Variable_QSS;

public: // Creation

	// Constructor
	Variable_time(
	 FMU_ME * fmu_me,
	 int const order,
	 std::string const & name,
	 Real const rTol_ = options::rTol,
	 Real const aTol_ = options::aTol,
	 Real const zTol_ = options::zTol,
	 Real const xIni_ = 0.0,
	 FMU_Variable const & var = FMU_Variable(),
	 FMU_Variable const & der = FMU_Variable()
	) :
	 Super( fmu_me, order, name, rTol_, aTol_, zTol_, xIni_, var, der ),
	 x_0_( xIni_ )
	{}

public: // Property

	// Continuous Value at Time t
	Real
	x( Time const t ) const override
	{
		return x_0_ + ( x_1_ * ( t - tX ) );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const ) const override
	{
		return x_1_;
	}

	// Quantized Value at Time t
	Real
	q( Time const t ) const override
	{
		return x_0_ + ( x_1_ * ( t - tX ) ); // Propagate continuous trajectory so that time doesn't need to requantize with QSS1
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const ) const override
	{
		return x_1_; // Propagate continuous trajectory so that time doesn't need to requantize with QSS1
	}

public: // Methods

	// Initialization
	void
	init() override
	{
		init_0();
		init_observers();
		init_1();
		init_F();
	}

	// Initialization: Stage 0
	void
	init_0() override
	{
		assert( !observes() );
		assert( p_1() == 1.0 );
		fmu_set_real( x_0_ = xIni );
		x_1_ = 1.0;
	}

	// Initialization: Stage Final
	void
	init_F() override
	{
		tE = infinity;
		add_QSS( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << std::noshowpos << "   tE=" << tE << std::endl;
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 1.0 }; // Continuous trajectory coefficients

}; // Variable_time

} // QSS

#endif
