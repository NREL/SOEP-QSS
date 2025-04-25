// rQSS3 Variable
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

#ifndef QSS_Variable_rQSS3_hh_INCLUDED
#define QSS_Variable_rQSS3_hh_INCLUDED

// QSS Headers
#include <QSS/Variable_QSS.hh>

namespace QSS {

// rQSS3 Variable
class Variable_rQSS3 final : public Variable_QSS
{

public: // Types

	using Super = Variable_QSS;

public: // Creation

	// Constructor
	Variable_rQSS3(
	 FMU_ME * fmu_me,
	 std::string const & name,
	 Real const rTol_ = options::rTol,
	 Real const aTol_ = options::aTol,
	 Real const zTol_ = options::zTol,
	 Real const xIni_ = 0.0,
	 FMU_Variable const & var = FMU_Variable(),
	 FMU_Variable const & der = FMU_Variable()
	) :
	 Super( fmu_me, 3, name, rTol_, aTol_, zTol_, xIni_, var, der ),
	 x_0_( xIni_ ),
	 q_0_( xIni_ )
	{
		set_qTol();
	}

public: // Predicate

	// Yo-yoing?
	bool
	yoyoing() const override
	{
		return yoyo_;
	}

public: // Property

	// Continuous Value at Time t
	Real
	x( Time const t ) const override
	{
		Time const tDel( t - tX );
		return x_0_ + ( ( x_1_ + ( ( x_2_ + ( x_3_ * tDel ) ) * tDel ) ) * tDel );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const t ) const override
	{
		Time const tDel( t - tX );
		return x_1_ + ( ( ( two * x_2_ ) + ( three * x_3_ * tDel ) ) * tDel );
	}

	// Continuous Second Derivative at Time t
	Real
	x2( Time const t ) const override
	{
		return ( two * x_2_ ) + ( six * x_3_ * ( t - tX ) );
	}

	// Continuous Third Derivative at Time t
	Real
	x3( Time const ) const override
	{
		return six * x_3_;
	}

	// Quantized Value at Time t
	Real
	q( Time const t ) const override
	{
		Time const tDel( t - tQ );
		return q_0_ + ( ( q_1_ + ( q_2_ * tDel ) ) * tDel );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const t ) const override
	{
		return q_1_ + ( two * q_2_ * ( t - tQ ) );
	}

	// Quantized Second Derivative at Time t
	Real
	q2( Time const ) const override
	{
		return two * q_2_;
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
		init_3();
		init_F();
	}

	// Initialization: Stage 0
	void
	init_0() override
	{
		init_observees();
		fmu_set_real( q_0_ = x_0_ = xIni );
	}

	// Initialization: Stage 1
	void
	init_1() override
	{
		q_1_ = x_1_ = p_1();
	}

	// Initialization: Stage 2
	void
	init_2() override
	{
		q_2_ = x_2_ = dd_2();
	}

	// Initialization: Stage 3
	void
	init_3() override
	{
		x_3_ = n_3();
		fmu_set_observees_x( t0() );
	}

	// Initialization: Stage Final
	void
	init_F() override
	{
		set_qTol();
		set_tE_aligned();
		add_QSS( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << " [q]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
	}

	// QSS Advance
	void
	advance_QSS() override
	{
		Time const tDel( tE - tX );
		tS = tE - tQ;
		tQ = tX = tE;
		Real const x_3_tDel( x_3_ * tDel );
		q_0_ = x_0_ += ( x_1_ + ( x_2_ + x_3_tDel ) * tDel ) * tDel;
		if ( yoyo_ ) { // Yo-yo mode
			q_1_ = x_1_ = c_1();
			q_2_ = x_2_ = dd_2();
			x_3_ = rlx_fac_ * n_3();
		} else { // QSS mode
			Real const x_1_in( x_1_ + ( ( ( two * x_2_ ) + ( three * x_3_tDel ) ) * tDel ) ); // Incoming slope
			x_1_ = c_1();
			Real const x_1_dif( x_1_ - x_1_in );
			Real const x_1_dif_lim( yoyo_mul_ * std::min( std::abs( x_1_ - q_1_2_ ), std::abs( x_1_ + q_1_2_ ) ) );
			bool const x_1_dif_sign( bool_sign( x_1_dif ) );
			q_1_2_ = q_1_;
			q_1_ = x_1_;
			q_2_ = x_2_ = dd_2();
			x_3_ = n_3();
			if ( ( std::abs( x_1_dif ) > x_1_dif_lim ) && ( ( n_yoyo_ == 0u ) || ( !x_1_dif_sign == x_1_dif_sign_ ) ) ) { // Yo-yoing criteria met
				x_1_dif_sign_ = x_1_dif_sign;
				yoyo_ = ( ++n_yoyo_ >= m_yoyo_ );
				if ( yoyo_ ) {
					x_3_ *= rlx_fac_;
					if ( options::output::d ) std::cout << name() << " yoyo on " << tE << std::endl;
				}
			} else {
				n_yoyo_ = 0u;
			}
		}
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << " [q]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << ( yoyo_ ? " yoyo" : "" ) << std::endl;
		if ( observed() ) advance_observers();
		if ( connected() ) advance_connections();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0() override
	{
		tDel_ = tE - tX;
		tS = tE - tQ;
		tQ = tX = tE;
		x_3_tDel_ = x_3_ * tDel_;
		q_0_ = x_0_ += ( x_1_ + ( x_2_ + x_3_tDel_ ) * tDel_ ) * tDel_;
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1( Real const x_1 ) override
	{
		if ( yoyo_ ) { // Yo-yo mode
			q_1_ = x_1_ = x_1;
		} else { // QSS mode
			Real const x_1_in( x_1_ + ( ( ( two * x_2_ ) + ( three * x_3_tDel_ ) ) * tDel_ ) ); // Incoming slope
			x_1_ = x_1;
			Real const x_1_dif( x_1_ - x_1_in );
			Real const x_1_dif_lim( yoyo_mul_ * std::min( std::abs( x_1_ - q_1_2_ ), std::abs( x_1_ + q_1_2_ ) ) );
			bool const x_1_dif_sign( bool_sign( x_1_dif ) );
			q_1_2_ = q_1_;
			q_1_ = x_1_;
			if ( ( std::abs( x_1_dif ) > x_1_dif_lim ) && ( ( n_yoyo_ == 0u ) || ( !x_1_dif_sign == x_1_dif_sign_ ) ) ) { // Yo-yoing criteria met
				x_1_dif_sign_ = x_1_dif_sign;
				yoyo_ = ( ++n_yoyo_ >= m_yoyo_ );
				if ( yoyo_ ) {
					if ( options::output::d ) std::cout << name() << " yoyo on " << tE << std::endl;
				}
			} else {
				n_yoyo_ = 0u;
			}
		}
	}

	// QSS Advance: Stage 2: Directional 2nd Derivative
	void
	advance_QSS_2_dd2( Real const dd2 ) override
	{
		q_2_ = x_2_ = one_half * dd2;
	}

	// QSS Advance: Stage 3: Directional 2nd Derivative
	void
	advance_QSS_3_dd2( Real const dd2_p ) override
	{
		x_3_ = n_3( one_half * dd2_p );
		if ( yoyo_ ) x_3_ *= rlx_fac_;
	}

	// QSS Advance: Stage Final
	void
	advance_QSS_F() override
	{
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( connected() ) advance_connections();
	}

	// QSS Advance: Stage Debug
	void
	advance_QSS_d() override
	{
		assert( options::output::d );
		std::cout << "!= " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << " [q]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << ( yoyo_ ? " yoyo" : "" ) << std::endl;
	}

	// Handler Advance
	void
	advance_handler( Time const t ) override
	{
		assert( ( tQ <= t ) && ( tX <= t ) && ( t <= tE ) );
		tS = t - tQ;
		tQ = tX = t;
		q_0_ = x_0_ = p_0();
		q_1_ = x_1_ = c_1();
		q_2_ = x_2_ = dd_2();
		x_3_ = n_3();
		yoyo_clear();
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "*  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << " [q]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
		if ( observed() ) advance_handler_observers();
		if ( connected() ) advance_connections();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t, Real const x_0 ) override
	{
		assert( ( tQ <= t ) && ( tX <= t ) && ( t <= tE ) );
		tS = t - tQ;
		tQ = tX = t;
		q_0_ = x_0_ = x_0;
	}

	// Handler Advance: Stage 1
	void
	advance_handler_1( Real const x_1 ) override
	{
		q_1_ = x_1_ = x_1;
	}

	// Handler Advance: Stage 2: Directional 2nd Derivative
	void
	advance_handler_2_dd2( Real const dd2 ) override
	{
		q_2_ = x_2_ = one_half * dd2;
	}

	// Handler Advance: Stage 3: Directional 2nd Derivative
	void
	advance_handler_3_dd2( Real const dd2_p ) override
	{
		x_3_ = n_3( one_half * dd2_p );
	}

	// Handler Advance: Stage Final
	void
	advance_handler_F() override
	{
		yoyo_clear();
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "*= " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << " [q]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
		if ( connected() ) advance_connections();
	}

	// Handler No-Advance
	void
	no_advance_handler() override
	{
		shift_QSS( tE );
	}

	// Observer Advance: Stage 1
	void
	advance_observer_1( Time const t, Real const x_1 ) override
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		tX = t;
		x_0_ += ( x_1_ + ( x_2_ + ( x_3_ * tDel ) ) * tDel ) * tDel;
		x_1_ = x_1;
	}

	// Observer Advance: Stage 2: Directional 2nd Derivative
	void
	advance_observer_2_dd2( Real const dd2 ) override
	{
		x_2_ = one_half * dd2;
	}

	// Observer Advance: Stage 3: Directional 2nd Derivative
	void
	advance_observer_3_dd2( Real const dd2_p ) override
	{
		x_3_ = yoyo_ ? rlx_fac_ * n_3( one_half * dd2_p ) : n_3( one_half * dd2_p );
	}

	// Observer Advance: Stage Final
	void
	advance_observer_F() override
	{
		set_tE_unaligned();
		shift_QSS( tE );
		if ( connected() ) advance_connections_observer();
	}

	// Observer Advance: Stage Final: Parallel
	void
	advance_observer_F_parallel() override
	{
		set_tE_unaligned();
	}

	// Observer Advance: Stage Final: Serial
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
		std::cout << " ^ " << name() << '(' << tX << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << " [q(" << std::noshowpos << tQ << std::showpos << ")]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << " [x]" << std::noshowpos << "   tE=" << tE << ( yoyo_ ? " yoyo" : "" ) << std::endl;
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
		assert( tQ == tX );
		assert( dt_min <= dt_max );
		clip();
		Time dt;
		Time dt_pre;
		if ( x_3_ != 0.0 ) {
			Real const x_3_inv( one / x_3_ );
			dt = dt_infinity( std::cbrt( qTol * std::abs( x_3_inv ) ) );
			if ( yoyo_ ) dt *= one_half;
			assert( dt != infinity );
			dt_pre = dt;
			Time const dt_inflectionFrac( dt * options::inflectionFrac );
			Time const dt_inflectionFrac2( dt * options::inflectionFrac2 );
			Time const dtI_1_root( min_root_quadratic( three * x_3_, two * x_2_, x_1_ ) ); // When 1st derivative is zero
			Time const dtI_1( dtI_1_root > dt_inflectionFrac ? dtI_1_root : infinity );
			Time const dtI_2_root( nonzero_and_signs_differ( x_2_, x_3_ ) ? -( x_2_ * ( one_third * x_3_inv ) ) : infinity ); // When 2nd derivative is zero
			Time const dtI_2( dtI_2_root > dt_inflectionFrac2 ? dtI_2_root : infinity );
			Time const dtI( std::min( dtI_1, dtI_2 ) );
			if ( dtI < dt ) { // Use inflection point time step
				dt = dtI;
			} else if ( yoyo_ ) { // Relax time step growth
				dt_pre = dt = dt_pre_ < dt_growth_inf_ ? std::min( dt, dt_growth_mul_ * dt_pre_ ) : dt;
			}
		} else {
			dt = dt_infinity_of_infinity();
			dt_pre = dt;
			if ( ( x_2_ != 0.0 ) && ( dt != infinity ) ) {
				Time const dtI_1_root( -( x_1_ / ( two * x_2_ ) ) ); // When 1st derivative is zero
				Time const dtI_1( dtI_1_root > dt * options::inflectionFrac ? dtI_1_root : infinity );
				if ( dtI_1 < dt ) { // Use inflection point time step
					dt = dtI_1;
				} else if ( yoyo_ ) { // Relax time step growth
					dt_pre = dt = dt_pre_ < dt_growth_inf_ ? std::min( dt, dt_growth_mul_ * dt_pre_ ) : dt;
				}
			} else if ( yoyo_ ) { // Relax time step growth
				dt_pre = dt = dt_pre_ < dt_growth_inf_ ? std::min( dt, dt_growth_mul_ * dt_pre_ ) : dt;
			}
		}
		dt_pre_ = dt_pre;
		dt = std::min( std::max( dt, dt_min ), dt_max );
		tE = dt != infinity ? tQ + dt : infinity;
		if ( tQ == tE ) {
			tE = std::nextafter( tE, infinity );
			dt = tE - tQ;
			dt_pre_ = std::max( dt_pre_, dt );
		}
	}

	// Set End Time: Quantized and Continuous Unaligned
	void
	set_tE_unaligned()
	{
		assert( tQ <= tX );
		assert( dt_min <= dt_max );
		clip_x();
		Time const tXQ( tX - tQ );
		Real const d_0( x_0_ - ( q_0_ + ( q_1_ + ( q_2_ * tXQ ) ) * tXQ ) );
		Real const d_1( x_1_ - ( q_1_ + ( two * q_2_ * tXQ ) ) );
		Real const d_2( x_2_ - q_2_ );
		Time dt;
		if ( ( x_3_ >= 0.0 ) && ( d_2 >= 0.0 ) && ( d_1 >= 0.0 ) ) { // Upper boundary crossing
			dt = min_root_cubic_upper( x_3_, d_2, d_1, d_0 - qTol, zTol );
		} else if ( ( x_3_ <= 0.0 ) && ( d_2 <= 0.0 ) && ( d_1 <= 0.0 ) ) { // Lower boundary crossing
			dt = min_root_cubic_lower( x_3_, d_2, d_1, d_0 + qTol, zTol );
		} else { // Both boundaries can have crossings
			dt = min_root_cubic_both( x_3_, d_2, d_1, d_0 + qTol, d_0 - qTol, zTol );
		}
		dt = dt_infinity( dt );
		if ( yoyo_ ) dt *= one_half;
		assert( dt > 0.0 ); // Might be infinity
		Time dt_pre( dt );
		if ( ( x_3_ != 0.0 ) && ( dt != infinity ) ) {
			Real const x_3_inv( one / x_3_ );
			Time const dt_inflectionFrac( dt * options::inflectionFrac );
			Time const dt_inflectionFrac2( dt * options::inflectionFrac2 );
			Time const dtI_1_root( min_root_quadratic( three * x_3_, two * x_2_, x_1_ ) ); // When 1st derivative is zero
			Time const dtI_1( dtI_1_root > dt_inflectionFrac ? dtI_1_root : infinity );
			Time const dtI_2_root( nonzero_and_signs_differ( x_2_, x_3_ ) ? -( x_2_ * ( one_third * x_3_inv ) ) : infinity ); // When 2nd derivative is zero
			Time const dtI_2( dtI_2_root > dt_inflectionFrac2 ? dtI_2_root : infinity );
			Time const dtI( std::min( dtI_1, dtI_2 ) );
			if ( dtI < dt ) { // Use inflection point time step
				dt = dtI;
			} else if ( yoyo_ ) { // Relax time step growth
				dt_pre = dt = dt_pre_ < dt_growth_inf_ ? std::min( dt, dt_growth_mul_ * dt_pre_ ) : dt;
			}
		} else {
			dt = dt_infinity_of_infinity();
			dt_pre = dt;
			if ( ( x_2_ != 0.0 ) && ( dt != infinity ) ) {
				Time const dtI_1_root( -( x_1_ / ( two * x_2_ ) ) ); // When 1st derivative is zero
				Time const dtI_1( dtI_1_root > dt * options::inflectionFrac ? dtI_1_root : infinity );
				if ( dtI_1 < dt ) { // Use inflection point time step
					dt = dtI_1;
				} else if ( yoyo_ ) { // Relax time step growth
					dt_pre = dt = dt_pre_ < dt_growth_inf_ ? std::min( dt, dt_growth_mul_ * dt_pre_ ) : dt;
				}
			} else if ( yoyo_ ) { // Relax time step growth
				dt_pre = dt = dt_pre_ < dt_growth_inf_ ? std::min( dt, dt_growth_mul_ * dt_pre_ ) : dt;
			}
		}
		dt_pre_ = dt_pre;
		dt = std::min( std::max( dt, dt_min ), dt_max );
		tE = dt != infinity ? tX + dt : infinity;
		if ( tX == tE ) {
			tE = std::nextafter( tE, infinity );
			dt = tE - tX;
			dt_pre_ = std::max( dt_pre_, dt );
		}
	}

	// Clip Small Trajectory Coefficients
	void
	clip()
	{
		if ( options::clipping ) {
			if ( std::abs( x_0_ ) <= options::clip ) x_0_ = 0.0;
			if ( std::abs( x_1_ ) <= options::clip ) x_1_ = 0.0;
			if ( std::abs( x_2_ ) <= options::clip ) x_2_ = 0.0;
			if ( std::abs( x_3_ ) <= options::clip ) x_3_ = 0.0;
			if ( std::abs( q_0_ ) <= options::clip ) q_0_ = 0.0;
			if ( std::abs( q_1_ ) <= options::clip ) q_1_ = 0.0;
			if ( std::abs( q_2_ ) <= options::clip ) q_2_ = 0.0;
		}
	}

	// Clip Small x Trajectory Coefficients
	void
	clip_x()
	{
		if ( options::clipping ) {
			if ( std::abs( x_0_ ) <= options::clip ) x_0_ = 0.0;
			if ( std::abs( x_1_ ) <= options::clip ) x_1_ = 0.0;
			if ( std::abs( x_2_ ) <= options::clip ) x_2_ = 0.0;
			if ( std::abs( x_3_ ) <= options::clip ) x_3_ = 0.0;
		}
	}

	// Coefficient 3 at Time tX
	Real
	n_3() const
	{
		Time const tN( tX + options::dtND );
		fmu_set_time( tN );
		fmu_set_observees_s( tN );
		Real const x_2_p( dd_2( tN ) );
		fmu_set_time( tX );
		return options::one_over_three_dtND * ( x_2_p - x_2_ ); //ND Forward Euler
	}

	// Coefficient 3: From Coefficient 2 at +dtND
	Real
	n_3( Real const x_2_p ) const
	{
		return options::one_over_three_dtND * ( x_2_p - x_2_ ); //ND Forward Euler
	}

	// Clear Yo-Yo State
	void
	yoyo_clear()
	{
		n_yoyo_ = 0u;
		yoyo_ = false;
		q_1_2_ = 0.0;
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }, x_3_{ 0.0 }; // Continuous trajectory coefficients
	Real q_0_{ 0.0 }, q_1_{ 0.0 }, q_2_{ 0.0 }; // Quantized trajectory coefficients

	// Relaxation
	Real q_1_2_{ 0.0 }; // Quantized trajectory 1st order coefficient from two requantizations earlier
	Real x_3_tDel_{ 0.0 }; // x_3_ * ( tE - tX )
	Time tDel_{ 0.0 }; // tE - tX
	Time dt_pre_{ infinity }; // Previous time step
	std::uint8_t n_yoyo_{ 0u }; // Number of yo-yo sequential requantization steps currently
	bool x_1_dif_sign_{ false }; // Sign of previous x_1_ - x_1_in
	bool yoyo_{ false }; // Yo-yoing mode on?

	// Yo-yoing parameters
	static constexpr std::uint8_t m_yoyo_{ 5u }; // Number of yo-yo sequential requantization steps threshold
	static constexpr double yoyo_mul_{ 100.0 }; // Yo-yo slope difference criterion multiplier
	static constexpr double dt_growth_mul_{ 1.5 }; // Time step growth damping multiplier
	static constexpr double dt_growth_inf_{ infinity / dt_growth_mul_ }; // Time step growth infinity threshold
	static constexpr double rlx_fac_{ 0.25 }; // Derivative relaxation factor

}; // Variable_rQSS3

} // QSS

#endif
