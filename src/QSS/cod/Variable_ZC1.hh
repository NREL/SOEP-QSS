// QSS1 Zero-Crossing Variable
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

#ifndef QSS_cod_Variable_ZC1_hh_INCLUDED
#define QSS_cod_Variable_ZC1_hh_INCLUDED

// QSS Headers
#include <QSS/cod/Variable_ZC.hh>

namespace QSS {
namespace cod {

// QSS1 Zero-Crossing Variable
template< template< typename > class F >
class Variable_ZC1 final : public Variable_ZC< F >
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
	using Super::detected_crossing_;
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
	using Super::dt_infinity;
	using Super::has;
	using Super::refine_root_ZC;
	using Super::shift_QSS_ZC;
	using Super::shift_ZC;
	using Super::x_mag_update;
	using Super::x_mag_zero;

protected: // Types

	using Super::crossing_type;

private: // Types

	using Super::f_;

public: // Creation

	// Constructor
	explicit
	Variable_ZC1(
	 std::string const & name,
	 Real const rTol = 1.0e-4,
	 Real const aTol = 1.0e-6,
	 Real const zTol = 1.0e-6
	) :
	 Super( 1, name, rTol, aTol, zTol )
	{}

public: // Property

	// Continuous Value at Time t
	Real
	x( Time const t ) const override final
	{
		return x_0_ + ( x_1_ * ( t - tX ) );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const ) const override final
	{
		return x_1_;
	}

	// Quantized Value at Time t
	Real
	q( Time const ) const override final
	{
		return x_0_;
	}

public: // Methods

	// Initialization
	void
	init() override final
	{
		// Initialize specs
		x_0_ = f_.x( tQ );
		x_mag_ = std::abs( x_0_ );
		x_1_ = f_.x1( tQ );
		set_qTol();
		set_tE();
		set_tZ();
		( tE < tZ ) ? add_QSS_ZC( tE ) : add_ZC( tZ );
		fixup_tE();
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << std::endl;
	}

	// QSS Advance
	void
	advance_QSS() override final
	{
		advance_pre( tE );
		x_0_ = f_.x( tX = tQ = tE );
		x_1_ = f_.x1( tQ );
		set_qTol();
		set_tE();
		crossing_detect();
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << std::endl;
	}

	// Zero-Crossing Advance
	void
	advance_ZC() override final
	{
		for ( typename If::Clause * clause : if_clauses ) clause->activity( tZ );
		for ( typename When::Clause * clause : when_clauses ) clause->activity( tZ );
		crossing_last = crossing;
		x_mag_zero();
		set_tZ( tZ_last = tZ ); // Next zero-crossing: Might be in active segment
		( tE < tZ ) ? shift_QSS_ZC( tE ) : shift_ZC( tZ );
		fixup_tE();
		if ( options::output::d ) std::cout << "Z  " << name() << '(' << tZ_last << ')' << "   tZ=" << tZ << std::endl;
	}

	// Observer Advance
	void
	advance_observer( Time const t ) override final
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		advance_pre( t );
		tX = tQ = t;
		x_0_ = ( t == tZ_last ? 0.0 : f_.x( t ) ); // Force exact zero if at zero-crossing time
		x_1_ = f_.x1( t );
		set_qTol();
		set_tE();
		crossing_detect();
		if ( options::output::d ) std::cout << " ^ " << name() << '(' << tX << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << std::endl;
	}

	// Observer Advance: Parallel
	void
	advance_observer_parallel( Time const t ) override final
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		advance_pre( t );
		tX = tQ = t;
		x_0_ = ( t == tZ_last ? 0.0 : f_.x( t ) ); // Force exact zero if at zero-crossing time
		x_1_ = f_.x1( t );
		set_qTol();
		set_tE();
	}

	// Observer Advance: Serial
	void
	advance_observer_serial() override final
	{
		crossing_detect();
	}

	// Observer Advance: Serial + Diagnostics
	void
	advance_observer_serial_d() override final
	{
		assert( options::output::d );
		crossing_detect();
		std::cout << " ^ " << name() << '(' << tX << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << std::endl;
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
		Real const x_t( this->Variable_ZC1::x( t ) );

		// Unpredicted zero crossing check setup
		check_crossing_ = past_tZ;
		if ( past_tZ || ( x_t != 0.0 ) ) sign_old_ = signum( x_t );

		// Anti-chatter trajectory magnitude updates for [tX,t] span
		if ( zChatter_ && past_tZ ) { // Anti-chatter is active
			x_mag_update( x_t ); // Trajectory can have a discontinuity at updates
		}
	}

	// Set End Time
	void
	set_tE()
	{
		assert( tX <= tQ );
		assert( dt_min <= dt_max );
		x_mag_update( x_0_ );
		Time dt( x_1_ != 0.0 ? qTol / std::abs( x_1_ ) : infinity );
		dt = std::min( std::max( dt_infinity( dt ), dt_min ), dt_max );
		tE = ( dt != infinity ? tQ + dt : infinity );
	}

	// Set Zero-Crossing Time and Type on Active Segment
	void
	set_tZ()
	{
		// Find root of continuous rep: Only robust for small active segments with continuous rep close to function
		Time const dt( zc_root_linear( x_1_, x_0_, zTol, x_mag_ ) );
		assert( dt > 0.0 );
		if ( dt != infinity ) { // Root exists
			tZ = tX + dt;
			Crossing const crossing_check( crossing_type( x_1_ ) );
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

	// Set Zero-Crossing Time and Type on (tB,tE]
	void
	set_tZ( Time const tB )
	{
		assert( tB >= tX );
		set_tZ();
		tZ = ( tZ > tB ? tZ : infinity );
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
		fixup_tE();
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }; // Coefficients

}; // Variable_ZC1

} // cod
} // QSS

#endif
