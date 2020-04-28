// FMU Variable Specifications
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

#ifndef QSS_fmu_FMU_Variable_hh_INCLUDED
#define QSS_fmu_FMU_Variable_hh_INCLUDED

// FMI Library Headers
#include <FMI2/fmi2_import.h>

// C++ Headers
#include <cstddef>

namespace QSS {
namespace fmu {

// FMU Variable Specifications
struct FMU_Variable final
{

public: // Types

	using size_type = std::size_t;

public: // Creation

	// Default Constructor
	FMU_Variable() = default;

	// Real Variable Constructor
	FMU_Variable(
	 fmi2_import_variable_t * const var,
	 fmi2_import_real_variable_t * const rvr,
	 fmi2_value_reference_t const ref,
	 size_type const idx,
	 size_type const ics = 0u
	) :
	 var( var ),
	 rvr( rvr ),
	 cau( var != nullptr ? fmi2_import_get_causality( var ) : fmi2_causality_enu_unknown ),
	 ref( ref ),
	 idx( idx ),
	 ics( ics )
	{}

	// Integer Variable Constructor
	FMU_Variable(
	 fmi2_import_variable_t * const var,
	 fmi2_import_integer_variable_t * const ivr,
	 fmi2_value_reference_t const ref,
	 size_type const idx,
	 size_type const ics = 0u
	) :
	 var( var ),
	 ivr( ivr ),
	 cau( var != nullptr ? fmi2_import_get_causality( var ) : fmi2_causality_enu_unknown ),
	 ref( ref ),
	 idx( idx ),
	 ics( ics )
	{}

	// Boolean Variable Constructor
	FMU_Variable(
	 fmi2_import_variable_t * const var,
	 fmi2_import_bool_variable_t * const bvr,
	 fmi2_value_reference_t const ref,
	 size_type const idx,
	 size_type const ics = 0u
	) :
	 var( var ),
	 bvr( bvr ),
	 cau( var != nullptr ? fmi2_import_get_causality( var ) : fmi2_causality_enu_unknown ),
	 ref( ref ),
	 idx( idx ),
	 ics( ics )
	{}

public: // Predicate: Causality

	// Independent?
	bool
	causality_independent() const
	{
		return ( cau == fmi2_causality_enu_independent );
	}

	// Input?
	bool
	causality_input() const
	{
		return ( cau == fmi2_causality_enu_input );
	}

	// Local?
	bool
	causality_local() const
	{
		return ( cau == fmi2_causality_enu_local );
	}

	// Output?
	bool
	causality_output() const
	{
		return ( cau == fmi2_causality_enu_output );
	}

	// Parameter?
	bool
	causality_parameter() const
	{
		return ( cau == fmi2_causality_enu_parameter );
	}

	// Calclated Parameter?
	bool
	causality_calculated_parameter() const
	{
		return ( cau == fmi2_causality_enu_calculated_parameter );
	}

	// Unknown?
	bool
	causality_unknown() const
	{
		return ( cau == fmi2_causality_enu_unknown );
	}

public: // Data

	fmi2_import_variable_t * var{ nullptr }; // FMU variable pointer
	union { // Support FMU real, integer, and boolean variables
		fmi2_import_real_variable_t * rvr{ nullptr }; // FMU real variable pointer
		fmi2_import_integer_variable_t * ivr; // FMU integer variable pointer
		fmi2_import_bool_variable_t * bvr; // FMU boolean variable pointer
	};
	fmi2_causality_enu_t cau{ fmi2_causality_enu_unknown };
	fmi2_value_reference_t ref{ 0 }; // FMU variable value reference
	size_type idx{ 0u }; // FMU variable index
	size_type ics{ 0u }; // FMU continuous state index

}; // FMU_Variable

} // fmu
} // QSS

#endif
