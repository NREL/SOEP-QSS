// FMU-Based QSS1 Explicit Zero-Crossing Variable
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

#ifndef QSS_fmu_Variable_ZCe1_hh_INCLUDED
#define QSS_fmu_Variable_ZCe1_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable_ZC.hh>

namespace QSS {
namespace fmu {

// FMU-Based QSS1 Zero-Crossing Variable
class Variable_ZCe1 final : public Variable_ZC
{

public: // Types

	using Super = Variable_ZC;

public: // Creation

	// Constructor
	Variable_ZCe1(
	 std::string const & name,
	 Real const rTol,
	 Real const aTol,
	 Real const zTol,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Super( 1, name, rTol, aTol, zTol, fmu_me, var, der )
	{}

public: // Predicate

	// Explicit Zero-Crossing Variable?
	bool
	is_ZCe() const
	{
		return true;
	}

public: // Property

	// Continuous Value at Time t
	Real
	x( Time const t ) const
	{
		return x_0_ + ( x_1_ * ( t - tX ) );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const ) const
	{
		return x_1_;
	}

	// Quantized Value at Time t
	Real
	q( Time const ) const
	{
		return x_0_;
	}

	// Zero-Crossing Bump Time for FMU Detection
	Time
	tZC_bump( Time const t ) const
	{
		if ( zTol > 0.0 ) {
			return t + ( x_1_ != 0.0 ? 2.0 * zTol / std::abs( x_1_ ) : options::dtZC ); // Hope FMU detects the crossing at 2x the zTol
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
		x_0_ = z_0();
		x_mag_ = std::abs( x_0_ );
		x_1_ = p_1();
		set_qTol();
		set_tE();
		set_tZ();
		( tE < tZ ) ? add_QSS_ZC( tE ) : add_ZC( tZ );
		if ( options::output::d ) std::cout << "! " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << '\n';
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
		set_qTol();
		set_tE();
#ifndef QSS_ZC_REQUANT_NO_CROSSING_CHECK
		crossing_detect( sign_old_, signum( x_0_ ), check_crossing_ );
#else
		set_tZ();
		( tE < tZ ) ? shift_QSS_ZC( tE ) : shift_ZC( tZ );
#endif
		if ( options::output::d ) std::cout << "! " << name() << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << '\n';
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
		x_0_ = ( t == tZ_last ? z_x() : z_0() ); // Force exact zero if at zero-crossing time
		x_mag_ = max( x_mag_, std::abs( x_t ), std::abs( x_0_ ) );
		x_1_ = p_1();
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
		x_0_ = ( t == tZ_last ? 0.0 : v ); // Force exact zero if at zero-crossing time
		x_mag_ = max( x_mag_, std::abs( x_t ), std::abs( x_0_ ) );
		x_1_ = d;
		set_qTol();
		set_tE();
		crossing_detect( sign_old_, signum( x_0_ ), check_crossing_ );
	}

	// Observer Advance: Stage d
	void
	advance_observer_d() const
	{
		std::cout << "  " << name() << '(' << tX << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ <<  '\n';
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
		Time dt( x_1_ != 0.0 ? qTol / std::abs( x_1_ ) : infinity );
		dt = std::min( std::max( dt, dt_min ), dt_max );
		tE = ( dt != infinity ? tQ + dt : infinity );
		tE_infinity_tQ();
	}

	// Set Zero-Crossing Time and Type on Active Segment
	void
	set_tZ()
	{
		if ( x_0_ == 0.0 ) { // Zero at segment start
			tZ = infinity;
		} else if ( zChatter_ && ( x_mag_ < zTol ) ) { // Chatter prevention
			tZ = infinity;
		} else { // Use root of continuous rep: Only robust for small active segments with continuous rep close to function
			int const sign_old( signum( x_0_ ) );
			int const sign_new( signum( x_1_ ) );
			Crossing const crossing_check( crossing_type( sign_old, sign_new ) );
			if ( has( crossing_check ) ) { // Crossing type is relevant
				if ( ( x_1_ != 0.0 ) && ( sign_old != sign_new ) ) { // Heading towards zero
					tZ = tX - ( x_0_ / x_1_ ); // Root of continuous rep
					if ( tZ > tX ) {
						crossing = crossing_check;
						if ( options::refine ) refine_root_ZCe( tX ); // Refine root: Expensive!
					} else { // Essentially flat
						tZ = infinity;
					}
				} else { // Heading away from zero
					tZ = infinity;
				}
			} else { // Crossing type not relevant
				tZ = infinity;
			}
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

	Real x_0_{ 0.0 }, x_1_{ 0.0 }; // Coefficients

}; // Variable_ZCe1

} // fmu
} // QSS

#endif
