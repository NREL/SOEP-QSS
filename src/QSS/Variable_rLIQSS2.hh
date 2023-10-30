// rLIQSS2 Variable
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

#ifndef QSS_Variable_rLIQSS2_hh_INCLUDED
#define QSS_Variable_rLIQSS2_hh_INCLUDED

// QSS Headers
#include <QSS/Variable_QSS.hh>

namespace QSS {

// rLIQSS2 Variable
class Variable_rLIQSS2 final : public Variable_QSS
{

public: // Types

	using Super = Variable_QSS;

public: // Creation

	// Constructor
	Variable_rLIQSS2(
	 FMU_ME * fmu_me,
	 std::string const & name,
	 Real const rTol_ = options::rTol,
	 Real const aTol_ = options::aTol,
	 Real const zTol_ = options::zTol,
	 Real const xIni_ = 0.0,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Super( fmu_me, 2, name, rTol_, aTol_, zTol_, xIni_, var, der ),
	 x_0_( xIni_ ),
	 q_c_( xIni_ ),
	 q_0_( xIni_ )
	{
		set_qTol();
	}

public: // Predicate

	// LIQSS Variable?
	bool
	is_LIQSS() const override
	{
		return true;
	}

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
		return q_0_ + ( q_1_ * ( t - tQ ) );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const ) const override
	{
		return q_1_;
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
		fmu_set_real( q_c_ = q_0_ = x_0_ = xIni );
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
		set_qTol();
		if ( self_observer() ) {
			advance_LIQSS_simultaneous();
		} else {
			x_2_ = dd_2();
			l_0_ = q_c_ + ( signum( x_2_ ) * qTol );
		}
	}

	// Initialization: Stage Final
	void
	init_F() override
	{
		q_0_ = l_0_;
		set_tE_aligned();
		add_QSS( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << " [q]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
	}

	// QSS Advance
	void
	advance_QSS() override
	{
		Time const tDel( tE - tX );
		tS = tE - tQ;
		tQ = tX = tE;
		x_2_tDel_ = x_2_ * tDel;
		q_c_ = x_0_ += ( x_1_ + x_2_tDel_ ) * tDel;
		set_qTol();
		if ( self_observer() ) {
			if ( yoyo_ ) { // Yo-yo mode
				advance_LIQSS();
				x_2_ *= x_2_rlx_;
			} else { // QSS mode
				Real const x_1_in( x_1_ + ( two * x_2_tDel_ ) ); // Incoming slope
				q_1_pre_ = q_1_;
				advance_LIQSS();
				Real const x_1_dif( x_1_ - x_1_in );
				bool const x_1_dif_sign( bool_sign( x_1_dif ) );
				if ( ( std::abs( x_1_dif ) > yoyo_mul_ * std::abs( x_1_ - q_1_pre_ ) ) && ( ( n_yoyo_ == 0u ) || ( x_1_dif_sign == x_1_dif_sign_ ) ) ) { // Yo-yoing criteria met
					x_1_dif_sign_ = x_1_dif_sign;
					yoyo_ = ( ++n_yoyo_ >= m_yoyo_ );
					if ( yoyo_ ) {
						x_2_ *= x_2_rlx_;
						if ( options::output::d ) std::cout << name() << " advance_QSS yoyo on " << tE << std::endl;
					}
				} else {
					n_yoyo_ = 0u;
				}
			}
		} else {
			q_1_ = x_1_ = c_1();
			x_2_ = dd_2();
			if ( yoyo_ ) { // Yo-yo mode
				x_2_ *= x_2_rlx_;
			} else { // QSS mode
				Real const x_1_in( x_1_pre_ + ( two * x_2_tDel_ ) ); // Incoming slope
				Real const x_1_dif( x_1_ - x_1_in );
				bool const x_1_dif_sign( bool_sign( x_1_dif ) );
				if ( ( std::abs( x_1_dif ) > yoyo_mul_ * std::abs( x_1_ - q_1_pre_ ) ) && ( ( n_yoyo_ == 0u ) || ( x_1_dif_sign == x_1_dif_sign_ ) ) ) { // Yo-yoing criteria met
					x_1_dif_sign_ = x_1_dif_sign;
					yoyo_ = ( ++n_yoyo_ >= m_yoyo_ );
					if ( yoyo_ ) {
						x_2_ *= x_2_rlx_;
						if ( options::output::d ) std::cout << name() << " advance_QSS yoyo on " << tE << std::endl;
					}
				} else {
					n_yoyo_ = 0u;
				}
			}
			q_0_ = q_c_ + ( signum( x_2_ ) * qTol );
		}
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << " [q]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
		if ( observed() ) advance_observers();
		if ( connected() ) advance_connections();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0() override
	{
		Time const tDel( tE - tX );
		tS = tE - tQ;
		tQ = tX = tE;
		x_2_tDel_ = x_2_ * tDel;
		q_c_ = q_0_ = x_0_ += ( x_1_ + x_2_tDel_ ) * tDel;
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1( Real const x_1 ) override
	{
		q_1_pre_ = q_1_;
		x_1_pre_ = x_1_;
		q_1_ = x_1_ = x_1;
	}

	// QSS Advance: Stage 2: Directional 2nd Derivative
	void
	advance_QSS_2_dd2( Real const dd2 ) override
	{
		set_qTol();
		if ( self_observer() ) {
			if ( yoyo_ ) { // Yo-yo mode
				advance_QSS_2_relax_self_observer_yoyo();
			} else { // QSS mode
				advance_QSS_2_relax_self_observer_QSS();
			}
		} else {
			if ( yoyo_ ) { // Yo-yo mode
				advance_QSS_2_relax_yoyo( one_half * dd2 );
			} else { // QSS mode
				advance_QSS_2_relax_QSS( one_half * dd2 );
			}
		}
	}

	// QSS Advance: Stage Final
	void
	advance_QSS_F() override
	{
		q_0_ = l_0_;
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "!= " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << " [q]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
		if ( connected() ) advance_connections();
	}

	// Handler Advance
	void
	advance_handler( Time const t ) override
	{
		assert( ( tQ <= t ) && ( tX <= t ) && ( t <= tE ) );
		tS = t - tQ;
		tQ = tX = t;
		q_c_ = q_0_ = x_0_ = p_0();
		q_1_ = x_1_ = c_1();
		x_2_ = dd_2();
		yoyo_clear();
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "*  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << " [q]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
		if ( observed() ) advance_observers();
		if ( connected() ) advance_connections();
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t, Real const x_0 ) override
	{
		assert( ( tQ <= t ) && ( tX <= t ) && ( t <= tE ) );
		tS = t - tQ;
		tQ = tX = t;
		q_c_ = q_0_ = x_0_ = x_0;
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
		x_2_ = one_half * dd2;
	}

	// Handler Advance: Stage Final
	void
	advance_handler_F() override
	{
		yoyo_clear();
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "*= " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << " [q]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
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
		x_0_ += ( x_1_ + ( x_2_ * tDel ) ) * tDel;
		x_1_ = x_1;
	}

	// Observer Advance: Stage 2: Directional 2nd Derivative
	void
	advance_observer_2_dd2( Real const dd2 ) override
	{
		x_2_ = ( yoyo_ ? x_2_rlx_ * one_half * dd2 : one_half * dd2 );
	}

	// Observer Advance: Stage Final
	void
	advance_observer_F() override
	{
		set_tE_unaligned();
		shift_QSS( tE );
		if ( connected() ) advance_connections_observer();
	}

	// Observer Advance: Stage d
	void
	advance_observer_d() const override
	{
		std::cout << " ^ " << name() << '(' << tX << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << " [q(" << std::noshowpos << tQ << std::showpos << ")]   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << ( yoyo_ ? " yoyo" : "" ) << std::endl;
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
		assert( tQ == tX );
		assert( dt_min <= dt_max );
		Time dt;
		if ( yoyo_ ) { // Yo-yo mode
			if ( x_2_ != 0.0 ) {
				Real const x_2_inv( one / x_2_ );
				dt = dt_infinity( std::sqrt( qTol * std::abs( x_2_inv ) ) );
				assert( dt != infinity );
				if ( nonzero_and_signs_differ( x_1_, x_2_ ) ) { // Inflection point
					Time const dtI( -( x_1_ * ( one_half * x_2_inv ) ) );
					if ( ( dtI < dt ) && ( dt * options::inflectionFrac < dtI ) ) {
						dt = dtI;
					} else {
						dt = ( dt_pre_ < dt_growth_inf_ ? std::min( dt_growth_mul_ * dt_pre_, dt ) : dt ); // Relax time step growth
					}
				} else {
					dt = ( dt_pre_ < dt_growth_inf_ ? std::min( dt_growth_mul_ * dt_pre_, dt ) : dt ); // Relax time step growth
				}
				dt = std::min( std::max( dt, dt_min ), dt_max );
				tE = tQ + dt;
			} else {
				dt = dt_infinity_of_infinity();
				dt = ( dt_pre_ < dt_growth_inf_ ? std::min( dt_growth_mul_ * dt_pre_, dt ) : dt ); // Relax time step growth
				dt = std::min( std::max( dt, dt_min ), dt_max );
				tE = ( dt != infinity ? tQ + dt : infinity );
			}
			dt_pre_ = dt;
		} else { // QSS mode
			if ( x_2_ != 0.0 ) {
				Real const x_2_inv( one / x_2_ );
				dt = dt_infinity( std::sqrt( qTol * std::abs( x_2_inv ) ) );
				assert( dt != infinity );
				if ( options::inflection && nonzero_and_signs_differ( x_1_, x_2_ ) ) { // Inflection point
					Time const dtI( -( x_1_ * ( one_half * x_2_inv ) ) );
					dt = ( ( dtI < dt ) && ( dt * options::inflectionFrac < dtI ) ? dtI : dt );
				}
				dt = std::min( std::max( dt, dt_min ), dt_max );
				tE = tQ + dt;
			} else {
				dt = std::min( std::max( dt_infinity_of_infinity(), dt_min ), dt_max );
				tE = ( dt != infinity ? tQ + dt : infinity );
			}
		}
		if ( tQ == tE ) {
			tE = std::nextafter( tE, infinity );
			dt = tE - tQ;
		}
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
		dt = dt_infinity( dt );
		assert( dt > 0.0 );
		if ( yoyo_ ) { // Yo-yo mode
			if ( nonzero_and_signs_differ( x_1_, x_2_ ) ) { // Inflection point
				Time const dtI( -( x_1_ / ( two * x_2_ ) ) );
				if ( ( dtI < dt ) && ( dt * options::inflectionFrac < dtI ) ) {
					dt = dtI;
				} else {
					dt = ( dt_pre_ < dt_growth_inf_ ? std::min( dt_growth_mul_ * dt_pre_, dt ) : dt ); // Relax time step growth
				}
			} else {
				dt = ( dt_pre_ < dt_growth_inf_ ? std::min( dt_growth_mul_ * dt_pre_, dt ) : dt ); // Relax time step growth
			}
			dt_pre_ = dt;
		} else { // QSS mode
			if ( options::inflection && nonzero_and_signs_differ( x_1_, x_2_ ) ) { // Inflection point
				Time const dtI( -( x_1_ / ( two * x_2_ ) ) );
				dt = ( ( dtI < dt ) && ( dt * options::inflectionFrac < dtI ) ? dtI : dt );
			}
		}
		dt = std::min( std::max( dt, dt_min ), dt_max );
		tE = ( dt != infinity ? tX + dt : infinity );
		if ( tX == tE ) {
			tE = std::nextafter( tE, infinity );
			dt = tE - tX;
		}
	}

	// Advance Self-Observing Trigger
	void
	advance_LIQSS();

	// Advance Self-Observing Trigger: Simultaneous
	void
	advance_LIQSS_simultaneous();

	// QSS Advance: Stage 2: Relaxation: Self-Observer: Yoyo
	void
	advance_QSS_2_relax_self_observer_yoyo()
	{
		advance_LIQSS_simultaneous();
		x_2_ *= x_2_rlx_;
	}

	// QSS Advance: Stage 2: Relaxation: Yoyo
	void
	advance_QSS_2_relax_yoyo( Real const x_2 )
	{
		x_2_ = x_2_rlx_ * x_2;
		l_0_ = q_c_ + ( signum( x_2_ ) * qTol );
	}

	// QSS Advance: Stage 2: Relaxation: Self-Observer: QSS
	void
	advance_QSS_2_relax_self_observer_QSS()
	{
		Real const x_1_in( x_1_pre_ + ( two * x_2_tDel_ ) ); // Incoming slope
		advance_LIQSS_simultaneous();
		Real const x_1_dif( x_1_ - x_1_in );
		bool const x_1_dif_sign( bool_sign( x_1_dif ) );
		if ( ( std::abs( x_1_dif ) > yoyo_mul_ * std::abs( x_1_ - q_1_pre_ ) ) && ( ( n_yoyo_ == 0u ) || ( x_1_dif_sign == x_1_dif_sign_ ) ) ) { // Yo-yoing criteria met
			x_1_dif_sign_ = x_1_dif_sign;
			yoyo_ = ( ++n_yoyo_ >= m_yoyo_ );
			if ( yoyo_ ) {
				x_2_ *= x_2_rlx_;
				if ( options::output::d ) std::cout << name() << " advance_QSS yoyo on " << tE << std::endl;
			}
		} else {
			n_yoyo_ = 0u;
		}
	}

	// QSS Advance: Stage 2: Relaxation: QSS
	void
	advance_QSS_2_relax_QSS( Real const x_2 )
	{
		x_2_ = x_2;
		l_0_ = q_c_ + ( signum( x_2_ ) * qTol );
		Real const x_1_in( x_1_pre_ + ( two * x_2_tDel_ ) ); // Incoming slope
		Real const x_1_dif( x_1_ - x_1_in );
		bool const x_1_dif_sign( bool_sign( x_1_dif ) );
		if ( ( std::abs( x_1_dif ) > yoyo_mul_ * std::abs( x_1_ - q_1_pre_ ) ) && ( ( n_yoyo_ == 0u ) || ( x_1_dif_sign == x_1_dif_sign_ ) ) ) { // Yo-yoing criteria met
			x_1_dif_sign_ = x_1_dif_sign;
			yoyo_ = ( ++n_yoyo_ >= m_yoyo_ );
			if ( yoyo_ ) {
				x_2_ *= x_2_rlx_;
				if ( options::output::d ) std::cout << name() << " advance_QSS yoyo on " << tE << std::endl;
			}
		} else {
			n_yoyo_ = 0u;
		}
	}

	// Clear Yo-Yo State
	void
	yoyo_clear()
	{
		n_yoyo_ = 0u;
		yoyo_ = false;
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }; // Continuous trajectory coefficients
	Real q_c_{ 0.0 }, q_0_{ 0.0 }, q_1_{ 0.0 }; // Quantized trajectory coefficients
	Real l_0_{ 0.0 }; // LIQSS-adjusted coefficient

	// Relaxation
	Real q_1_pre_{ 0.0 }; // Previous 1st order quantized trajectory coefficient
	Real x_1_pre_{ 0.0 }; // Previous 1st order continuous trajectory coefficient
	Real x_2_tDel_{ 0.0 }; // x_2_ * ( tE - tX )
	Time dt_pre_{ infinity }; // Previous time step
	std::uint8_t n_yoyo_{ 0u }; // Number of yo-yo sequential requantization steps currently
	bool x_1_dif_sign_{ false }; // Sign of previous x_1_ - x_1_in
	bool yoyo_{ false }; // Yo-yoing mode on?

	// Yo-yoing parameters
	static constexpr std::uint8_t m_yoyo_{ 5u }; // Number of yo-yo sequential requantization steps threshold
	static constexpr double yoyo_mul_{ 100.0 }; // Yo-yo slope difference criterion multiplier
	static constexpr double dt_growth_mul_{ 1.5 }; // Time step growth damping multiplier
	static constexpr double dt_growth_inf_{ infinity / dt_growth_mul_ }; // Time step growth infinity threshold
	static constexpr double x_2_rlx_{ one_half }; // 2nd order coefficient relaxation factor

}; // Variable_rLIQSS2

} // QSS

#endif
