// Relaxation xQSS2 Variable
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

#ifndef QSS_Variable_rxQSS2_hh_INCLUDED
#define QSS_Variable_rxQSS2_hh_INCLUDED

// QSS Headers
#include <QSS/Variable_QSS.hh>

namespace QSS {

// Relaxation xQSS2 Variable
class Variable_rxQSS2 final : public Variable_QSS
{

public: // Types

	using Super = Variable_QSS;

public: // Creation

	// Constructor
	Variable_rxQSS2(
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
	 q_0_( xIni_ ),
	 x_0_( xIni_ )
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
		q_2_ = x_2_ = c_2( tQ, x_1_ );
	}

	// Initialization: Stage Final
	void
	init_F() override
	{
		set_qTol();
		set_tE_aligned();
		add_QSS( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << " [q]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
	}

	// QSS Advance
	void
	advance_QSS() override
	{
		if ( options::stiff ) liqss_qss_ratio_pass();
		Time const tDel( tE - tX );
		tS = tE - tQ;
		tQ = tX = tE;
		x_2_tDel_ = x_2_ * tDel;
		q_0_ = x_0_ += ( x_1_ + x_2_tDel_ ) * tDel;
		if ( yoyo_ ) { // Yo-yo mode
			q_1_ = x_1_ = c_1( tE );
			q_2_ = x_2_ = x_2_rlx_ * ( x_2_QSS_ = c_2( tE, x_1_ ) );
		} else { // QSS mode
			Real const x_1_in( x_1_ + ( two * x_2_tDel_ ) ); // Incoming slope
			q_1_pre_ = q_1_;
			q_1_ = x_1_ = c_1( tE );
			q_2_ = x_2_ = c_2( tE, x_1_ );
			Real const x_1_dif( x_1_ - x_1_in );
			bool const x_1_dif_sign( bool_sign( x_1_dif ) );
			if ( ( std::abs( x_1_dif ) > yoyo_mul_ * std::abs( x_1_ - q_1_pre_ ) ) && ( ( n_yoyo_ == 0u ) || ( x_1_dif_sign == x_1_dif_sign_ ) ) ) { // Yo-yoing criteria met
				x_1_dif_sign_ = x_1_dif_sign;
				yoyo_ = ( ++n_yoyo_ >= m_yoyo_ );
				q_2_ = x_2_ = ( yoyo_ ? x_2_rlx_ * ( x_2_QSS_ = x_2_ ) : x_2_ );
				if ( yoyo_ && options::output::d ) std::cout << name() << " advance_QSS yoyo on " << tE << std::endl;
			} else {
				n_yoyo_ = 0u;
			}
		}
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << " [q]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << ( yoyo_ ? " yoyo" : "" ) << std::endl;
		if ( observed() ) advance_observers();
		if ( connected() ) advance_connections();
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0() override
	{
		if ( options::stiff ) liqss_qss_ratio_pass();
		Time const tDel( tE - tX );
		tS = tE - tQ;
		tQ = tX = tE;
		x_2_tDel_ = x_2_ * tDel;
		q_0_ = x_0_ += ( x_1_ + x_2_tDel_ ) * tDel;
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1( Real const x_1 ) override
	{
		q_1_pre_ = q_1_;
		x_1_pre_ = x_1_;
		q_1_ = x_1_ = x_1;
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2( Real const x_1_p ) override
	{
		if ( yoyo_ ) { // Yo-yo mode
			advance_QSS_2_relax_yoyo( n_2( x_1_p ) );
		} else { // QSS mode
			advance_QSS_2_relax_QSS( n_2( x_1_p ) );
		}
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2( Real const x_1_m, Real const x_1_p ) override
	{
		if ( yoyo_ ) { // Yo-yo mode
			advance_QSS_2_relax_yoyo( n_2( x_1_m, x_1_p ) );
		} else { // QSS mode
			advance_QSS_2_relax_QSS( n_2( x_1_m, x_1_p ) );
		}
	}

	// QSS Advance: Stage 2: Forward ND
	void
	advance_QSS_2_forward( Real const x_1_p, Real const x_1_2p ) override
	{
		if ( yoyo_ ) { // Yo-yo mode
			advance_QSS_2_relax_yoyo( f_2( x_1_p, x_1_2p ) );
		} else { // QSS mode
			advance_QSS_2_relax_QSS( f_2( x_1_p, x_1_2p ) );
		}
	}

	// QSS Advance: Stage Final
	void
	advance_QSS_F() override
	{
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "!= " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << " [q]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << ( yoyo_ ? " yoyo" : "" ) << std::endl;
		if ( connected() ) advance_connections();
	}

	// QSS Advance LIQSS/QSS Step Ratio
	Real
	advance_LIQSS_QSS_step_ratio() override;

	// Handler Advance
	void
	advance_handler( Time const t ) override
	{
		assert( ( tQ <= t ) && ( tX <= t ) && ( t <= tE ) );
		tS = t - tQ;
		tQ = tX = t;
		q_0_ = x_0_ = p_0();
		q_1_ = x_1_ = h_1();
		q_2_ = x_2_ = c_2( t, x_1_ );
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		yoyo_clear();
		if ( options::output::d ) std::cout << "*  " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << " [q]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
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
		q_0_ = x_0_ = x_0;
	}

	// Handler Advance: Stage 1
	void
	advance_handler_1( Real const x_1 ) override
	{
		q_1_ = x_1_ = x_1;
	}

	// Handler Advance: Stage 2
	void
	advance_handler_2( Real const x_1_p ) override
	{
		q_2_ = x_2_ = n_2( x_1_p );
	}

	// Handler Advance: Stage 2
	void
	advance_handler_2( Real const x_1_m, Real const x_1_p ) override
	{
		q_2_ = x_2_ = n_2( x_1_m, x_1_p );
	}

	// QSS Advance: Stage 2: Forward ND
	void
	advance_handler_2_forward( Real const x_1_p, Real const x_1_2p ) override
	{
		q_2_ = x_2_ = f_2( x_1_p, x_1_2p );
	}

	// Handler Advance: Stage Final
	void
	advance_handler_F() override
	{
		set_qTol();
		set_tE_aligned();
		shift_QSS( tE );
		yoyo_clear();
		if ( options::output::d ) std::cout << "*= " << name() << '(' << tQ << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << " [q]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << std::endl;
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

	// Observer Advance: Stage 2
	void
	advance_observer_2( Real const x_1_p ) override
	{
		x_2_ = ( yoyo_ ? x_2_rlx_ * ( x_2_QSS_ = n_2( x_1_p ) ) : n_2( x_1_p ) );
	}

	// Observer Advance: Stage 2
	void
	advance_observer_2( Real const x_1_m, Real const x_1_p ) override
	{
		x_2_ = ( yoyo_ ? x_2_rlx_ * ( x_2_QSS_ = n_2( x_1_m, x_1_p ) ) : n_2( x_1_m, x_1_p ) );
	}

	// Observer Advance: Stage 2: Forward ND
	void
	advance_observer_2_forward( Real const x_1_p, Real const x_1_2p ) override
	{
		x_2_ = ( yoyo_ ? x_2_rlx_ * ( x_2_QSS_ = f_2( x_1_p, x_1_2p ) ) : f_2( x_1_p, x_1_2p ) );
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
		std::cout << " ^ " << name() << '(' << tX << ')' << " = " << std::showpos << q_0_ << q_1_ << x_delta << q_2_ << x_delta_2 << " [q(" << std::noshowpos << tQ << std::showpos << ")]" << "   = " << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << " [x]" << std::noshowpos << "   tE=" << tE << ( yoyo_ ? " yoyo" : "" ) << std::endl;
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
		Time dt;
		if ( yoyo_ ) { // Yo-yo mode
			if ( x_2_ != 0.0 ) {
				Real const x_2_inv( one / x_2_ );
				dt = dt_infinity( std::sqrt( qTol * x_2_rlx_ * std::abs( x_2_inv ) ) ); // x_2_rlx_ * std::abs( x_2_inv ) == 1 / std::abs( x_2_QSS_ )
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
		Real const d_0( x_0_ - ( q_0_ + ( q_1_ * ( tX - tQ ) ) ) );
		Real const d_1( x_1_ - q_1_ );
		Time dt;
		if ( yoyo_ ) { // Yo-yo mode
			if ( ( d_1 >= 0.0 ) && ( x_2_QSS_ >= 0.0 ) ) { // Upper boundary crossing
				dt = min_root_quadratic_upper( x_2_QSS_, d_1, d_0 - qTol );
			} else if ( ( d_1 <= 0.0 ) && ( x_2_QSS_ <= 0.0 ) ) { // Lower boundary crossing
				dt = min_root_quadratic_lower( x_2_QSS_, d_1, d_0 + qTol );
			} else { // Both boundaries can have crossings
				dt = min_root_quadratic_both( x_2_QSS_, d_1, d_0 + qTol, d_0 - qTol );
			}
			dt = dt_infinity( dt );
			assert( dt > 0.0 );
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
			if ( ( d_1 >= 0.0 ) && ( x_2_ >= 0.0 ) ) { // Upper boundary crossing
				dt = min_root_quadratic_upper( x_2_, d_1, d_0 - qTol );
			} else if ( ( d_1 <= 0.0 ) && ( x_2_ <= 0.0 ) ) { // Lower boundary crossing
				dt = min_root_quadratic_lower( x_2_, d_1, d_0 + qTol );
			} else { // Both boundaries can have crossings
				dt = min_root_quadratic_both( x_2_, d_1, d_0 + qTol, d_0 - qTol );
			}
			dt = dt_infinity( dt );
			assert( dt > 0.0 );
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

	// QSS Advance: Stage 2: Relaxation: Yoyo
	void
	advance_QSS_2_relax_yoyo( Real const x_2 )
	{
		q_2_ = x_2_ = x_2_rlx_ * ( x_2_QSS_ = x_2 );
	}

	// QSS Advance: Stage 2: Relaxation: QSS
	void
	advance_QSS_2_relax_QSS( Real const x_2 )
	{
		q_2_ = x_2_ = x_2;
		Real const x_1_in( x_1_pre_ + ( two * x_2_tDel_ ) ); // Incoming slope
		Real const x_1_dif( x_1_ - x_1_in );
		bool const x_1_dif_sign( bool_sign( x_1_dif ) );
		if ( ( std::abs( x_1_dif ) > yoyo_mul_ * std::abs( x_1_ - q_1_pre_ ) ) && ( ( n_yoyo_ == 0u ) || ( x_1_dif_sign == x_1_dif_sign_ ) ) ) { // Yo-yoing criteria met
			x_1_dif_sign_ = x_1_dif_sign;
			yoyo_ = ( ++n_yoyo_ >= m_yoyo_ );
			q_2_ = x_2_ = ( yoyo_ ? x_2_rlx_ * ( x_2_QSS_ = x_2_ ) : x_2_ );
			if ( yoyo_ && options::output::d ) std::cout << name() << " advance_QSS yoyo on " << tE << std::endl;
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

	Real q_0_{ 0.0 }, q_1_{ 0.0 }, q_2_{ 0.0 }; // Quantized trajectory coefficients
	Real x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }; // Continuous trajectory coefficients

	Real q_1_pre_{ 0.0 }; // Previous 1st order quantized trajectory coefficient
	Real x_1_pre_{ 0.0 }; // Previous 1st order continuous trajectory coefficient
	Real x_2_QSS_{ 0.0 }; // QSS 2nd order coefficient
	Real x_2_tDel_{ 0.0 }; // x_2_ * ( tE - tX )
	Time dt_pre_{ infinity }; // Previous time step
	std::uint8_t n_yoyo_{ 0u }; // Number of yo-yo sequential requantization steps currently
	bool x_1_dif_sign_{ false }; // Sign of previous x_1_ - x_1_in
	bool yoyo_{ false }; // Yo-yoing mode on?

	static constexpr std::uint8_t m_yoyo_{ 5u }; // Number of yo-yo sequential requantization steps threshold
	static constexpr double yoyo_mul_{ 100.0 }; // Yo-yo slope difference criterion multiplier
	static constexpr double dt_growth_mul_{ 1.5 }; // Time step growth damping multiplier
	static constexpr double dt_growth_inf_{ infinity / dt_growth_mul_ }; // Time step growth infinity threshold
	static constexpr double x_2_rlx_{ one_half }; // 2nd order coefficient relaxation factor

}; // Variable_rxQSS2

} // QSS

#endif
