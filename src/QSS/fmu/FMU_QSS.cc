// QSS FMU-QSS Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2019 Objexx Engineering, Inc. All rights reserved.
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

// QSS Headers
#include <QSS/fmu/FMU_QSS.hh>
#include <QSS/path.hh>
#include <QSS/string.hh>

// C++ Headers
#include <cassert>
#include <iostream>

namespace QSS {
namespace fmu {

	// Destructor
	FMU_QSS::
	~FMU_QSS()
	{
		std::free( states );
		std::free( derivatives );
		std::free( event_indicators );
		std::free( event_indicators_last );
		std::free( var_list );
		std::free( der_list );
		if ( fmu ) fmi2_import_free( fmu );
		if ( context ) fmi_import_free_context( context );
	}

	// FMU Resource Location URI
	std::string
	FMU_QSS::
	fmuResourceLocation() const
	{
		std::string uri( path::uri( unzip_dir ) );
#ifdef _WIN32
		if ( ( uri.length() > 2 ) && ( uri[ 0 ] == '/' ) && ( uri[ 1 ] == '/' ) ) uri.erase( 0, 2 ); // UNC path: Avoid file:////
#endif
		return "file://" + uri + "/resources";
	}

	// Initialize
	void
	FMU_QSS::
	init( std::string const & path )
	{
		// Check FMU file name form
		if ( ! has_suffix( path, "_QSS.fmu" ) ) {
			std::cerr << "FMU-QSS name is not of the form <model>_QSS.fmu: " << path << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Set up callbacks and context
		callbacks.malloc = std::malloc;
		callbacks.calloc = std::calloc;
		callbacks.realloc = std::realloc;
		callbacks.free = std::free;
		callbacks.logger = jm_default_logger;
		callbacks.log_level = jm_log_level_warning;
		callbacks.context = 0;
		context = fmi_import_allocate_context( &callbacks );

		// Unzip the FMU-QSS in a temporary directory
		name = path::base( path );
		unzip_dir = path::tmp + path::sep + name; //Do Randomize the path to avoid collisions
		if ( ! path::make_dir( unzip_dir ) ) {
			std::cerr << "\nError: FMU-QSS unzip directory creation failed: " << unzip_dir << std::endl;
			std::exit( EXIT_FAILURE );
		}
		fmi_version_enu_t const fmi_version( fmi_import_get_fmi_version( context, path.c_str(), unzip_dir.c_str() ) );
		if ( fmi_version != fmi_version_2_0_enu ) {
			std::cerr << "\nError: FMU-QSS is not FMI 2.0" << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Parse the XML
		fmi2_xml_callbacks_t * xml_callbacks( nullptr );
		fmu = fmi2_import_parse_xml( context, unzip_dir.c_str(), xml_callbacks );
		if ( !fmu ) {
			std::cerr << "\nError: FMU-QSS XML parsing error" << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Check model name form
		name = fmi2_import_get_model_name( fmu );
		if ( ! has_suffix( name, "_QSS" ) ) {
			std::cerr << "FMU-QSS model name is not of the form <model>_QSS: " << name << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Check FMU-QSS is ME
		if ( fmi2_import_get_fmu_kind( fmu ) == fmi2_fmu_kind_cs ) {
			std::cerr << "\nError: FMU-QSS is CS not ME" << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Initialize the FMU-ME object
		std::string const fmu_me_name( name, 0, name.length() - 4 ); // Strip off _QSS
		std::string const fmu_me_path( unzip_dir + path::sep + std::string( "resources" ) + path::sep + fmu_me_name + ".fmu" );
		fmu_me.init( fmu_me_path );

		// Load the FMU-QSS library
		callBackFunctions.logger = fmi2_log_forwarding;
		callBackFunctions.allocateMemory = std::calloc;
		callBackFunctions.freeMemory = std::free;
		callBackFunctions.componentEnvironment = fmu;
		if ( fmi2_import_create_dllfmu( fmu, fmi2_fmu_kind_me, &callBackFunctions ) == jm_status_error ) {
			std::cerr << "\nError: Could not create the FMU-QSS library loading mechanism" << std::endl;
			std::exit( EXIT_FAILURE );
		}
	}

	// Pre-Instantiation for 3rd-Party Master Algorithms
	void
	FMU_QSS::
	pre_instantiate( std::string const & fmuLocation )
	{
		unzip_dir = fmuLocation;

		// Set up callbacks and context
		callbacks.malloc = std::malloc;
		callbacks.calloc = std::calloc;
		callbacks.realloc = std::realloc;
		callbacks.free = std::free;
		callbacks.logger = jm_default_logger;
		callbacks.log_level = jm_log_level_warning;
		callbacks.context = 0;
		context = fmi_import_allocate_context( &callbacks );

		// Parse the XML
		fmi2_xml_callbacks_t * xml_callbacks( nullptr );
		fmu = fmi2_import_parse_xml( context, unzip_dir.c_str(), xml_callbacks );
		if ( !fmu ) {
			std::cerr << "\nError: FMU-QSS XML parsing error" << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Check model name form
		name = fmi2_import_get_model_name( fmu );
		if ( ! has_suffix( name, "_QSS" ) ) {
			std::cerr << "FMU-QSS model name is not of the form <model>_QSS: " << name << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Check FMU is ME
		if ( fmi2_import_get_fmu_kind( fmu ) == fmi2_fmu_kind_cs ) {
			std::cerr << "\nError: FMU-QSS is CS not ME" << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Initialize the FMU-ME object
		std::string const fmu_me_name( name, 0, name.length() - 4 ); // Strip off _QSS
		std::string const fmu_me_path( unzip_dir + path::sep + std::string( "resources" ) + path::sep + fmu_me_name + ".fmu" );
		fmu_me.init( fmu_me_path );

		// Load the FMU-QSS library
		callBackFunctions.logger = fmi2_log_forwarding;
		callBackFunctions.allocateMemory = std::calloc;
		callBackFunctions.freeMemory = std::free;
		callBackFunctions.componentEnvironment = fmu;
		if ( fmi2_import_create_dllfmu( fmu, fmi2_fmu_kind_me, &callBackFunctions ) == jm_status_error ) {
			std::cerr << "\nError: Could not create the FMU-QSS library loading mechanism" << std::endl;
			std::exit( EXIT_FAILURE );
		}
	}

	// Instantiation
	void
	FMU_QSS::
	instantiate()
	{
		// Get generation tool
		std::string const fmu_generation_tool( fmi2_import_get_generation_tool( fmu ) );
		std::cout << "\nFMU-QSS generated by " << fmu_generation_tool << std::endl;
		fmu_generator = (
		 fmu_generation_tool.find( "JModelica" ) == 0u ? FMU_Generator::JModelica : (
		 fmu_generation_tool.find( "Dymola" ) == 0u ? FMU_Generator::Dymola : FMU_Generator::Other )
		);

		// Check SI units
		fmi2_import_unit_definitions_t * unit_defs( fmi2_import_get_unit_definitions( fmu ) );
		if ( unit_defs != nullptr ) {
			size_type const n_units( fmi2_import_get_unit_definitions_number( unit_defs ) );
			std::cout << n_units << " units defined" << std::endl;
			//bool units_error( false );
			for ( size_type i = 0; i < n_units; ++i ) {
				fmi2_import_unit_t * unit( fmi2_import_get_unit( unit_defs, static_cast< unsigned >( i ) ) );
				if ( unit != nullptr ) {
					double const scl( fmi2_import_get_SI_unit_factor( unit ) );
					double const del( fmi2_import_get_SI_unit_offset( unit ) );
					if ( ( scl != 1.0 ) || ( del != 0.0 ) ) {
						std::cerr << "\nError: Non-SI unit present: " << fmi2_import_get_unit_name( unit ) << std::endl;
						//units_error = true;
					}
				}
			}
			//if ( units_error ) std::exit( EXIT_FAILURE ); // Not a fatal error since some non-SI units don't affect integration
		}

		n_states = fmi2_import_get_number_of_continuous_states( fmu );
		if ( n_states > 0u ) {
			std::cerr << "\nError: FMU-QSS has " << n_states << " continuous states instead of zero" << std::endl;
			std::exit( EXIT_FAILURE );
		}
		n_event_indicators = fmi2_import_get_number_of_event_indicators( fmu );
		if ( n_event_indicators > 0u ) {
			std::cerr << "\nError: FMU-QSS has " << n_event_indicators << " event indicators instead of zero" << std::endl;
			std::exit( EXIT_FAILURE );
		}

		states = (fmi2_real_t*)std::calloc( n_states, sizeof( double ) );
		derivatives = (fmi2_real_t*)std::calloc( n_states, sizeof( double ) );
		event_indicators = (fmi2_real_t*)std::calloc( n_event_indicators, sizeof( double ) );
		event_indicators_last = (fmi2_real_t*)std::calloc( n_event_indicators, sizeof( double ) );
	}

	// Pre-Simulation Setup
	void
	FMU_QSS::
	pre_simulate()
	{
		// Create collections of variables and their value references
		var_list = fmi2_import_get_variable_list( fmu, 0 ); // sort order = 0 for original order
		size_type const n_vars( fmi2_import_get_variable_list_size( var_list ) );
		fmi2_value_reference_t const * vrs( fmi2_import_get_value_referece_list( var_list ) ); // reference is misspelled in FMIL API
		var_refs.clear();
		inp_var_refs.clear();
		out_var_refs.clear();
		for ( size_type i = 0; i < n_vars; ++i ) {
			fmi2_import_variable_t * var( fmi2_import_get_variable( var_list, i ) );
			fmi2_causality_enu_t const var_causality( fmi2_import_get_causality( var ) );
			var_refs.push_back( vrs[ i ] );
			if ( var_causality == fmi2_causality_enu_input ) {
				inp_var_refs.push_back( vrs[ i ] );
			} else if ( var_causality == fmi2_causality_enu_output ) {
				out_var_refs.push_back( vrs[ i ] );
			}
		}
		fmu_me.out_var_refs = out_var_refs;
	}

	// Unzip Location from FMU Resource Location URI
	std::string
	FMU_QSS::
	unzip_loc( std::string const & uri )
	{
		std::string unz( uri );
		if ( has_suffix( unz, "/resources" ) ) unz.erase( unz.length() - 10 );
		unz = path::un_uri( unz );
		return unz;
	}

} // fmu
} // QSS
