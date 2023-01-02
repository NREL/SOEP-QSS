// FMU Variable Specifications
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

#ifndef QSS_FMU_Variable_hh_INCLUDED
#define QSS_FMU_Variable_hh_INCLUDED

// FMI Library Headers
#include <FMI2/fmi2_import.h>

// C++ Headers
#include <cassert>
#include <cstddef>
#include <ostream>
#include <string>

namespace QSS {

// FMU Variable Specifications
class FMU_Variable final
{

public: // Types

	using Index = std::size_t;
	using Real = fmi2_real_t;
	using Integer = fmi2_integer_t;
	using Boolean = fmi2_boolean_t;
	using String = fmi2_string_t;

public: // Types

	enum class Typ { None, Real, Integer, Boolean, String, Enum }; // Variable value type
	enum class Cat { None, Discrete, Continuous }; // Variable value category

public: // Creation

	// Default Constructor
	FMU_Variable() = default;

	// Variable Index Constructor
	explicit
	FMU_Variable( Index const idx ) :
	 idx( idx )
	{}

	// Real Variable Constructor
	FMU_Variable(
	 Index const idx,
	 fmi2_import_variable_t * const var,
	 fmi2_import_real_variable_t * const rvr
	) :
	 idx( idx ),
	 typ( Typ::Real ),
	 cat( Cat::Continuous ),
	 var( var ),
	 rvr( rvr )
	{}

	// Integer Variable Constructor
	FMU_Variable(
	 Index const idx,
	 fmi2_import_variable_t * const var,
	 fmi2_import_integer_variable_t * const ivr
	) :
	 idx( idx ),
	 typ( Typ::Integer ),
	 cat( Cat::Discrete ),
	 var( var ),
	 ivr( ivr )
	{}

	// Boolean Variable Constructor
	FMU_Variable(
	 Index const idx,
	 fmi2_import_variable_t * const var,
	 fmi2_import_bool_variable_t * const bvr
	) :
	 idx( idx ),
	 typ( Typ::Boolean ),
	 cat( Cat::Discrete ),
	 var( var ),
	 bvr( bvr )
	{}

	// String Variable Constructor
	FMU_Variable(
	 Index const idx,
	 fmi2_import_variable_t * const var,
	 fmi2_import_string_variable_t * const svr
	) :
	 idx( idx ),
	 typ( Typ::String ),
	 cat( Cat::Discrete ),
	 var( var ),
	 svr( svr )
	{}

	// Enum Variable Constructor
	FMU_Variable(
	 Index const idx,
	 fmi2_import_variable_t * const var,
	 fmi2_import_enum_variable_t * const evr
	) :
	 idx( idx ),
	 typ( Typ::Enum ),
	 cat( Cat::Discrete ),
	 var( var ),
	 evr( evr )
	{}

public: // Predicate: Variable Type

	// Type Not Set?
	bool
	is_None() const
	{
		return ( typ == Typ::None );
	}

	// Real?
	bool
	is_Real() const
	{
		return ( typ == Typ::Real );
	}

	// Integer?
	bool
	is_Integer() const
	{
		return ( typ == Typ::Integer );
	}

	// Boolean?
	bool
	is_Boolean() const
	{
		return ( typ == Typ::Boolean );
	}

	// String?
	bool
	is_String() const
	{
		return ( typ == Typ::String );
	}

	// Enum?
	bool
	is_Enum() const
	{
		return ( typ == Typ::Enum );
	}

	// Discrete?
	bool
	is_Discrete() const
	{
		return ( cat == Cat::Discrete );
	}

	// Continuous?
	bool
	is_Continuous() const
	{
		return ( cat == Cat::Continuous );
	}

	// State?
	bool
	is_State() const
	{
		return ( ( idd != 0u ) && !is_ei );
	}

	// Derivative?
	bool
	is_Derivative() const
	{
		return ( ids != 0u );
	}

	// Event Indicator?
	bool
	is_EventIndicator() const
	{
		return is_ei;
	}

public: // Predicate: Causality

	// Parameter?
	bool
	causality_parameter() const
	{
		return ( var != nullptr ? fmi2_import_get_causality( var ) == fmi2_causality_enu_parameter : false );
	}

	// Parameter?
	bool
	causality_calculated_parameter() const
	{
		return ( var != nullptr ? fmi2_import_get_causality( var ) == fmi2_causality_enu_calculated_parameter : false );
	}

	// Input?
	bool
	causality_input() const
	{
		return ( var != nullptr ? fmi2_import_get_causality( var ) == fmi2_causality_enu_input : false );
	}

	// Output?
	bool
	causality_output() const
	{
		return ( var != nullptr ? fmi2_import_get_causality( var ) == fmi2_causality_enu_output : false );
	}

	// Local?
	bool
	causality_local() const
	{
		return ( var != nullptr ? fmi2_import_get_causality( var ) == fmi2_causality_enu_local : false );
	}

	// Independent?
	bool
	causality_independent() const
	{
		return ( var != nullptr ? fmi2_import_get_causality( var ) == fmi2_causality_enu_independent : false );
	}

	// Unknown?
	bool
	causality_unknown() const
	{
		return ( var != nullptr ? fmi2_import_get_causality( var ) == fmi2_causality_enu_unknown : true );
	}

public: // Predicate: Variability

	// Constant?
	bool
	variability_constant() const
	{
		return ( var != nullptr ? fmi2_import_get_variability( var ) == fmi2_variability_enu_constant : false );
	}

	// Fixed?
	bool
	variability_fixed() const
	{
		return ( var != nullptr ? fmi2_import_get_variability( var ) == fmi2_variability_enu_fixed : false );
	}

	// Tunable?
	bool
	variability_tunable() const
	{
		return ( var != nullptr ? fmi2_import_get_variability( var ) == fmi2_variability_enu_tunable : false );
	}

	// Discrete?
	bool
	variability_discrete() const
	{
		return ( var != nullptr ? fmi2_import_get_variability( var ) == fmi2_variability_enu_discrete : false );
	}

	// Continuous?
	bool
	variability_continuous() const
	{
		return ( var != nullptr ? fmi2_import_get_variability( var ) == fmi2_variability_enu_continuous : false );
	}

	// Unknown?
	bool
	variability_unknown() const
	{
		return ( var != nullptr ? fmi2_import_get_variability( var ) == fmi2_variability_enu_unknown : true );
	}

public: // Predicate: Initial

	// Exact?
	bool
	initial_exact() const
	{
		return ( var != nullptr ? fmi2_import_get_initial( var ) == fmi2_initial_enu_exact : false );
	}

	// Approx?
	bool
	initial_approx() const
	{
		return ( var != nullptr ? fmi2_import_get_initial( var ) == fmi2_initial_enu_approx : false );
	}

	// Calculated?
	bool
	initial_calculated() const
	{
		return ( var != nullptr ? fmi2_import_get_initial( var ) == fmi2_initial_enu_calculated : false );
	}

	// Unknown?
	bool
	initial_unknown() const
	{
		return ( var != nullptr ? fmi2_import_get_initial( var ) == fmi2_initial_enu_unknown : true );
	}

public: // Predicate: Start

	// Has Start Value?
	bool
	has_start() const
	{
		return ( var != nullptr ? fmi2_import_get_variable_has_start( var ) == 1 : false );
	}

public: // Property

	// Value Reference
	fmi2_value_reference_t
	ref() const
	{
		return ( var != nullptr ? fmi2_import_get_variable_vr( var ) : fmi2_value_reference_t( 0u ) );
	}

	// Index
	Index
	index() const
	{
		return idx;
	}

	// State Index
	Index
	state() const
	{
		return ids;
	}

	// Derivative Index
	Index
	derivative() const
	{
		return idd;
	}

	// State/Derivative/Nominal Array Index
	Index
	array() const
	{
		return isa;
	}

	// Name
	std::string
	name() const
	{
		return ( var != nullptr ? std::string( fmi2_import_get_variable_name( var ) ) : std::string() );
	}

	// Description
	std::string
	description() const
	{
		return ( ( var != nullptr ) && fmi2_import_get_variable_description( var ) ? std::string( fmi2_import_get_variable_description( var ) ) : std::string() );
	}

	// Real Variable
	fmi2_import_real_variable_t const *
	var_real() const
	{
		assert( typ == Typ::Real );
		return rvr;
	}

	// Real Variable
	fmi2_import_real_variable_t *
	var_real()
	{
		assert( typ == Typ::Real );
		return rvr;
	}

	// Integer Variable
	fmi2_import_integer_variable_t const *
	var_int() const
	{
		assert( typ == Typ::Integer );
		return ivr;
	}

	// Integer Variable
	fmi2_import_integer_variable_t *
	var_int()
	{
		assert( typ == Typ::Integer );
		return ivr;
	}

	// Boolean Variable
	fmi2_import_bool_variable_t const *
	var_bool() const
	{
		assert( typ == Typ::Boolean );
		return bvr;
	}

	// Boolean Variable
	fmi2_import_bool_variable_t *
	var_bool()
	{
		assert( typ == Typ::Boolean );
		return bvr;
	}

public: // Methods

	// Set Real Variable to Discrete Based on Model Information: Only Depends on Discrete Variables and/or is Modified Discretely by Event Handlers
	void
	to_Discrete()
	{
		assert( typ == Typ::Real );
		cat = Cat::Discrete;
	}

	// Set Real Variable to Event Indicator on Model Information
	void
	to_EventIndicator()
	{
		assert( typ == Typ::Real );
		is_ei = true;
	}

	// Set State Index
	void
	state( Index const ids_ )
	{
		assert( typ == Typ::Real );
		assert( cat == Cat::Continuous );
		ids = ids_;
	}

	// Set Derivative Index
	void
	derivative( Index const idd_ )
	{
		assert( typ == Typ::Real );
		assert( cat == Cat::Continuous );
		idd = idd_;
	}

	// Set State/Derivative/Nominal Array Index
	void
	array( Index const isa_ )
	{
		assert( typ == Typ::Real );
		assert( cat == Cat::Continuous );
		isa = isa_;
	}

public: // Operator

	// FMU_Variable < FMU_Variable
	friend
	constexpr
	bool
	operator <( FMU_Variable const & v1, FMU_Variable const & v2 )
	{
		return ( v1.idx < v2.idx );
	}

public: // I/O

	// Stream << FMU_Variable
	friend
	std::ostream &
	operator <<( std::ostream & stream, FMU_Variable const & v )
	{
		stream << "\nFMU_Variable:\n";
		stream << " name: " << v.name() << '\n';
		stream << " desc: " << v.description() << '\n';
		stream << " idx: " << v.idx << '\n';
		if ( v.ids != 0 ) stream << " ids: " << v.ids << '\n';
		if ( v.idd != 0 ) stream << " idd: " << v.idd << '\n';
		if ( v.isa != 0 ) stream << " isa: " << v.isa << '\n';
		switch ( v.typ ) {
		case FMU_Variable::Typ::None:
			stream << " Type: None\n";
			break;
		case FMU_Variable::Typ::Real:
			stream << " Type: Real\n";
			break;
		case FMU_Variable::Typ::Integer:
			stream << " Type: Integer\n";
			break;
		case FMU_Variable::Typ::Boolean:
			stream << " Type: Boolean\n";
			break;
		case FMU_Variable::Typ::String:
			stream << " Type: String\n";
			break;
		case FMU_Variable::Typ::Enum:
			stream << " Type: Enum\n";
			break;
		}
		switch ( v.cat ) {
		case FMU_Variable::Cat::None:
			stream << " Cat: None\n";
			break;
		case FMU_Variable::Cat::Discrete:
			stream << " Cat: Discrete\n";
			break;
		case FMU_Variable::Cat::Continuous:
			stream << " Cat: Continuous\n";
			break;
		}
		if ( v.is_ei ) stream << " Event Indicator\n";
		return stream;
	}

public: // Data

	Index idx{ 0u }; // FMU variable index
	Index ids{ 0u }; // FMU state index (for derivatives)
	Index idd{ 0u }; // FMU derivative index (for states (or event indicators if/when they get derivatives))
	Index isa{ 0u }; // FMU variable state/derivative/nominal array index (0-based)
	Typ typ{ Typ::None }; // FMU Variable value type
	Cat cat{ Cat::None }; // FMU Variable value subtype
	bool is_ei{ false }; // FMU Variable is an event indicator?
	bool has_upstream_state_or_ei_observer{ false }; // FMU variable has a state or event indicator direct or upstream observer
	fmi2_import_variable_t * var{ nullptr }; // FMU variable pointer
	union { // FMU type-specific variable pointer // C++17 std::variant could be used here instead
		void * vvr{ nullptr }; // FMU unknown type variable pointer
		fmi2_import_real_variable_t * rvr; // FMU real variable pointer
		fmi2_import_integer_variable_t * ivr; // FMU integer variable pointer
		fmi2_import_bool_variable_t * bvr; // FMU boolean variable pointer
		fmi2_import_string_variable_t * svr; // FMU string variable pointer
		fmi2_import_enum_variable_t * evr; // FMU enum variable pointer
	};

}; // FMU_Variable

} // QSS

#endif
