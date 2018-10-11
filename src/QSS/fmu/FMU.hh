// FMU Lookup Functions
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

#ifndef QSS_fmu_FMU_hh_INCLUDED
#define QSS_fmu_FMU_hh_INCLUDED

// FMI Library Headers
#include <fmilib.h>

// C++ Headers
#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>

namespace QSS {
namespace fmu {

using Time = double;
using Real = double;
using Integer = int;

// Globals
extern fmi2_import_t * fmu; // FMU instance
extern std::size_t n_ders; // Number of derivatives
extern fmi2_real_t * derivatives; // Derivatives
extern Time t_fmu; // FMU time

// FMI Status Check/Report
inline
bool
status_check( std::string const & fxn_name, fmi2_status_t const status )
{
	switch ( status ) {
	case fmi2_status_ok:
		return true;
	case fmi2_status_warning:
		std::cerr << fxn_name << " FMI status = warning" << std::endl;
		return false;
	case fmi2_status_discard:
		std::cerr << fxn_name << " FMI status = discard" << std::endl;
		return false;
	case fmi2_status_error:
		std::cerr << fxn_name << " FMI status = error" << std::endl;
		return false;
	case fmi2_status_fatal:
		std::cerr << fxn_name << " FMI status = fatal" << std::endl;
		return false;
	case fmi2_status_pending:
		std::cerr << fxn_name << " FMI status = pending" << std::endl;
		return false;
	default:
		return false;
	}
}

// Get FMU Time
inline
Time
get_time()
{
	return t_fmu;
}

// Set FMU Time
inline
void
set_time( Time const t )
{
	assert( fmu != nullptr );
	fmi2_status_t const fmi_status = fmi2_import_set_time( fmu, t_fmu = t );
	assert( ( t > fmi2_import_get_default_experiment_stop( fmu ) ) || status_check( "set_time", fmi_status ) ); // Suppress errors for t>tstop for now to allow numeric differentiation at last step
}

// Initialize Derivatives Array Size
inline
void
init_derivatives( std::size_t const n_derivatives )
{
	if ( derivatives != nullptr ) delete derivatives;
	n_ders = n_derivatives;
	derivatives = new fmi2_real_t[ n_ders ]; // Call cleanup() before exit to prevent apparent "leak"
}

// Get a Real FMU Variable Value
inline
Real
get_real( fmi2_value_reference_t const ref )
{
	assert( fmu != nullptr );
	Real val;
	fmi2_status_t const fmi_status = fmi2_import_get_real( fmu, &ref, std::size_t( 1u ), &val );
	assert( status_check( "get_real", fmi_status ) );
	return val;
}

// Get Real FMU Variable Values
inline
void
get_reals( std::size_t const n, fmi2_value_reference_t const refs[], Real vals[] )
{
	assert( fmu != nullptr );
	fmi2_status_t const fmi_status = fmi2_import_get_real( fmu, refs, n, vals );
	assert( status_check( "get_reals", fmi_status ) );
}

// Set a Real FMU Variable Value
inline
void
set_real( fmi2_value_reference_t const ref, Real const val )
{
	assert( fmu != nullptr );
	fmi2_status_t const fmi_status = fmi2_import_set_real( fmu, &ref, std::size_t( 1u ), &val );
	assert( status_check( "set_real", fmi_status ) );
}

// Set a Real FMU Variable Value
inline
void
set_reals( std::size_t const n, fmi2_value_reference_t const refs[], Real const vals[] )
{
	assert( fmu != nullptr );
	fmi2_status_t const fmi_status = fmi2_import_set_real( fmu, refs, n, vals );
	assert( status_check( "set_reals", fmi_status ) );
}

// Get All Derivatives Array: FMU Time and Variable Values Must be Set First
inline
void
get_derivatives()
{
	assert( derivatives != nullptr );
	fmi2_status_t const fmi_status = fmi2_import_get_derivatives( fmu, derivatives, n_ders );
	assert( status_check( "get_derivatives", fmi_status ) );
}

// Get a Derivative: First call get_derivatives
inline
Real
get_derivative( std::size_t const der_idx )
{
	assert( der_idx - 1 < n_ders );
	return derivatives[ der_idx - 1 ];
}

// Get an Integer FMU Variable Value
inline
Integer
get_integer( fmi2_value_reference_t const ref )
{
	assert( fmu != nullptr );
	Integer val;
	fmi2_status_t const fmi_status = fmi2_import_get_integer( fmu, &ref, std::size_t( 1u ), &val );
	assert( status_check( "get_integer", fmi_status ) );
	return val;
}

// Set an Integer FMU Variable Value
inline
void
set_integer( fmi2_value_reference_t const ref, Integer const val )
{
	assert( fmu != nullptr );
	fmi2_status_t const fmi_status = fmi2_import_set_integer( fmu, &ref, std::size_t( 1u ), &val );
	assert( status_check( "set_integer", fmi_status ) );
}

// Get an Boolean FMU Variable Value
inline
bool
get_boolean( fmi2_value_reference_t const ref )
{
	assert( fmu != nullptr );
	int val; // FMI2 uses int for booleans
	fmi2_status_t const fmi_status = fmi2_import_get_boolean( fmu, &ref, std::size_t( 1u ), &val );
	assert( status_check( "get_boolean", fmi_status ) );
	return ( val != 0 );
}

// Set an Boolean FMU Variable Value
inline
void
set_boolean( fmi2_value_reference_t const ref, bool const val )
{
	assert( fmu != nullptr );
	int const ival( static_cast< int >( val ) ); // FMI2 uses int for booleans
	fmi2_status_t const fmi_status = fmi2_import_set_boolean( fmu, &ref, std::size_t( 1u ), &ival );
	assert( status_check( "set_boolean", fmi_status ) );
}

// Discrete Event Processing
inline
void
do_event_iteration( fmi2_import_t * fmu, fmi2_event_info_t * eventInfo )
{
	eventInfo->newDiscreteStatesNeeded = fmi2_true;
	eventInfo->terminateSimulation = fmi2_false;
	while ( eventInfo->newDiscreteStatesNeeded && !eventInfo->terminateSimulation ) {
		fmi2_import_new_discrete_states( fmu, eventInfo );
	}
}

// Cleanup Allocations
inline
void
cleanup()
{
	delete derivatives;
	derivatives = nullptr;
}

} // fmu
} // QSS

#endif
