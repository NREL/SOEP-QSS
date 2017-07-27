// FMU-Based QSS2 Zero-Crossing Variable
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
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef QSS_fmu_Variable_ZC2_hh_INCLUDED
#define QSS_fmu_Variable_ZC2_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable_ZC.hh>

namespace QSS {
namespace fmu {

// FMU-Based QSS2 Zero-Crossing Variable
class Variable_ZC2 final : public Variable_ZC
{

public: // Types

	using Super = Variable_ZC;

public: // Creation

	// Constructor
	explicit
	Variable_ZC2(
	 std::string const & name,
	 Value const rTol = 1.0e-4,
	 Value const aTol = 1.0e-6,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Super( name, rTol, aTol, var, der )
	{}

public: // Properties

	// Order of Method
	int
	order() const
	{
		return 2;
	}

	// Continuous Value at Time t
	Value
	x( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		return x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// Continuous First Derivative at Time t
	Value
	x1( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		return x_1_ + ( two * x_2_ * ( t - tX ) );
	}

	// Continuous Second Derivative at Time t
	Value
	x2( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		(void)t; // Suppress unused parameter warning
		return two * x_2_;
	}

	// Quantized Value at Time t
	Value
	q( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		return q_0_ + ( q_1_ * ( t - tQ ) );
	}

	// Quantized First Derivative at Time t
	Value
	q1( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		(void)t; // Suppress unused parameter warning
		return q_1_;
	}

public: // Methods

	// Initialization
	void
	init()
	{
		init_0();
		init_1();
		init_2();
	}

	// Initialization: Stage 0
	void
	init_0()
	{
		if ( self_observer ) {
			std::cerr << "Zero-crossing variable is self-observer: " << name << std::endl;
			std::exit( EXIT_FAILURE );
		}
		init_observers();
		fmu_set_observees_q( tQ );
		x_0_ = q_0_ = fmu_get_value();
		set_qTol();
	}

	// Initialization: Stage 1
	void
	init_1()
	{
		x_1_ = q_1_ = fmu_get_deriv();
	}

	// Initialization: Stage 2
	void
	init_2()
	{
		fmu_set_observees_q( tD = tQ + options::dtND );
		x_2_ = options::one_half_over_dtND * ( fmu_get_deriv() - x_1_ ); // Forward Euler //API one_half * fmu_get_deriv2() when 2nd derivative is available
		set_tE();
		set_tZ();
		event( tE < tZ ? events.add_QSS( tE, this ) : events.add_ZC( tZ, this ) );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << "   tZ=" << tZ << '\n';
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
		x_0_ = q_0_ = fmu_get_value();
		set_qTol();
		x_1_ = q_1_ = fmu_get_deriv();
		fmu::set_time( tD = tQ + options::dtND );
		fmu_set_observees_q( tD );
		x_2_ = options::one_half_over_dtND * ( fmu_get_deriv() - x_1_ ); // Forward Euler //API one_half * fmu_get_deriv2() when 2nd derivative is available
		set_tE();
		set_tZ();
		event( tE < tZ ? events.shift_QSS( tE, event() ) : events.shift_ZC( tZ, event() ) );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << "   tZ=" << tZ << '\n';
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0()
	{
		fmu_set_observees_q( tX = tQ = tE );
		x_0_ = q_0_ = fmu_get_value();
		set_qTol();
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1()
	{
		x_1_ = q_1_ = fmu_get_deriv();
	}

	// QSS Advance: Stage 2
	void
	advance_QSS_2()
	{
		fmu_set_observees_q( tD = tQ + options::dtND );
		x_2_ = options::one_half_over_dtND * ( fmu_get_deriv() - x_1_ ); // Forward Euler //API one_half * fmu_get_deriv2() when 2nd derivative is available
		set_tE();
		set_tZ();
		event( tE < tZ ? events.shift_QSS( tE, event() ) : events.shift_ZC( tZ, event() ) );
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << "   tZ=" << tZ << '\n';
	}

	// Observer Advance: Stage 1
	void
	advance_observer_1( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		sign_old_ = ( t == tZ_prev ? 0 : signum( x( tX = tQ = t ) ) ); // Treat as if exactly zero if t is previous zero-crossing event time
		x_0_ = q_0_ = fmu_get_value();
		set_qTol();
		x_1_ = q_1_ = fmu_get_deriv();
	}

	// Observer Advance: Stage 2
	void
	advance_observer_2()
	{
		x_2_ = options::one_half_over_dtND * ( fmu_get_deriv() - x_1_ ); // Forward Euler //API one_half * fmu_get_deriv2() when 2nd derivative is available
		set_tE();
		crossing_detect( sign_old_, signum( x_0_ ) );
	}

	// Observer Advance: Stage d
	void
	advance_observer_d()
	{
		std::cout << "  " << name << '(' << tX << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << "   tZ=" << tZ <<  '\n';
	}

	// Zero-Crossing Advance
	void
	advance_ZC()
	{
		shift_handlers();
		if ( options::output::d ) std::cout << "Z " << name << '(' << tZ << ')' << '\n';
		set_tZ( tZ_prev = tZ ); // Next zero-crossing: Might be in active segment
		event( tE < tZ ? events.shift_QSS( tE, event() ) : events.shift_ZC( tZ, event() ) );
	}

private: // Methods

	// Continuous First Derivative at Time t
	Value
	x1x( Time const t ) const
	{
		return x_1_ + ( two * x_2_ * ( t - tX ) ); // Allows t beyond tE for set_tZ use
	}

	// New End Time
	Time
	new_tE()
	{
		assert( tX <= tQ );
		assert( dt_min <= dt_max );
		Time tEnd( x_2_ != 0.0 ? tQ + std::sqrt( qTol / std::abs( x_2_ ) ) : infinity );
		if ( dt_max != infinity ) tEnd = std::min( tEnd, tQ + dt_max );
		tEnd = std::max( tEnd, tQ + dt_min );
		if ( ( options::inflection ) && ( x_2_ != 0.0 ) && ( signum( x_1_ ) != signum( x_2_ ) ) ) {
			Time const tI( tX - ( x_1_ / ( two * x_2_ ) ) );
			if ( tQ < tI ) tEnd = std::min( tEnd, tI );
		}
		if ( ( tEnd == infinity ) && ( dt_inf != infinity ) ) tEnd = tQ + dt_inf;
		return tEnd;
	}

	// Set End Time
	void
	set_tE()
	{
		assert( tX <= tQ );
		assert( dt_min <= dt_max );
		tE = ( x_2_ != 0.0 ? tQ + std::sqrt( qTol / std::abs( x_2_ ) ) : infinity );
		if ( dt_max != infinity ) tE = std::min( tE, tQ + dt_max );
		tE = std::max( tE, tQ + dt_min );
		if ( ( options::inflection ) && ( x_2_ != 0.0 ) && ( signum( x_1_ ) != signum( x_2_ ) ) ) {
			Time const tI( tX - ( x_1_ / ( two * x_2_ ) ) );
			if ( tQ < tI ) tE = std::min( tE, tI );
		}
		if ( ( tE == infinity ) && ( dt_inf != infinity ) ) tE = tQ + dt_inf;
	}

	// Set Zero-Crossing Time and Type on Active Segment
	void
	set_tZ()
	{
		assert( tE == new_tE() ); // tE must be set

		// Simple root search: Only robust for small active segments with continuous rep close to function //Do Make robust version
		Time const dtX( min_positive_root_quadratic( x_2_, x_1_, x_0_ ) ); // Root of continuous rep
		assert( dtX > 0.0 );
		if ( dtX != infinity ) { // Root found on (tX,tE]
			tZ = tX + dtX;
			Crossing const crossing_check( x_0_ == 0.0 ?
			 ( tZ == tX ? Crossing::Flat : crossing_type( -x_1_ ) ) :
			 crossing_type( x_0_ > 0.0 ? std::min( x1x( tZ ), Value( 0.0 ) ) : std::max( x1x( tZ ), Value( 0.0 ) ) ) );
			if ( has( crossing_check ) ) { // Crossing type is relevant
				crossing = crossing_check;
			} else { // Crossing type not relevant
				tZ = infinity;
				return;
			}
		} else { // Root not found
			tZ = infinity;
			return;
		}

//		// Refine root // Root refinement would be too expensive and complex through an FMU
//		Time t( tZ ), t_p( t );
//		Value const vZ( f_.q( tZ ) );
//		Value v( vZ ), v_p( vZ );
//		Value m( 1.0 ); // Multiplier
//		std::size_t i( 0 );
//		std::size_t const n( 10u ); // Max iterations
//		int const sign_0( signum( x_0_ ) );
//		while ( ( ++i <= n ) && ( std::abs( v ) > aTol ) ) {
//			Value const d( f_.q1( t ) );
//			if ( d == 0.0 ) break;
//			if ( ( signum( d ) != sign_0 ) && ( tE < std::min( t_p, t ) ) ) break; // Zero-crossing seems to be >tE so don't refine further
//			t -= m * ( v / d );
//			v = f_.q( t );
//			if ( std::abs( v ) >= std::abs( v_p ) ) m *= 0.5; // Non-converging step: Reduce step size
//			t_p = t;
//			v_p = v;
//		}
//		if ( ( t >= tX ) && ( std::abs( v ) < std::abs( vZ ) ) ) tZ = t;
//		if ( ( i == n ) && ( options::output::d ) ) std::cout << "  " << name << '(' << t << ')' << " tZ may not have converged" <<  '\n';
	}

	// Set Zero-Crossing Time and Type on (tB,tE]
	void
	set_tZ( Time const /*tB*/ )
	{
		tZ = infinity; //! For now we don't handle multiple roots in active segment
	}

	// Crossing Detection
	void
	crossing_detect( int const sign_old, int const sign_new )
	{
		if ( sign_old != sign_new ) { // Zero-crossing occurs at t
			Crossing const crossing_check( crossing_type( sign_old, sign_new ) );
			if ( has( crossing_check ) ) { // Crossing type is relevant
				crossing = crossing_check;
				tZ = tX;
				event( events.shift_ZC( tZ, event() ) );
			} else {
				set_tZ();
				event( tE < tZ ? events.shift_QSS( tE, event() ) : events.shift_ZC( tZ, event() ) );
			}
		} else {
			set_tZ();
			event( tE < tZ ? events.shift_QSS( tE, event() ) : events.shift_ZC( tZ, event() ) );
		}
	}

private: // Data

	Value x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }; // Continuous rep coefficients
	Value q_0_{ 0.0 }, q_1_{ 0.0 }; // Quantized rep coefficients

};

} // fmu
} // QSS

#endif
