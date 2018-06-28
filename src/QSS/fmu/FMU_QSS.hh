// QSS FMU-QSS Class
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

#ifndef QSS_fmu_FMU_QSS_hh_INCLUDED
#define QSS_fmu_FMU_QSS_hh_INCLUDED

// FMI Library Headers
#include <fmilib.h>

// C++ Headers
#include <cstdlib>
#include <string>
#include <vector>

namespace QSS {
namespace fmu {

// FMU-QSS Class
struct FMU_QSS final
{

public: // Types

	using size_type = std::size_t;

	enum class FMU_Generator { JModelica, Dymola, Other };

public: // Creation

	// Destructor
	~FMU_QSS()
	{
//		if ( fmu ) fmi2_import_terminate( fmu );
//		if ( fmu ) fmi2_import_free_instance( fmu );
		std::free( states );
		std::free( states_der );
		std::free( event_indicators );
		std::free( event_indicators_last );
		std::free( var_list );
		std::free( der_list );
//		if ( fmu ) fmi2_import_destroy_dllfmu( fmu );
//		if ( fmu ) fmi2_import_free( fmu );
//		if ( context ) fmi_import_free_context( context );
	}

public: // Methods

	// Initialize
	void
	init( std::string const & path );

	// Instantiation
	void
	instantiate();

	// Pre-Simulation Setup
	void
	pre_simulate();

public: // Data

	std::string name;
	std::string unzip_dir;
	fmi2_import_t * fmu = nullptr; // FMU pointer
	fmi2_real_t * states = nullptr;
	fmi2_real_t * states_der = nullptr;
	fmi2_real_t * event_indicators = nullptr;
	fmi2_real_t * event_indicators_last = nullptr;
	fmi_import_context_t * context = nullptr;
	fmi2_import_variable_list_t * var_list = nullptr;
	fmi2_import_variable_list_t * der_list = nullptr;
	std::vector< fmi2_value_reference_t > var_refs;
	std::vector< fmi2_value_reference_t > inp_var_refs;
	std::vector< fmi2_value_reference_t > out_var_refs;

	FMU_Generator fmu_generator;

	jm_callbacks callbacks;
	fmi2_callback_functions_t callBackFunctions;

	size_type n_states = 0;
	size_type n_event_indicators = 0;

};

// Globals
extern FMU_QSS fmu_qss;

} // fmu
} // QSS

#endif
