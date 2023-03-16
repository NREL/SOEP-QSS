// QSS Connection Variable
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2023 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_Variable_Con_hh_INCLUDED
#define QSS_Variable_Con_hh_INCLUDED

// QSS Headers
#include <QSS/Variable.hh>
#include <QSS/SmoothToken.hh>

namespace QSS {

// QSS Connection Variable
class Variable_Con final : public Variable
{

public: // Types

	using Super = Variable;

public: // Creation

	// Name Constructor
	Variable_Con(
	 FMU_ME * fmu_me,
	 int const order,
	 std::string const & name,
	 Real const xIni_ = 0.0,
	 FMU_Variable const var = FMU_Variable()
	) :
	 Super( fmu_me, order, name, xIni_, var )
	{}

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
		return true;
	}

public: // Property

	// Continuous Value at Time t
	Real
	x( Time const t ) const override
	{
		assert( out_var_ != nullptr );
		return out_var_->x( t );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const t ) const override
	{
		assert( out_var_ != nullptr );
		return out_var_->x1( t );
	}

	// Continuous Second Derivative at Time t
	Real
	x2( Time const t ) const override
	{
		assert( out_var_ != nullptr );
		return out_var_->x2( t );
	}

	// Continuous Third Derivative at Time t
	Real
	x3( Time const t ) const override
	{
		assert( out_var_ != nullptr );
		return out_var_->x3( t );
	}

	// Quantized Value at Time t
	Real
	q( Time const t ) const override
	{
		assert( out_var_ != nullptr );
		return out_var_->q( t );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const t ) const override
	{
		assert( out_var_ != nullptr );
		return out_var_->q1( t );
	}

	// Quantized Second Derivative at Time t
	Real
	q2( Time const t ) const override
	{
		assert( out_var_ != nullptr );
		return out_var_->q2( t );
	}

	// Quantized Third Derivative at Time t
	Real
	q3( Time const t ) const override
	{
		assert( out_var_ != nullptr );
		return out_var_->q3( t );
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
		assert( out_var_ != nullptr );
		assert( observees().empty() );
	}

	// Initialization: Stage Final
	void
	init_F() override
	{
		tQ = out_var_->tQ;
		tX = out_var_->tX;
		tE = out_var_->tE;
		tS = out_var_->tS;
		Real const x_0( out_var_->x( tQ ) );
		fmu_set_real( x_0 );
		if ( options::output::d ) {
			std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0;
			int const out_var_order( out_var_->order() );
			if ( out_var_order >= 1 ) std::cout << out_var_->x1( tQ ) << x_delta;
			if ( out_var_order >= 2 ) std::cout << one_half * out_var_->x2( tQ ) << x_delta_2;
			if ( out_var_order >= 3 ) std::cout << one_sixth * out_var_->x3( tQ ) << x_delta_3;
			std::cout << std::noshowpos << "   tD=" << out_var_->tD << std::endl;
		}
	}

	// Connection Advance
	void
	advance_connection( Time const t )
	{
		assert( out_var_ != nullptr );
		tQ = out_var_->tQ;
		tX = out_var_->tX;
		tE = out_var_->tE;
		tS = out_var_->tS;
		Real const x_( out_var_->x( t ) );
		fmu_set_time( t ); // Different FMU-ME than trigger
		fmu_set_real( x_ );
		if ( options::output::d ) std::cout << "|  " << name() << '(' << tX << ')' << " = " << std::showpos << x_ << std::noshowpos << std::endl;
		if ( observed() ) advance_observers();
	}

	// Connection Observer Advance
	void
	advance_connection_observer()
	{
		assert( out_var_ != nullptr );
		tX = out_var_->tX;
		tE = out_var_->tE;
		tS = out_var_->tS;
	}

private: // Data

	Variable * out_var_{ nullptr }; // Connected output variable

}; // Variable_Con

} // QSS

#endif
