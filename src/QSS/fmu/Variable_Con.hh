// FMU-Based QSS Connection Iput Variable
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

#ifndef QSS_fmu_Variable_Con_hh_INCLUDED
#define QSS_fmu_Variable_Con_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable.hh>
#include <QSS/SmoothToken.hh>

namespace QSS {
namespace fmu {

// FMU-Based QSS Connection Variable
class Variable_Con final : public Variable
{

public: // Types

	using Super = Variable;

public: // Creation

	// Name Constructor
	Variable_Con(
	 int const order,
	 std::string const & name,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable()
	) :
	 Super( order, name, fmu_me, var )
	{}

public: // Predicate

	// Input Variable?
	bool
	is_Input() const
	{
		return true;
	}

	// Connection Input Variable?
	bool
	is_connection() const
	{
		return true;
	}

public: // Properties

	// Continuous Value at Time t
	Real
	x( Time const t ) const
	{
		return out_var_->x( t );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const t ) const
	{
		return out_var_->x1( t );
	}

	// Continuous Second Derivative at Time t
	Real
	x2( Time const t ) const
	{
		return out_var_->x2( t );
	}

	// Continuous Third Derivative at Time t
	Real
	x3( Time const t ) const
	{
		return out_var_->x3( t );
	}

	// Quantized Value at Time t
	Real
	q( Time const t ) const
	{
		return out_var_->q( t );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const t ) const
	{
		return out_var_->q1( t );
	}

	// Quantized Second Derivative at Time t
	Real
	q2( Time const t ) const
	{
		return out_var_->q2( t );
	}

	// Quantized Third Derivative at Time t
	Real
	q3( Time const t ) const
	{
		return out_var_->q3( t );
	}

	// Simultaneous Value at Time t
	Real
	s( Time const t ) const
	{
		return out_var_->s( t );
	}

	// Simultaneous Numeric Differentiation Value at Time t
	Real
	sn( Time const t ) const
	{
		return out_var_->sn( t );
	}

	// Simultaneous First Derivative at Time t
	Real
	s1( Time const t ) const
	{
		return out_var_->s1( t );
	}

	// Simultaneous Second Derivative at Time t
	Real
	s2( Time const t ) const
	{
		return out_var_->s2( t );
	}

	// Simultaneous Third Derivative at Time t
	Real
	s3( Time const t ) const
	{
		return out_var_->s3( t );
	}

	// Output Variable
	Variable const *
	out_var() const
	{
		return out_var_;
	}

	// Output Variable
	Variable * &
	out_var()
	{
		return out_var_;
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
		tQ = out_var_->tQ;
		tX = out_var_->tX;
		tE = out_var_->tE;
		Real const x_( out_var_->x( tQ ) );
		fmu_set_real( x_ );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << x_ << std::noshowpos << '\n';
	}

	// Connection Advance
	void
	advance_connection( Time const t )
	{
		tQ = out_var_->tQ;
		tX = out_var_->tX;
		tE = out_var_->tE;
		Real const x_( out_var_->x( t ) );
		fmu_set_time( t ); // Different FMU-ME than trigger
		fmu_set_real( x_ );
		if ( options::output::d ) std::cout << "| " << name << '(' << tX << ')' << " = " << std::showpos << x_ << std::noshowpos << '\n';

		if ( have_observers_ ) {
			advance_observers_1();
			if ( have_observers_2_ ) {
				fmu_set_time( tN = tQ + options::dtNum );
				advance_observers_2();
				fmu_set_time( tQ );
			}
			if ( options::output::d ) {
				advance_observers_d();
			}
		}
	}

	// Connection Observer Advance
	void
	advance_connection_observer()
	{
		tX = out_var_->tX;
		tE = out_var_->tE;
	}

private: // Data

	Variable * out_var_{ nullptr }; // Connected output variable

};

} // fmu
} // QSS

#endif
