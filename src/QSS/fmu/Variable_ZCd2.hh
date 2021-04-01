// FMU-Based QSS2 Directional Derivative Zero-Crossing Variable
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

#ifndef QSS_fmu_Variable_ZCd2_hh_INCLUDED
#define QSS_fmu_Variable_ZCd2_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable_ZC.hh>

namespace QSS {
namespace fmu {

// FMU-Based QSS2 Directional Derivative Zero-Crossing Variable
class Variable_ZCd2 final : public Variable_ZC
{

public: // Types

	using Super = Variable_ZC;

public: // Creation

	// Constructor
	Variable_ZCd2(
	 std::string const & name,
	 Real const rTol,
	 Real const aTol,
	 Real const zTol,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable()
	) :
	 Super( 2, name, rTol, aTol, zTol, fmu_me, var )
	{}

public: // Predicate

	// Directional Derivative Zero-Crossing Variable?
	bool
	is_ZCd() const override final
	{
		return true;
	}

public: // Property

	// Continuous Value at Time t
	Real
	x( Time const t ) const override final
	{
		Time const tDel( t - tX );
		return x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const t ) const override final
	{
		return x_1_ + ( two * x_2_ * ( t - tX ) );
	}

	// Continuous Second Derivative at Time t
	Real
	x2( Time const ) const override final
	{
		return two * x_2_;
	}

	// Quantized Value at Time t
	Real
	q( Time const t ) const override final
	{
		return x_0_ + ( x_1_ * ( t - tQ ) );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const ) const override final
	{
		return x_1_;
	}

	// Zero-Crossing Bump Time for FMU Detection
	Time
	tZC_bump( Time const t ) const override final
	{
		if ( zChatter_ ) {
			Real const x_1_t( x_1_ + ( two * x_2_ * ( t - tX ) ) );
			Real const bTol( options::zMul * zTol ); // Hope FMU detects the crossing at this bump tolerance
			Time dt_bump;
			if ( ( x_1_t >= 0.0 ) && ( x_2_ >= 0.0 ) ) { // Upper boundary crossing
				dt_bump = min_root_quadratic_upper( x_2_, x_1_t, -bTol );
			} else if ( ( x_1_t <= 0.0 ) && ( x_2_ <= 0.0 ) ) { // Lower boundary crossing
				dt_bump = min_root_quadratic_lower( x_2_, x_1_t, bTol );
			} else { // Both boundaries can have crossings
				dt_bump = min_root_quadratic_both( x_2_, x_1_t, bTol, -bTol );
			}
			if ( ( dt_bump <= 0.0 ) || ( dt_bump == infinity ) ) dt_bump = ( x_1_t != 0.0 ? bTol / std::abs( x_1_t ) : options::dtZC ); // Fall back to 1st order estimate
			return t + dt_bump;
		} else {
			return t + options::dtZC;
		}
	}

public: // Methods

	// Initialization
	void
	init() override final
	{
		// Check no observers
		if ( self_observer() || observed() ) {
			std::cerr << "Error: Zero-crossing variable has observers: " << name() << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Initialize observees
		init_observees();

		// Initialize specs
		detected_crossing_ = false;
		x_0_ = z_0();
		x_1_ = n_1();
		x_2_ = n_2();
		set_qTol();
		set_tE();
		set_tZ();
		( tE < tZ ) ? add_QSS_ZC( tE ) : add_ZC( tZ );
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << std::endl;
	}

	// QSS Advance
	void
	advance_QSS() override final
	{
		advance_pre( tE );
		tX = tQ = tE;
		x_0_ = z_0();
		x_1_ = n_1();
		x_2_ = n_2();
		set_qTol();
		set_tE();
		crossing_detect();
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << std::endl;
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0( Real const x_0 ) override final
	{
		advance_pre( tE );
		tX = tQ = tE;
		x_0_ = x_0;
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1( Real const x_1 ) override final
	{
		x_1_ = x_1;
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2( Real const x_1_p ) override final
	{
		x_2_ = n_2( x_1_p );
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2( Real const x_1_m, Real const x_1_p ) override final
	{
		x_2_ = n_2( x_1_m, x_1_p );
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2_forward( Real const x_1_p, Real const x_1_2p ) override final
	{
		x_2_ = f_2( x_1_p, x_1_2p );
	}

	// QSS Advance: Stage Final
	void
	advance_QSS_F() override final
	{
		set_qTol();
		set_tE();
		crossing_detect();
		if ( options::output::d ) std::cout << "!= " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << std::endl;
	}

	// Zero-Crossing Advance
	void
	advance_ZC() override final
	{
		assert( in_conditional() );
		conditional->activity( tZ );
		crossing_last = crossing;
		x_mag_zero();
		set_tZ( tZ_last = tZ ); // Next zero-crossing: Might be in active segment
		( tE < tZ ) ? shift_QSS_ZC( tE ) : shift_ZC( tZ );
		if ( options::output::d ) std::cout << "Z  " << name() << '(' << tZ_last << ')' << "   tZ=" << tZ << std::endl;
	}

	// Observer Advance
	void
	advance_observer( Time const t ) override final
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		advance_pre( t );
		tX = tQ = t;
		x_0_ = ( !handler_modified_ && ( t == tZ_last ) ? 0.0 : z_0() ); // Force exact zero if at zero-crossing time
		x_1_ = n_1();
		x_2_ = n_2();
		set_qTol();
		set_tE();
		crossing_detect();
	}

	// Observer Advance: Stage 1
	void
	advance_observer_1( Time const t, Real const x_0, Real const x_1 ) override final
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		advance_pre( t );
		tX = tQ = t;
		assert( x_0 == z_0() );
		assert( x_1 == n_1() );
		x_0_ = ( !handler_modified_ && ( t == tZ_last ) ? 0.0 : x_0 ); // Force exact zero if at zero-crossing time
		x_1_ = x_1;
	}

	// Observer Advance: Stage 2
	void
	advance_observer_2( Real const x_1_p ) override final
	{
		x_2_ = n_2( x_1_p );
		set_qTol();
		set_tE();
		crossing_detect();
	}

	// Observer Advance: Stage 2
	void
	advance_observer_2( Real const x_1_m, Real const x_1_p ) override final
	{
		x_2_ = n_2( x_1_m, x_1_p );
		set_qTol();
		set_tE();
		crossing_detect();
	}

	// Observer Advance: Stage 2
	void
	advance_observer_2_forward( Real const x_1_p, Real const x_1_2p ) override final
	{
		x_2_ = f_2( x_1_p, x_1_2p );
		set_qTol();
		set_tE();
		crossing_detect();
	}

	// Observer Advance: Stage d
	void
	advance_observer_d() const override final
	{
		std::cout << " ^ " << name() << '(' << tX << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << std::endl;
	}

private: // Methods

	// Set QSS Tolerance
	void
	set_qTol()
	{
		qTol = std::max( rTol * std::abs( x_0_ ), aTol );
		assert( qTol > 0.0 );
	}

	// Updates Before Trajectory Advance to Time t
	void
	advance_pre( Time const t )
	{
		bool const past_tZ( t > tZ_last );
		Real const x_t( this->Variable_ZCd2::x( t ) );

		// Unpredicted zero crossing check setup
		check_crossing_ = past_tZ;
		if ( past_tZ || ( x_t != 0.0 ) ) sign_old_ = signum( x_t );

		// Anti-chatter trajectory magnitude updates for [tX,t] span
		if ( zChatter_ && past_tZ ) { // Anti-chatter is active
			x_mag_update( x_t ); // Trajectory can have a discontinuity at updates
			if ( nonzero_and_signs_differ( x_1_, x_2_ ) ) { // Critical point > tX
				Real const two_x_2_inv( 1.0 / ( 2.0 * x_2_ ) );
				if ( tX + ( -( x_1_ * two_x_2_inv ) ) < t ) { // Critical point < t: Update magnitude with critical point value
					x_mag_update( x_0_ - ( one_half * square( x_1_ ) * two_x_2_inv ) );
				}
			}
		}
	}

	// Set End Time
	void
	set_tE()
	{
		assert( tX <= tQ );
		assert( dt_min <= dt_max );
		x_mag_update( x_0_ );
		Time dt( x_2_ != 0.0 ? std::sqrt( qTol / std::abs( x_2_ ) ) : infinity );
		dt = std::min( std::max( dt_infinity( dt ), dt_min ), dt_max );
		tE = ( dt != infinity ? tQ + dt : infinity );
		if ( ( options::inflection ) && ( x_2_ != 0.0 ) && ( signum( x_1_ ) != signum( x_2_ ) ) ) {
			Time const tI( tX - ( x_1_ / ( two * x_2_ ) ) );
			if ( tQ < tI ) tE = std::min( tE, tI );
		}
	}

	// Set Zero-Crossing Time and Type on Active Segment
	void
	set_tZ()
	{
		// Find root of continuous rep: Only robust for small active segments with continuous rep close to function
		Time const dt( zc_root_quadratic( x_2_, x_1_, x_0_, zTol, x_mag_ ) );
		assert( dt > 0.0 );
		if ( dt != infinity ) { // Root exists
			tZ = tX + dt;
			Crossing const crossing_check( x_0_ == 0.0 ? ( tZ == tX ? Crossing::Flat : crossing_type( -x_1_ ) ) : crossing_type( x_0_ > 0.0 ? std::min( this->Variable_ZCd2::x1( tZ ), Real( 0.0 ) ) : std::max( this->Variable_ZCd2::x1( tZ ), Real( 0.0 ) ) ) );
			if ( has( crossing_check ) ) { // Crossing type is relevant
				crossing = crossing_check;
				if ( options::refine ) refine_root_ZCd( tX ); // Refine root: Expensive!
			} else { // Crossing type not relevant
				tZ = infinity;
			}
		} else { // Root not found
			tZ = infinity;
		}
	}

	// Set Zero-Crossing Time and Type on (tB,tE]
	void
	set_tZ( Time const tB )
	{
		// Find root of continuous rep: Only robust for small active segments with continuous rep close to function
		Time const dB( tB - tX );
		assert( dB >= 0.0 );
		Real const x_0( tB == tZ_last ? 0.0 : x_0_ + ( x_1_ * dB ) + ( x_2_ * square( dB ) ) );
		Real const x_1( x_1_ + ( two * x_2_ * dB ) );
		Time const dt( zc_root_quadratic( x_2_, x_1, x_0, zTol, x_mag_ ) ); // Positive root using trajectory shifted to tB
		assert( dt > 0.0 );
		if ( dt != infinity ) { // Root exists
			tZ = tB + dt;
			Crossing const crossing_check( x_0 == 0.0 ? ( tZ == tB ? Crossing::Flat : crossing_type( -x_1 ) ) : crossing_type( x_0 > 0.0 ? std::min( this->Variable_ZCd2::x1( tZ ), Real( 0.0 ) ) : std::max( this->Variable_ZCd2::x1( tZ ), Real( 0.0 ) ) ) );
			if ( has( crossing_check ) ) { // Crossing type is relevant
				crossing = crossing_check;
				if ( options::refine ) refine_root_ZCd( tB ); // Refine root: Expensive!
			} else { // Crossing type not relevant
				tZ = infinity;
			}
		} else { // Root not found
			tZ = infinity;
		}
	}

	// Zero Crossing Detection and Set Next Crossing Time
	void
	crossing_detect()
	{
		if ( zChatter_ && ( x_mag_ < zTol ) ) { // Anti-chatter => Don't check for crossing
			set_tZ();
			( tE < tZ ) ? shift_QSS_ZC( tE ) : shift_ZC( tZ );
		} else { // Maybe check for crossing
			int const sign_new( signum( x_0_ ) );
			if ( ( ! check_crossing_ ) || ( sign_old_ == sign_new ) ) { // Don't check for crossing
				set_tZ();
				( tE < tZ ) ? shift_QSS_ZC( tE ) : shift_ZC( tZ );
			} else { // Check zero-crossing
				Crossing const crossing_check( crossing_type( sign_old_, sign_new ) );
				if ( has( crossing_check ) ) { // Crossing type is relevant
					crossing = crossing_check;
					detected_crossing_ = true;
					shift_ZC( tZ = tX );
				} else {
					set_tZ();
					( tE < tZ ) ? shift_QSS_ZC( tE ) : shift_ZC( tZ );
				}
			}
		}
	}

	// Coefficient 1 from FMU at Time tQ
	Real
	n_1() const
	{
		return Z_1();
	}

	// Coefficient 2 from FMU at Time tQ
	Real
	n_2() const
	{
		return Z_2( x_1_ );
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

	Real x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }; // Coefficients

}; // Variable_ZCd2

} // fmu
} // QSS

#endif
