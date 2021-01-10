// FMU-Based LIQSS3 Variable
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2021 Objexx Engineering, Inc. All rights reserved.
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

private: // Types

	using Super::c_1;
	using Super::c_2;
	using Super::c_3;

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

public: // Methods

	// Initialization
	void
	init()
	{
		init_0();
		init_1();
		init_2();
		init_2_1();
		init_3();
		init_F();
	}

	// Initialization to a Value
	void
	init( Real const x )
	{
		init_0( x );
		init_1();
		init_2();
		init_2_1();
		init_3();
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
		x_1_ = q_1_ = p_1();
	}

	// Initialization: Stage 2
	void
	init_2()
	{
		x_2_ = s_2();
	}

	// Initialization: Stage 2.1
	void
	init_2_1()
	{
		q_2_ = x_2_; //ND Deferred
	}

	// Initialization: Stage 3
	void
	init_3()
	{
		set_qTol();
		if ( self_observer() ) {
			advance_LIQSS_s();
		} else {
			x_3_ = s_3();
		}
	}

	// Initialization: Stage Final
	void
	init_F()
	{
		if ( self_observer() ) {
			q_0_ = l_0_;
			q_1_ = x_1_;
			q_2_ = x_2_;
		} else {
			q_0_ += signum( x_3_ ) * qTol;
		}
		set_tE_aligned();
		add_QSS( tE );
		if ( options::output::d ) std::cout << "! " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << q_2_ << "*t^2" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
	}

	// QSS Advance
	void
	advance_QSS() override final
	{
		Time const tDel( tE - tX );
		tX = tQ = tE;
		x_0_ = q_c_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
		set_qTol();
		if ( self_observer() ) {
			advance_LIQSS();
		} else {
			x_1_ = q_1_ = h_1();
			x_2_ = q_2_ = h_2();
			x_3_ = h_3();
			q_0_ += signum( x_3_ ) * qTol;
		}
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "! " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << q_2_ << "*t^2" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( observed() ) advance_observers();
		if ( connected() ) advance_connections();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0() override final
	{
		Time const tDel( tE - tX );
		tX = tQ = tE;
		x_0_ = q_c_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1( Real const x_1 ) override final
	{
		x_1_ = q_1_ = x_1;
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2( Real const x_1_m, Real const x_1_p ) override final
	{
		x_2_ = n_2( x_1_m, x_1_p );
	}

	// QSS Advance: Stage 2.1
	void
	advance_QSS_2_1() override final
	{
		q_2_ = x_2_; //ND Deferred
	}

	// QSS Advance: Stage 3
	void
	advance_QSS_3() override final
	{
		set_qTol();
		if ( self_observer() ) {
			advance_LIQSS_s();
		} else {
			x_3_ = n_3();
		}
	}

	// QSS Advance: Stage Final
	void
	advance_QSS_F() override final
	{
		if ( self_observer() ) {
			q_0_ = l_0_;
			q_1_ = x_1_;
			q_2_ = x_2_;
		} else {
			q_0_ += signum( x_3_ ) * qTol;
		}
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "= " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << q_2_ << "*t^2" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( connected() ) advance_connections();
	}

	// Handler Advance
	void
	advance_handler( Time const t ) override final
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_c_ = q_0_ = p_0();
		x_1_ = q_1_ = h_1();
		x_2_ = q_2_ = c_2();
		x_3_ = c_3();
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "* " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << q_2_ << "*t^2" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( observed() ) advance_observers();
		if ( connected() ) advance_connections();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t ) override final
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_c_ = q_0_ = p_0();
	}

	// Handler Advance: Stage 1
	void
	advance_handler_1() override final
	{
		x_1_ = q_1_ = h_1();
	}

	// Handler Advance: Stage 2
	void
	advance_handler_2() override final
	{
		x_2_ = s_2();
	}

	// Handler Advance: Stage 2.1
	void
	advance_handler_2_1() override final
	{
		q_2_ = x_2_; //ND Deferred
	}

	// Handler Advance: Stage 3
	void
	advance_handler_3() override final
	{
		x_3_ = s_3();
	}

	// Handler Advance: Stage Final
	void
	advance_handler_F() override final
	{
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "* " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << q_2_ << "*t^2" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( connected() ) advance_connections();
	}

	// Handler No-Advance
	void
	no_advance_handler() override final
	{
		shift_QSS( tE );
	}

	// Observer Advance
	void
	advance_observer( Time const t ) override final
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		tX = t;
		x_0_ += ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
		x_1_ = c_1( t );
		x_2_ = c_2( t );
		x_3_ = c_3();
		set_tE_unaligned();
		shift_QSS( tE );
		if ( connected() ) advance_connections_observer();
	}

	// Observer Advance: Stage 1
	void
	advance_observer_1( Time const t, Real const x_1 ) override final
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		assert( x_1 == p_1() );
		Time const tDel( t - tX );
		tX = t;
		x_0_ += ( ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel );
		x_1_ = x_1;
	}

	// Observer Advance: Stage 1 Parallel
	void
	advance_observer_1_parallel( Time const t, Real const x_1 ) override final
	{
		advance_observer_1( t, x_1 );
	}

	// Observer Advance: Stage 2
	void
	advance_observer_2( Real const x_1_m, Real const x_1_p ) override final
	{
		x_2_ = n_2( x_1_m, x_1_p );
	}

	// Observer Advance: Stage 2 Parallel
	void
	advance_observer_2_parallel( Real const x_1_m, Real const x_1_p ) override final
	{
		x_2_ = n_2( x_1_m, x_1_p );
	}

	// Observer Advance: Stage 3
	void
	advance_observer_3() override final
	{
		x_3_ = n_3();
		set_tE_unaligned();
		shift_QSS( tE );
		if ( connected() ) advance_connections_observer();
	}

	// Observer Advance: Stage 3 Parallel
	void
	advance_observer_3_parallel() override final
	{
		x_3_ = n_3();
	}

	// Observer Advance: Stage Final Parallel
	void
	advance_observer_F_parallel() override final
	{
		set_tE_unaligned();
	}

	// Observer Advance: Stage Final Serial
	void
	advance_observer_F_serial() override final
	{
		shift_QSS( tE );
		if ( connected() ) advance_connections_observer();
	}

	// Observer Advance: Stage d
	void
	advance_observer_d() const override final
	{
		std::cout << "  " << name() << '(' << tX << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << q_2_ << "*t^2" << " [q]" << '(' << std::noshowpos << tQ << std::showpos << ')' << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
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
		Time dt( x_3_ != 0.0 ? std::cbrt( qTol / std::abs( x_3_ ) ) : infinity );
		dt = std::min( std::max( dt_infinity( dt ), dt_min ), dt_max );
		tE = ( dt != infinity ? tQ + dt : infinity );
		if ( ( options::inflection ) && ( x_3_ != 0.0 ) && ( signum( x_2_ ) != signum( x_3_ ) ) ) {
			Time const tI( tX - ( x_2_ / ( three * x_3_ ) ) );
			if ( tQ < tI ) tE = std::min( tE, tI );
		}
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
		dt = std::min( std::max( dt_infinity( dt ), dt_min ), dt_max );
		tE = ( dt != infinity ? tX + dt : infinity );
		if ( ( options::inflection ) && ( x_3_ != 0.0 ) && ( signum( x_2_ ) != signum( x_3_ ) ) && ( signum( x_2_ ) == signum( q_2_ ) ) ) {
			Time const tI( tX - ( x_2_ / ( three * x_3_ ) ) );
			if ( tX < tI ) tE = std::min( tE, tI );
		}
	}

	// Advance Self-Observing Trigger
	void
	advance_LIQSS();

	// Advance Self-Observing Trigger: Simultaneous
	void
	advance_LIQSS_s();

	// Coefficient 1 from FMU at Time tQ
	Real
	c_1() const
	{
		return c_1( tQ, q_0_ );
	}

	// Coefficient 2 from FMU
	Real
	n_2( Real const x_1_m, Real const x_1_p ) const
	{
		return options::one_over_four_dtND * ( ( x_1_p_ = x_1_p ) - ( x_1_m_ = x_1_m ) ); //ND Centered difference
	}

	// Coefficient 2 from FMU at Time tQ
	Real
	c_2() const
	{
		return c_2( tQ );
	}

	// Coefficient 2 from FMU at Time t
	Real
	c_2( Time const t ) const
	{
		Time tN( t - options::dtND );
		fmu_set_time( tN );
		x_1_m_ = c_1( tN );
		tN = t + options::dtND;
		fmu_set_time( tN );
		x_1_p_ = c_1( tN );
		fmu_set_time( t );
		return options::one_over_four_dtND * ( x_1_p_ - x_1_m_ ); //ND Centered difference
	}

	// Coefficient 2 from FMU at Time tQ
	Real
	h_2() const
	{
		return h_2( tQ );
	}

	// Coefficient 2 from FMU at Time t
	Real
	h_2( Time const t ) const
	{
		Time tN( t - options::dtND );
		fmu_set_time( tN );
		x_1_m_ = h_1( tN );
		tN = t + options::dtND;
		fmu_set_time( tN );
		x_1_p_ = h_1( tN );
		fmu_set_time( t );
		return options::one_over_four_dtND * ( x_1_p_ - x_1_m_ ); //ND Centered difference
	}

	// Coefficient 2 from FMU at Time tQ
	Real
	s_2() const
	{
		return c_2( tQ, x_1_ );
	}

	// Coefficient 3 from FMU
	Real
	n_3() const
	{
		return options::one_over_six_dtND_squared * ( ( x_1_p_ - x_1_ ) + ( x_1_m_ - x_1_ ) ); //ND Centered difference
	}

	// Coefficient 3 from FMU
	Real
	c_3() const
	{
		return options::one_over_six_dtND_squared * ( ( x_1_p_ - x_1_ ) + ( x_1_m_ - x_1_ ) ); //ND Centered difference
	}

	// Coefficient 3 from FMU
	Real
	h_3() const
	{
		return options::one_over_six_dtND_squared * ( ( x_1_p_ - x_1_ ) + ( x_1_m_ - x_1_ ) ); //ND Centered difference
	}

	// Coefficient 3 from FMU
	Real
	s_3() const
	{
		return c_3( tQ, x_1_ );
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }, x_3_{ 0.0 }; // Continuous rep coefficients
	Real q_c_{ 0.0 }, q_0_{ 0.0 }, q_1_{ 0.0 }, q_2_{ 0.0 }; // Quantized rep coefficients
	Real l_0_{ 0.0 }; // LIQSS-adjusted coefficient
	mutable Real x_1_m_{ 0.0 }, x_1_p_{ 0.0 }; // Coefficient 1 at minus and plus delta-t for numeric differentiation

}; // Variable_LIQSS3

} // fmu
} // QSS

#endif
