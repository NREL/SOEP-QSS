#ifndef QSS_FMU_hh_INCLUDED
#define QSS_FMU_hh_INCLUDED

// FMU Support
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// FMI Library Headers
#include <fmilib.h>

// C++ Headers
#include <cassert>
#include <cstddef>

namespace FMU {

using Value = double;

// Globals
extern fmi2_import_t * fmu; // FMU instance
extern std::size_t n_ders; // Number of derivatives
extern fmi2_real_t * derivatives; // Derivatives

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
Value
get_real( fmi2_value_reference_t const ref )
{
	assert( fmu != nullptr );
	Value val;
	fmi2_import_get_real( fmu, &ref, std::size_t( 1u ), &val ); //Do Check status returned
	return val;
}

// Set a Real FMU Variable Value
inline
void
set_real( fmi2_value_reference_t const ref, Value const val )
{
	assert( fmu != nullptr );
	fmi2_import_set_real( fmu, &ref, std::size_t( 1u ), &val ); //Do Check status returned
}

// Get All Derivatives Array: FMU Time and Variable Values Must be Set First
inline
void
get_derivatives()
{
	assert( derivatives != nullptr );
	fmi2_import_get_derivatives( fmu, derivatives, n_ders );
}

// Get a Derivative: First call get_derivatives
inline
Value
get_derivative( std::size_t const der_idx )
{
	assert( der_idx - 1 < n_ders );
	return derivatives[ der_idx - 1 ];
}

// Cleanup Allocations
inline
void
cleanup()
{
	delete derivatives;
	derivatives = nullptr;
}

} // FMU

#endif
