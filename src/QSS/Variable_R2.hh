// QSS Real Variable
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

#ifndef QSS_Variable_R2_hh_INCLUDED
#define QSS_Variable_R2_hh_INCLUDED

// QSS Headers
#include <QSS/Variable.hh>

namespace QSS {

// QSS Real Order 2 Variable
class Variable_R2 final : public Variable
{

public: // Types

	using Super = Variable;

public: // Creation

	// Name + Value Constructor
	Variable_R2(
	 FMU_ME * fmu_me,
	 std::string const & name,
	 Real const rTol_ = options::rTol,
	 Real const aTol_ = options::aTol,
	 Real const xIni_ = 0.0,
	 FMU_Variable const var = FMU_Variable()
	) :
	 Super( fmu_me, 2, name, rTol_, aTol_, xIni_, var ),
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
		Time const tDel( t - tX );
		return x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// Continuous Value at Time t
	Real
	x( Time const t ) const override
	{
		Time const tDel( t - tX );
		return x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const t ) const override
	{
		return x_1_ + ( two * x_2_ * ( t - tX ) );
	}

	// Continuous Second Derivative at Time t
	Real
	x2( Time const ) const override
	{
		return two * x_2_;
	}

	// Quantized Value at Time t
	Real
	q( Time const t ) const override
	{
		Time const tDel( t - tQ );
		return x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const t ) const override
	{
		return x_1_ + ( two * x_2_ * ( t - tQ ) );
	}

	// Quantized Second Derivative at Time t
	Real
	q2( Time const ) const override
	{
		return two * x_2_;
	}

public: // Methods

	// Initialization
	void
	init() override
	{
		init_0();
		init_observers();
		init_1();
		init_2();
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

	// Initialization: Stage 2
	void
	init_2() override
	{
		x_2_ = n_2();
	}

	// Initialization: Stage Final
	void
	init_F() override
	{
		set_qTol();
		set_tE();
		add_QSS_R( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << std::noshowpos << "   tE=" << tE << std::endl;
	}

	// QSS Advance
	void
	advance_QSS() override
	{
		tQ = tX = tE;
		x_0_ = r_0();
		x_1_ = n_1();
		x_2_ = n_2();
		set_qTol();
		set_tE();
		shift_QSS_R( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << std::noshowpos << "   tE=" << tE << std::endl;
		if ( observed() ) advance_observers();
		if ( connected() ) advance_connections();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0( Real const x_0 ) override
	{
		tQ = tX = tE;
		x_0_ = x_0;
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1( Real const x_1 ) override
	{
		x_1_ = x_1;
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2( Real const x_1_p ) override
	{
		x_2_ = n_2( x_1_p );
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2( Real const x_1_m, Real const x_1_p ) override
	{
		x_2_ = n_2( x_1_m, x_1_p );
	}

	// QSS Advance: Stage 2: Forward ND
	void
	advance_QSS_2_forward( Real const x_1_p, Real const x_1_2p ) override
	{
		x_2_ = f_2( x_1_p, x_1_2p );
	}

	// QSS Advance: Stage Final
	void
	advance_QSS_F() override
	{
		set_qTol();
		set_tE();
		shift_QSS_R( tE );
		if ( options::output::d ) std::cout << "!= " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << std::noshowpos << "   tE=" << tE << std::endl;
		if ( connected() ) advance_connections();
	}

	// Handler Advance
	void
	advance_handler( Time const t ) override
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		tQ = tX = t;
		x_0_ = r_f();
		x_1_ = n_1();
		x_2_ = n_2();
		set_qTol();
		set_tE();
		shift_QSS_R( tE );
		if ( options::output::d ) std::cout << "*  " << name() << '(' << tX << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << std::noshowpos << "   tE=" << tE << std::endl;
		if ( observed() ) advance_observers();
		if ( connected() ) advance_connections();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t, Real const x_0 ) override
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		tQ = tX = t;
		x_0_ = x_0;
	}

	// Handler Advance: Stage 1
	void
	advance_handler_1( Real const x_1 ) override
	{
		x_1_ = x_1;
	}

	// Handler Advance: Stage 2
	void
	advance_handler_2( Real const x_1_p ) override
	{
		x_2_ = n_2( x_1_p );
	}

	// Handler Advance: Stage 2
	void
	advance_handler_2( Real const x_1_m, Real const x_1_p ) override
	{
		x_2_ = n_2( x_1_m, x_1_p );
	}

	// QSS Advance: Stage 2: Forward ND
	void
	advance_handler_2_forward( Real const x_1_p, Real const x_1_2p ) override
	{
		x_2_ = f_2( x_1_p, x_1_2p );
	}

	// Handler Advance: Stage Final
	void
	advance_handler_F() override
	{
		set_qTol();
		set_tE();
		shift_QSS_R( tE );
		if ( options::output::d ) std::cout << "*= " << name() << '(' << tX << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << std::noshowpos << "   tE=" << tE << std::endl;
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
		tQ = tX = t;
		x_0_ = x_0;
		x_1_ = x_1;
	}

	// Observer Advance: Stage 2
	void
	advance_observer_2( Real const x_1_p ) override
	{
		x_2_ = n_2( x_1_p );
	}

	// Observer Advance: Stage 2
	void
	advance_observer_2( Real const x_1_m, Real const x_1_p ) override
	{
		x_2_ = n_2( x_1_m, x_1_p );
	}

	// Observer Advance: Stage 2
	void
	advance_observer_2_forward( Real const x_1_p, Real const x_1_2p ) override
	{
		x_2_ = f_2( x_1_p, x_1_2p );
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

	// Observer Advance: Stage d
	void
	advance_observer_d() const override
	{
		std::cout << " ^ " << name() << '(' << tX << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << std::noshowpos << "   tE=" << tE << std::endl;
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
		assert( tX <= tQ );
		assert( dt_min <= dt_max );
		Time dt( x_2_ != 0.0 ? std::sqrt( qTol / std::abs( x_2_ ) ) : infinity );
		dt = std::min( std::max( dt_infinity( dt ), dt_min ), dt_max );
		tE = ( dt != infinity ? tQ + dt : infinity );
		if ( ( options::inflection ) && ( x_2_ != 0.0 ) && ( signum( x_1_ ) != signum( x_2_ ) ) ) {
			Time const tI( tX - ( x_1_ / ( two * x_2_ ) ) );
			if ( tQ < tI ) tE = std::min( tE, tI );
		}
	}

	// Coefficient 1 from FMU at Time tQ
	Real
	n_1() const
	{
		return R_1();
	}

	// Coefficient 2 from FMU at Time tQ
	Real
	n_2() const
	{
		return R_2( x_1_ );
	}

	// Coefficient 2 from FMU
	Real
	n_2( Real const x_1_p ) const
	{
		return options::one_over_two_dtND * ( x_1_p - x_1_ ); //ND Forward Euler
	}

	// Coefficient 2 from FMU
	Real
	n_2( Real const x_1_m, Real const x_1_p ) const
	{
		return options::one_over_four_dtND * ( x_1_p - x_1_m ); //ND Centered difference
	}

	// Coefficient 2 from FMU
	Real
	f_2( Real const x_1_p, Real const x_1_2p ) const
	{
		return options::one_over_four_dtND * ( ( three * ( x_1_p - x_1_ ) ) + ( x_1_p - x_1_2p ) ); //ND Forward 3-point
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }; // Trajectory coefficients

}; // Variable_R2

} // QSS

#endif
