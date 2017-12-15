// FMU-Based QSS1 Zero-Crossing Variable
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017 Objexx Engineerinc, Inc. All rights reserved.
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

#ifndef QSS_fmu_Variable_ZC1_hh_INCLUDED
#define QSS_fmu_Variable_ZC1_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable_ZC.hh>

namespace QSS {
namespace fmu {

// FMU-Based QSS1 Zero-Crossing Variable
class Variable_ZC1 final : public Variable_ZC
{

public: // Types

	using Super = Variable_ZC;

public: // Creation

	// Constructor
	explicit
	Variable_ZC1(
	 std::string const & name,
	 Value const rTol = 1.0e-4,
	 Value const aTol = 1.0e-6,
	 Value const zTol = 0.0,
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
		return 1;
	}

	// Continuous Value at Time t
	Value
	x( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		return x_0_ + ( x_1_ * ( t - tX ) );
	}

	// Continuous First Derivative at Time t
	Value
	x1( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		(void)t; // Suppress unused parameter warning
		return x_1_;
	}

	// Quantized Value at Time t
	Value
	q( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		(void)t; // Suppress unused parameter warning
		return q_0_;
	}

public: // Methods

	// Initialization
	void
	init()
	{
		init_0();
		init_1();
	}

	// Initialization: Stage 0
	void
	init_0()
	{
		if ( self_observer ) {
			std::cerr << "Error: Zero-crossing variable is self-observer: " << name << std::endl;
			std::exit( EXIT_FAILURE );
		}
		if ( ! observers_.empty() ) {
			std::cerr << "Error: Zero-crossing variable has observers: " << name << std::endl;
			std::exit( EXIT_FAILURE );
		}
		fmu_set_observees_q( tQ );
		x_0_ = q_0_ = fmu_get_value();
		x_mag_ = std::abs( x_0_ );
		set_qTol();
	}

	// Initialization: Stage 1
	void
	init_1()
	{
		x_1_ = fmu_get_deriv();
		set_tE();
		set_tZ();
		tE < tZ ? add_QSS( tE ) : add_ZC( tZ );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << "   tZ=" << tZ << '\n';
	}

	// Set Current Tolerance
	void
	set_qTol()
	{
		qTol = std::max( rTol * std::abs( q_0_ ), aTol );
		assert( qTol > 0.0 );
	}

	// QSS Advance
	void
	advance_QSS()
	{
		fmu_set_observees_q( tX = tQ = tE );
		Value const x_tE( zChatter_ ? x( tE ) : 0.0 );
#ifndef QSS_ZC_REQUANT_NO_CROSSING_CHECK
		sign_old_ = ( tE == tZ_last ? 0 : signum( zChatter_ ? x_tE : x( tE ) ) ); // Treat as if exactly zero if tE is last zero-crossing event time
#endif
		x_0_ = q_0_ = fmu_get_value();
		if ( zChatter_ ) x_mag_ = max( x_mag_, std::abs( x_tE ), std::abs( x_0_ ) );
		set_qTol();
		x_1_ = fmu_get_deriv();
		set_tE();
#ifndef QSS_ZC_REQUANT_NO_CROSSING_CHECK
		crossing_detect( sign_old_, signum( x_0_ ) );
#else
		set_tZ();
		tE < tZ ? shift_QSS( tE ) : shift_ZC( tZ );
#endif
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << "   tZ=" << tZ << '\n';
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0()
	{
		fmu_set_observees_q( tX = tQ = tE );
		Value const x_tE( zChatter_ ? x( tE ) : 0.0 );
		sign_old_ = ( tE == tZ_last ? 0 : signum( zChatter_ ? x_tE : x( tE ) ) ); // Treat as if exactly zero if tE is last zero-crossing event time
		x_0_ = q_0_ = fmu_get_value();
		if ( zChatter_ ) x_mag_ = max( x_mag_, std::abs( x_tE ), std::abs( x_0_ ) );
		set_qTol();
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1()
	{
		x_1_ = fmu_get_deriv();
		set_tE();
		crossing_detect( sign_old_, signum( x_0_ ) );
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << "   tZ=" << tZ << '\n';
	}

	// Observer Advance: Stage 1
	void
	advance_observer_1( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		tX = tQ = t;
		Value const x_t( zChatter_ ? x( t ) : 0.0 );
		sign_old_ = ( t == tZ_last ? 0 : signum( zChatter_ ? x_t : x( t ) ) ); // Treat as if exactly zero if t is last zero-crossing event time
		x_0_ = q_0_ = fmu_get_value();
		if ( zChatter_ ) x_mag_ = max( x_mag_, std::abs( x_t ), std::abs( x_0_ ) );
		set_qTol();
		x_1_ = fmu_get_deriv();
		set_tE();
		crossing_detect( sign_old_, signum( x_0_ ) );
	}

	// Observer Advance: Stage d
	void
	advance_observer_d() const
	{
		std::cout << "  " << name << '(' << tX << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << "   tZ=" << tZ <<  '\n';
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
		tE < tZ ? shift_QSS( tE ) : shift_ZC( tZ );
		advance_observees(); // set_tZ refinement sets observees so we do this after
	}

private: // Methods

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
					assert( tX < tZ );
					crossing = crossing_check;
					if ( options::refine ) { // Refine root: Expensive!
						Time t( tZ ), t_p( tZ );
						fmu_set_observees_q( tZ );
						Value const vZ( fmu_get_value() );
						Value v( vZ ), v_p( vZ );
						Value m( 1.0 ); // Multiplier
						std::size_t i( 0 );
						std::size_t const n( 10u ); // Max iterations
						while ( ( ++i <= n ) && ( ( std::abs( v ) > aTol ) || ( std::abs( v ) < std::abs( v_p ) ) ) ) {
							Value const d( fmu_get_deriv() );
							if ( d == 0.0 ) break;
							//if ( ( signum( d ) != sign_old ) && ( tE < std::min( t_p, t ) ) ) break; // Zero-crossing seems to be >tE so don't refine further
							t -= m * ( v / d );
							fmu_set_observees_q( t );
							v = fmu_get_value();
							if ( std::abs( v ) >= std::abs( v_p ) ) m *= 0.5; // Non-converging step: Reduce step size
							t_p = t;
							v_p = v;
						}
						if ( ( t >= tX ) && ( std::abs( v ) < std::abs( vZ ) ) ) tZ = t;
						if ( ( i == n ) && ( options::output::d ) ) std::cout << "  " << name << '(' << t << ')' << " tZ may not have converged" <<  '\n';
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
	crossing_detect( int const sign_old, int const sign_new )
	{
		if ( zChatter_ && ( x_mag_ < zTol ) ) { // Chatter prevention
			tZ = infinity;
			shift_QSS( tE );
		} else if ( sign_old != sign_new ) { // Zero-crossing occurs at t
			Crossing const crossing_check( crossing_type( sign_old, sign_new ) );
			if ( has( crossing_check ) ) { // Crossing type is relevant
				crossing = crossing_check;
				shift_ZC( tZ = tX );
			} else {
				set_tZ();
				tE < tZ ? shift_QSS( tE ) : shift_ZC( tZ );
			}
		} else {
			set_tZ();
			tE < tZ ? shift_QSS( tE ) : shift_ZC( tZ );
		}
	}

private: // Data

	Value x_0_{ 0.0 }, x_1_{ 0.0 }; // Continuous rep coefficients
	Value q_0_{ 0.0 }; // Quantized rep coefficients

};

} // fmu
} // QSS

#endif
