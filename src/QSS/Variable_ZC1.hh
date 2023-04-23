// QSS1 Zero-Crossing Variable
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

#ifndef QSS_Variable_ZC1_hh_INCLUDED
#define QSS_Variable_ZC1_hh_INCLUDED

// QSS Headers
#include <QSS/Variable_ZC.hh>

namespace QSS {

// QSS1 Zero-Crossing Variable
class Variable_ZC1 final : public Variable_ZC
{

public: // Types

	using Super = Variable_ZC;

public: // Creation

	// Constructor
	Variable_ZC1(
	 FMU_ME * fmu_me,
	 std::string const & name,
	 Real const rTol_ = options::rTol,
	 Real const aTol_ = options::aTol,
	 Real const zTol_ = options::zTol,
	 Real const xIni_ = 0.0,
	 FMU_Variable const var = FMU_Variable()
	) :
	 Super( fmu_me, 1, name, rTol_, aTol_, zTol_, xIni_, var ),
	 x_0_( xIni_ )
	{
		set_qTol();
	}

public: // Property

	// Continuous Value at Time t
	Real
	x( Time const t ) const override
	{
		return x_0_ + ( x_1_ * ( t - tX ) );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const ) const override
	{
		return x_1_;
	}

	// Quantized Value at Time t
	Real
	q( Time const ) const override
	{
		return x_0_;
	}

	// Zero-Crossing Bump Time for FMU Detection
	Time
	tZC_bump( Time const t ) const override
	{
		if ( zTol > 0.0 ) {
			return t + ( x_1_ != 0.0 ? options::zMul * zTol / std::abs( x_1_ ) : options::dtZC ); // Hope FMU detects the crossing at this bump tolerance
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
		set_qTol();
		set_tE();
		set_tZ();
		( tE < tZ ) ? add_QSS_ZC( tE ) : add_ZC( tZ );
		fixup_tE();
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << std::endl;
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
		set_qTol();
		set_tE();
		crossing_detect();
		if ( options::output::d ) std::cout << "!  " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << std::endl;
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

	// QSS Advance: Stage Final
	void
	advance_QSS_F() override
	{
		set_qTol();
		set_tE();
		crossing_detect();
		if ( options::output::d ) std::cout << "!= " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << std::endl;
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
		set_qTol();
		set_tE();
		crossing_detect();
		if ( options::output::d ) std::cout << "*  " << name() << '(' << tX << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << std::endl;
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

	// Handler Advance: Stage Final
	void
	advance_handler_F() override
	{
		set_qTol();
		set_tE();
		crossing_detect();
		if ( options::output::d ) std::cout << "*= " << name() << '(' << tX << ')' << " = " << std::showpos << x_0_ << x_1_ << x_delta << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << std::endl;
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

	// Observer Advance: Stage Final
	void
	advance_observer_F() override
	{
		set_qTol();
		set_tE();
		crossing_detect();
	}

	// Observer Advance: Stage d
	void
	advance_observer_d() const override
	{
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
		assert( tQ == tX );
		assert( dt_min <= dt_max );
		Time dt( x_1_ != 0.0 ? qTol / std::abs( x_1_ ) : infinity );
		dt = std::min( std::max( dt_infinity( dt ), dt_min ), dt_max );
		tE = ( dt != infinity ? tQ + dt : infinity );
		x_mag_update( x_0_ );
	}

	// Set Zero-Crossing Time and Type on Active Segment
	void
	set_tZ()
	{
		// Find root of continuous trajectory: Only robust for small active segments with continuous trajectory close to function
		Time const dt( zc_root_linear( x_1_, x_0_, zTol, x_mag_ ) );
		assert( dt > 0.0 );
		if ( dt != infinity ) { // Root exists
			tZ = tX + dt;
			if ( tZ <= tZ_last ) { // Crossing already processed
				tZ = infinity;
			} else {
				Crossing const crossing_check( crossing_type( x_1_ ) );
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

	// Coefficient 1 from FMU at Time tQ
	Real
	n_1() const
	{
		return Z_1();
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }; // Trajectory coefficients

}; // Variable_ZC1

} // QSS

#endif
