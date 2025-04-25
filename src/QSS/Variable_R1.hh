// QSS Real Variable
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

#ifndef QSS_Variable_R1_hh_INCLUDED
#define QSS_Variable_R1_hh_INCLUDED

// QSS Headers
#include <QSS/Variable.hh>

namespace QSS {

// QSS Real Order 1 Variable
class Variable_R1 final : public Variable
{

public: // Types

	using Super = Variable;

public: // Creation

	// Name + Value Constructor
	Variable_R1(
	 FMU_ME * fmu_me,
	 std::string const & name,
	 Real const rTol_ = options::rTol,
	 Real const aTol_ = options::aTol,
	 Real const xIni_ = 0.0,
	 FMU_Variable const & var = FMU_Variable()
	) :
	 Super( fmu_me, 1, name, rTol_, aTol_, xIni_, var ),
	 x_0_( xIni_ )
	{
		set_qTol();
	}

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

public: // Property

	// Real Value at Time t
	Real
	r( Time const t ) const override
	{
		return x_0_ + ( x_1_ * ( t - tX ) );
	}

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
		return x_0_ + ( x_1_ * ( t - tQ ) );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const ) const override
	{
		return x_1_;
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
		init_observees();
		x_0_ = xIni;
		assert( fmu_get_real() == x_0_ );
	}

	// Initialization: Stage 1
	void
	init_1() override
	{
		x_1_ = n_1();
	}

	// Initialization: Stage Final
	void
	init_F() override
	{
		set_qTol();
		set_tE();
		add_QSS_R( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << std::noshowpos << "   tE=" << tE << std::endl;
	}

	// QSS Advance
	void
	advance_QSS() override
	{
		tS = tE - tQ;
		tQ = tX = tE;
		x_0_ = r_0();
		x_1_ = n_dso_1();
		set_qTol();
		set_tE();
		shift_QSS_R( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << std::noshowpos << "   tE=" << tE << std::endl;
		if ( observed() ) advance_observers();
		if ( connected() ) advance_connections();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0( Real const x_0 ) override
	{
		tS = tE - tQ;
		tQ = tX = tE;
		x_0_ = x_0;
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1( Real const x_1 ) override
	{
		x_1_ = x_1;
	}

	// QSS Advance: Stage Final
	void
	advance_QSS_F() override
	{
		set_qTol();
		set_tE();
		shift_QSS_R( tE );
		if ( connected() ) advance_connections();
	}

	// QSS Advance: Stage Debug
	void
	advance_QSS_d() override
	{
		assert( options::output::d );
		std::cout << "!= " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << std::noshowpos << "   tE=" << tE << std::endl;
	}

	// Handler Advance
	void
	advance_handler( Time const t ) override
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		tS = t - tQ;
		tQ = tX = t;
		x_0_ = r_dso_0();
		x_1_ = n_1();
		set_qTol();
		set_tE();
		shift_QSS_R( tE );
		if ( options::output::d ) std::cout << "*  " << name() << '(' << tX << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << std::noshowpos << "   tE=" << tE << std::endl;
		if ( observed() ) advance_handler_observers();
		if ( connected() ) advance_connections();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t, Real const x_0 ) override
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		tS = t - tQ;
		tQ = tX = t;
		x_0_ = x_0;
	}

	// Handler Advance: Stage 1
	void
	advance_handler_1( Real const x_1 ) override
	{
		x_1_ = x_1;
	}

	// Handler Advance: Stage Final
	void
	advance_handler_F() override
	{
		set_qTol();
		set_tE();
		shift_QSS_R( tE );
		if ( options::output::d ) std::cout << "*= " << name() << '(' << tX << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << std::noshowpos << "   tE=" << tE << std::endl;
		if ( connected() ) advance_connections();
	}

	// Handler No-Advance
	void
	no_advance_handler() override
	{
		shift_QSS_R( tE );
	}

	// Observer Advance: Stage 1
	void
	advance_observer_1( Time const t, Real const x_0, Real const x_1 ) override
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		tS = t - tQ;
		tQ = tX = t;
		x_0_ = x_0;
		x_1_ = x_1;
	}

	// Observer Advance: Stage Final
	void
	advance_observer_F() override
	{
		set_qTol();
		set_tE();
		shift_QSS_R( tE );
		if ( connected() ) advance_connections_observer();
	}

	// Observer Advance: Stage Final: Parallel
	void
	advance_observer_F_parallel() override
	{
		set_qTol();
		set_tE();
	}

	// Observer Advance: Stage Final: Serial
	void
	advance_observer_F_serial() override
	{
		shift_QSS_R( tE );
		if ( connected() ) advance_connections_observer();
	}

	// Observer Advance: Stage d
	void
	advance_observer_d() const override
	{
		std::cout << " ^ " << name() << '(' << tX << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << std::noshowpos << "   tE=" << tE << std::endl;
	}

private: // Methods

	// Set QSS Tolerance
	void
	set_qTol()
	{
		qTol = std::max( rTol * std::abs( x_0_ ), aTol );
		assert( qTol > 0.0 );
	}

	// Set End Time
	void
	set_tE()
	{
		assert( tQ == tX );
		assert( dt_min <= dt_max );
		clip();
		Time dt( x_1_ != 0.0 ? qTol / std::abs( x_1_ ) : infinity );
		dt = std::min( std::max( dt_infinity( dt ), dt_min ), dt_max );
		tE = dt != infinity ? tQ + dt : infinity;
	}

	// Clip Small Trajectory Coefficients
	void
	clip()
	{
		if ( options::clipping ) {
			if ( std::abs( x_0_ ) <= options::clip ) x_0_ = 0.0;
			if ( std::abs( x_1_ ) <= options::clip ) x_1_ = 0.0;
		}
	}

	// Coefficient 1 at Time tQ
	Real
	n_1() const
	{
		return X_1();
	}

	// Coefficient 1 at Time tQ: Don't Set Observees
	Real
	n_dso_1() const
	{
		return X_dso_1();
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }; // Trajectory coefficients

}; // Variable_R1

} // QSS

#endif
