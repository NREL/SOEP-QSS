// FMU-Based QSS2 Variable
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2020 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_fmu_Variable_QSS2_hh_INCLUDED
#define QSS_fmu_Variable_QSS2_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable_QSS.hh>

namespace QSS {
namespace fmu {

// FMU-Based QSS2 Variable
class Variable_QSS2 final : public Variable_QSS
{

public: // Types

	using Super = Variable_QSS;

private: // Types

	using Super::c_1;
	using Super::c_2;
	using Super::p_2;

public: // Creation

	// Constructor
	Variable_QSS2(
	 std::string const & name,
	 Real const rTol,
	 Real const aTol,
	 Real const xIni,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Super( 2, name, rTol, aTol, xIni, fmu_me, var, der ),
	 x_0_( xIni ),
	 q_0_( xIni )
	{
		set_qTol();
	}

public: // Property

	// Continuous Value at Time t
	Real
	x( Time const t ) const
	{
		Time const tDel( t - tX );
		return x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const t ) const
	{
		return x_1_ + ( two * x_2_ * ( t - tX ) );
	}

	// Continuous Second Derivative at Time t
	Real
	x2( Time const ) const
	{
		return two * x_2_;
	}

	// Quantized Value at Time t
	Real
	q( Time const t ) const
	{
		return q_0_ + ( q_1_ * ( t - tQ ) );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const ) const
	{
		return q_1_;
	}

public: // Methods

	// Initialization
	void
	init()
	{
		init_0();
		init_1();
		init_2();
		init_F();
	}

	// Initialization to a Value
	void
	init( Real const x )
	{
		init_0( x );
		init_1();
		init_2();
		init_F();
	}

	// Initialization: Stage 0
	void
	init_0()
	{
		init_observers();
		init_observees();
		fmu_set_real( x_0_ = q_0_ = xIni );
	}

	// Initialization to a Value: Stage 0
	void
	init_0( Real const x )
	{
		init_observers();
		init_observees();
		fmu_set_real( x_0_ = q_0_ = x );
	}

	// Initialization: Stage 1
	void
	init_1()
	{
		x_1_ = q_1_ = p_1();
	}

	// Initialization: Stage 2
	void
	init_2()
	{
		x_2_ = c_2();
	}

	// Initialization: Stage Final
	void
	init_F()
	{
		set_qTol();
		set_tE_aligned();
		add_QSS( tE );
		if ( options::output::d ) std::cout << "! " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
	}

	// QSS Advance
	void
	advance_QSS()
	{
		Time const tDel( tE - tX );
		tX = tQ = tE;
		x_0_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
		x_1_ = q_1_ = c_1();
		x_2_ = c_2();
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "! " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( observed() ) advance_observers();
		if ( connected() ) advance_connections();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0()
	{
		Time const tDel( tE - tX );
		tX = tQ = tE;
		x_0_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1()
	{
		x_1_ = q_1_ = c_1();
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2()
	{
		x_2_ = c_2();
	}

	// QSS Advance: Stage Final
	void
	advance_QSS_F()
	{
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "= " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( connected() ) advance_connections();
	}

	// Handler Advance
	void
	advance_handler( Time const t )
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_0_ = p_0(); // Assume FMU ran zero-crossing handler
		x_1_ = q_1_ = h_1();
		x_2_ = c_2();
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "* " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( observed() ) advance_observers();
		if ( connected() ) advance_connections();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t )
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_0_ = p_0(); // Assume FMU ran zero-crossing handler
	}

	// Handler Advance: Stage 1
	void
	advance_handler_1()
	{
		x_1_ = q_1_ = h_1();
	}

	// Handler Advance: Stage 2
	void
	advance_handler_2()
	{
		x_2_ = c_2();
	}

	// Handler Advance: Stage Final
	void
	advance_handler_F()
	{
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "* " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( connected() ) advance_connections();
	}

	// Handler No-Advance
	void
	no_advance_handler()
	{
		shift_QSS( tE );
	}

	// Observer Advance
	void
	advance_observer( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		tX = t;
		x_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
		x_1_ = c_1( t );
		x_2_ = c_2( t );
		set_tE_unaligned();
		shift_QSS( tE );
		if ( connected() ) advance_connections_observer();
	}

	// Observer Advance: Stage 1
	void
	advance_observer_1( Time const t, Real const d )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		assert( d == p_1() );
		Time const tDel( t - tX );
		tX = t;
		x_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
		x_1_ = d;
	}

	// Observer Advance: Stage 2
	void
	advance_observer_2( Real const d )
	{
		x_2_ = p_2( d );
		set_tE_unaligned();
		shift_QSS( tE );
		if ( connected() ) advance_connections_observer();
	}

	// Observer Advance: Stage d
	void
	advance_observer_d() const
	{
		std::cout << "  " << name() << '(' << tX << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << " [q]" << '(' << std::noshowpos << tQ << std::showpos << ')' << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
	}

private: // Methods

	// Set QSS Tolerance
	void
	set_qTol()
	{
		qTol = std::max( rTol * std::abs( q_0_ ), aTol );
		assert( qTol > 0.0 );
	}

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE_aligned()
	{
		assert( tX <= tQ );
		assert( dt_min <= dt_max );
		Time dt( x_2_ != 0.0 ? std::sqrt( qTol / std::abs( x_2_ ) ) : infinity );
		dt = std::min( std::max( dt, dt_min ), dt_max );
		tE = ( dt != infinity ? tQ + dt : infinity );
		if ( ( options::inflection ) && ( x_2_ != 0.0 ) && ( signum( x_1_ ) != signum( x_2_ ) ) ) {
			Time const tI( tX - ( x_1_ / ( two * x_2_ ) ) );
			if ( tQ < tI ) tE = std::min( tE, tI );
		}
		tE_infinity_tQ();
	}

	// Set End Time: Quantized and Continuous Unaligned
	void
	set_tE_unaligned()
	{
		assert( tQ <= tX );
		assert( dt_min <= dt_max );
		Real const d_0( x_0_ - ( q_0_ + ( q_1_ * ( tX - tQ ) ) ) );
		Real const d_1( x_1_ - q_1_ );
		Time dt;
		if ( ( d_1 >= 0.0 ) && ( x_2_ >= 0.0 ) ) { // Upper boundary crossing
			dt = min_root_quadratic_upper( x_2_, d_1, d_0 - qTol );
		} else if ( ( d_1 <= 0.0 ) && ( x_2_ <= 0.0 ) ) { // Lower boundary crossing
			dt = min_root_quadratic_lower( x_2_, d_1, d_0 + qTol );
		} else { // Both boundaries can have crossings
			dt = min_root_quadratic_both( x_2_, d_1, d_0 + qTol, d_0 - qTol );
		}
		dt = std::min( std::max( dt, dt_min ), dt_max );
		tE = ( dt != infinity ? tX + dt : infinity );
		if ( ( options::inflection ) && ( x_2_ != 0.0 ) && ( signum( x_1_ ) != signum( x_2_ ) ) && ( signum( x_1_ ) == signum( q_1_ ) ) ) {
			Time const tI( tX - ( x_1_ / ( two * x_2_ ) ) );
			if ( tX < tI ) tE = std::min( tE, tI );
		}
		tE_infinity_tX();
	}

	// Coefficient 1 from FMU at Time tQ
	Real
	c_1() const
	{
		return c_1( tQ, q_0_ );
	}

	// Coefficient 2 from FMU
	Real
	p_2( Real const d ) const
	{
		return p_2( d, x_1_ );
	}

	// Coefficient 2 from FMU at Time tQ
	Real
	c_2() const
	{
		return c_2( tQ, x_1_ );
	}

	// Coefficient 2 from FMU at Time t
	Real
	c_2( Time const t ) const
	{
		return c_2( t, x_1_ );
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }; // Continuous rep coefficients
	Real q_0_{ 0.0 }, q_1_{ 0.0 }; // Quantized rep coefficients

}; // Variable_QSS2

} // fmu
} // QSS

#endif
