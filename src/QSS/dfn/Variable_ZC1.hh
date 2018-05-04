// QSS1 Zero-Crossing Variable
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

#ifndef QSS_dfn_Variable_ZC1_hh_INCLUDED
#define QSS_dfn_Variable_ZC1_hh_INCLUDED

// QSS Headers
#include <QSS/dfn/Variable_ZC.hh>

namespace QSS {
namespace dfn {

// QSS1 Zero-Crossing Variable
template< template< typename > class F >
class Variable_ZC1 final : public Variable_ZC< F >
{

public: // Types

	using Super = Variable_ZC< F >;
	using Time = Variable::Time;
	using Value = Variable::Value;
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
	using Super::dt_inf;
	using Super::self_observer;
	using Super::sign_old_;
	using Super::when_clauses;
	using Super::x_mag_;
	using Super::zChatter_;
	using Super::zTol;

	using Super::add_QSS_ZC;
	using Super::add_ZC;
	using Super::event;
	using Super::has;
	using Super::shift_QSS_ZC;
	using Super::shift_ZC;
	using Super::tE_infinity_tQ;

protected: // Types

	using Super::crossing_type;

private: // Types

	using Super::event_;
	using Super::f_;
	using Super::observers_;

public: // Creation

	// Constructor
	explicit
	Variable_ZC1(
	 std::string const & name,
	 Value const rTol = 1.0e-4,
	 Value const aTol = 1.0e-6,
	 Value const zTol = 0.0
	) :
	 Super( name, rTol, aTol, zTol )
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
		return x_0_;
	}

public: // Methods

	// Initialization
	void
	init()
	{
		// Initialize trajectory specs
		x_0_ = f_.x( tQ );
		x_mag_ = std::abs( x_0_ );
		set_qTol();
		x_1_ = f_.x1( tQ );
		set_tE();
		set_tZ();
		tE < tZ ? add_QSS_ZC( tE ) : add_ZC( tZ );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << '\n';
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
		Value const x_tE( zChatter_ ? x( tE ) : Value( 0.0 ) );
#ifndef QSS_ZC_REQUANT_NO_CROSSING_CHECK
		bool const check_crossing( ( tE > tZ_last ) || ( x_mag_ != 0.0 ) );
		int const sign_old( check_crossing ? signum( zChatter_ ? x_tE : x( tE ) ) : 0 );
#endif
		x_0_ = f_.x( tX = tQ = tE );
		x_mag_ = max( x_mag_, std::abs( x_tE ), std::abs( x_0_ ) );
		set_qTol();
		x_1_ = f_.x1( tE );
		set_tE();
#ifndef QSS_ZC_REQUANT_NO_CROSSING_CHECK
		crossing_detect( sign_old, signum( x_0_ ), check_crossing );
#else
		set_tZ();
		tE < tZ ? shift_QSS_ZC( tE ) : shift_ZC( tZ );
#endif
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ << '\n';
	}

	// Observer Advance
	void
	advance_observer( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Value const x_t( zChatter_ ? x( t ) : Value( 0.0 ) );
		bool const check_crossing( ( t > tZ_last ) || ( x_mag_ != 0.0 ) );
		int const sign_old( check_crossing ? signum( zChatter_ ? x_t : x( t ) ) : 0 );
		x_0_ = f_.x( tX = tQ = t );
		x_mag_ = max( x_mag_, std::abs( x_t ), std::abs( x_0_ ) );
		set_qTol();
		x_1_ = f_.x1( t );
		set_tE();
		crossing_detect( sign_old, signum( x_0_ ), check_crossing );
		if ( options::output::d ) std::cout << "  " << name << '(' << tX << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ <<  '\n';
	}

	// Observer Advance: Parallel
	void
	advance_observer_parallel( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Value const x_t( zChatter_ ? x( t ) : Value( 0.0 ) );
		check_crossing_ = ( t > tZ_last ) || ( x_mag_ != 0.0 );
		sign_old_ = ( check_crossing_ ? signum( zChatter_ ? x_t : x( t ) ) : 0 );
		x_0_ = f_.x( tX = tQ = t );
		x_mag_ = max( x_mag_, std::abs( x_t ), std::abs( x_0_ ) );
		set_qTol();
		x_1_ = f_.x1( t );
		set_tE();
	}

	// Observer Advance: Sequential
	void
	advance_observer_sequential()
	{
		crossing_detect( sign_old_, signum( x_0_ ), check_crossing_ );
		if ( options::output::d ) std::cout << "  " << name << '(' << tX << ')' << " = " << std::showpos << x_0_ << x_1_ << "*t" << std::noshowpos << "   tE=" << tE << "   tZ=" << tZ <<  '\n';
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
					if ( tZ > tX ) {
						crossing = crossing_check;
						if ( options::refine ) { // Refine root: Expensive!
							Time t( tZ ), t_p( tZ );
							Value const vZ( f_.x( tZ ) );
							Value v( vZ ), v_p( vZ );
							Value m( 1.0 ); // Multiplier
							std::size_t i( 0 );
							std::size_t const n( 10u ); // Max iterations
							while ( ( ++i <= n ) && ( ( std::abs( v ) > aTol ) || ( std::abs( v ) < std::abs( v_p ) ) ) ) {
								Value const d( f_.x1( t ) );
								if ( d == 0.0 ) break;
								//if ( ( signum( d ) != sign_old ) && ( tE < std::min( t_p, t ) ) ) break; // Zero-crossing seems to be >tE so don't refine further
								t -= m * ( v / d );
								v = f_.x( t );
								if ( std::abs( v ) >= std::abs( v_p ) ) m *= 0.5; // Non-converging step: Reduce step size
								t_p = t;
								v_p = v;
							}
							if ( ( t >= tX ) && ( std::abs( v ) < std::abs( vZ ) ) ) tZ = t;
							if ( ( i == n ) && ( options::output::d ) ) std::cout << "  " << name << '(' << t << ')' << " tZ may not have converged" <<  '\n';
						}
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

	Value x_0_{ 0.0 }, x_1_{ 0.0 }; // Continuous rep coefficients

};

} // dfn
} // QSS

#endif
