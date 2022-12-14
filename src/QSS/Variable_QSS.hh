// QSS Variable Abstract Base Class
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

#ifndef QSS_Variable_QSS_hh_INCLUDED
#define QSS_Variable_QSS_hh_INCLUDED

// QSS Headers
#include <QSS/LIQSS_QSS_Step_Ratio.hh>
#include <QSS/Variable.hh>

namespace QSS {

// QSS Variable Abstract Base Class
class Variable_QSS : public Variable
{

public: // Types

	using Super = Variable;

protected: // Creation

	// Copy Constructor
	Variable_QSS( Variable_QSS const & ) = default;

	// Move Constructor
	Variable_QSS( Variable_QSS && ) noexcept = default;

	// Constructor
	Variable_QSS(
	 FMU_ME * fmu_me,
	 int const order,
	 std::string const & name,
	 Real const rTol_ = options::rTol,
	 Real const aTol_ = options::aTol,
	 Real const zTol_ = options::zTol,
	 Real const xIni_ = 0.0,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Super( fmu_me, order, name, rTol_, aTol_, zTol_, xIni_, var, der )
	{}

protected: // Assignment

	// Copy Assignment
	Variable_QSS &
	operator =( Variable_QSS const & ) = default;

	// Move Assignment
	Variable_QSS &
	operator =( Variable_QSS && ) noexcept = default;

public: // Predicate

	// Real Variable?
	bool
	is_Real() const override
	{
		return true;
	}

	// QSS Variable?
	bool
	is_QSS() const override
	{
		return true;
	}

public: // Methods

	// LIQSS/QSS Step Ratio Pass
	void
	liqss_qss_ratio_pass()
	{
		assert( !is_LIQSS() );
		if ( liqss_qss_step_ratio.check_ratio() ) {
			liqss_qss_step_ratio.add( advance_LIQSS_QSS_step_ratio() );
		}
	}

public: // Data

	LIQSS_QSS_Step_Ratio liqss_qss_step_ratio; // LIQSS/QSS step size ratio metric

}; // Variable_QSS

} // QSS

#endif