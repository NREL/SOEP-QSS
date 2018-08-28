// QSS Input Variable Abstract Base Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2018 Objexx Engineerinc, Inc. All rights reserved.
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

#ifndef QSS_cod_Variable_Inp_hh_INCLUDED
#define QSS_cod_Variable_Inp_hh_INCLUDED

// QSS Headers
#include <QSS/cod/Variable.hh>

namespace QSS {
namespace cod {

// QSS Input Variable Abstract Base Class
template< class F >
class Variable_Inp : public Variable
{

public: // Types

	using Super = Variable;
	using Time = Variable::Time;
	using Value = Variable::Value;
	using Function = F;

protected: // Creation

	// Constructor
	explicit
	Variable_Inp(
	 std::string const & name,
	 Value const rTol = 1.0e-4,
	 Value const aTol = 1.0e-6
	) :
	 Super( name, rTol, aTol )
	{
		xIni = f_.vs( tQ );
		tD = f_.tD( tQ );
	}

	// Copy Constructor
	Variable_Inp( Variable_Inp const & ) = default;

	// Move Constructor
	Variable_Inp( Variable_Inp && ) noexcept = default;

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
	is_Input() const
	{
		return true;
	}

public: // Properties

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

};

} // cod
} // QSS

#endif
