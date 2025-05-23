// QSS Input Variable Abstract Base Class
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

#ifndef QSS_Variable_Inp_hh_INCLUDED
#define QSS_Variable_Inp_hh_INCLUDED

// QSS Headers
#include <QSS/Variable.hh>
#include <QSS/SmoothToken.hh>

// C++ Headers
#include <functional>

namespace QSS {

// QSS Input Variable Abstract Base Class
class Variable_Inp : public Variable
{

public: // Types

	using Super = Variable;

	using Function = std::function< SmoothToken ( Time const ) >;

protected: // Creation

	// Copy Constructor
	Variable_Inp( Variable_Inp const & ) = default;

	// Move Constructor
	Variable_Inp( Variable_Inp && ) noexcept = default;

	// Name + Tolerance Constructor
	Variable_Inp(
	 FMU_ME * fmu_me,
	 int const order,
	 std::string const & name,
	 Real const rTol_ = options::rTol,
	 Real const aTol_ = options::aTol,
	 Real const xIni_ = 0.0,
	 FMU_Variable const & var = FMU_Variable(),
	 Function f = Function()
	) :
	 Super( fmu_me, order, name, rTol_, aTol_, xIni_, var ),
	 f_( f ),
	 is_connection_( f_ == nullptr )
	{}

	// Name Constructor
	Variable_Inp(
	 FMU_ME * fmu_me,
	 int const order,
	 std::string const & name,
	 Real const xIni_ = 0.0,
	 FMU_Variable const & var = FMU_Variable(),
	 Function f = Function()
	) :
	 Super( fmu_me, order, name, xIni_, var ),
	 f_( f ),
	 is_connection_( f_ == nullptr )
	{}

protected: // Assignment

	// Copy Assignment
	Variable_Inp &
	operator =( Variable_Inp const & ) = default;

	// Move Assignment
	Variable_Inp &
	operator =( Variable_Inp && ) noexcept = default;

public: // Predicate

	// Input Variable?
	bool
	is_Input() const override
	{
		return true;
	}

	// Connection Input Variable?
	bool
	is_connection() const override
	{
		return is_connection_;
	}

public: // Property

	// Function
	Function const &
	f() const
	{
		return f_;
	}

	// Function
	Function &
	f()
	{
		return f_;
	}

protected: // Data

	Function f_; // Input function
	SmoothToken s_; // Smooth token

private: // Data

	bool is_connection_{ false }; // Connection input?

}; // Variable_Inp

} // QSS

#endif
