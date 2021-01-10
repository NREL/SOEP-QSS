// QSS2 Zero-Crossing Variable
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

#ifndef QSS_cod_Variable_ZC2_hh_INCLUDED
#define QSS_cod_Variable_ZC2_hh_INCLUDED

// QSS Headers
#include <QSS/cod/Variable_ZC.hh>

namespace QSS {
namespace cod {

// QSS2 Zero-Crossing Variable
template< template< typename > class F >
class Variable_ZC2 final : public Variable_ZC< F >
{

public: // Types

	using Super = Variable_ZC< F >;

	using Real = Variable::Real;
	using Time = Variable::Time;

	using Crossing = Variable::Crossing;
	using typename Super::If;
	using typename Super::When;

	using Super::check_crossing_;
	using Super::crossing;
	using Super::crossing_last;
	using Super::if_clauses;
	using Super::name;
	using Super::rTol;
	using Super::aTol;
	using Super::qTol;
	using Super::xIni;
	using Super::tQ;
	using Super::tX;
	using Super::tE;
	using Super::tZ;
	using Super::tZ_last;
	using Super::dt_min;
	using Super::dt_max;
	using Super::sign_old_;
	using Super::when_clauses;
	using Super::x_mag_;
	using Super::zChatter_;
	using Super::zTol;

	using Super::add_QSS_ZC;
	using Super::add_ZC;
	using Super::has;
	using Super::refine_root_ZC;
	using Super::shift_QSS_ZC;
	using Super::shift_ZC;

protected: // Types

	using Super::crossing_type;

private: // Types

	using Super::f_;

public: // Creation

	// Constructor
	explicit
	Variable_ZC2(
	 std::string const & name,
	 Real const rTol = 1.0e-4,
	 Real const aTol = 1.0e-6,
	 Real const zTol = 0.0
	) :
	 Super( 2, name, rTol, aTol, zTol )
	{}

public: // Property

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
	x2( Time const t ) const
	{
		(void)t; // Suppress unused warning
		return two * x_2_;
	}

	// Quantized Value at Time t
	Real
	q( Time const t ) const
	{
		return x_0_ + ( x_1_ * ( t - tQ ) );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const t ) const
	{
		(void)t; // Suppress unused warning
		return x_1_;
	}

public: // Methods

	// Initialization
	void
	init()
	{
		// Initialize trajectory specs
		x_0_ = f_.x( tQ );
		x_mag_ = std::abs( x_0_ );
		x_1_ = f_.x1( tQ );
		x_2_ = one_half * f_.x2( tQ );
		set_qTol();
		set_tE();
		set_tZ();
		( tE < tZ ) ? add_QSS_ZC( tE ) : add_ZC( tZ );
		if ( options::output::d ) std::cout << "! " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << '\n';
	}

	// QSS Advance
	void
	advance_QSS() override final
	{
		Real const x_tE( zChatter_ ? x( tE ) : Real( 0.0 ) );
#ifndef QSS_ZC_REQUANT_NO_CROSSING_CHECK
		bool const check_crossing( ( tE > tZ_last ) || ( x_mag_ != 0.0 ) );
		int const sign_old( check_crossing ? signum( zChatter_ ? x_tE : x( tE ) ) : 0 );
#endif
		x_0_ = f_.x( tX = tQ = tE );
		x_mag_ = max( x_mag_, std::abs( x_tE ), std::abs( x_0_ ) );
		x_1_ = f_.x1( tQ );
		x_2_ = one_half * f_.x2( tQ );
		set_qTol();
		set_tE();
#ifndef QSS_ZC_REQUANT_NO_CROSSING_CHECK
		crossing_detect( sign_old, signum( x_0_ ), check_crossing );
#else
		set_tZ();
		( tE < tZ ) ? shift_QSS_ZC( tE ) : shift_ZC( tZ );
#endif
		if ( options::output::d ) std::cout << "! " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << '\n';
	}

	// Zero-Crossing Advance
	void
	advance_ZC() override final
	{
		for ( typename If::Clause * clause : if_clauses ) clause->activity( tZ );
		for ( typename When::Clause * clause : when_clauses ) clause->activity( tZ );
		if ( options::output::d ) std::cout << "Z " << name() << '(' << tZ << ')' << '\n';
		crossing_last = crossing;
		x_mag_ = 0.0;
		set_tZ( tZ_last = tZ ); // Next zero-crossing: Might be in active segment
		( tE < tZ ) ? shift_QSS_ZC( tE ) : shift_ZC( tZ );
	}

	// Observer Advance
	void
	advance_observer( Time const t ) override final
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		tX = tQ = t;
		Real const x_t( zChatter_ ? x( t ) : Real( 0.0 ) );
		bool const check_crossing( ( t > tZ_last ) || ( x_mag_ != 0.0 ) );
		int const sign_old( check_crossing ? signum( zChatter_ ? x_t : x( t ) ) : 0 );
		x_0_ = ( t == tZ_last ? 0.0 : f_.x( t ) ); // Force exact zero if at zero-crossing time
		x_mag_ = max( x_mag_, std::abs( x_t ), std::abs( x_0_ ) );
		x_1_ = f_.x1( t );
		x_2_ = one_half * f_.x2( t );
		set_qTol();
		set_tE();
		crossing_detect( sign_old, signum( x_0_ ), check_crossing );
		if ( options::output::d ) std::cout << "  " << name() << '(' << tX << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ <<  '\n';
	}

	// Observer Advance: Parallel
	void
	advance_observer_parallel( Time const t ) override final
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		tX = tQ = t;
		Real const x_t( zChatter_ ? x( t ) : Real( 0.0 ) );
		check_crossing_ = ( t > tZ_last ) || ( x_mag_ != 0.0 );
		sign_old_ = ( check_crossing_ ? signum( zChatter_ ? x_t : x( t ) ) : 0 );
		x_0_ = ( t == tZ_last ? 0.0 : f_.x( t ) ); // Force exact zero if at zero-crossing time
		x_mag_ = max( x_mag_, std::abs( x_t ), std::abs( x_0_ ) );
		x_1_ = f_.x1( t );
		x_2_ = one_half * f_.x2( t );
		set_qTol();
		set_tE();
	}

	// Observer Advance: Serial
	void
	advance_observer_serial() override final
	{
		crossing_detect( sign_old_, signum( x_0_ ), check_crossing_ );
	}

	// Observer Advance: Serial + Diagnostics
	void
	advance_observer_serial_d() override final
	{
		assert( options::output::d );
		crossing_detect( sign_old_, signum( x_0_ ), check_crossing_ );
		std::cout << "  " << name() << '(' << tX << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ <<  '\n';
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
		if ( zChatter_ && ( x_mag_ < zTol ) ) { // Chatter prevention
			tZ = infinity;
		} else { // Use root of continuous rep: Only robust for small active segments with continuous rep close to function
			Time const dt( min_positive_root_quadratic( x_2_, x_1_, x_0_ ) );
			assert( dt > 0.0 );
			if ( dt != infinity ) { // Root exists
				tZ = tX + dt;
				Crossing const crossing_check( x_0_ == 0.0 ? ( tZ == tX ? Crossing::Flat : crossing_type( -x_1_ ) ) :
				 crossing_type( x_0_ > 0.0 ? std::min( x1( tZ ), Real( 0.0 ) ) : std::max( x1( tZ ), Real( 0.0 ) ) ) );
				if ( has( crossing_check ) ) { // Crossing type is relevant
					crossing = crossing_check;
					if ( options::refine ) refine_root_ZC( tX ); // Refine root: Expensive!
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
			Time const dt( min_positive_root_quadratic( x_2_, x_1, x_0 ) ); // Positive root using trajectory shifted to tB
			assert( dt > 0.0 );
			if ( dt != infinity ) { // Root exists
				tZ = tB + dt;
				Crossing const crossing_check( x_0 == 0.0 ? ( tZ == tB ? Crossing::Flat : crossing_type( -x_1 ) ) :
				 crossing_type( x_0 > 0.0 ? std::min( x1( tZ ), Real( 0.0 ) ) : std::max( x1( tZ ), Real( 0.0 ) ) ) );
				if ( has( crossing_check ) ) { // Crossing type is relevant
					crossing = crossing_check;
					if ( options::refine ) refine_root_ZC( tB ); // Refine root: Expensive!
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

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }; // Continuous rep coefficients

}; // Variable_ZC2

} // cod
} // QSS

#endif
