// FMU-Based LIQSS1 Variable
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

#ifndef QSS_fmu_Variable_LIQSS1_hh_INCLUDED
#define QSS_fmu_Variable_LIQSS1_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable_QSS.hh>

namespace QSS {
namespace fmu {

// FMU-Based LIQSS1 Variable
class Variable_LIQSS1 final : public Variable_QSS
{

public: // Types

	using Super = Variable_QSS;

public: // Creation

	// Constructor
	Variable_LIQSS1(
	 std::string const & name,
	 Real const rTol,
	 Real const aTol,
	 Real const xIni,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Super( 1, name, rTol, aTol, xIni, fmu_me, var, der ),
	 x_0_( xIni ),
	 q_c_( xIni ),
	 q_0_( xIni )
	{
		set_qTol();
	}

public: // Predicate

	// LIQSS Variable?
	bool
	is_LIQSS() const
	{
		return true;
	}

public: // Property

	// Continuous Value at Time t
	Real
	x( Time const t ) const
	{
		return x_0_ + ( x_1_ * ( t - tX ) );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const ) const
	{
		return x_1_;
	}

	// Quantized Value at Time t
	Real
	q( Time const ) const
	{
		return q_0_;
	}

public: // Methods

	// Initialization
	void
	init()
	{
		init_0();
		init_1();
		init_F();
	}

	// Initialization to a Value
	void
	init( Real const x )
	{
		init_0( x );
		init_1();
		init_F();
	}

	// Initialization: Stage 0
	void
	init_0()
	{
		init_observers();
		init_observees();
		fmu_set_real( x_0_ = q_c_ = q_0_ = xIni );
	}

	// Initialization to a Value: Stage 0
	void
	init_0( Real const x )
	{
		init_observers();
		init_observees();
		fmu_set_real( x_0_ = q_c_ = q_0_ = x );
	}

	// Initialization: Stage 1
	void
	init_1()
	{
		set_qTol();
		if ( self_observer ) {
			advance_LIQSS_i();
		} else {
			x_1_ = p_1();
		}
	}

	// Initialization: Stage Final
	void
	init_F()
	{
		if ( self_observer ) {
			q_0_ = l_0_;
		} else {
			q_0_ += signum( x_1_ ) * qTol;
		}
		set_tE_aligned();
		add_QSS( tE );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << " [q]" << "   = " << x_0_ << x_1_ << "*t" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
	}

	// QSS Advance
	void
	advance_QSS()
	{
		Time const tDel( tE - tX );
		tX = tQ = tE;
		x_0_ = q_c_ = q_0_ = x_0_ + ( x_1_ * tDel );
		set_qTol();
		if ( self_observer ) {
			advance_LIQSS();
		} else {
			x_1_ = h_1();
			q_0_ += signum( x_1_ ) * qTol;
		}
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << " [q]" << "   = " << x_0_ << x_1_ << "*t" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( observed_ ) advance_observers();
		if ( have_connections ) advance_connections();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0()
	{
		Time const tDel( tE - tX );
		tX = tQ = tE;
		x_0_ = q_c_ = q_0_ = x_0_ + ( x_1_ * tDel );
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1()
	{
		set_qTol();
		if ( self_observer ) {
			advance_LIQSS_s();
		} else {
			x_1_ = h_1();
		}
	}

	// QSS Advance: Stage Final
	void
	advance_QSS_F()
	{
		if ( self_observer ) {
			q_0_ = l_0_;
		} else {
			q_0_ += signum( x_1_ ) * qTol;
		}
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << " [q]" << "   = " << x_0_ << x_1_ << "*t" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( have_connections ) advance_connections();
	}

	// Handler Advance
	void
	advance_handler( Time const t )
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_c_ = q_0_ = p_0(); // Assume FMU ran zero-crossing handler
		x_1_ = h_1();
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << " [q]" << "   = " << x_0_ << x_1_ << "*t" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( observed_ ) advance_observers();
		if ( have_connections ) advance_connections();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t )
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_c_ = q_0_ = p_0(); // Assume FMU ran zero-crossing handler
	}

	// Handler Advance: Stage 1
	void
	advance_handler_1()
	{
		x_1_ = h_1();
	}

	// Handler Advance: Stage Final
	void
	advance_handler_F()
	{
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << " [q]" << "   = " << x_0_ << x_1_ << "*t" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( have_connections ) advance_connections();
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
		x_0_ = x_0_ + ( x_1_ * tDel );
		x_1_ = c_1( t );
		set_tE_unaligned();
		shift_QSS( tE );
		if ( have_connections ) advance_connections_observer();
	}

	// Observer Advance: Stage 1
	void
	advance_observer_1( Time const t, Real const d )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		assert( d == p_1() );
		Time const tDel( t - tX );
		tX = t;
		x_0_ = x_0_ + ( x_1_ * tDel );
		x_1_ = d;
		set_tE_unaligned();
		shift_QSS( tE );
		if ( have_connections ) advance_connections_observer();
	}

	// Observer Advance: Stage d
	void
	advance_observer_d() const
	{
		std::cout << "  " << name << '(' << tX << ')' << " = " << std::showpos << q_0_ << " [q]" << '(' << std::noshowpos << tQ << std::showpos << ')' << "   = " << x_0_ << x_1_ << "*t" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
	}

private: // Methods

	// Set QSS Tolerance
	void
	set_qTol()
	{
		qTol = std::max( rTol * std::abs( q_c_ ), aTol );
		assert( qTol > 0.0 );
	}

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE_aligned()
	{
		assert( tX <= tQ );
		assert( dt_min <= dt_max );
		Time dt( x_1_ != 0.0 ? qTol / std::abs( x_1_ ) : infinity );
		dt = std::min( std::max( dt, dt_min ), dt_max );
		tE = ( dt != infinity ? tQ + dt : infinity );
		tE_infinity_tQ();
	}

	// Set End Time: Quantized and Continuous Unaligned
	void
	set_tE_unaligned()
	{
		assert( tQ <= tX );
		assert( dt_min <= dt_max );
		Time dt(
		 ( x_1_ > 0.0 ? ( q_c_ + qTol - x_0_ ) / x_1_ :
		 ( x_1_ < 0.0 ? ( q_c_ - qTol - x_0_ ) / x_1_ :
		 infinity ) ) );
		dt = std::min( std::max( dt, dt_min ), dt_max );
		tE = ( dt != infinity ? tX + dt : infinity );
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

		// Derivative at +/- qTol
		fmu_set_observees_q( tQ );
		fmu_set_real( q_l );
		Real const x_1_l( p_1() );
		int const x_1_l_s( signum( x_1_l ) );
		fmu_set_real( q_u );
		Real const x_1_u( p_1() );
		int const x_1_u_s( signum( x_1_u ) );

		// Set coefficients based on derivative signs
		if ( ( x_1_l_s == -1 ) && ( x_1_u_s == -1 ) ) { // Downward trajectory
			q_0_ = q_l;
			x_1_ = x_1_l;
		} else if ( ( x_1_l_s == +1 ) && ( x_1_u_s == +1 ) ) { // Upward trajectory
			q_0_ = q_u;
			x_1_ = x_1_u;
		} else if ( ( x_1_l_s == 0 ) && ( x_1_u_s == 0 ) ) { // Flat trajectory
			// Keep q_0_ == q_c_
			x_1_ = 0.0;
		} else { // Flat trajectory
			q_0_ = std::min( std::max( ( ( q_l * x_1_u ) - ( q_u * x_1_l ) ) / ( x_1_u - x_1_l ), q_l ), q_u ); // Value where deriv is ~ 0 // Clipped in case of roundoff
			x_1_ = 0.0;
		}
	}

	// Advance Self-Observing Trigger: Initialization
	void
	advance_LIQSS_i()
	{
		assert( qTol > 0.0 );
		assert( self_observer );
		assert( q_c_ == q_0_ );
		assert( x_0_ == q_0_ );

		// Value at +/- qTol
		Real const q_l( q_c_ - qTol );
		Real const q_u( q_c_ + qTol );

		// Derivative at +/- qTol
		fmu_set_real( q_l );
		Real const x_1_l( p_1() );
		int const x_1_l_s( signum( x_1_l ) );
		fmu_set_real( q_u );
		Real const x_1_u( p_1() );
		int const x_1_u_s( signum( x_1_u ) );
		fmu_set_real( q_c_ );

		// Set coefficients based on derivative signs
		if ( ( x_1_l_s == -1 ) && ( x_1_u_s == -1 ) ) { // Downward trajectory
			l_0_ = q_l;
			x_1_ = x_1_l;
		} else if ( ( x_1_l_s == +1 ) && ( x_1_u_s == +1 ) ) { // Upward trajectory
			l_0_ = q_u;
			x_1_ = x_1_u;
		} else if ( ( x_1_l_s == 0 ) && ( x_1_u_s == 0 ) ) { // Flat trajectory
			l_0_ = q_c_;
			x_1_ = 0.0;
		} else { // Flat trajectory
			l_0_ = std::min( std::max( ( ( q_l * x_1_u ) - ( q_u * x_1_l ) ) / ( x_1_u - x_1_l ), q_l ), q_u ); // Value where deriv is ~ 0 // Clipped in case of roundoff
			x_1_ = 0.0;
		}
	}

	// Advance Self-Observing Trigger: Simultaneous
	void
	advance_LIQSS_s()
	{
		assert( qTol > 0.0 );
		assert( self_observer );
		assert( q_c_ == q_0_ );
		assert( x_0_ == q_0_ );

		// Value at +/- qTol
		Real const q_l( q_c_ - qTol );
		Real const q_u( q_c_ + qTol );

		// Derivative at +/- qTol
		fmu_set_observees_q( tQ );
		fmu_set_real( q_l );
		Real const x_1_l( p_1() );
		int const x_1_l_s( signum( x_1_l ) );
		fmu_set_real( q_u );
		Real const x_1_u( p_1() );
		int const x_1_u_s( signum( x_1_u ) );

		// Set coefficients based on derivative signs
		if ( ( x_1_l_s == -1 ) && ( x_1_u_s == -1 ) ) { // Downward trajectory
			l_0_ = q_l;
			x_1_ = x_1_l;
		} else if ( ( x_1_l_s == +1 ) && ( x_1_u_s == +1 ) ) { // Upward trajectory
			l_0_ = q_u;
			x_1_ = x_1_u;
		} else if ( ( x_1_l_s == 0 ) && ( x_1_u_s == 0 ) ) { // Flat trajectory
			l_0_ = q_c_;
			x_1_ = 0.0;
		} else { // Flat trajectory
			l_0_ = std::min( std::max( ( ( q_l * x_1_u ) - ( q_u * x_1_l ) ) / ( x_1_u - x_1_l ), q_l ), q_u ); // Value where deriv is ~ 0 // Clipped in case of roundoff
			x_1_ = 0.0;
		}
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }; // Continuous rep coefficients
	Real q_c_{ 0.0 }, q_0_{ 0.0 }; // Quantized rep coefficients
	Real l_0_{ 0.0 }; // LIQSS-adjusted coefficient

};

} // fmu
} // QSS

#endif
