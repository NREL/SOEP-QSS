// FMU-Based QSS Zero-Crossing Variable Abstract Base Class
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

#ifndef QSS_fmu_Variable_ZC_hh_INCLUDED
#define QSS_fmu_Variable_ZC_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable.hh>
#include <QSS/EnumHash.hh> // GCC 5.x needs this

// C++ Headers
#include <unordered_set>

namespace QSS {
namespace fmu {

// FMU-Based QSS Zero-Crossing Variable Abstract Base Class
class Variable_ZC : public Variable
{

public: // Types

	using Super = Variable;
	using Crossings = std::unordered_set< Crossing, EnumHash >;

protected: // Creation

	// Constructor
	explicit
	Variable_ZC(
	 std::string const & name,
	 Value const rTol = 1.0e-4,
	 Value const aTol = 1.0e-6,
	 Value const zTol = 0.0,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Super( name, rTol, aTol, 0.0, var, der ),
	 zTol( zTol ),
	 zChatter_( zTol > 0.0 )
	{
		add_crossings_all(); // FMI API doesn't currently expose crossing information
	}

	// Copy Constructor
	Variable_ZC( Variable_ZC const & ) = default;

	// Move Constructor
	Variable_ZC( Variable_ZC && ) noexcept = default;

public: // Creation

	// Destructor
	virtual
	~Variable_ZC()
	{}

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
	is_ZC() const
	{
		return true;
	}

	// Has Crossing Type?
	bool
	has( Crossing const c )
	{
		return ( crossings.find( c ) != crossings.end() );
	}

public: // Properties

	// Boolean Value at Time t
	bool
	b( Time const t ) const
	{
		return ( t == tZ_last );
	}

	// Zero-Crossing Time
	Time
	tZC() const
	{
		return tZ;
	}

public: // Methods

	// Add Crossing Type
	Variable_ZC &
	add( Crossing const c )
	{
		crossings.insert( c );
		return *this;
	}

	// Add All Crossing Types
	Variable_ZC &
	add_crossings_all()
	{
		add_crossings_Dn();
		crossings.insert( Crossing::Flat );
		add_crossings_Up();
		return *this;
	}

	// Add All non-Flat Crossing Types
	Variable_ZC &
	add_crossings_non_Flat()
	{
		add_crossings_Dn();
		add_crossings_Up();
		return *this;
	}

	// Add All Downward Crossing Types
	Variable_ZC &
	add_crossings_Dn()
	{
		crossings.insert( Crossing::DnPN );
		crossings.insert( Crossing::DnPZ );
		crossings.insert( Crossing::DnZN );
		crossings.insert( Crossing::Dn );
		return *this;
	}

	// Add All Downward and Flat Crossing Types
	Variable_ZC &
	add_crossings_Dn_Flat()
	{
		add_crossings_Dn();
		crossings.insert( Crossing::Flat );
		return *this;
	}

	// Add All Upward Crossing Types
	Variable_ZC &
	add_crossings_Up()
	{
		crossings.insert( Crossing::Up );
		crossings.insert( Crossing::UpZP );
		crossings.insert( Crossing::UpNZ );
		crossings.insert( Crossing::UpNP );
		return *this;
	}

	// Add All Upward and Flat Crossing Types
	Variable_ZC &
	add_crossings_Up_Flat()
	{
		crossings.insert( Crossing::Flat );
		add_crossings_Up();
		return *this;
	}

	// Advance Observees Slightly Past Zero-Crossing Time for FMU Detection
	void
	advance_observees()
	{
		assert( tZ_last != infinity );
		fmu_set_observees_q( tZ_last + options::dtZC ); // Use slightly later time to let FMU detect the zero crossing: This is not robust
	}

protected: // Methods

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

	// Crossed Value for Signaling FMU Event Processing
	static
	Value
	crossed( Crossing const c )
	{
		static Value const zc_eps( 1.0e-6 ); //Do Set this based on how FMU behaves wrt chattering prevention and tolerances
		if ( ( c == Crossing::Dn ) || ( c == Crossing::DnPN ) || ( c == Crossing::DnZN ) ) {
			return -zc_eps;
		} else if ( ( c == Crossing::DnPZ ) || ( c == Crossing::Flat ) || ( c == Crossing::UpNZ ) ) {
			return 0.0;
		} else if ( ( c == Crossing::Up ) || ( c == Crossing::UpNP ) || ( c == Crossing::UpZP ) ) {
			return +zc_eps;
		} else {
			assert( false ); return 0.0;
		}
	}

public: // Data

	Value zTol{ 0.0 }; // Zero-crossing anti-chatter tolerance
	Time tZ{ infinity }; // Zero-crossing time: tQ <= tZ and tX <= tZ
	Time tZ_last{ infinity }; // Zero-crossing time of last crossing
	Crossing crossing{ Crossing::Flat }; // Zero-crossing type
	Crossing crossing_last{ Crossing::Flat }; // Zero-crossing type of last crossing
	Crossings crossings; // Zero-crossing types handled

protected: // Data

	bool zChatter_{ false }; // Zero-crossing chatter control active?
	Value x_mag_{ 0.0 }; // Value max magnitude since last zero crossing
	int sign_old_{ 0 }; // Sign of zero-crossing function before advance

};

} // fmu
} // QSS

#endif
