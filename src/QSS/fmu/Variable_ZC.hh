// FMU-Based QSS Zero-Crossing Variable Abstract Base Class
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

#ifndef QSS_fmu_Variable_ZC_hh_INCLUDED
#define QSS_fmu_Variable_ZC_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable.hh>

namespace QSS {
namespace fmu {

// FMU-Based QSS Zero-Crossing Variable Abstract Base Class
class Variable_ZC : public Variable
{

public: // Types

	using Super = Variable;

	using Crossings = std::vector< Crossing >;

protected: // Creation

	// Copy Constructor
	Variable_ZC( Variable_ZC const & ) = default;

	// Move Constructor
	Variable_ZC( Variable_ZC && ) noexcept = default;

	// Constructor
	Variable_ZC(
	 int const order,
	 std::string const & name,
	 Real const rTol,
	 Real const aTol,
	 Real const zTol,
	 FMU_ME * fmu_me,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Super( order, name, rTol, aTol, zTol, fmu_me, var, der ),
	 zChatter_( zTol > 0.0 )
	{
		add_crossings_Dn_Up(); // FMI API doesn't currently expose crossing information
	}

protected: // Assignment

	// Copy Assignment
	Variable_ZC &
	operator =( Variable_ZC const & ) = default;

	// Move Assignment
	Variable_ZC &
	operator =( Variable_ZC && ) noexcept = default;

public: // Predicate

	// Zero-Crossing Variable?
	bool
	is_ZC() const override
	{
		return true;
	}

	// Unpredicted Crossing Detected?
	bool
	detected_crossing() const override
	{
		return detected_crossing_;
	}

protected: // Predicate

	// Has Crossing Type?
	bool
	has( Crossing const c ) const
	{
		return ( std::find( crossings_.begin(), crossings_.end(), c ) != crossings_.end() );
	}

public: // Property

	// Boolean Value at Time t
	Boolean
	b( Time const t ) const override
	{
		return ( t == tZ_last );
	}

	// Zero-Crossing Time
	Time
	tZC() const
	{
		return tZ;
	}

	// Zero-Crossing Bump Time for FMU Detection
	virtual
	Time
	tZC_bump( Time const t ) const = 0;

public: // Methods

	// Bump Time for FMU Zero-Crossing Detection
	void
	bump_time( Time const t_bump ) const
	{
		fmu_set_x( t_bump );
		fmu_set_observees_x( t_bump );
		x_0_bump_ = fmu_get_real();
		if ( conditional != nullptr ) {
			for ( Variable * handler : conditional->observers() ) {
				handler->x_0_bump = handler->fmu_get_as_real();
			}
		}
	}

	// Re-Bump Time for FMU Zero-Crossing Detection
	void
	re_bump_time( Time const t_bump ) const
	{
		fmu_set_x( t_bump );
		fmu_set_observees_x( t_bump );
	}

	// Un-Bump Time for FMU Zero-Crossing Detection
	void
	un_bump_time( Time const t, Variable const * const handler ) const
	{
		handler_modified_ = ( fmu_get_real() != x_0_bump_ );
		fmu_set_observees_x( t, handler );
	}

	// Un-Bump Time for FMU Zero-Crossing Detection
	void
	un_bump_time( Time const t, Variables const & handlers ) const
	{
		handler_modified_ = ( fmu_get_real() != x_0_bump_ );
		fmu_set_observees_x( t, handlers );
	}

public: // Crossing Methods

	// Add Crossing Type
	Variable_ZC &
	add( Crossing const c )
	{
		crossings_.push_back( c );
		return *this;
	}

	// Add All Crossing Types
	Variable_ZC &
	add_crossings_all()
	{
		add_crossings_Dn();
		crossings_.push_back( Crossing::DnZN );
		crossings_.push_back( Crossing::Flat );
		crossings_.push_back( Crossing::UpZP );
		add_crossings_Up();
		return *this;
	}

	// Add All Non-Flat Crossing Types
	Variable_ZC &
	add_crossings_non_Flat()
	{
		add_crossings_Dn();
		crossings_.push_back( Crossing::DnZN );
		crossings_.push_back( Crossing::UpZP );
		add_crossings_Up();
		return *this;
	}

	// Add All Downward Crossing Types
	Variable_ZC &
	add_crossings_Dn()
	{
		crossings_.push_back( Crossing::DnPN );
		crossings_.push_back( Crossing::DnPZ );
		crossings_.push_back( Crossing::Dn );
		return *this;
	}

	// Add All Downward to Flat Crossing Types
	Variable_ZC &
	add_crossings_Dn_Flat()
	{
		add_crossings_Dn();
		crossings_.push_back( Crossing::DnZN );
		crossings_.push_back( Crossing::Flat );
		return *this;
	}

	// Add All Upward Crossing Types
	Variable_ZC &
	add_crossings_Up()
	{
		crossings_.push_back( Crossing::Up );
		crossings_.push_back( Crossing::UpNZ );
		crossings_.push_back( Crossing::UpNP );
		return *this;
	}

	// Add All Upward and Flat Crossing Types
	Variable_ZC &
	add_crossings_Up_Flat()
	{
		crossings_.push_back( Crossing::Flat );
		crossings_.push_back( Crossing::UpZP );
		add_crossings_Up();
		return *this;
	}

	// Add All Downward and Upward Crossing Types
	Variable_ZC &
	add_crossings_Dn_Up()
	{
		add_crossings_Dn();
		add_crossings_Up();
		return *this;
	}

protected: // Methods

	// Set Trajectory Magnitude to Zero
	void
	x_mag_zero()
	{
		x_mag_ = Real( 0.0 );
	}

	// Update Trajectory Magnitude with Given Value
	void
	x_mag_update( Real const val )
	{
		x_mag_ = std::max( x_mag_, std::abs( val ) );
	}

	// Refine Zero-Crossing Time: Event Indicator Zero-Crossing Variable
	void
	refine_root_ZC( Time const tBeg );

	// Refine Zero-Crossing Time: Event Indicator Directional Derivative Zero-Crossing Variable
	void
	refine_root_ZCd( Time const tBeg );

	// Refine Zero-Crossing Time: Explicit Zero-Crossing Variable
	void
	refine_root_ZCe( Time const tBeg );

	// Fix Up tE < tZ if Needed
	void
	fixup_tE()
	{
		if ( ( options::dtZMax > 0.0 ) && ( tZ <= tE ) && ( tX < tZ - options::dtZMax ) ) { // Move tE before tZ
			tE = tZ - options::dtZMax;
			shift_QSS_ZC( tE );
		}
	}

protected: // Static Methods

	// Crossing Type from Values
	template< typename T >
	static
	Crossing
	crossing_type( T const val1, T const val2 )
	{
		if ( val1 == T( 0 ) ) {
			return ( val2 > T( 0 ) ? Crossing::UpZP : ( val2 == T( 0 ) ? Crossing::Flat : Crossing::DnZN ) );
		} else if ( val2 == T( 0 ) ) {
			return ( val1 > T( 0 ) ? Crossing::DnPZ : Crossing::UpNZ );
		} else {
			assert( val1 != val2 );
			return ( val1 > T( 0 ) ? Crossing::DnPN : Crossing::UpNP );
		}
	}

	// Crossing Type from Slope
	template< typename T >
	static
	Crossing
	crossing_type( T const slope )
	{
		if ( slope == T( 0 ) ) {
			return Crossing::Flat;
		} else if ( slope > T( 0 ) ) {
			return Crossing::UpNP;
		} else {
			return Crossing::DnPN;
		}
	}

public: // Data

	Time tZ{ infinity }; // Zero-crossing time: tQ <= tZ and tX <= tZ
	Time tZ_last{ neg_infinity }; // Zero-crossing time of last crossing
	Crossing crossing{ Crossing::Flat }; // Zero-crossing type
	Crossing crossing_last{ Crossing::Flat }; // Zero-crossing type of last crossing

protected: // Data

	bool zChatter_{ false }; // Zero-crossing chatter control active?
	Real x_mag_{ 0.0 }; // Max trajectory magnitude since last zero crossing
	bool check_crossing_{ false }; // Check for zero crossing?
	bool detected_crossing_{ false }; // Unpredicted zero crossing detected?
	int sign_old_{ 0 }; // Sign of zero-crossing function before advance
	mutable bool handler_modified_{ false }; // Did last handler modify this value?
	mutable Real x_0_bump_{ 0.0 }; // Last bumped value

private: // Data

	Crossings crossings_; // Zero-crossing types handled

}; // Variable_ZC

} // fmu
} // QSS

#endif
