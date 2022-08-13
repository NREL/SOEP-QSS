// FMU-Based QSS1 Variable
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

#ifndef QSS_fmu_Variable_QSS1_hh_INCLUDED
#define QSS_fmu_Variable_QSS1_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable_QSS.hh>

namespace QSS {
namespace fmu {

// FMU-Based QSS1 Variable
class Variable_QSS1 final : public Variable_QSS
{

public: // Types

	using Super = Variable_QSS;

private: // Types

	using Super::c_1;

public: // Creation

	// Constructor
	Variable_QSS1(
	 std::string const & name,
	 Real const rTol,
	 Real const aTol,
	 Real const zTol,
	 Real const xIni,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Super( 1, name, rTol, aTol, zTol, xIni, fmu_me, var, der ),
	 x_0_( xIni ),
	 q_0_( xIni )
	{
		set_qTol();
	}

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
	q( Time const ) const override
	{
		return q_0_;
	}

public: // Methods

	// Initialization
	void
	init() override
	{
		init_0();
		init_1();
		init_F();
	}

	// Initialization to a Value
	void
	init( Real const x ) override
	{
		init_0( x );
		init_1();
		init_F();
	}

	// Initialization: Stage 0
	void
	init_0() override
	{
		init_observers();
		init_observees();
		fmu_set_real( x_0_ = q_0_ = xIni );
	}

	// Initialization to a Value: Stage 0
	void
	init_0( Real const x ) override
	{
		init_observers();
		init_observees();
		fmu_set_real( x_0_ = q_0_ = x );
	}

	// Initialization: Stage 1
	void
	init_1() override
	{
		x_1_ = p_1();
	}

	// Initialization: Stage Final
	void
	init_F() override
	{
		set_qTol();
		set_tE_aligned();
		add_QSS( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << " [q]" << "   = " << x_0_ << x_1_ << x_delta << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
	}

	// QSS Advance
	void
	advance_QSS() override
	{
		if ( options::stiff ) liqss_qss_ratio_pass();
		Time const tDel( tE - tX );
		tX = tQ = tE;
		x_0_ = q_0_ = x_0_ + ( x_1_ * tDel );
		x_1_ = c_1();
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << " [q]" << "   = " << x_0_ << x_1_ << x_delta << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
		if ( observed() ) advance_observers();
		if ( connected() ) advance_connections();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0() override
	{
		if ( options::stiff ) liqss_qss_ratio_pass();
		Time const tDel( tE - tX );
		tX = tQ = tE;
		x_0_ = q_0_ = x_0_ + ( x_1_ * tDel );
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
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "!= " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << " [q]" << "   = " << x_0_ << x_1_ << x_delta << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
		if ( connected() ) advance_connections();
	}

	// QSS Advance LIQSS/QSS Step Ratio
	Real
	advance_LIQSS_QSS_step_ratio() override
	{
		if ( !self_observer() ) return 1.0; // Same step size

		Time const t_fmu( fmu_get_time() ); // Save FMU time

		Time const tDel( tE - tX );
		Real const x_0( x_0_ + ( x_1_ * tDel ) );
		Real const q( std::max( rTol * std::abs( x_0 ), aTol ) );
		Real x_1;

		fmu_set_time( tE );

		// QSS
		x_1 = c_1( tE, x_0 );
		Time const dt_QSS( x_1 != 0.0 ? q / std::abs( x_1 ) : infinity );

		// LIQSS /////

		// Value at +/- q
		Real const q_l( x_0 - q );
		Real const q_u( x_0 + q );

		// Derivative at +/- q
		fmu_set_observees_q( tE );
		fmu_set_real( q_l );
		Real const x_1_l( p_1() );
		int const x_1_l_s( signum( x_1_l ) );
		fmu_set_real( q_u );
		Real const x_1_u( p_1() );
		int const x_1_u_s( signum( x_1_u ) );

		// Set coefficients based on derivative signs
		if ( ( x_1_l_s == -1 ) && ( x_1_u_s == -1 ) ) { // Downward trajectory
			x_1 = x_1_l;
		} else if ( ( x_1_l_s == +1 ) && ( x_1_u_s == +1 ) ) { // Upward trajectory
			x_1 = x_1_u;
		} else { // Flat trajectory
			x_1 = 0.0;
		}
		Time const dt_LIQSS( x_1 != 0.0 ? q / std::abs( x_1 ) : infinity );

		fmu_set_time( t_fmu ); // Restore FMU time

		return ( dt_QSS > 0.0 ? dt_LIQSS / dt_QSS : ( dt_LIQSS > 0.0 ? infinity : 1.0 ) );
	}

	// Handler Advance
	void
	advance_handler( Time const t ) override
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_0_ = p_0(); // Assume FMU ran event handler
		x_1_ = h_1();
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "*  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << " [q]" << "   = " << x_0_ << x_1_ << x_delta << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
		if ( observed() ) advance_observers();
		if ( connected() ) advance_connections();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t ) override
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_0_ = p_0(); // Assume FMU ran event handler
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
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "*= " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << " [q]" << "   = " << x_0_ << x_1_ << x_delta << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
		if ( connected() ) advance_connections();
	}

	// Handler No-Advance
	void
	no_advance_handler() override
	{
		shift_QSS( tE );
	}

	// Observer Advance
	void
	advance_observer( Time const t ) override
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		tX = t;
		x_0_ += ( x_1_ * tDel );
		x_1_ = c_1( t );
		set_tE_unaligned();
		shift_QSS( tE );
		if ( connected() ) advance_connections_observer();
	}

	// Observer Advance: Stage 1
	void
	advance_observer_1( Time const t, Real const x_1 ) override
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		assert( x_1 == p_1() );
		Time const tDel( t - tX );
		tX = t;
		x_0_ += ( x_1_ * tDel );
		x_1_ = x_1;
		set_tE_unaligned();
		shift_QSS( tE );
		if ( connected() ) advance_connections_observer();
	}

	// Observer Advance: Stage 1 Parallel
	void
	advance_observer_1_parallel( Time const t, Real const x_1 ) override
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		assert( x_1 == p_1() );
		Time const tDel( t - tX );
		tX = t;
		x_0_ += ( x_1_ * tDel );
		x_1_ = x_1;
	}

	// Observer Advance: Stage Final Parallel
	void
	advance_observer_F_parallel() override
	{
		set_tE_unaligned();
	}

	// Observer Advance: Stage Final Serial
	void
	advance_observer_F_serial() override
	{
		shift_QSS( tE );
		if ( connected() ) advance_connections_observer();
	}

	// Observer Advance: Stage d
	void
	advance_observer_d() const override
	{
		std::cout << " ^ " << name() << '(' << tX << ')' << " = " << std::showpos << q_0_ << " [q(" << std::noshowpos << tQ << std::showpos << ")]" << "   = " << x_0_ << x_1_ << x_delta << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
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
		Time dt( x_1_ != 0.0 ? qTol / std::abs( x_1_ ) : infinity );
		dt = std::min( std::max( dt_infinity( dt ), dt_min ), dt_max );
		tE = ( dt != infinity ? tQ + dt : infinity );
	}

	// Set End Time: Quantized and Continuous Unaligned
	void
	set_tE_unaligned()
	{
		assert( tQ <= tX );
		assert( dt_min <= dt_max );
		Time dt(
		 ( x_1_ > 0.0 ? ( q_0_ + qTol - x_0_ ) / x_1_ :
		 ( x_1_ < 0.0 ? ( q_0_ - qTol - x_0_ ) / x_1_ :
		 infinity ) ) );
		dt = std::min( std::max( dt_infinity( dt ), dt_min ), dt_max );
		tE = ( dt != infinity ? tX + dt : infinity );
	}

	// Coefficient 1 from FMU at Time tQ
	Real
	c_1() const
	{
		return c_1( tQ, x_0_ );
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }; // Continuous rep coefficients
	Real q_0_{ 0.0 }; // Quantized rep coefficients

}; // Variable_QSS1

} // fmu
} // QSS

#endif
