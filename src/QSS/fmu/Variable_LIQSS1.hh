// FMU-Based LIQSS1 Variable
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

#ifndef QSS_fmu_Variable_LIQSS1_hh_INCLUDED
#define QSS_fmu_Variable_LIQSS1_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable_QSS.hh>

namespace QSS {
namespace fmu {

// FMU-Based LIQSS1 Variable
class Variable_LIQSS1 final : public Variable_QSS
{

public: // Types

	using Super = Variable_QSS;

public: // Creation

	// Constructor
	explicit
	Variable_LIQSS1(
	 std::string const & name,
	 Value const rTol = 1.0e-4,
	 Value const aTol = 1.0e-6,
	 Value const xIni = 0.0,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Super( name, rTol, aTol, xIni, var, der ),
	 x_0_( xIni ),
	 q_c_( xIni ),
	 q_0_( xIni )
	{
		set_qTol();
	}

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
		return x_0_ + ( x_1_ * ( t - tX ) );
	}

	// Continuous First Derivative at Time t
	Value
	x1( Time const ) const
	{
		return x_1_;
	}

	// Quantized Value at Time t
	Value
	q( Time const ) const
	{
		return q_0_;
	}

	// Simultaneous Value at Time t
	Value
	s( Time const ) const
	{
		return ( st == events.active_superdense_time() ? q_c_ : q_0_ );
	}

	// Simultaneous Numeric Differentiation Value at Time t
	Value
	sn( Time const t ) const
	{
		return ( st == events.active_superdense_time() ? q_c_ : q_0_ );
	}

public: // Methods

	// Initialization
	void
	init()
	{
		init_0();
		init_1();
	}

	// Initialization to a Value
	void
	init( Value const x )
	{
		init_0( x );
		init_1();
	}

	// Initialization: Stage 0
	void
	init_0()
	{
		init_observers();
		fmu_set_value( x_0_ = q_c_ = q_0_ = xIni );
		set_qTol();
	}

	// Initialization to a Value: Stage 0
	void
	init_0( Value const x )
	{
		init_observers();
		fmu_set_value( x_0_ = q_c_ = q_0_ = x );
		set_qTol();
	}

	// Initialization: Stage 1
	void
	init_1()
	{
		if ( self_observer ) {
			advance_LIQSS();
			fmu_set_value( x_0_ );
		} else {
			x_1_ = fmu_get_deriv();
			q_0_ += signum( x_1_ ) * qTol;
		}
		set_tE_aligned();
		add_QSS( tE );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
	}

	// Set Current Tolerance
	void
	set_qTol()
	{
		qTol = std::max( rTol * std::abs( q_c_ ), aTol );
		assert( qTol > 0.0 );
	}

	// QSS Advance
	void
	advance_QSS()
	{
		x_0_ = q_c_ = q_0_ = x_0_ + ( x_1_ * ( ( tQ = tE ) - tX ) );
		set_qTol();
		fmu_set_observees_q( tX = tQ );
		if ( self_observer ) {
			advance_LIQSS();
		} else {
			x_1_ = fmu_get_deriv();
			q_0_ += signum( x_1_ ) * qTol;
		}
		advance_observers_1();
		if ( observers_max_order_ >= 2 ) {
			fmu::set_time( tN = tQ + options::dtNum );
			advance_observers_2();
		}
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) {
			std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
			advance_observers_d();
		}
	}

	// QSS Advance: Stage 0
	void
	advance_QSS_0()
	{
		x_0_ = q_c_ = q_0_ = x_0_ + ( x_1_ * ( ( tQ = tE ) - tX ) );
		tX = tE;
		set_qTol();
	}

	// QSS Advance: Stage 1
	void
	advance_QSS_1()
	{
		fmu_set_observees_s( tQ );
		if ( self_observer ) {
			advance_LIQSS();
		} else {
			x_1_ = fmu_get_deriv();
			q_0_ += signum( x_1_ ) * qTol;
		}
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
	}

	// Observer Advance: Stage 1
	void
	advance_observer_1( Time const t, Value const d )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		x_0_ = x_0_ + ( x_1_ * ( t - tX ) );
		tX = t;
		x_1_ = d;
		set_tE_unaligned();
		shift_QSS( tE );
	}

	// Observer Advance: Stage d
	void
	advance_observer_d() const
	{
		std::cout << "  " << name << '(' << tX << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
	}

	// Handler Advance
	void
	advance_handler( Time const t )
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_c_ = q_0_ = fmu_get_value(); // Assume FMU ran zero-crossing handler
		set_qTol();
		advance_observers_1();
		fmu_set_observees_q( tQ );
		if ( ( self_observer ) && ( observers_.empty() ) ) fmu_set_value( q_0_ );
		x_1_ = fmu_get_deriv();
		if ( observers_max_order_ >= 2 ) {
			fmu::set_time( tN = tQ + options::dtNum );
			advance_observers_2();
		}
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) {
			std::cout << "* " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
			advance_observers_d();
		}
	}

	// Handler Advance: Stage 0
	void
	advance_handler_0( Time const t )
	{
		assert( ( tX <= t ) && ( tQ <= t ) && ( t <= tE ) );
		tX = tQ = t;
		x_0_ = q_c_ = q_0_ = fmu_get_value(); // Assume FMU ran zero-crossing handler
		set_qTol();
	}

	// Handler Advance: Stage 1
	void
	advance_handler_1()
	{
		fmu_set_observees_q( tQ );
		if ( ( self_observer ) && ( observers_.empty() ) ) fmu_set_value( q_0_ );
		x_1_ = fmu_get_deriv();
		set_tE_aligned();
		shift_QSS( tE );
		if ( options::output::d ) std::cout << "* " << name << '(' << tQ << ')' << " = " << q_0_ << " quantized, " << x_0_ << "+" << x_1_ << "*t internal   tE=" << tE << '\n';
	}

	// Handler No-Advance
	void
	no_advance_handler()
	{
		shift_QSS( tE );
	}

private: // Methods

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE_aligned()
	{
		assert( tX <= tQ );
		assert( dt_min <= dt_max );
		Time dt( x_1_ != 0.0 ? qTol / std::abs( x_1_ ) : infinity );
		dt = std::min( std::max( dt, dt_min ), dt_max );
		tE = ( dt != infinity ? tQ + dt : infinity );
		tE_infinity_tQ();
	}

	// Set End Time: Quantized and Continuous Unaligned
	void
	set_tE_unaligned()
	{
		assert( tQ <= tX );
		assert( dt_min <= dt_max );
		Time dt(
		 ( x_1_ > 0.0 ? ( q_c_ + qTol - x_0_ ) / x_1_ :
		 ( x_1_ < 0.0 ? ( q_c_ - qTol - x_0_ ) / x_1_ :
		 infinity ) ) );
		dt = std::min( std::max( dt, dt_min ), dt_max );
		tE = ( dt != infinity ? tX + dt : infinity );
		tE_infinity_tX();
	}

	// Advance Self-Observing Trigger
	void
	advance_LIQSS()
	{
		assert( qTol > 0.0 );
		assert( self_observer );

		// Value at +/- qTol
		Value const q_l( q_c_ - qTol );
		Value const q_u( q_c_ + qTol );

		// Derivative at +/- qTol
		fmu_set_value( q_l );
		Value const d_l( fmu_get_deriv() );
		int const d_l_s( signum( d_l ) );
		fmu_set_value( q_u );
		Value const d_u( fmu_get_deriv() );
		int const d_u_s( signum( d_u ) );

		// Set coefficients based on derivative signs
		if ( ( d_l_s == -1 ) && ( d_u_s == -1 ) ) { // Downward trajectory
			q_0_ -= qTol;
			x_1_ = d_l;
		} else if ( ( d_l_s == +1 ) && ( d_u_s == +1 ) ) { // Upward trajectory
			q_0_ += qTol;
			x_1_ = d_u;
		} else { // Flat trajectory
			q_0_ = std::min( std::max( ( ( q_l * d_u ) - ( q_u * d_l ) ) / ( d_u - d_l ), q_l ), q_u ); // Value where deriv is ~ 0 // Clipped in case of roundoff
			x_1_ = 0.0;
		}
	}

private: // Data

	Value x_0_{ 0.0 }, x_1_{ 0.0 }; // Continuous rep coefficients
	Value q_c_{ 0.0 }, q_0_{ 0.0 }; // Quantized rep coefficients

};

} // fmu
} // QSS

#endif
