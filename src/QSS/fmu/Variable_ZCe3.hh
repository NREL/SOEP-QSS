// FMU-Based QSS3 Explicit Zero-Crossing Variable
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2020 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_fmu_Variable_ZCe3_hh_INCLUDED
#define QSS_fmu_Variable_ZCe3_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable_ZC.hh>

namespace QSS {
namespace fmu {

// FMU-Based QSS3 Zero-Crossing Variable
class Variable_ZCe3 final : public Variable_ZC
{

public: // Types

	using Super = Variable_ZC;

private: // Types

	using Super::p_2;

public: // Creation

	// Constructor
	Variable_ZCe3(
	 std::string const & name,
	 Real const rTol,
	 Real const aTol,
	 Real const zTol,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Super( 3, name, rTol, aTol, zTol, fmu_me, var, der )
	{}

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
		return x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const t ) const
	{
		return x_1_ + ( two * x_2_ * ( t - tQ ) );
	}

	// Quantized Second Derivative at Time t
	Real
	q2( Time const ) const
	{
		return two * x_2_;
	}

	// Zero-Crossing Bump Time for FMU Detection
	Time
	tZC_bump( Time const t ) const
	{
		if ( zTol > 0.0 ) {
			Time const tDel( t - tX );
			Real const x_1_t( x_1_ + ( ( ( two * x_2_ ) + ( three * x_3_ * tDel ) ) * tDel ) );
			Real const x_2_t( x_2_ + ( three * x_3_ * tDel ) );
			Real const zTol2( 2.0 * zTol ); // Hope FMU detects the crossing at 2x the zTol
			Real dt_bump( min_root_cubic_both( x_3_, x_2_t, x_1_t, zTol2, -zTol2 ) );
			if ( dt_bump <= 0.0 ) dt_bump = min_root_quadratic_both( x_2_t, x_1_t, zTol2, -zTol2 ); // Fall back to 2nd order estimate
			if ( dt_bump <= 0.0 ) dt_bump = ( x_1_t != 0.0 ? zTol2 / std::abs( x_1_t ) : options::dtZC ); // Fall back to 1st order estimate
			return t + dt_bump;
		} else {
			return t + options::dtZC;
		}
	}

public: // Methods

	// Initialization
	void
	init()
	{
		// Check no observers
		if ( self_observer() || observed() ) {
			std::cerr << "Error: Zero-crossing variable has observers: " << name() << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Initialize observees
		init_observees();

		// Initialize specs
		x_0_ = p_0();
		x_mag_ = std::abs( x_0_ );
		x_1_ = p_1();
		x_2_ = z_2();
		x_3_ = z_3();
		set_qTol();
		set_tE();
		set_tZ();
		( tE < tZ ) ? add_QSS_ZC( tE ) : add_ZC( tZ );
		if ( options::output::d ) std::cout << "! " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << '\n';
	}

	// QSS Advance
	void
	advance_QSS()
	{
		tX = tQ = tE;
		Real const x_tE( zChatter_ ? x( tE ) : Real( 0.0 ) );
#ifndef QSS_ZC_REQUANT_NO_CROSSING_CHECK
		check_crossing_ = ( tE > tZ_last ) || ( x_mag_ != 0.0 );
		sign_old_ = ( check_crossing_ ? signum( zChatter_ ? x_tE : x( tE ) ) : 0 );
#endif
		x_0_ = z_0();
		x_mag_ = max( x_mag_, std::abs( x_tE ), std::abs( x_0_ ) );
		x_1_ = p_1();
		x_2_ = z_2();
		x_3_ = z_3();
		set_qTol();
		set_tE();
#ifndef QSS_ZC_REQUANT_NO_CROSSING_CHECK
		crossing_detect( sign_old_, signum( x_0_ ), check_crossing_ );
#else
		set_tZ();
		( tE < tZ ) ? shift_QSS_ZC( tE ) : shift_ZC( tZ );
#endif
		if ( options::output::d ) std::cout << "! " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << '\n';
	}

	// Zero-Crossing Advance
	void
	advance_ZC()
	{
		assert( in_conditional() );
		conditional->activity( tZ );
		if ( options::output::d ) std::cout << "Z " << name() << '(' << tZ << ')' << '\n';
		crossing_last = crossing;
		x_mag_ = 0.0;
		set_tZ( tZ_last = tZ ); // Next zero-crossing: Might be in active segment
		( tE < tZ ) ? shift_QSS_ZC( tE ) : shift_ZC( tZ );
	}

	// Observer Advance
	void
	advance_observer( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		tX = tQ = t;
		Real const x_t( zChatter_ ? x( t ) : Real( 0.0 ) );
		check_crossing_ = ( t > tZ_last ) || ( x_mag_ != 0.0 );
		sign_old_ = ( check_crossing_ ? signum( zChatter_ ? x_t : x( t ) ) : 0 );
		x_0_ = z_0( t );
		x_mag_ = max( x_mag_, std::abs( x_t ), std::abs( x_0_ ) );
		x_1_ = p_1();
		x_2_ = z_2();
		x_3_ = z_3();
		set_qTol();
		set_tE();
		crossing_detect( sign_old_, signum( x_0_ ), check_crossing_ );
	}

	// Observer Advance: Stage 1
	void
	advance_observer_1( Time const t, Real const d, Real const v )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		assert( d == p_1() );
		assert( v == p_0() );
		tX = tQ = t;
		Real const x_t( zChatter_ ? x( t ) : Real( 0.0 ) );
		check_crossing_ = ( t > tZ_last ) || ( x_mag_ != 0.0 );
		sign_old_ = ( check_crossing_ ? signum( zChatter_ ? x_t : x( t ) ) : 0 );
		x_0_ = v;
		x_mag_ = max( x_mag_, std::abs( x_t ), std::abs( x_0_ ) );
		x_1_ = d;
	}

	// Observer Advance: Stage 2
	void
	advance_observer_2( Real const d )
	{
		assert( d == p_1() );
		x_2_ = p_2( d );
	}

	// Observer Advance: Stage 3
	void
	advance_observer_3( Real const d )
	{
		assert( d == p_1() );
		x_3_ = p_3( d );
		set_qTol();
		set_tE();
		crossing_detect( sign_old_, signum( x_0_ ), check_crossing_ );
	}

	// Observer Advance: Stage d
	void
	advance_observer_d() const
	{
		std::cout << "  " << name() << '(' << tX << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ <<  '\n';
	}

private: // Methods

	// Set QSS Tolerance
	void
	set_qTol()
	{
		qTol = std::max( rTol * std::abs( x_0_ ), aTol ) * options::zFac;
		assert( qTol > 0.0 );
	}

	// Set End Time
	void
	set_tE()
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

	// Set Zero-Crossing Time and Type on Active Segment
	void
	set_tZ()
	{
		if ( zChatter_ && ( x_mag_ < zTol ) ) { // Chatter prevention
			tZ = infinity;
		} else { // Use root of continuous rep: Only robust for small active segments with continuous rep close to function
			Time const dt( min_positive_root_cubic( x_3_, x_2_, x_1_, x_0_ ) );
			assert( dt > 0.0 );
			if ( dt != infinity ) { // Root exists
				tZ = tX + dt;
				Crossing const crossing_check( x_0_ == 0.0 ? ( tZ == tX ? Crossing::Flat : crossing_type( -x_1_ ) ) :
				 crossing_type( x_0_ > 0.0 ? std::min( x1( tZ ), Real( 0.0 ) ) : std::max( x1( tZ ), Real( 0.0 ) ) ) );
				if ( has( crossing_check ) ) { // Crossing type is relevant
					crossing = crossing_check;
					if ( options::refine ) refine_root_ZCe( tX ); // Refine root: Expensive!
				} else { // Crossing type not relevant
					tZ = infinity;
				}
			} else { // Root not found
				tZ = infinity;
			}
		}
	}

	// Set Zero-Crossing Time and Type on (tB,tE]
	void
	set_tZ( Time const tB )
	{
		if ( zChatter_ && ( x_mag_ < zTol ) ) { // Chatter prevention
			tZ = infinity;
		} else { // Use root of continuous rep: Only robust for small active segments with continuous rep close to function
			Time const dB( tB - tX );
			assert( dB >= 0.0 );
			Real const x_0( tB == tZ_last ? 0.0 : x_0_ + ( x_1_ * dB ) + ( x_2_ * square( dB ) ) );
			Real const x_1( x_1_ + ( two * x_2_ * dB ) );
			Time const dt( min_positive_root_cubic( x_3_, x_2_, x_1, x_0 ) ); // Positive root using trajectory shifted to tB
			assert( dt > 0.0 );
			if ( dt != infinity ) { // Root exists
				tZ = tB + dt;
				Crossing const crossing_check( x_0 == 0.0 ? ( tZ == tB ? Crossing::Flat : crossing_type( -x_1 ) ) :
				 crossing_type( x_0 > 0.0 ? std::min( x1( tZ ), Real( 0.0 ) ) : std::max( x1( tZ ), Real( 0.0 ) ) ) );
				if ( has( crossing_check ) ) { // Crossing type is relevant
					crossing = crossing_check;
					if ( options::refine ) refine_root_ZCe( tB ); // Refine root: Expensive!
				} else { // Crossing type not relevant
					tZ = infinity;
				}
			} else { // Root not found
				tZ = infinity;
			}
		}
	}

	// Crossing Detection
	void
	crossing_detect( int const sign_old, int const sign_new, bool const check_crossing = true )
	{
		if ( zChatter_ && ( x_mag_ < zTol ) ) { // Chatter prevention
			tZ = infinity;
			shift_QSS_ZC( tE );
		} else if ( ( ! check_crossing ) || ( sign_old == sign_new ) ) {
			set_tZ();
			( tE < tZ ) ? shift_QSS_ZC( tE ) : shift_ZC( tZ );
		} else { // Check zero-crossing
			Crossing const crossing_check( crossing_type( sign_old, sign_new ) );
			if ( has( crossing_check ) ) { // Crossing type is relevant
				crossing = crossing_check;
				shift_ZC( tZ = tX );
			} else {
				set_tZ();
				( tE < tZ ) ? shift_QSS_ZC( tE ) : shift_ZC( tZ );
			}
		}
	}

	// Coefficient 2 from FMU
	Real
	p_2( Real const d ) const
	{
		return p_2( x_1_p_ = d, x_1_ );
	}

	// Coefficient 2 from FMU at Time tQ
	Real
	z_2() const
	{
		Time tN( tQ - options::dtNum );
		fmu_set_time( tN );
		x_1_m_ = z_1( tN );
		tN = tQ + options::dtNum;
		fmu_set_time( tN );
		x_1_p_ = z_1( tN );
		fmu_set_time( tQ );
		return options::one_over_four_dtNum * ( x_1_p_ - x_1_m_ ); //ND Centered difference
	}

	// Coefficient 3 from FMU
	Real
	p_3( Real const d ) const
	{
		return options::one_over_six_dtNum_squared * ( x_1_p_ - ( two * x_1_ ) + d ); //ND Centered difference
	}

	// Coefficient 3 from FMU
	Real
	z_3() const
	{
		return options::one_over_two_dtNum_squared * ( x_1_p_ - ( two * x_1_ ) + x_1_m_ ); //ND Centered difference
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }, x_3_{ 0.0 }; // Continuous rep coefficients
	mutable Real x_1_m_{ 0.0 }, x_1_p_{ 0.0 }; // Coefficient 1 at minus and plus delta-t for numeric differentiation

}; // Variable_ZCe3

} // fmu
} // QSS

#endif