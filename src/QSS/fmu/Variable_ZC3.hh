// FMU-Based QSS3 Zero-Crossing Variable
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2018 Objexx Engineerinc, Inc. All rights reserved.
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

#ifndef QSS_fmu_Variable_ZC3_hh_INCLUDED
#define QSS_fmu_Variable_ZC3_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable_ZC.hh>

namespace QSS {
namespace fmu {

// FMU-Based QSS3 Zero-Crossing Variable
class Variable_ZC3 final : public Variable_ZC
{

public: // Types

	using Super = Variable_ZC;

public: // Creation

	// Constructor
	explicit
	Variable_ZC3(
	 std::string const & name,
	 Real const rTol = 1.0e-4,
	 Real const aTol = 1.0e-6,
	 Real const zTol = 0.0,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Super( name, rTol, aTol, zTol, var, der )
	{}

public: // Properties

	// Order of Method
	int
	order() const
	{
		return 3;
	}

	// Continuous Value at Time t
	Real
	x( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		return x_0_ + ( ( x_1_ + ( ( x_2_ + ( x_3_ * tDel ) ) * tDel ) ) * tDel );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		return x_1_ + ( ( ( two * x_2_ ) + ( three * x_3_ * tDel ) ) * tDel );
	}

	// Continuous Second Derivative at Time t
	Real
	x2( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		return ( two * x_2_ ) + ( six * x_3_ * ( t - tX ) );
	}

	// Continuous Third Derivative at Time t
	Real
	x3( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		(void)t; // Suppress unused parameter warning
		return six * x_3_;
	}

	// Quantized Value at Time t
	Real
	q( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		Time const tDel( t - tQ );
		return x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		return x_1_ + ( two * x_2_ * ( t - tQ ) );
	}

	// Quantized Second Derivative at Time t
	Real
	q2( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		(void)t; // Suppress unused parameter warning
		return two * x_2_;
	}

public: // Methods

	// Initialization
	void
	init()
	{
		init_0();
		init_1();
		init_2();
		init_3();
	}

	// Initialization: Stage 0
	void
	init_0()
	{
		// Check no observers
		if ( ! observers_.empty() ) {
			std::cerr << "Error: Zero-crossing variable has observers: " << name << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Shrink observees
		init_observees();

		// Initialize trajectory specs
		fmu_set_observees_x( tQ );
		x_0_ = fmu_get_real();
		x_mag_ = std::abs( x_0_ );
		set_qTol();
	}

	// Initialization: Stage 1
	void
	init_1()
	{
		x_1_ = fmu_get_poly_1();
	}

	// Initialization: Stage 2
	void
	init_2()
	{
		x_2_ = fmu_get_poly_2();
	}

	// Initialization: Stage 3
	void
	init_3()
	{
		x_3_ = fmu_get_poly_3();
		set_tE();
		set_tZ();
		tE < tZ ? add_QSS_ZC( tE ) : add_ZC( tZ );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << '\n';
	}

	// Set Current Tolerance
	void
	set_qTol()
	{
		qTol = std::max( rTol * std::abs( x_0_ ), aTol );
		assert( qTol > 0.0 );
	}

	// QSS Advance
	void
	advance_QSS()
	{
		Real const x_tE( zChatter_ ? x( tE ) : Real( 0.0 ) );
#ifndef QSS_ZC_REQUANT_NO_CROSSING_CHECK
		check_crossing_ = ( tE > tZ_last ) || ( x_mag_ != 0.0 );
		sign_old_ = ( check_crossing_ ? signum( zChatter_ ? x_tE : x( tE ) ) : 0 );
#endif
		fmu_set_observees_x( tX = tQ = tE );
		x_0_ = fmu_get_real();
		x_mag_ = max( x_mag_, std::abs( x_tE ), std::abs( x_0_ ) );
		set_qTol();
		x_1_ = fmu_get_poly_1();
		x_2_ = fmu_get_poly_2();
		x_3_ = fmu_get_poly_3();
		set_tE();
#ifndef QSS_ZC_REQUANT_NO_CROSSING_CHECK
		crossing_detect( sign_old_, signum( x_0_ ), check_crossing_ );
#else
		set_tZ();
		tE < tZ ? shift_QSS_ZC( tE ) : shift_ZC( tZ );
#endif
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << '\n';
	}

	// Observer Advance
	void
	advance_observer( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Real const x_t( zChatter_ ? x( t ) : Real( 0.0 ) );
		check_crossing_ = ( t > tZ_last ) || ( x_mag_ != 0.0 );
		sign_old_ = ( check_crossing_ ? signum( zChatter_ ? x_t : x( t ) ) : 0 );
		tX = tQ = t;
		x_0_ = fmu_get_real();
		x_mag_ = max( x_mag_, std::abs( x_t ), std::abs( x_0_ ) );
		set_qTol();
		x_1_ = fmu_get_poly_1();
		x_2_ = fmu_get_poly_2();
		x_3_ = fmu_get_poly_3();
		set_tE();
		crossing_detect( sign_old_, signum( x_0_ ), check_crossing_ );
		if ( options::output::d ) std::cout << "  " << name << '(' << tX << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << x_2_ << "*t^2" << x_3_ << "*t^3" << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ <<  '\n';
	}

	// Observer Advance: Simultaneous
	void
	advance_observer_s( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		fmu_set_observees_x( t );
		advance_observer( t );
	}

	// Zero-Crossing Advance
	void
	advance_ZC()
	{
		for ( typename If::Clause * clause : if_clauses ) clause->activity( tZ );
		for ( typename When::Clause * clause : when_clauses ) clause->activity( tZ );
		if ( options::output::d ) std::cout << "Z " << name << '(' << tZ << ')' << '\n';
		crossing_last = crossing;
		x_mag_ = 0.0;
		set_tZ( tZ_last = tZ ); // Next zero-crossing: Might be in active segment
		tE < tZ ? shift_QSS_ZC( tE ) : shift_ZC( tZ );
		bump_observees(); // set_tZ refinement sets observees so we do this after
	}

private: // Methods

	// Continuous First Derivative at Time t
	Real
	x1x( Time const t ) const
	{
		Time const tDel( t - tX );
		return x_1_ + ( ( ( two * x_2_ ) + ( three * x_3_ * tDel ) ) * tDel ); // Allows t beyond tE for set_tZ use
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
				 crossing_type( x_0_ > 0.0 ? std::min( x1x( tZ ), Real( 0.0 ) ) : std::max( x1x( tZ ), Real( 0.0 ) ) ) );
				if ( has( crossing_check ) ) { // Crossing type is relevant
					crossing = crossing_check;
					if ( options::refine ) { // Refine root: Expensive!
						Time t( tZ );
						//Time t_p( tZ );
						Time const t_fmu( fmu::get_time() );
						fmu::set_time( tZ ); // Don't seem to need this
						fmu_set_observees_x( tZ );
						Real const vZ( fmu_get_real() );
						Real v( vZ ), v_p( vZ );
						Real m( 1.0 ); // Multiplier
						std::size_t i( 0 );
						std::size_t const n( 10u ); // Max iterations
						//int const sign_0( signum( x_0_ ) );
						while ( ( ++i <= n ) && ( ( std::abs( v ) > aTol ) || ( std::abs( v ) < std::abs( v_p ) ) ) ) {
							Real const d( fmu_get_deriv() );
							if ( d == 0.0 ) break;
							//if ( ( signum( d ) != sign_0 ) && ( tE < std::min( t_p, t ) ) ) break; // Zero-crossing seems to be >tE so don't refine further
							t -= m * ( v / d );
							fmu::set_time( t ); // Don't seem to need this
							fmu_set_observees_x( t );
							v = fmu_get_real();
							if ( std::abs( v ) >= std::abs( v_p ) ) m *= 0.5; // Non-converging step: Reduce step size
							//t_p = t;
							v_p = v;
						}
						if ( ( t >= tX ) && ( std::abs( v ) < std::abs( vZ ) ) ) tZ = t;
						if ( ( i == n ) && ( options::output::d ) ) std::cout << "  " << name << '(' << t << ')' << " tZ may not have converged" <<  '\n';
						fmu::set_time( t_fmu ); // Don't seem to need this
					}
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
				 crossing_type( x_0 > 0.0 ? std::min( x1x( tZ ), Real( 0.0 ) ) : std::max( x1x( tZ ), Real( 0.0 ) ) ) );
				if ( has( crossing_check ) ) { // Crossing type is relevant
					crossing = crossing_check;
					if ( options::refine ) { // Refine root: Expensive!
						Time t( tZ );
						//Time t_p( tZ );
						Time const t_fmu( fmu::get_time() );
						fmu::set_time( tZ ); // Don't seem to need this
						fmu_set_observees_x( tZ );
						Real const vZ( fmu_get_real() );
						Real v( vZ ), v_p( vZ );
						Real m( 1.0 ); // Multiplier
						std::size_t i( 0 );
						std::size_t const n( 10u ); // Max iterations
						//int const sign_0( signum( x_0 ) );
						while ( ( ++i <= n ) && ( ( std::abs( v ) > aTol ) || ( std::abs( v ) < std::abs( v_p ) ) ) ) {
							Real const d( fmu_get_deriv() );
							if ( d == 0.0 ) break;
							//if ( ( signum( d ) != sign_0 ) && ( tE < std::min( t_p, t ) ) ) break; // Zero-crossing seems to be >tE so don't refine further
							t -= m * ( v / d );
							fmu::set_time( t ); // Don't seem to need this
							fmu_set_observees_x( t );
							v = fmu_get_real();
							if ( std::abs( v ) >= std::abs( v_p ) ) m *= 0.5; // Non-converging step: Reduce step size
							//t_p = t;
							v_p = v;
						}
						if ( ( t >= tB ) && ( std::abs( v ) < std::abs( vZ ) ) ) tZ = t;
						if ( ( i == n ) && ( options::output::d ) ) std::cout << "  " << name << '(' << t << ')' << " tZ may not have converged" <<  '\n';
						fmu::set_time( t_fmu ); // Don't seem to need this
					}
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
			tE < tZ ? shift_QSS_ZC( tE ) : shift_ZC( tZ );
		} else { // Check zero-crossing
			Crossing const crossing_check( crossing_type( sign_old, sign_new ) );
			if ( has( crossing_check ) ) { // Crossing type is relevant
				crossing = crossing_check;
				shift_ZC( tZ = tX );
			} else {
				set_tZ();
				tE < tZ ? shift_QSS_ZC( tE ) : shift_ZC( tZ );
			}
		}
	}

private: // Data

	Real x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }, x_3_{ 0.0 }; // Continuous rep coefficients

};

} // fmu
} // QSS

#endif
