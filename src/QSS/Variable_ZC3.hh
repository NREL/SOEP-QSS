// QSS3 Zero-Crossing Variable
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

#ifndef QSS_Variable_ZC3_hh_INCLUDED
#define QSS_Variable_ZC3_hh_INCLUDED

// QSS Headers
#include <QSS/Variable_ZC.hh>

namespace QSS {

// QSS3 Zero-Crossing Variable
class Variable_ZC3 final : public Variable_ZC
{

public: // Types

	using Super = Variable_ZC;

public: // Creation

	// Constructor
	Variable_ZC3(
	 FMU_ME * fmu_me,
	 std::string const & name,
	 Real const rTol_ = options::rTol,
	 Real const aTol_ = options::aTol,
	 Real const zTol_ = options::zTol,
	 Real const xIni_ = 0.0,
	 FMU_Variable const & var = FMU_Variable()
	) :
	 Super( fmu_me, 3, name, rTol_, aTol_, zTol_, xIni_, var ),
	 x_0_( xIni_ )
	{
		set_qTol();
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
		return x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const t ) const override
	{
		return x_1_ + ( two * x_2_ * ( t - tQ ) );
	}

	// Quantized Second Derivative at Time t
	Real
	q2( Time const ) const override
	{
		return two * x_2_;
	}

	// Zero-Crossing Bump Time for FMU Detection
	Time
	tZC_bump( Time const t ) const override
	{
		if ( zTol > 0.0 ) {
			Time const tDel( t - tX );
			Real const x_1_t( x_1_ + ( ( ( two * x_2_ ) + ( three * x_3_ * tDel ) ) * tDel ) );
			Real const x_2_t( x_2_ + ( three * x_3_ * tDel ) );
			Real const bTol( options::zMul * zTol ); // Hope FMU detects the crossing at this bump tolerance
			Time dt_bump;
			if ( ( x_3_ >= 0.0 ) && ( x_2_t >= 0.0 ) && ( x_1_t >= 0.0 ) ) { // Upper boundary crossing
				dt_bump = min_root_cubic_upper( x_3_, x_2_t, x_1_t, -bTol, zTol );
			} else if ( ( x_3_ <= 0.0 ) && ( x_2_t <= 0.0 ) && ( x_1_t <= 0.0 ) ) { // Lower boundary crossing
				dt_bump = min_root_cubic_lower( x_3_, x_2_t, x_1_t, bTol, zTol );
			} else { // Both boundaries can have crossings
				dt_bump = min_root_cubic_both( x_3_, x_2_t, x_1_t, bTol, -bTol, zTol );
			}
			if ( ( dt_bump <= 0.0 ) || ( dt_bump == infinity ) ) dt_bump = min_root_quadratic_both( x_2_t, x_1_t, bTol, -bTol ); // Fall back to 2nd order estimate
			if ( ( dt_bump <= 0.0 ) || ( dt_bump == infinity ) ) dt_bump = ( x_1_t != 0.0 ? bTol / std::abs( x_1_t ) : options::dtZC ); // Fall back to 1st order estimate
			return t + dt_bump;
		} else {
			return t + options::dtZC;
		}
	}

public: // Methods

	// Initialization: Stage 0
	void
	init_0() override
	{
		assert( !connected() );

		// Check no observers
		assert( !self_observer() );
		if ( observed() ) {
			std::cerr << "Error: Zero-crossing variable has observers: " << name() << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Initialize specs
		detected_crossing_ = false;
		x_0_ = z_0();
		x_mag_ = std::abs( x_0_ );
		x_1_ = n_1();
		x_2_ = f_2();
		fmu_set_observees_x( t0() );
		x_3_ = f_3();
		set_qTol();
		set_tE();
		set_tZ();
		( tE < tZ ) ? add_QSS_ZC( tE ) : add_ZC( tZ );
		fixup_tE();
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << std::endl;
	}

	// QSS Advance
	void
	advance_QSS() override
	{
		advance_pre( tE );
		tS = tE - tQ;
		tQ = tX = tE;
		x_0_ = z_0();
		x_1_ = n_1();
		if ( fwd_time_ND( tE ) ) { // Centered ND
			x_2_ = n_2();
			x_3_ = n_3();
		} else { // Forward ND
			x_2_ = f_2();
			x_3_ = f_3();
		}
		set_qTol();
		set_tE();
		crossing_detect();
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << std::endl;
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0( Real const x_0 ) override
	{
		advance_pre( tE );
		tS = tE - tQ;
		tQ = tX = tE;
		x_0_ = x_0;
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1( Real const x_1 ) override
	{
		x_1_ = x_1;
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2( Real const x_1_m, Real const x_1_p ) override
	{
		x_2_ = n_2( x_1_m, x_1_p );
	}

	// QSS Advance: Stage 2: Forward ND
	void
	advance_QSS_2_forward( Real const x_1_p, Real const x_1_2p ) override
	{
		x_2_ = f_2( x_1_p, x_1_2p );
	}

	// QSS Advance: Stage 3
	void
	advance_QSS_3() override
	{
		x_3_ = n_3();
	}

	// QSS Advance: Stage 3: Forward ND
	void
	advance_QSS_3_forward() override
	{
		x_3_ = f_3();
	}

	// QSS Advance: Stage Final
	void
	advance_QSS_F() override
	{
		set_qTol();
		set_tE();
		crossing_detect();
		if ( options::output::d ) std::cout << "!= " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << std::endl;
	}

	// QSS Advance: Stage Debug
	void
	advance_QSS_d() override
	{
		assert( options::output::d );
		std::cout << "!= " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << std::endl;
	}

	// Zero-Crossing Advance
	void
	advance_ZC() override
	{
		assert( in_conditional() );
		conditional->activity( tZ );
		crossing_last = crossing;
		x_mag_zero();
		set_tZ( tZ_last = tZ ); // Next zero-crossing: Might be in active segment
		( tE < tZ ) ? shift_QSS_ZC( tE ) : shift_ZC( tZ );
		fixup_tE();
		if ( options::output::d ) std::cout << "Z  " << name() << '(' << tZ_last << ')' << "   tE=" << tE << "   tZ=" << tZ << std::endl;
	}

	// Handler Advance
	void
	advance_handler( Time const t ) override
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		advance_pre( t );
		tS = t - tQ;
		tQ = tX = t;
		x_0_ = p_0();
		x_1_ = n_1();
		if ( fwd_time_ND( t ) ) { // Centered ND
			x_2_ = n_2();
			x_3_ = n_3();
		} else { // Forward ND
			x_2_ = f_2();
			x_3_ = f_3();
		}
		set_qTol();
		set_tE();
		crossing_detect();
		if ( options::output::d ) std::cout << "*  " << name() << '(' << tX << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << std::endl;
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t, Real const x_0 ) override
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		advance_pre( t );
		tS = t - tQ;
		tQ = tX = t;
		x_0_ = x_0;
	}

	// Handler Advance: Stage 1
	void
	advance_handler_1( Real const x_1 ) override
	{
		x_1_ = x_1;
	}

	// Handler Advance: Stage 2
	void
	advance_handler_2( Real const x_1_m, Real const x_1_p ) override
	{
		x_2_ = n_2( x_1_m, x_1_p );
	}

	// QSS Advance: Stage 2: Forward ND
	void
	advance_handler_2_forward( Real const x_1_p, Real const x_1_2p ) override
	{
		x_2_ = f_2( x_1_p, x_1_2p );
	}

	// Handler Advance: Stage 3
	void
	advance_handler_3() override
	{
		x_3_ = n_3();
	}

	// Handler Advance: Stage 3: Forward ND
	void
	advance_handler_3_forward() override
	{
		x_3_ = f_3();
	}

	// Handler Advance: Stage Final
	void
	advance_handler_F() override
	{
		set_qTol();
		set_tE();
		crossing_detect();
		if ( options::output::d ) std::cout << "*= " << name() << '(' << tX << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << std::endl;
	}

	// Handler No-Advance
	void
	no_advance_handler() override
	{
		crossing_detect();
	}

	// Observer Advance: Stage 1
	void
	advance_observer_1( Time const t, Real const x_0, Real const x_1 ) override
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		advance_pre( t );
		tS = t - tQ;
		tQ = tX = t;
		x_0_ = ( !handler_modified_ && ( t == tZ_last ) ? 0.0 : x_0 ); // Force exact zero if at zero-crossing time
		x_1_ = x_1;
	}

	// Observer Advance: Stage 2
	void
	advance_observer_2( Real const x_1_m, Real const x_1_p ) override
	{
		x_2_ = n_2( x_1_m, x_1_p );
	}

	// Observer Advance: Stage 2: Forward ND
	void
	advance_observer_2_forward( Real const x_1_p, Real const x_1_2p ) override
	{
		x_2_ = f_2( x_1_p, x_1_2p );
	}

	// Observer Advance: Stage 3
	void
	advance_observer_3() override
	{
		x_3_ = n_3();
	}

	// Observer Advance: Stage 3: Forward ND
	void
	advance_observer_3_forward() override
	{
		x_3_ = f_3();
	}

	// Observer Advance: Stage Final
	void
	advance_observer_F() override
	{
		set_qTol();
		set_tE();
		crossing_detect();
	}

	// Observer Advance: Stage Final: Parallel
	void
	advance_observer_F_parallel() override
	{
		set_qTol();
		set_tE();
	}

	// Observer Advance: Stage Final: Serial
	void
	advance_observer_F_serial() override
	{
		crossing_detect();
	}

	// Observer Advance: Stage d
	void
	advance_observer_d() const override
	{
		std::cout << " ^ " << name() << '(' << tX << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << x_2_ << x_delta_2 << x_3_ << x_delta_3 << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << std::endl;
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
		Real const x_t( this->Variable_ZC3::x( t ) );

		// Unpredicted zero crossing check setup
		check_crossing_ = past_tZ;
		if ( past_tZ || ( x_t != 0.0 ) ) sign_old_ = signum( x_t );

		// Anti-chatter trajectory magnitude updates for [tX,t] span
		if ( zChatter_ && past_tZ ) { // Anti-chatter is active
			x_mag_update( x_t ); // Trajectory can have a discontinuity at updates
			x_mag_update( critical_point_magnitude_cubic( x_3_, x_2_, x_1_, x_0_, t - tX ) );
		}
	}

	// Set End Time
	void
	set_tE()
	{
		assert( tQ == tX );
		assert( dt_min <= dt_max );
		Time dt;
		if ( x_3_ != 0.0 ) {
			Real const x_3_inv( one / x_3_ );
			dt = dt_infinity( std::cbrt( qTol * std::abs( x_3_inv ) ) );
			assert( dt != infinity );
			if ( options::inflection && nonzero_and_signs_differ( x_2_, x_3_ ) ) { // Inflection point
				Time const dtI( -( x_2_ * ( one_third * x_3_inv ) ) );
				dt = ( ( dtI < dt ) && ( dt * options::inflectionFrac < dtI ) ? dtI : dt );
			}
			dt = std::min( std::max( dt, dt_min ), dt_max );
			tE = tQ + dt;
		} else {
			dt = std::min( std::max( dt_infinity_of_infinity(), dt_min ), dt_max );
			tE = ( dt != infinity ? tQ + dt : infinity );
		}
		if ( tQ == tE ) {
			tE = std::nextafter( tE, infinity );
			dt = tE - tQ;
		}
		x_mag_update( x_0_ );
	}

	// Set Zero-Crossing Time and Type on Active Segment
	void
	set_tZ()
	{
		// Find root of continuous trajectory: Only robust for small active segments with continuous trajectory close to function
		Time const dt( zc_root_cubic( x_3_, x_2_, x_1_, x_0_, zTol, x_mag_ ) );
		assert( dt > 0.0 );
		if ( dt != infinity ) { // Root exists
			tZ = tX + dt;
			if ( tZ <= tZ_last ) { // Crossing already processed
				tZ = infinity;
			} else { // Check crossing
				Crossing const crossing_check( x_0_ == 0.0 ? ( tZ == tX ? Crossing::Flat : crossing_type( -x_1_ ) ) : crossing_type( x_0_ > 0.0 ? std::min( this->Variable_ZC3::x1( tZ ), Real( 0.0 ) ) : std::max( this->Variable_ZC3::x1( tZ ), Real( 0.0 ) ) ) );
				if ( has( crossing_check ) ) { // Crossing type is relevant
					crossing = crossing_check;
					if ( options::refine ) refine_root_ZC( tX ); // Refine root: Expensive!
				} else { // Crossing type not relevant
					tZ = infinity;
				}
			}
		} else { // Root not found
			tZ = infinity;
		}
	}

	// Set Zero-Crossing Time and Type on (tB,tE]
	void
	set_tZ( Time const tB )
	{
		// Find root of continuous trajectory: Only robust for small active segments with continuous trajectory close to function
		Time const dB( tB - tX );
		assert( dB >= 0.0 );
		Real const x_0( ( tB == tZ_last ) && !( handler_modified_ = ( fmu_get_real() != x_0_bump_ ) ) ? 0.0 : x_0_ + ( x_1_ * dB ) + ( x_2_ * square( dB ) ) );
		Real const x_1( x_1_ + ( two * x_2_ * dB ) );
		Time const dt( zc_root_cubic( x_3_, x_2_, x_1, x_0, zTol, x_mag_ ) ); // Positive root using trajectory shifted to tB
		assert( dt > 0.0 );
		if ( dt != infinity ) { // Root exists
			tZ = tB + dt;
			if ( tZ <= tZ_last ) { // Crossing already processed
				tZ = infinity;
			} else { // Check crossing
				Crossing const crossing_check( x_0 == 0.0 ? ( tZ == tB ? Crossing::Flat : crossing_type( -x_1 ) ) : crossing_type( x_0 > 0.0 ? std::min( this->Variable_ZC3::x1( tZ ), Real( 0.0 ) ) : std::max( this->Variable_ZC3::x1( tZ ), Real( 0.0 ) ) ) );
				if ( has( crossing_check ) ) { // Crossing type is relevant
					crossing = crossing_check;
					if ( options::refine ) refine_root_ZC( tB ); // Refine root: Expensive!
				} else { // Crossing type not relevant
					tZ = infinity;
				}
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
			if ( !check_crossing_ || ( sign_old_ == sign_new ) || ( tX <= tZ_last ) ) { // Don't check for crossing
				set_tZ();
				( tE < tZ ) ? shift_QSS_ZC( tE ) : shift_ZC( tZ );
			} else { // Check for crossing
				Crossing const crossing_check( crossing_type( sign_old_, sign_new ) );
				if ( has( crossing_check ) ) { // Crossing type is relevant
					crossing = crossing_check;
					detected_crossing_ = true;
					shift_ZC( tZ = tX );
				} else { // Crossing type is not relevant
					set_tZ();
					( tE < tZ ) ? shift_QSS_ZC( tE ) : shift_ZC( tZ );
				}
			}
		}
		fixup_tE();
	}

	// Coefficient 1 at Time tQ
	Real
	n_1() const
	{
		return X_1();
	}

	// Coefficient 2 at Time tQ
	Real
	n_2() const
	{
		Time tN( tQ - options::dtND );
		fmu_set_time( tN );
		x_1_m_ = X_1( tN );
		tN = tQ + options::dtND;
		fmu_set_time( tN );
		x_1_p_ = X_1( tN );
		fmu_set_time( tQ );
		return options::one_over_four_dtND * ( x_1_p_ - x_1_m_ ); //ND Centered difference
	}

	// Coefficient 2 at Time tQ
	Real
	f_2() const
	{
		Time tN( tQ + options::dtND );
		fmu_set_time( tN );
		x_1_p_ = X_1( tN );
		tN = tQ + options::two_dtND;
		fmu_set_time( tN );
		x_1_2p_ = X_1( tN );
		fmu_set_time( tQ );
		return options::one_over_four_dtND * ( ( three * ( x_1_p_ - x_1_ ) ) + ( x_1_p_ - x_1_2p_ ) ); //ND Forward 3-point
	}

	// Coefficient 2
	Real
	n_2( Real const x_1_m, Real const x_1_p ) const
	{
		return options::one_over_four_dtND * ( ( x_1_p_ = x_1_p ) - ( x_1_m_ = x_1_m ) ); //ND Centered difference
	}

	// Coefficient 2
	Real
	f_2( Real const x_1_p, Real const x_1_2p ) const
	{
		return options::one_over_four_dtND * ( ( three * ( ( x_1_p_ = x_1_p ) - x_1_ ) ) + ( x_1_p - ( x_1_2p_ = x_1_2p ) ) ); //ND Forward 3-point
	}

	// Coefficient 3
	Real
	n_3() const
	{
		return options::one_over_six_dtND_squared * ( ( x_1_p_ - x_1_ ) + ( x_1_m_ - x_1_ ) ); //ND Centered difference
	}

	// Coefficient 3
	Real
	f_3() const
	{
		return options::one_over_six_dtND_squared * ( ( x_1_2p_ - x_1_p_ ) + ( x_1_ - x_1_p_ ) ); //ND Forward 3-point
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }, x_3_{ 0.0 }; // Trajectory coefficients
	mutable Real x_1_m_{ 0.0 }, x_1_p_{ 0.0 }, x_1_2p_{ 0.0 }; // Trajectory coefficient 1 at numeric differentiation time offsets

}; // Variable_ZC3

} // QSS

#endif
