// QSS Zero-Crossing Variable Abstract Base Class
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

#ifndef QSS_cod_Variable_ZC_hh_INCLUDED
#define QSS_cod_Variable_ZC_hh_INCLUDED

// QSS Headers
#include <QSS/cod/Variable.hh>

namespace QSS {
namespace cod {

// QSS Zero-Crossing Variable Abstract Base Class
template< template< typename > class F >
class Variable_ZC : public Variable
{

public: // Types

	using Super = Variable;

	using Real = Variable::Real;
	using Time = Variable::Time;

	using Function = F< Variable >;
	using Crossing = Variable::Crossing;
	using Crossings = std::vector< Crossing >;

	using Super::observees_;

	using Super::init_observees;

protected: // Creation

	// Constructor
	explicit
	Variable_ZC(
	 std::string const & name,
	 Real const rTol = 1.0e-4,
	 Real const aTol = 1.0e-6,
	 Real const zTol = 0.0
	) :
	 Super( name, rTol, aTol ),
	 zTol( zTol ),
	 zChatter_( zTol > 0.0 )
	{}

	// Copy Constructor
	Variable_ZC( Variable_ZC const & ) = default;

	// Move Constructor
	Variable_ZC( Variable_ZC && ) noexcept = default;

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

	// Non-Zero-Crossing Variable?
	bool
	not_ZC() const
	{
		return false;
	}

	// Has Crossing Type?
	bool
	has( Crossing const c )
	{
		return ( std::find( crossings.begin(), crossings.end(), c ) != crossings.end() );
	}

public: // Properties

	// Boolean Value at Time t
	Boolean
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

	// Function
	Function const &
	f() const
	{
		return f_;
	}

	// Function
	Function &
	f()
	{
		return f_;
	}

public: // Methods

	// Initialization: Stage 0
	void
	init_0()
	{
		// Check no observers
		if ( ! observers_.empty() ) {
			std::cerr << "Error: Zero-crossing variable has observers: " << name << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Add drill-through observees
		for ( size_type i = 0, n = observees_.size(); i < n; ++i ) {
			Variable * vo( observees_[ i ] );
			for ( Variable * voo : vo->observees() ) {
				observe_ZC( voo ); // Only need back-observer to force observer updates when observees update since ZC variable value doesn't depend on these 2nd level observees
			}
		}

		// Shrink observees
		init_observees();
	}

public: // Function Methods

	// Add Constant
	Variable_ZC &
	add( Coefficient const c0 )
	{
		f_.add( c0 );
		return *this;
	}

	// Add a Variable
	Variable_ZC &
	add( Variable * v )
	{
		f_.add( v );
		observe( v );
		return *this;
	}

	// Add a Coefficient * Variable
	Variable_ZC &
	add(
	 Coefficient const c,
	 Variable * v
	)
	{
		f_.add( c, v );
		observe( v );
		return *this;
	}

public: // Crossing Methods

	// Add Crossing Type
	Variable_ZC &
	add( Crossing const c )
	{
		crossings.push_back( c );
		return *this;
	}

	// Add All Crossing Types
	Variable_ZC &
	add_crossings_all()
	{
		add_crossings_Dn();
		crossings.push_back( Crossing::DnZN );
		crossings.push_back( Crossing::Flat );
		crossings.push_back( Crossing::UpZP );
		add_crossings_Up();
		return *this;
	}

	// Add All Non-Flat Crossing Types
	Variable_ZC &
	add_crossings_non_Flat()
	{
		add_crossings_Dn();
		crossings.push_back( Crossing::DnZN );
		crossings.push_back( Crossing::UpZP );
		add_crossings_Up();
		return *this;
	}

	// Add All Downward Crossing Types
	Variable_ZC &
	add_crossings_Dn()
	{
		crossings.push_back( Crossing::DnPN );
		crossings.push_back( Crossing::DnPZ );
		crossings.push_back( Crossing::Dn );
		return *this;
	}

	// Add All Downward to Flat Crossing Types
	Variable_ZC &
	add_crossings_Dn_Flat()
	{
		add_crossings_Dn();
		crossings.push_back( Crossing::DnZN );
		crossings.push_back( Crossing::Flat );
		return *this;
	}

	// Add All Upward Crossing Types
	Variable_ZC &
	add_crossings_Up()
	{
		crossings.push_back( Crossing::Up );
		crossings.push_back( Crossing::UpNZ );
		crossings.push_back( Crossing::UpNP );
		return *this;
	}

	// Add All Upward and Flat Crossing Types
	Variable_ZC &
	add_crossings_Up_Flat()
	{
		crossings.push_back( Crossing::Flat );
		crossings.push_back( Crossing::UpZP );
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

public: // Data

	Real zTol{ 0.0 }; // Zero-crossing anti-chatter tolerance
	Time tZ{ infinity }; // Zero-crossing time: tQ <= tZ and tX <= tZ
	Time tZ_last{ 0.0 }; // Zero-crossing time of last crossing
	Crossing crossing{ Crossing::Flat }; // Zero-crossing type
	Crossing crossing_last{ Crossing::Flat }; // Zero-crossing type of last crossing
	Crossings crossings; // Zero-crossing types handled

protected: // Data

	bool zChatter_{ false }; // Zero-crossing chatter control active?
	Real x_mag_{ 0.0 }; // Value max magnitude since last zero crossing
	bool check_crossing_{ false }; // Check for zero crossing?
	int sign_old_{ 0 }; // Sign of zero-crossing function before advance
	Function f_; // Zero-crossing function

};

} // cod
} // QSS

#endif
