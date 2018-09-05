// FMU-Based LIQSS2 Variable
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

#ifndef QSS_fmu_Variable_LIQSS2_hh_INCLUDED
#define QSS_fmu_Variable_LIQSS2_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable_QSS.hh>

namespace QSS {
namespace fmu {

// FMU-Based LIQSS2 Variable
class Variable_LIQSS2 final : public Variable_QSS
{

public: // Types

	using Super = Variable_QSS;

public: // Creation

	// Constructor
	explicit
	Variable_LIQSS2(
	 std::string const & name,
	 Real const rTol = 1.0e-4,
	 Real const aTol = 1.0e-6,
	 Real const xIni = 0.0,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Super( name, rTol, aTol, xIni, var, der ),
	 x_0_( xIni ),
	 q_c_( xIni ),
	 q_0_( xIni )
	{
		set_qTol();
	}

public: // Properties

	// Order of Method
	int
	order() const
	{
		return 2;
	}

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

	// Simultaneous Value at Time t
	Real
	s( Time const t ) const
	{
		assert( ( st != events.active_superdense_time() ) || ( t == tQ ) );
		return ( st == events.active_superdense_time() ? q_c_ : q_0_ + ( q_1_ * ( t - tQ ) ) );
	}

	// Simultaneous Numeric Differentiation Value at Time t
	Real
	sn( Time const t ) const
	{
		return ( st == events.active_superdense_time() ? q_c_ + ( s_1_ * ( t - tQ ) ) : q_0_ + ( q_1_ * ( t - tQ ) ) );
	}

	// Simultaneous First Derivative at Time t
	Real
	s1( Time const ) const
	{
		return ( st == events.active_superdense_time() ? s_1_ : q_1_ );
	}

public: // Methods

	// Initialization
	void
	init()
	{
		init_0();
		init_1();
		init_2();
	}

	// Initialization to a Value
	void
	init( Real const x )
	{
		init_0( x );
		init_1();
		init_2();
	}

	// Initialization: Stage 0
	void
	init_0()
	{
		init_observers();
		init_observees();
		fmu_set_value( x_0_ = q_c_ = q_0_ = xIni );
		set_qTol();
	}

	// Initialization to a Value: Stage 0
	void
	init_0( Real const x )
	{
		init_observers();
		init_observees();
		fmu_set_value( x_0_ = q_c_ = q_0_ = x );
		set_qTol();
	}

	// Initialization: Stage 1
	void
	init_1()
	{
		if ( self_observer ) {
			advance_LIQSS_1();
			fmu_set_value( x_0_ );
		}
		x_1_ = q_1_ = s_1_ = fmu_get_deriv();
	}

	// Initialization: Stage 2
	void
	init_2()
	{
		if ( self_observer ) {
			tN = tQ + options::dtNum;
			advance_LIQSS_2();
			fmu_set_sn( tN );
		} else {
			x_2_ = options::one_half_over_dtNum * ( fmu_get_deriv() - x_1_ ); // Forward Euler //API one_half * fmu_get_deriv2() when 2nd derivative is available
			q_0_ += signum( x_2_ ) * qTol;
		}
		set_tE_aligned();
		add_QSS( tE );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
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
		Time const tDel( ( tQ = tE ) - tX );
		x_0_ = q_c_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
		set_qTol();
		fmu_set_observees_q( tX = tQ );
		if ( self_observer ) {
			advance_LIQSS_1();
			fmu::set_time( tN = tQ + options::dtNum );
			fmu_set_observees_q( tN );
			advance_LIQSS_2();
		} else {
			x_1_ = q_1_ = fmu_get_deriv();
			fmu::set_time( tN = tQ + options::dtNum );
			fmu_set_observees_q( tN );
			x_2_ = options::one_half_over_dtNum * ( fmu_get_deriv() - x_1_ ); // Forward Euler //API one_half * fmu_get_deriv2() when 2nd derivative is available
			q_0_ += signum( x_2_ ) * qTol;
		}
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( have_observers_ ) advance_observers_tQ();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0()
	{
		Time const tDel( ( tQ = tE ) - tX );
		x_0_ = q_c_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
		tX = tE;
		set_qTol();
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1()
	{
		fmu_set_observees_s( tQ );
		if ( self_observer ) {
			advance_LIQSS_1();
			fmu_set_value( x_0_ );
		}
		x_1_ = q_1_ = s_1_ = fmu_get_deriv();
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2()
	{
		fmu_set_observees_sn( tN = tQ + options::dtNum );
		if ( self_observer ) {
			advance_LIQSS_2();
		} else {
			x_2_ = options::one_half_over_dtNum * ( fmu_get_deriv() - x_1_ ); // Forward Euler //API one_half * fmu_get_deriv2() when 2nd derivative is available
			q_0_ += signum( x_2_ ) * qTol;
		}
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
	}

	// Observer Advance: Stage 1
	void
	advance_observer_1( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		fmu_set_observees_q( t );
		if ( self_observer ) fmu_set_q( t );
		Time const tDel( t - tX );
		tX = t;
		x_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
		x_1_ = fmu_get_deriv();
	}

	// Observer Advance: Stage 1
	void
	advance_observer_1( Time const t, Real const d )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		assert( d == fmu_get_deriv() );
		Time const tDel( t - tX );
		tX = t;
		x_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
		x_1_ = d;
	}

	// Observer Advance: Stage 2
	void
	advance_observer_2( Time const t )
	{
		assert( tX <= t );
		fmu_set_observees_q( t );
		if ( self_observer ) fmu_set_q( t );
		x_2_ = options::one_half_over_dtNum * ( fmu_get_deriv() - x_1_ ); // Forward Euler //API one_half * fmu_get_deriv2() when 2nd derivative is available
		set_tE_unaligned();
		shift_QSS( tE );
	}

	// Observer Advance: Stage 2
	void
	advance_observer_2( Time const t, Real const d )
	{
		assert( tX <= t );
		assert( d == fmu_get_deriv() );
		(void)t; // Suppress unused parameter warning
		x_2_ = options::one_half_over_dtNum * ( d - x_1_ ); // Forward Euler //API one_half * fmu_get_deriv2() when 2nd derivative is available
		set_tE_unaligned();
		shift_QSS( tE );
	}

	// Observer Advance: Stage d
	void
	advance_observer_d() const
	{
		std::cout << "  " << name << '(' << tX << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << " [q]" << '(' << std::noshowpos << tQ << std::showpos << ')' << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
	}

	// Handler Advance
	void
	advance_handler( Time const t )
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		x_0_ = q_c_ = q_0_ = fmu_get_value(); // Assume FMU ran zero-crossing handler
		set_qTol();
		fmu_set_observees_q( tX = tQ = t );
		if ( self_observer ) fmu_set_value( q_0_ );
		x_1_ = q_1_ = fmu_get_deriv();
		fmu::set_time( tN = tQ + options::dtNum );
		fmu_set_observees_q( tN );
		if ( self_observer ) fmu_set_q( tN );
		x_2_ = options::one_half_over_dtNum * ( fmu_get_deriv() - x_1_ ); // Forward Euler
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
		if ( have_observers_ ) advance_observers_tQ();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t )
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_c_ = q_0_ = fmu_get_value(); // Assume FMU ran zero-crossing handler
		set_qTol();
	}

	// Handler Advance: Stage 1
	void
	advance_handler_1()
	{
		fmu_set_observees_q( tQ );
		if ( ( self_observer ) && ( observers_.empty() ) ) fmu_set_value( q_0_ );
		x_1_ = q_1_ = fmu_get_deriv();
	}

	// Handler Advance: Stage 2
	void
	advance_handler_2()
	{
		fmu_set_observees_q( tN = tQ + options::dtNum );
		if ( self_observer ) fmu_set_q( tN );
		x_2_ = options::one_half_over_dtNum * ( fmu_get_deriv() - x_1_ ); // Forward Euler
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << "*t" << " [q]" << "   = " << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << " [x]" << std::noshowpos << "   tE=" << tE << '\n';
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
		Real const d_0( x_0_ - ( q_c_ + ( q_1_ * ( tX - tQ ) ) ) );
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

	// Advance Self-Observing Trigger: Stage 1
	void
	advance_LIQSS_1()
	{
		assert( qTol > 0.0 );
		assert( self_observer );
		assert( q_c_ == q_0_ );

		// Derivative at +/- qTol
		fmu_set_value( q_c_ - qTol );
		d_l_ = fmu_get_deriv();
		fmu_set_value( q_c_ + qTol );
		d_u_ = fmu_get_deriv();
	}

	// Advance Self-Observing Trigger: Stage 2
	void
	advance_LIQSS_2()
	{
		assert( qTol > 0.0 );
		assert( self_observer );
		assert( q_c_ == q_0_ );
		assert( x_0_ == q_0_ );
		assert( tN == tQ + options::dtNum );

		// Value at +/- qTol
		Real const q_l( q_c_ - qTol );
		Real const q_u( q_c_ + qTol );

		// Second derivative at +/- qTol
		fmu_set_value( q_l + ( d_l_ * options::dtNum ) );
		Real const d2_l( options::one_half_over_dtNum * ( fmu_get_deriv() - d_l_ ) ); // 1/2 * 2nd derivative
		int const d2_l_s( signum( d2_l ) );
		fmu_set_value( q_u + ( d_u_ * options::dtNum ) );
		Real const d2_u( options::one_half_over_dtNum * ( fmu_get_deriv() - d_u_ ) ); // 1/2 * 2nd derivative
		int const d2_u_s( signum( d2_u ) );

		// Set coefficients based on second derivative signs
		if ( ( d2_l_s == -1 ) && ( d2_u_s == -1 ) ) { // Downward curving trajectory
			q_0_ = q_l;
			x_1_ = q_1_ = d_l_;
			x_2_ = d2_l;
		} else if ( ( d2_l_s == +1 ) && ( d2_u_s == +1 ) ) { // Upward curving trajectory
			q_0_ = q_u;
			x_1_ = q_1_ = d_u_;
			x_2_ = d2_u;
		} else if ( ( d2_l_s == 0 ) && ( d2_u_s == 0 ) ) { // Non-curving trajectory
			// Keep q_0_ == q_c_
			x_1_ = q_1_ = one_half * ( d_l_ + d_u_ ); // Interpolated 1st deriv at q_0_ == q_c_
			x_2_ = 0.0;
		} else { // Straight trajectory
			q_0_ = std::min( std::max( ( ( q_l * d2_u ) - ( q_u * d2_l ) ) / ( d2_u - d2_l ), q_l ), q_u ); // Value where 2nd deriv is ~ 0 // Clipped in case of roundoff
			x_1_ = q_1_ = ( ( ( q_u - q_0_ ) * d_l_ ) + ( ( q_0_ - q_l ) * d_u_ ) ) / ( two * qTol ); // Interpolated 1st deriv at q_0_
			x_2_ = 0.0;
		}
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }; // Continuous rep coefficients
	Real q_c_{ 0.0 }, q_0_{ 0.0 }, q_1_{ 0.0 }; // Quantized rep coefficients
	Real s_1_{ 0.0 }; // Simultaneuous rep coefficients
	Real d_l_{ 0.0 }, d_u_{ 0.0 }; // Derivative at +/- qTol

};

} // fmu
} // QSS

#endif
