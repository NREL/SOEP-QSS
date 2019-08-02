// FMU-Based LIQSS3 Variable
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

#ifndef QSS_fmu_Variable_LIQSS3_hh_INCLUDED
#define QSS_fmu_Variable_LIQSS3_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable_QSS.hh>

namespace QSS {
namespace fmu {

// FMU-Based LIQSS3 Variable
class Variable_LIQSS3 final : public Variable_QSS
{

public: // Types

	using Super = Variable_QSS;

public: // Creation

	// Constructor
	Variable_LIQSS3(
	 std::string const & name,
	 Real const rTol,
	 Real const aTol,
	 Real const xIni,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Super( 3, name, rTol, aTol, xIni, fmu_me, var, der ),
	 x_0_( xIni ),
	 q_c_( xIni ),
	 q_0_( xIni )
	{
		set_qTol();
	}

public: // Properties

	// Continuous Value at Time t
	Real
	x( Time const t ) const
	{
		Time const tDel( t - tX );
		return x_0_ + ( ( x_1_ + ( ( x_2_ + ( x_3_ * tDel ) ) * tDel ) ) * tDel );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const t ) const
	{
		Time const tDel( t - tX );
		return x_1_ + ( ( ( two * x_2_ ) + ( three * x_3_ * tDel ) ) * tDel );
	}

	// Continuous Second Derivative at Time t
	Real
	x2( Time const t ) const
	{
		return ( two * x_2_ ) + ( six * x_3_ * ( t - tX ) );
	}

	// Continuous Third Derivative at Time t
	Real
	x3( Time const ) const
	{
		return six * x_3_;
	}

	// Quantized Value at Time t
	Real
	q( Time const t ) const
	{
		Time const tDel( t - tQ );
		return q_0_ + ( ( q_1_ + ( q_2_ * tDel ) ) * tDel );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const t ) const
	{
		return q_1_ + ( two * q_2_ * ( t - tQ ) );
	}

	// Quantized Second Derivative at Time t
	Real
	q2( Time const ) const
	{
		return two * q_2_;
	}

	// Simultaneous Value at Time t
	Real
	s( Time const t ) const
	{
		assert( ( t == tQ ) || ( st != events_->active_superdense_time() ) );
		return ( st == events_->active_superdense_time() ? q_c_ : q( t ) );
	}

public: // Methods

	// Initialization
	void
	init()
	{
		init_0();
		init_1();
		init_2();
		init_3();
	}

	// Initialization to a Value
	void
	init( Real const x )
	{
		init_0( x );
		init_1();
		init_2();
		init_3();
	}

	// Initialization: Stage 0
	void
	init_0()
	{
		init_observers();
		init_observees();
		fmu_set_real( x_0_ = q_c_ = q_0_ = xIni );
		set_qTol();
	}

	// Initialization to a Value: Stage 0
	void
	init_0( Real const x )
	{
		init_observers();
		init_observees();
		fmu_set_real( x_0_ = q_c_ = q_0_ = x );
		set_qTol();
	}

	// Initialization: Stage 1
	void
	init_1()
	{
		x_1_ = q_1_ = fmu_get_poly_1();
	}

	// Initialization: Stage 2
	void
	init_2()
	{
		x_2_ = q_2_ = fmu_get_poly_2();
	}

	// Initialization: Stage 3
	void
	init_3()
	{
		if ( self_observer ) {
			advance_LIQSS();
			fmu_set_real( x_0_ );
		} else {
			x_3_ = fmu_get_poly_3();
			q_0_ += signum( x_3_ ) * qTol;
		}
		set_tE_aligned();
		add_QSS( tE );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << q_2_ << "*t^2" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
	}

	// Set Current Tolerance
	void
	set_qTol()
	{
		qTol = std::max( rTol * std::abs( q_c_ ), aTol );
		assert( qTol > 0.0 );
	}

	// QSS Advance
	void
	advance_QSS()
	{
		Time const tDel( tE - tX );
		tX = tQ = tE;
		x_0_ = q_c_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
		set_qTol();
		fmu_set_observees_q( tQ );
		if ( self_observer ) {
			advance_LIQSS();
		} else {
			x_1_ = q_1_ = fmu_get_poly_1();
			x_2_ = q_2_ = fmu_get_poly_2();
			x_3_ = fmu_get_poly_3();
			q_0_ += signum( x_3_ ) * qTol;
		}
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << q_2_ << "*t^2" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( have_observers_ ) advance_observers();
		if ( have_connections ) advance_connections();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0()
	{
		Time const tDel( tE - tX );
		tX = tQ = tE;
		x_0_ = q_c_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
		set_qTol();
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1()
	{
		fmu_set_observees_s( tQ );
		x_1_ = q_1_ = fmu_get_poly_1();
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2()
	{
		x_2_ = q_2_ = fmu_get_poly_2();
	}

	// QSS Advance: Stage 3
	void
	advance_QSS_3()
	{
		if ( self_observer ) {
			advance_LIQSS();
			fmu_set_real( x_0_ );
		} else {
			x_3_ = fmu_get_poly_3();
			q_0_ += signum( x_3_ ) * qTol;
		}
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << q_2_ << "*t^2" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( have_connections ) advance_connections();
	}

	// Observer Advance
	void
	advance_observer( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		tX = t;
		x_0_ = x_0_ + ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
		x_1_ = fmu_get_poly_1();
		x_2_ = fmu_get_poly_2();
		x_3_ = fmu_get_poly_3();
		set_tE_unaligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "  " << name << '(' << tX << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << q_2_ << "*t^2" << " [q]" << '(' << std::noshowpos << tQ << std::showpos << ')' << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( have_connections ) advance_connections_observer();
	}

	// Observer Advance: Simultaneous
	void
	advance_observer_s( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		fmu_set_observees_q( t );
		if ( self_observer ) fmu_set_q( t );
		advance_observer( t );
	}

	// Handler Advance
	void
	advance_handler( Time const t )
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		x_0_ = q_c_ = q_0_ = fmu_get_real(); // Assume FMU ran zero-crossing handler
		set_qTol();
		fmu_set_observees_q( tX = tQ = t );
		x_1_ = q_1_ = fmu_get_poly_1();
		x_2_ = q_2_ = fmu_get_poly_2();
		x_3_ = fmu_get_poly_3();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << q_2_ << "*t^2" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( have_observers_ ) advance_observers();
		if ( have_connections ) advance_connections();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t )
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_c_ = q_0_ = fmu_get_real(); // Assume FMU ran zero-crossing handler
		set_qTol();
	}

	// Handler Advance: Stage 1
	void
	advance_handler_1()
	{
		fmu_set_observees_q( tQ );
		x_1_ = q_1_ = fmu_get_poly_1();
	}

	// Handler Advance: Stage 2
	void
	advance_handler_2()
	{
		x_2_ = q_2_ = fmu_get_poly_2();
	}

	// Handler Advance: Stage 3
	void
	advance_handler_3()
	{
		x_3_ = fmu_get_poly_3();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << q_2_ << "*t^2" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( have_connections ) advance_connections();
	}

	// Handler No-Advance
	void
	no_advance_handler()
	{
		shift_QSS( tE );
	}

private: // Methods

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE_aligned()
	{
		assert( tX <= tQ );
		assert( dt_min <= dt_max );
		Time dt( x_3_ != 0.0 ? std::cbrt( qTol / std::abs( x_3_ ) ) : infinity );
		dt = std::min( std::max( dt, dt_min ), dt_max );
		tE = ( dt != infinity ? tQ + dt : infinity );
		if ( ( options::inflection ) && ( x_3_ != 0.0 ) && ( signum( x_2_ ) != signum( x_3_ ) ) ) {
			Time const tI( tX - ( x_2_ / ( three * x_3_ ) ) );
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
		Time const tXQ( tX - tQ );
		Real const d_0( x_0_ - ( q_c_ + ( q_1_ + ( q_2_ * tXQ ) ) * tXQ ) );
		Real const d_1( x_1_ - ( q_1_ + ( two * q_2_ * tXQ ) ) );
		Real const d_2( x_2_ - q_2_ );
		Time dt;
		if ( ( x_3_ >= 0.0 ) && ( d_2 >= 0.0 ) && ( d_1 >= 0.0 ) ) { // Upper boundary crossing
			dt = min_root_cubic_upper( x_3_, d_2, d_1, d_0 - qTol );
		} else if ( ( x_3_ <= 0.0 ) && ( d_2 <= 0.0 ) && ( d_1 <= 0.0 ) ) { // Lower boundary crossing
			dt = min_root_cubic_lower( x_3_, d_2, d_1, d_0 + qTol );
		} else { // Both boundaries can have crossings
			dt = min_root_cubic_both( x_3_, d_2, d_1, d_0 + qTol, d_0 - qTol );
		}
		dt = std::min( std::max( dt, dt_min ), dt_max );
		tE = ( dt != infinity ? tX + dt : infinity );
		if ( ( options::inflection ) && ( x_3_ != 0.0 ) && ( signum( x_2_ ) != signum( x_3_ ) ) && ( signum( x_2_ ) == signum( q_2_ ) ) ) {
			Time const tI( tX - ( x_2_ / ( three * x_3_ ) ) );
			if ( tX < tI ) tE = std::min( tE, tI );
		}
		tE_infinity_tX();
	}

	// Advance Self-Observing Trigger
	void
	advance_LIQSS()
	{
		assert( qTol > 0.0 );
		assert( self_observer );
		assert( q_c_ == q_0_ );
		assert( x_0_ == q_0_ );

		// Value at +/- qTol
		Real const q_l( q_c_ - qTol );
		Real const q_u( q_c_ + qTol );

		// Derivatives at -qTol
		fmu_set_real( q_l );
		Real const x_1_l( fmu_get_poly_1() );
		Real const x_2_l( fmu_get_poly_2() );
		Real const x_3_l( fmu_get_poly_3() );
		int const x_3_l_s( signum( x_3_l ) );

		// Derivatives at +qTol
		fmu_set_real( q_u );
		Real const x_1_u( fmu_get_poly_1() );
		Real const x_2_u( fmu_get_poly_2() );
		Real const x_3_u( fmu_get_poly_3() );
		int const x_3_u_s( signum( x_3_u ) );

		// Set coefficients based on second derivative signs
		if ( ( x_3_l_s == -1 ) && ( x_3_u_s == -1 ) ) { // Downward curve-changing trajectory
			q_0_ = q_l;
			x_1_ = q_1_ = x_1_l;
			x_2_ = q_2_ = x_2_l;
			x_3_ = x_3_l;
		} else if ( ( x_3_l_s == +1 ) && ( x_3_u_s == +1 ) ) { // Upward curve-changing trajectory
			q_0_ = q_u;
			x_1_ = q_1_ = x_1_u;
			x_2_ = q_2_ = x_2_u;
			x_3_ = x_3_u;
		} else if ( ( x_3_l_s == 0 ) && ( x_3_u_s == 0 ) ) { // Non-curve-changing trajectory
			// Keep q_0_ == q_c_
			x_1_ = q_1_ = one_half * ( x_1_l + x_1_u ); // Interpolated 1st order coefficient at q_0_ == q_c_
			x_2_ = q_2_ = one_half * ( x_2_l + x_2_u ); // Interpolated 2nd order coefficient at q_0_ == q_c_
			x_3_ = 0.0;
		} else { // Quadratic trajectory
			q_0_ = std::min( std::max( ( ( q_l * x_3_u ) - ( q_u * x_3_l ) ) / ( x_3_u - x_3_l ), q_l ), q_u ); // Value where 2nd deriv is ~ 0 // Clipped in case of roundoff
			x_1_ = q_1_ = ( ( ( q_u - q_0_ ) * x_1_l ) + ( ( q_0_ - q_l ) * x_1_u ) ) / ( two * qTol ); // Interpolated 1st order coefficient at q_0_
			x_2_ = q_2_ = ( ( ( q_u - q_0_ ) * x_2_l ) + ( ( q_0_ - q_l ) * x_2_u ) ) / ( two * qTol ); // Interpolated 2nd order coefficient at q_0_
			x_3_ = 0.0;
		}
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }, x_3_{ 0.0 }; // Continuous rep coefficients
	Real q_c_{ 0.0 }, q_0_{ 0.0 }, q_1_{ 0.0 }, q_2_{ 0.0 }; // Quantized rep coefficients

};

} // fmu
} // QSS

#endif
