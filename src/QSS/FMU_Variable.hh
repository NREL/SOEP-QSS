#ifndef QSS_FMU_Variable_hh_INCLUDED
#define QSS_FMU_Variable_hh_INCLUDED

// QSS FMU Variable Specifications
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// FMI Library Headers
#include <FMI2/fmi2_import.h>

// C++ Headers
#include <cassert>
#include <cstddef>

// QSS FMU Variable Specifications
class FMU_Variable
{

public: // Types

	using size_type = std::size_t;

public: // Creation

	// Default Constructor
	FMU_Variable()
	{}

	// Constructor
	FMU_Variable(
	 fmi2_import_variable_t * const var,
	 fmi2_import_real_variable_t * const rvr,
	 fmi2_value_reference_t const ref,
	 size_type const idx,
	 size_type const ics = 0u
	) :
	 var( var ),
	 rvr( rvr ),
	 ref( ref ),
	 idx( idx ),
	 ics( ics )
	{}

public: // Data

	fmi2_import_variable_t * var{ nullptr }; // FMU variable pointer (non-owning)
	fmi2_import_real_variable_t * rvr{ nullptr }; // FMU real variable pointer (non-owning)
	fmi2_value_reference_t ref{ 0 }; // FMU variable value reference
	size_type idx{ 0u }; // FMU variable index
	size_type ics{ 0u }; // FMU continuous state index

};

#endif
