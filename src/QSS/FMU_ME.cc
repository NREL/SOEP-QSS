// FMU-ME Class
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

// QSS Headers
#include <QSS/FMU_ME.hh>
#include <QSS/annotation.hh>
#include <QSS/BinOptimizer.hh>
#include <QSS/clusters.hh>
#include <QSS/Conditional.hh>
#include <QSS/container.hh>
#include <QSS/cpu_time.hh>
#include <QSS/cycles.hh>
#include <QSS/EventIndicators.hh>
#include <QSS/Function_Inp_constant.hh>
#include <QSS/Function_Inp_sin.hh>
#include <QSS/Function_Inp_step.hh>
#include <QSS/Function_Inp_toggle.hh>
#include <QSS/Handlers.hh>
#include <QSS/math.hh>
#include <QSS/options.hh>
#include <QSS/path.hh>
#include <QSS/Range.hh>
#include <QSS/string.hh>
#include <QSS/Timers.hh>
#include <QSS/Triggers.hh>
#include <QSS/Triggers_ZC.hh>
#include <QSS/Triggers_R.hh>
#include <QSS/Variable_all.hh>
#include <QSS/Variable_name_decoration.hh>

// FMI Library Headers
#include <FMI2/fmi2_xml_variable.h>

// OpenMP Headers
#ifdef _OPENMP
#include <omp.h>
#endif

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <utility>

namespace QSS {

	// Default Constructor
	FMU_ME::
	FMU_ME() :
	 eventq( new EventQ() )
	{}

	// FMU-ME Path Constructor
	FMU_ME::
	FMU_ME( std::string const & path ) :
	 eventq( new EventQ() )
	{
		initialize( path, false );
	}

	// FMU-ME Path + Event Queue Constructor
	FMU_ME::
	FMU_ME( std::string const & path, EventQ * eventq_ ) :
	 eventq( eventq_ ),
	 eventq_own( false )
	{
		initialize( path, false );
	}

	// Destructor
	FMU_ME::
	~FMU_ME()
	{
		cleanup();
		std::free( states );
		std::free( derivatives );
		std::free( x_nominal );
		std::free( event_indicators );
		std::free( event_indicators_last );
		std::free( var_list );
		std::free( der_list );
		if ( fmu ) fmi2_import_free( fmu );
		if ( context ) fmi_import_free_context( context );
		for ( Variable * var : vars ) delete var;
		for ( Conditional< Variable_ZC > * con : cons ) delete con;
		for ( auto & f_out : f_outs ) f_out.flush();
		for ( auto & l_out : l_outs ) l_out.flush();
		for ( auto & k_out : k_qss_outs ) k_out.flush();
		if ( eventq_own ) delete eventq;
	}

	// Variable Lookup by Name (for Testing)
	Variable const *
	FMU_ME::
	var_named( std::string const & var_name ) const
	{
		for ( Variable const * var : vars ) {
			if ( var->name() == var_name ) return var;
		}
		return nullptr; // Not found
	}

	// Variable Lookup by Name (for Testing)
	Variable *
	FMU_ME::
	var_named( std::string const & var_name )
	{
		for ( Variable * var : vars ) {
			if ( var->name() == var_name ) return var;
		}
		return nullptr; // Not found
	}

	// Initialize
	void
	FMU_ME::
	initialize( std::string const & path, bool const in_place )
	{
		if ( !has_suffix( path, ".fmu" ) ) {
			std::cerr << "\nFMU-ME name is not of the form <model>.fmu" << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Set up FMU callbacks and context
		callbacks.malloc = std::malloc;
		callbacks.calloc = std::calloc;
		callbacks.realloc = std::realloc;
		callbacks.free = std::free;
		callbacks.logger = jm_default_logger;
		switch ( options::log ) {
		case options::LogLevel::fatal:
			callbacks.log_level = jm_log_level_fatal;
			break;
		case options::LogLevel::error:
			callbacks.log_level = jm_log_level_error;
			break;
		case options::LogLevel::warning:
			callbacks.log_level = jm_log_level_warning;
			break;
		case options::LogLevel::info:
			callbacks.log_level = jm_log_level_info;
			break;
		case options::LogLevel::verbose:
			callbacks.log_level = jm_log_level_verbose;
			break;
		case options::LogLevel::debug:
			callbacks.log_level = jm_log_level_debug;
			break;
		case options::LogLevel::all:
			callbacks.log_level = jm_log_level_all;
			break;
		}
		callbacks.context = 0;
		context = fmi_import_allocate_context( &callbacks );

		// Check FMU-ME exists and is FMI 2.0
		if ( !path::is_file( path ) ) {
			std::cerr << "\nError: FMU file not found: " << path << std::endl;
			std::exit( EXIT_FAILURE );
		}
		name = path::base( path );
		std::cout << '\n' + name + " Initialization =====" << std::endl;

		// Set unzip directory
		if ( in_place ) { // Use FMU directory
			unzip_dir = path::dir( path );
		} else { // Use temporary directory
			unzip_dir = path::tmp + path::sep + name; //Do Randomize the path to avoid collisions
			if ( !path::make_dir( unzip_dir ) ) {
				std::cerr << "\nError: FMU-ME unzip directory creation failed: " << unzip_dir << std::endl;
				std::exit( EXIT_FAILURE );
			}
		}

		// Get FMU's FMI version
		fmi_version_enu_t const fmi_version( fmi_import_get_fmi_version( context, path.c_str(), unzip_dir.c_str() ) );
		if ( fmi_version != fmi_version_2_0_enu ) {
			std::cerr << "\nError: FMU-ME is not FMI 2.0" << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Parse the XML: Set up EventIndicators and Dependencies data structures
		all_eventindicators.emplace_back( this );
		all_dependencies.emplace_back( this );
		fmu = fmi2_import_parse_xml( context, unzip_dir.c_str(), &xml_callbacks );
		if ( !fmu ) {
			std::cerr << "\nError: FMU-ME XML parsing error" << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Check FMU-ME is ME
		if ( fmi2_import_get_fmu_kind( fmu ) == fmi2_fmu_kind_cs ) {
			std::cerr << "\nError: FMU-ME is CS not ME" << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Load the FMU-ME library
		callBackFunctions.logger = fmi2_log_forwarding;
		callBackFunctions.allocateMemory = std::calloc;
		callBackFunctions.freeMemory = std::free;
		callBackFunctions.componentEnvironment = fmu;
		if ( fmi2_import_create_dllfmu( fmu, fmi2_fmu_kind_me, &callBackFunctions ) == jm_status_error ) {
			std::cerr << "\nError: Could not create the FMU-ME library loading mechanism" << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Get/check generation tool
		std::string const fmu_generation_tool( fmi2_import_get_generation_tool( fmu ) );
		std::cout << '\n' + name + " FMU-ME generated by " << fmu_generation_tool << std::endl;
		fmu_generator = (
		 fmu_generation_tool.find( "Optimica" ) == 0u ? FMU_Generator::Optimica : (
		 fmu_generation_tool.find( "JModelica" ) == 0u ? FMU_Generator::JModelica : (
		 fmu_generation_tool.find( "Dymola" ) == 0u ? FMU_Generator::Dymola : FMU_Generator::Other ) )
		);
		if ( fmu_generator != FMU_Generator::Optimica ) {
			std::cerr << "\nError: Only Optimica-generated FMU-MEs are supported by QSS" << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Check FMU supports directional derivatives
		if ( !bool( fmi2_import_get_capability( fmu, fmi2_me_providesDirectionalDerivatives ) ) ) { // FMU doesn't support directional derivatives
			std::cerr << "\nError: " + name + " FMU-ME was not built with directional derivative support" << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Check SI units
		fmi2_import_unit_definitions_t * unit_defs( fmi2_import_get_unit_definitions( fmu ) );
		if ( unit_defs != nullptr ) {
			size_type const n_units( fmi2_import_get_unit_definitions_number( unit_defs ) );
			std::cout << "\nUnits: " << n_units << " units defined" << std::endl;
//			for ( size_type i = 0; i < n_units; ++i ) {
//				fmi2_import_unit_t * unit( fmi2_import_get_unit( unit_defs, static_cast< unsigned >( i ) ) );
//				if ( unit != nullptr ) {
//					std::string const unit_name( fmi2_import_get_unit_name( unit ) );
//					double const unit_scl( fmi2_import_get_SI_unit_factor( unit ) );
//					double const unit_del( fmi2_import_get_SI_unit_offset( unit ) );
//					unsigned int const unit_dun( fmi2_import_get_unit_display_unit_number( unit ) );
//					std::cout << unit_name << "  Scale: " << unit_scl << "  Offset: " << unit_del << "  Display unit: " << unit_dun << std::endl;
//					if ( ( unit_scl != 1.0 ) || ( unit_del != 0.0 ) ) {
//						std::cerr << "\nWarning: Non-SI unit present: " << unit_name << std::endl;
//					}
//				}
//			}
		}

		// Get sizes
		n_states = fmi2_import_get_number_of_continuous_states( fmu );
		std::cout << n_states << " continuous state variables" << std::endl;
		n_event_indicators = fmi2_import_get_number_of_event_indicators( fmu );
		std::cout << n_event_indicators << " event indicators" << std::endl;

		// Allocate arrays
		states = (fmi2_real_t*)std::calloc( n_states, sizeof( double ) );
		derivatives = (fmi2_real_t*)std::calloc( n_states, sizeof( double ) );
		x_nominal = (fmi2_real_t*)std::calloc( n_states, sizeof( double ) );
		event_indicators = (fmi2_real_t*)std::calloc( n_event_indicators, sizeof( double ) );
		event_indicators_last = (fmi2_real_t*)std::calloc( n_event_indicators, sizeof( double ) );
	}

	// Instantiate FMU
	void
	FMU_ME::
	instantiate()
	{
		// Instantiate the FMU
		if ( fmi2_import_instantiate( fmu, "FMU-ME model instance", fmi2_model_exchange, 0, 0 ) == jm_status_error ) {
			std::cerr << "\nError: fmi2_import_instantiate failed" << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Set FMU debug logging
		if ( options::log >= options::LogLevel::debug ) {
			fmi2_import_set_debug_logging( fmu, fmi2_true, 0, 0 );
		} else {
			fmi2_import_set_debug_logging( fmu, fmi2_false, 0, 0 );
		}

		// Get/report FMU run specs
		fmi2_real_t const tstart( fmi2_import_get_default_experiment_start( fmu ) ); // [0.0]
		fmi2_real_t const tstop( options::specified::tEnd ? options::tEnd : fmi2_import_get_default_experiment_stop( fmu ) ); // [1.0]
		fmi2_real_t const relativeTolerance( fmi2_import_get_default_experiment_tolerance( fmu ) ); // [0.0001]
		fmi2_boolean_t const toleranceControlled( fmi2_false ); // FMIL says tolerance control not supported for ME
		fmi2_boolean_t const stopTimeDefined( fmi2_true );
		std::cout << "\nSimulation Time Range:  Start: " << tstart << "  Stop: " << tstop << std::endl;
		if ( tstart > tstop ) {
			std::cerr << "\nError: Start Time > Stop Time" << std::endl;
			std::exit( EXIT_FAILURE );
		}
		std::cout << "\nRelative Tolerance in FMU-ME: " << relativeTolerance << std::endl;
		if ( fmi2_import_setup_experiment( fmu, toleranceControlled, relativeTolerance, tstart, stopTimeDefined, tstop ) >= fmi2_status_error ) {
			std::cerr << "\nError: fmi2_import_setup_experiment failed" << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// Mandatory FMU initialization mode
		fmi2_import_enter_initialization_mode( fmu );
		fmi2_import_exit_initialization_mode( fmu );

		// QSS time and tolerance run controls
		t0 = tstart; // Simulation start time
		tE = tstop; // Simulation end time
		rTol = relativeTolerance;
	}

	// Options Setup
	void
	FMU_ME::
	set_options(
	 Real const tBeg,
	 Real const tEnd,
	 Real const rTolerance
	)
	{
		t0 = tBeg;
		tE = tEnd;
		options::dtMin = std::max( options::dtMin, 2.0 * std::numeric_limits< Time >::epsilon() * std::max( std::abs( t0 ), std::abs( tE ) ) ); // Prevent t + dt == t
		options::dtMax = std::max( options::dtMax, options::dtMin );
		rTol = rTolerance;
	}

	// Pre-Simulation Setup
	void
	FMU_ME::
	pre_simulate()
	{
		// Types
		using Variables = Variable::Variables;
		using size_type = Variables::size_type;
		using Time = Variable::Time;
		using Real = Variable::Real;
		using Name = std::string;
		using Var_Names = std::vector< Name >;
		using Function = std::function< SmoothToken ( Time const ) >;

		// I/o setup
		std::cout << std::setprecision( 16 );
		std::cerr << std::setprecision( 16 );
		output_filter = OutputFilter( options::var );

		// Report QSS method
		if ( options::qss == options::QSS::QSS1 ) {
			std::cout << "\nQSS Method: QSS1" << std::endl;
		} else if ( options::qss == options::QSS::QSS2 ) {
			std::cout << "\nQSS Method: QSS2" << std::endl;
		} else if ( options::qss == options::QSS::QSS3 ) {
			std::cout << "\nQSS Method: QSS3" << std::endl;
		} else if ( options::qss == options::QSS::LIQSS1 ) {
			std::cout << "\nQSS Method: LIQSS1" << std::endl;
		} else if ( options::qss == options::QSS::LIQSS2 ) {
			std::cout << "\nQSS Method: LIQSS2" << std::endl;
		} else if ( options::qss == options::QSS::LIQSS3 ) {
			std::cout << "\nQSS Method: LIQSS3" << std::endl;
		} else if ( options::qss == options::QSS::xQSS1 ) {
			std::cout << "\nQSS Method: xQSS1" << std::endl;
		} else if ( options::qss == options::QSS::xQSS2 ) {
			std::cout << "\nQSS Method: xQSS2" << std::endl;
		} else if ( options::qss == options::QSS::xQSS3 ) {
			std::cout << "\nQSS Method: xQSS3" << std::endl;
		} else {
			std::cerr << "\nError: Unsupported QSS method" << std::endl;
			std::exit( EXIT_FAILURE );
		}

		// QSS time and tolerance run controls
		t = t0; // Simulation current time
		options::dtOut_set( tE - t0 ); // Set dtOut to default if not specified
		tOut = t0 + options::dtOut; // Sampling time
		iOut = 1u; // Output step index
		if ( !options::specified::rTol ) options::rTol = rTol; // Quantization relative tolerance (FMU doesn't have an absolute tolerance)
		std::cout << "Relative Tolerance: " << options::rTol << std::endl;
		if ( options::specified::aTol ) {
			std::cout << "Absolute Tolerance: " << options::aTol << std::endl;
		} else if ( options::aFac == 1.0 ) {
			std::cout << "Absolute Tolerances: " << options::rTol << " * nominal value" << std::endl;
		} else {
			std::cout << "Absolute Tolerances: " << options::rTol << " * " << options::aFac << " * nominal value" << std::endl;
		}

		// Report numeric differentiation time step
		std::cout << "\nNumeric differentiation time step: " << options::dtND << " (s)" << ( options::dtND_optimizer ? " before optimization" : "" ) << std::endl;

		// FMU event info initialization
		eventInfo.newDiscreteStatesNeeded = fmi2_false;
		eventInfo.terminateSimulation = fmi2_false;
		eventInfo.nominalsOfContinuousStatesChanged = fmi2_false;
		eventInfo.valuesOfContinuousStatesChanged = fmi2_true;
		eventInfo.nextEventTimeDefined = fmi2_false;
		eventInfo.nextEventTime = -0.0;

		// FMU pre-simulation calls
		fmi2_import_enter_continuous_time_mode( fmu );
		fmi2_import_enter_event_mode( fmu );
		do_event_iteration(); // This bumps the event indicators a bit
		fmi2_import_enter_continuous_time_mode( fmu );
		fmi2_import_get_continuous_states( fmu, states, n_states ); // Should get initial values
		fmi2_import_get_nominals_of_continuous_states( fmu, x_nominal, n_states ); // Should return 1 for variables with no nominal value specified
		fmi2_import_get_event_indicators( fmu, event_indicators, n_event_indicators );
		// if ( options::output::d ) {
		// 	std::cout << "FMU event indicators: Pre-simulation:" << std::endl;
		// 	for ( size_type k = 0; k < n_event_indicators; ++k ) std::cout << event_indicators[ k ] << std::endl;
		// }

		// FMU Query: Model
		std::cout << "\nModel name: " << fmi2_import_get_model_name( fmu ) << std::endl;
		std::cout << "Model identifier: " << fmi2_import_get_model_identifier_ME( fmu ) << std::endl;

		// Collections
		Var_Names var_names; // Variable names (to check for duplicates)

		// FMU variable list
		var_list = fmi2_import_get_variable_list( fmu, 0 ); // sort order = 0 for original order
		size_type const n_fmu_vars( fmi2_import_get_variable_list_size( var_list ) );
		fmu_variables.clear();
		fmu_variables.reserve( n_fmu_vars );
		// fmi2_value_reference_t const * vrs( fmi2_import_get_value_referece_list( var_list ) ); // reference is misspelled in FMIL API

		// Set up FMU variable spec array
		std::cout << "\nFMU Variable Processing: " << n_fmu_vars << " Variables =====" << std::endl;
		std::unordered_map< fmi2_import_real_variable_t *, fmi2_import_variable_t * > fmu_real_to_var_map; // FMU real to generic variable pointer map
		for ( size_type i = 0; i < n_fmu_vars; ++i ) {
			size_type const idx( i + 1 ); // FMU variable index
			fmi2_import_variable_t * var( fmi2_import_get_variable( var_list, i ) );
			std::string const var_name( fmi2_import_get_variable_name( var ) );
			var_names.push_back( var_name );
			fmi2_value_reference_t const var_ref( fmi2_import_get_variable_vr( var ) );
			var_name_ref[ var_name ] = var_ref;
			fmi2_base_type_enu_t const var_base_type( fmi2_import_get_variable_base_type( var ) );
			switch ( var_base_type ) {
			case fmi2_base_type_real: // Real
				fmu_variables.emplace_back( idx, var, fmi2_import_get_variable_as_real( var ) );
				fmu_real_to_var_map[ fmi2_import_get_variable_as_real( var ) ] = var;
				break;
			case fmi2_base_type_int: // Integer
				fmu_variables.emplace_back( idx, var, fmi2_import_get_variable_as_integer( var ) );
				break;
			case fmi2_base_type_bool: // Boolean
				fmu_variables.emplace_back( idx, var, fmi2_import_get_variable_as_boolean( var ) );
				break;
			case fmi2_base_type_str: // String
				fmu_variables.emplace_back( idx, var, fmi2_import_get_variable_as_string( var ) );
				break;
			case fmi2_base_type_enum: // Enum
				fmu_variables.emplace_back( idx, var, fmi2_import_get_variable_as_enum( var ) );
				break;
			default:
				std::cerr << " Error: Unsupported value type in FMU variable: " << var_name << std::endl;
				std::exit( EXIT_FAILURE );
				break;
			}
		}

		// FMU Event Indicator Processing
		std::cout << "\nFMU Event Indicator Processing =====" << std::endl;
		size_type n_ZC_vars( 0 );
		has_event_indicators = false;
		auto const ieis( std::find_if( all_eventindicators.begin(), all_eventindicators.end(), [this]( FMU_EventIndicators const & feis ){ return feis.context == this; } ) );
		if ( ieis == all_eventindicators.end() ) {
			std::cerr << "\nError: FMU event indicators collection lookup failed for FMU-ME " << name << std::endl;
			std::exit( EXIT_FAILURE );
		}
		for ( EventIndicator & ei : ieis->event_indicators ) {
			FMU_Variable & ei_var( fmu_variables[ ei.index - 1 ] );
			ei_var.to_EventIndicator(); // Mark FMU variable as an event indicator
			fmi2_import_variable_t * var( ei_var.var ); // == fmi2_import_get_variable( var_list, ei.index - 1 )
			std::string const var_name( ei_var.name() ); // == fmi2_import_get_variable_name( var )
			if ( ( fmi2_import_get_variable_base_type( var ) == fmi2_base_type_real ) && ( fmi2_import_get_variability( var ) == fmi2_variability_enu_continuous ) ) {
				fmi2_import_real_variable_t * var_real( fmi2_import_get_variable_as_real( var ) );
				if ( !SI_unit_check( fmi2_import_get_real_variable_unit( var_real ) ) ) { // May not be necessary: LBL preference
					std::cerr << " Error: Non-SI unit used for event indicator variable: Not currently supported: " << var_name << std::endl;
					//std::exit( EXIT_FAILURE );
				}
				std::cout << "\nEvent Indicator: " << var_name << std::endl;
				++n_ZC_vars;
				has_event_indicators = true;
			} else {
				std::cerr << "\nError: FMU event indicator variable is not real-valued and continuous: " << var_name << std::endl;
				//std::cerr << "       Termination supppressed pending OCT update: Results may be invalid!" << std::endl; //OCT std::exit( EXIT_FAILURE );
				std::exit( EXIT_FAILURE );
			}
		}
		if ( has_event_indicators ) {
			std::cout << "\n" << n_ZC_vars << " event indicator" << ( n_ZC_vars > 1u ? "s" : "" )  << " present" << std::endl;
			std::cout << " Directional derivatives used for event indicator first derivative" << std::endl;
			std::cout << "\nZero Crossing Tolerance: zTol = " << options::zTol << std::endl;
			std::cout << "\nZero Crossing Tolerance Bump Multiplier: zMul = " << options::zMul << std::endl;
			std::cout << "\nZero Crossing Time Step: dtZC = " << options::dtZC << " (s)" << std::endl;
		}
		if ( fmu_generator == FMU_Generator::Dymola ) {
			if ( n_event_indicators != 2 * n_ZC_vars ) { // Dymola has 2x as many event indicators
				std::cerr << "\nWarning: Number of FMU-ME event indicators (" << n_event_indicators << ") is not equal to twice the number of zero-crossing variables found (" << n_ZC_vars << ") as expected for Dymola FMUs" << std::endl;
			}
		} else if ( n_event_indicators != n_ZC_vars ) {
			std::cerr << "\nWarning: Number of FMU-ME event indicators (" << n_event_indicators << ") is not equal to the number of zero-crossing variables found (" << n_ZC_vars << ')' << std::endl;
		}

		// FMU Dependencies Retrieval
		std::cout << "\nFMU Dependencies Retrieval =====" << std::endl;
		auto const ideps( std::find_if( all_dependencies.begin(), all_dependencies.end(), [this]( FMU_Dependencies const & fdeps ){ return fdeps.context == this; } ) );
		if ( ideps == all_dependencies.end() ) {
			std::cerr << "\nError: FMU dependencies collection lookup failed for FMU-ME " << name << std::endl;
			std::exit( EXIT_FAILURE );
		}
		FMU_Dependencies & fmu_dependencies( *ideps );
		if ( !fmu_dependencies.empty() ) { // Report dependencies from XML <Dependencies> annotation section
			std::cout << "\nDependencies:" << std::endl;
			for ( FMU_Dependencies::Variables::value_type const & idx_var : fmu_dependencies.variables ) {
				if ( ( idx_var.first <= 0 ) || ( idx_var.first > n_fmu_vars ) ) {
					std::cerr << "\nError: Dependencies specified for non-existent variable index: " << idx_var.first << std::endl;
					std::exit( EXIT_FAILURE );
				}
				std::cout << ' ' << idx_var.first << " -> ";
				for ( FMU_Dependencies::Index const & idx_observee : idx_var.second.observees ) {
					std::cout << ' ' << idx_observee;
					if ( ( idx_observee <= 0 ) || ( idx_observee > n_fmu_vars ) ) {
						std::cerr << "\nError: Non-existent dependency variable index: " << idx_observee << std::endl;
						std::exit( EXIT_FAILURE );
					}
				}
				std::cout << '\n';
			}
		}

		// FMU Derivative Processing
		der_list = fmi2_import_get_derivatives_list( fmu );
		n_derivatives = fmi2_import_get_variable_list_size( der_list );
		std::cout << "\nFMU Derivative Processing: " << n_derivatives << " Derivatives =====" << std::endl;
		// fmi2_value_reference_t const * drs( fmi2_import_get_value_referece_list( der_list ) ); // reference is spelled wrong in FMIL API
		std::unordered_multimap< fmi2_value_reference_t, FMU_Variable::Index > fmu_ref_to_state; // Reference to FMU_Variable map to check for duplicate state variable reference numbers
		for ( size_type i = 0; i < n_derivatives; ++i ) { // i is also index into states and x_nominal arrays
			fmi2_import_variable_t * der( fmi2_import_get_variable( der_list, i ) );
			std::string const der_name( fmi2_import_get_variable_name( der ) );
			Index const der_idx( fmi2_import_get_variable_original_order( der ) + 1u ); // fmi2_import_get_variable_original_order returns 0-based index
			fmi2_base_type_enu_t const der_base_type( fmi2_import_get_variable_base_type( der ) );
			switch ( der_base_type ) {
			case fmi2_base_type_real:
				{
				fmi2_import_real_variable_t * der_real( fmi2_import_get_variable_as_real( der ) );
				fmi2_import_real_variable_t * var_real( fmi2_import_get_real_variable_derivative_of( der_real ) );
				fmi2_import_variable_t * var( fmu_real_to_var_map[ var_real ] );
				std::string const var_name( fmi2_import_get_variable_name( var ) );
				Index const var_idx( fmi2_import_get_variable_original_order( var ) + 1u ); // fmi2_import_get_variable_original_order returns 0-based index
				if ( var_real != nullptr ) { // Add to Variable to Derivative Map
					FMU_Variable & fmu_der( fmu_variables[ der_idx - 1 ] );
					assert( der_idx == fmu_der.idx );
					FMU_Variable & fmu_var( fmu_variables[ var_idx - 1 ] );
					std::cout << "\n Derivative:" << std::endl;
					std::cout << "  Index: " << der_idx << std::endl;
					std::cout << "  Name: " << der_name << std::endl;
					std::cout << " State:" << std::endl;
					std::cout << "  Index: " << var_idx << std::endl;
					std::cout << "  Name: " << var_name << std::endl;
					assert( var_idx == fmu_var.idx );
					fmu_der.ids = fmu_var.idx; // Derivative's state index
					fmu_var.idd = fmu_der.idx; // State's derivative index
					fmu_der.isa = fmu_var.isa = i; // State/deriviative/nominal array index
					fmu_ref_to_state.insert( std::pair( fmu_var.ref(), fmu_var.idx ) );
					fmu_dvrs.add( der_real, fmu_var );
				} else {
					std::cerr << " Error: Associated real-valued variable not found for derivative: " << der_name << std::endl;
					std::exit( EXIT_FAILURE );
				}
				}
				break;
			case fmi2_base_type_int:
				std::cerr << " Error: Derivative value type is integer, not real: " << der_name << std::endl;
				std::exit( EXIT_FAILURE );
				break;
			case fmi2_base_type_bool:
				std::cerr << " Error: Derivative value type is boolean, not real: " << der_name << std::endl;
				std::exit( EXIT_FAILURE );
				break;
			case fmi2_base_type_str:
				std::cerr << " Error: Derivative value type is string, not real: " << der_name << std::endl;
				std::exit( EXIT_FAILURE );
				break;
			case fmi2_base_type_enum:
				std::cerr << " Error: Derivative value type is enum, not real: " << der_name << std::endl;
				std::exit( EXIT_FAILURE );
				break;
			default:
				std::cerr << " Error: Derivative value type is unknown, not real: " << der_name << std::endl;
				std::exit( EXIT_FAILURE );
				break;
			}
		}

		// FMU Dependency Processing
		std::cout << "\nFMU Dependency Processing =====" << std::endl;
		bool derivative_observees( true );
		while ( derivative_observees ) { // Short-circuit dependencies on derivatives (as OCT does in <Derivatives> section of XML): This is not currently efficient!
			derivative_observees = false;
			for ( FMU_Dependencies::Variables::value_type & idx_var : fmu_dependencies.variables ) { // Pair of index and dep::Variable
				dep::Variable & dep_var( idx_var.second );
				dep::Variable::Observees & dep_var_observees( dep_var.observees );
				dep::Variable::Observees new_dep_var_observees;
				for ( dep::Variable::Observees::size_type ioe = 0; ioe < dep_var_observees.size(); ++ioe ) {
					dep::Variable::Index const ide( dep_var_observees[ ioe ] );
					FMU_Variable const & observee_fmu_var( fmu_variables[ ide - 1 ] ); // FMU variable corresponding to the dep::Variable
					if ( observee_fmu_var.is_Derivative() ) { // Observee is a derivative
						assert( dep_var.idx != ide ); // OCT removes derivative self-dependencies
						derivative_observees = true;
						dep_var_observees.erase( std::next( dep_var_observees.begin(), ioe ) ); // Remove the derivative observee
						FMU_Dependencies::iterator i_ide( fmu_dependencies.find( ide ) );
						if ( i_ide != fmu_dependencies.end() ) { // Derivative has dependencies
							dep::Variable::Observees const & der_var_observees( i_ide->second.observees );
							assert( !std::binary_search( der_var_observees.begin(), der_var_observees.end(), ide ) ); // OCT removes derivative self-dependencies
							for ( dep::Variable::Index const der_var_observee_idx : der_var_observees ) new_dep_var_observees.push_back( der_var_observee_idx );
						}
					}
				}
				if ( !new_dep_var_observees.empty() ) { // Merge in derivative dependencies
					uniquify( new_dep_var_observees );
					for ( dep::Variable::Index const new_observee_idx : new_dep_var_observees ) dep_var_observees.push_back( new_observee_idx );
					uniquify( dep_var_observees ); // Sort and remove duplicates
				}
			}
		}
		for ( FMU_Dependencies::Variables::value_type & idx_var : fmu_dependencies.variables ) { // Check for event indicator (direct) dependencies on event indicators
			FMU_Variable const & dep_fmu_var( fmu_variables[ idx_var.first - 1 ] ); // FMU variable corresponding to the dep::Variable
			if ( dep_fmu_var.is_EventIndicator() ) { // Event indicator
				dep::Variable & dep_var( idx_var.second );
				for ( dep::Variable::Index const observee : dep_var.observees ) {
					FMU_Variable const & observee_fmu_var( fmu_variables[ observee - 1 ] ); // FMU variable corresponding to the observee index
					if ( observee_fmu_var.is_EventIndicator() ) { // Observee is an event indicator
						std::cerr << " Note: FMU dependency of event indicator " << dep_fmu_var.name() << " on event indicator " << observee_fmu_var.name() << std::endl;
						// Not an error if a temporary variable was short-circuited such as for EIs generated by integer() calls
					}
				}
			}
		}
		// for ( FMU_Dependencies::Variables::value_type & idx_var : fmu_dependencies.variables ) { // Short-circuit dependencies of event indicators on event indicators: This assumes these dependencies appear when their zero-crossing functions are related
		// 	FMU_Variable const & dep_fmu_var( fmu_variables[ idx_var.first - 1 ] ); // FMU variable corresponding to the dep::Variable
		// 	if ( dep_fmu_var.is_EventIndicator() ) { // Event indicator
		// 		dep::Variable & dep_var( idx_var.second );
		// 		DepIdxSet nei_observees; // Non event indicator observees in dependency subgraph
		// 		DepIdxSet ei_observees; // Event indicator observees in dependency subgraph
		// 		ei_observees.insert( idx_var.first ); // Put observing event indicator in so we skip self-dependency
		// 		subgraph_ei_observees( fmu_dependencies, dep_var.observees, nei_observees, ei_observees );
		// 		dep::Variable::Observees new_dep_var_observees( nei_observees.begin(), nei_observees.end() );
		// 		dep_var.observees = new_dep_var_observees;
		// 	}
		// }
		for ( FMU_Dependencies::Variables::value_type & idx_var : fmu_dependencies.variables ) { // Drill thru dependencies where event indicators depend on event indicators: Temporary hack for OCT EI->EI dependencies that appear when their zero-crossing functions are related
			FMU_Variable const & dep_fmu_var( fmu_variables[ idx_var.first - 1 ] ); // FMU variable corresponding to the dep::Variable
			if ( dep_fmu_var.is_EventIndicator() ) { // Event indicator
				dep::Variable & dep_var( idx_var.second );
				DepIdxSet nei_observees; // Non event indicator observees in dependency subgraph
				DepIdxSet ei_observees; // Event indicator observees in dependency subgraph
				ei_observees.insert( idx_var.first ); // Put observing event indicator in so we skip self-dependency
				subgraph_ei_observees( fmu_dependencies, dep_var.observees, nei_observees, ei_observees );
				for ( auto const idx_observee : nei_observees ) {
					dep_var.observees.push_back( idx_observee ); // This may add duplicates so we uniquify when we finalize
				}
			}
		}
		fmu_dependencies.finalize();
		for ( FMU_Dependencies::Variables::value_type & idx_var : fmu_dependencies.variables ) { // Mark variables with upstream state or event indicator observers
			FMU_Variable const & dep_fmu_var( fmu_variables[ idx_var.first - 1 ] ); // FMU variable corresponding to the dep::Variable
			if ( dep_fmu_var.is_State() || dep_fmu_var.is_Derivative() || dep_fmu_var.is_EventIndicator() ) { // State/Derivative or Event indicator
				dep::Variable & dep_var( idx_var.second );
				mark_downstream_observees( fmu_dependencies, dep_var );
			}
		}

		// QSS Variable Processing
		std::cout << "\nQSS Variable Processing =====" << std::endl;
		for ( size_type i = 0; i < n_fmu_vars; ++i ) {
			FMU_Variable & fmu_var( fmu_variables[ i ] );
			size_type const idx( i + 1 );
			assert( idx == fmu_var.idx );
			fmi2_import_variable_t * var( fmu_var.var );
			fmi2_value_reference_t const var_ref( fmu_var.ref() );
			std::string const var_name( fmu_var.name() );
			bool const var_has_xml_start( fmu_var.has_start() );

			std::cout << "\nVariable  Index: " << idx << " Ref: " << var_ref << std::endl;
			std::cout << " Name: " << var_name << std::endl;
			//std::cout << " FMU Variable: " << fmu_var << std::endl; //Debug/////
			std::cout << " Desc: " << fmu_var.description() << std::endl;

			if ( fmu_var.causality_parameter() ) {
				std::cout << " Causality: Parameter" << std::endl;
			} else if ( fmu_var.causality_calculated_parameter() ) {
				std::cout << " Causality: Calculated Parameter" << std::endl;
			} else if ( fmu_var.causality_input() ) {
				std::cout << " Causality: Input" << std::endl;
			} else if ( fmu_var.causality_output() ) {
				std::cout << " Causality: Output" << std::endl;
			} else if ( fmu_var.causality_local() ) {
				std::cout << " Causality: Local" << std::endl;
			} else if ( fmu_var.causality_independent() ) {
				std::cout << " Causality: Independent" << std::endl;
			} else if ( fmu_var.causality_unknown() ) {
				std::cout << " Causality: Unknown" << std::endl;
			} else {
				std::cout << " Causality: Unspecified" << std::endl;
			}

			if ( fmu_var.variability_constant() ) {
				std::cout << " Variability: Constant" << std::endl;
			} else if ( fmu_var.variability_fixed() ) {
				std::cout << " Variability: Fixed" << std::endl;
			} else if ( fmu_var.variability_tunable() ) {
				std::cout << " Variability: Tunable" << std::endl;
			} else if ( fmu_var.variability_discrete() ) {
				std::cout << " Variability: Discrete" << std::endl;
			} else if ( fmu_var.variability_continuous() ) {
				std::cout << " Variability: Continuous" << std::endl;
			} else if ( fmu_var.variability_unknown() ) {
				std::cout << " Variability: Unknown" << std::endl;
			} else {
				std::cout << " Variability: Unspecified" << std::endl;
			}

			if ( fmu_var.initial_exact() ) {
				std::cout << " Initial: Exact" << std::endl;
			} else if ( fmu_var.initial_approx() ) {
				std::cout << " Initial: Approx" << std::endl;
			} else if ( fmu_var.initial_calculated() ) {
				std::cout << " Initial: Calculated" << std::endl;
			} else if ( fmu_var.initial_unknown() ) {
				std::cout << " Initial: Unknown" << std::endl;
			} else {
				std::cout << " Initial: Unspecified" << std::endl;
			}

			switch ( fmu_var.typ ) {
			case FMU_Variable::Typ::Real:
				std::cout << " Type: Real" << std::endl;
				{
				fmi2_import_real_variable_t * var_real( fmu_var.rvr );
				Real const var_start( get_real( fmu_var.ref() ) );
				std::cout << " Start: " << var_start << std::endl;
				Real const xml_start( fmi2_import_get_real_variable_start( var_real ) );
				if ( var_has_xml_start ) {
					if ( xml_start != var_start ) {
						std::cout << " Start: XML: " << xml_start << std::endl;
						if ( fmu_var.initial_exact() ) {
							std::cerr << "  Error: Specified exact start value differs from initial FMU value" << std::endl;
							//! Suppressed pending OCT fix std::exit( EXIT_FAILURE );
						} else if ( fmu_var.initial_approx() ) {
							std::cerr << "  Info: Specified approximate start value differs from initial FMU value" << std::endl;
						} else if ( fmu_var.initial_calculated() ) {
							std::cerr << "  Info: Specified calculated start value differs from initial FMU value" << std::endl;
						}
					}
				}
				Real const var_nominal( fmi2_xml_get_real_variable_nominal( var_real ) ); // == x_nominal[ fmu_var.isa ]
				std::cout << " Nominal: " << var_nominal << std::endl;
				if ( var_nominal <= 0.0 ) { // FMU generation should fail if nominal is zero and should convert negative nominals to their absolute value // FMU should return a nominal of 1 if not specified in the model
					std::cerr << " Error: Nonpositive nominal value for " << var_name << ": " << var_nominal << std::endl;
					std::exit( EXIT_FAILURE );
				}

				if ( fmu_var.variability_continuous() || fmu_var.variability_discrete() ) {
					if ( ( options::output::F && fmu_var.causality_output() ) || ( options::output::L && fmu_var.causality_local() ) ) { // Add to FMU outputs
						if ( output_filter.fmu( var_name ) ) fmu_outs.add( var_real, fmu_var );
					}
				}

				if ( fmu_var.variability_continuous() ) { // Continuous
					std::cout << " Type: Real: Continuous" << std::endl;
					if ( !SI_unit_check( fmi2_import_get_real_variable_unit( var_real ) ) ) { // May not be necessary: LBL preference
						std::cerr << " Error: Non-SI unit used for real continuous variable: Not currently supported: " << var_name << std::endl;
						//std::exit( EXIT_FAILURE );
					}
					fmu_vars.add( var_real, fmu_var );
					if ( fmu_var.causality_input() ) { // Input
						std::cout << " Type: Real: Continuous: Input" << std::endl;
						Function inp_fxn;
						auto i_fxn_var( options::fxn.find( var_name ) );
						if ( i_fxn_var == options::fxn.end() ) i_fxn_var = options::fxn.find( name + '.' + var_name ); // Try as mdl.var
						auto i_con_var( options::con.find( var_name ) );
						if ( i_con_var == options::con.end() ) i_con_var = options::con.find( name + '.' + var_name ); // Try as mdl.var
						if ( ( i_fxn_var != options::fxn.end() ) && ( i_con_var != options::con.end() ) ) {
							std::cerr << " Error: Both function and connections specified for input variable: " << var_name << std::endl;
							std::exit( EXIT_FAILURE );
						}
						if ( i_fxn_var != options::fxn.end() ) { // Input function specified
							std::string const & fxn_spec( i_fxn_var->second );
							std::string::size_type const ilb( fxn_spec.find( '[' ) );
							if ( ilb == std::string::npos ) {
								std::cerr << " Error: Input function spec missing [args]: " << fxn_spec << std::endl;
								std::exit( EXIT_FAILURE );
							}
							std::string::size_type const irb( fxn_spec.find( ']', ilb ) );
							if ( irb == std::string::npos ) {
								std::cerr << " Error: Input function spec [args] missing closing ]: " << fxn_spec << std::endl;
								std::exit( EXIT_FAILURE );
							}
							std::string const fxn_name( fxn_spec.substr( 0u, ilb ) );
							std::string const fxn_args( fxn_spec.substr( ilb + 1, irb - ( ilb + 1 ) ) );
							if ( fxn_name == "constant" ) {
								if ( is_double( fxn_args ) ) {
									inp_fxn = Function_Inp_constant( double_of( fxn_args ) ); // Constant start value
								} else {
									std::cerr << " Error: Input function spec constant[c] argument c is not a valid double: " << fxn_spec << std::endl;
									std::exit( EXIT_FAILURE );
								}
							} else if ( fxn_name == "sin" ) {
								std::vector< std::string > const args( split( fxn_args, ',' ) );
								if ( args.size() != 3u ) {
									std::cerr << " Error: Input function spec sin[a,b,c] doesn't have 3 arguments: " << fxn_spec << std::endl;
									std::exit( EXIT_FAILURE );
								}
								Real a, b, c;
								if ( is_double( args[ 0 ] ) ) {
									a = double_of( args[ 0 ] ); // Constant start value
								} else {
									std::cerr << " Error: Input function spec sin[a,b,c] argument a is not a valid double: " << fxn_spec << std::endl;
									std::exit( EXIT_FAILURE );
								}
								if ( is_double( args[ 1 ] ) ) {
									b = double_of( args[ 1 ] ); // Constant start value
								} else {
									std::cerr << " Error: Input function spec sin[a,b,c] argument b is not a valid double: " << fxn_spec << std::endl;
									std::exit( EXIT_FAILURE );
								}
								if ( is_double( args[ 2 ] ) ) {
									c = double_of( args[ 2 ] ); // Constant start value
								} else {
									std::cerr << " Error: Input function spec sin[a,b,c] argument c is not a valid double: " << fxn_spec << std::endl;
									std::exit( EXIT_FAILURE );
								}
								inp_fxn = Function_Inp_sin( a, b, c ); // a * sin( b * t ) + c
							} else if ( fxn_name == "step" ) {
								std::vector< std::string > const args( split( fxn_args, ',' ) );
								if ( args.size() != 3u ) {
									std::cerr << " Error: Input function spec step[h0,h,d] doesn't have 3 arguments: " << fxn_spec << std::endl;
									std::exit( EXIT_FAILURE );
								}
								Real h0, h, d;
								if ( is_double( args[ 0 ] ) ) {
									h0 = double_of( args[ 0 ] ); // Constant start value
								} else {
									std::cerr << " Error: Input function spec step[h0,h,d] argument h0 is not a valid double: " << fxn_spec << std::endl;
									std::exit( EXIT_FAILURE );
								}
								if ( is_double( args[ 1 ] ) ) {
									h = double_of( args[ 1 ] ); // Constant start value
								} else {
									std::cerr << " Error: Input function spec step[h0,h,d] argument h is not a valid double: " << fxn_spec << std::endl;
									std::exit( EXIT_FAILURE );
								}
								if ( is_double( args[ 2 ] ) ) {
									d = double_of( args[ 2 ] ); // Constant start value
								} else {
									std::cerr << " Error: Input function spec step[h0,h,d] argument d is not a valid double: " << fxn_spec << std::endl;
									std::exit( EXIT_FAILURE );
								}
								inp_fxn = Function_Inp_step( h0, h, d ); // h0 + h * floor( t / d )
							} else if ( fxn_name == "toggle" ) {
								std::vector< std::string > const args( split( fxn_args, ',' ) );
								if ( args.size() != 3u ) {
									std::cerr << " Error: Input function spec toggle[h0,h,d] doesn't have 3 arguments: " << fxn_spec << std::endl;
									std::exit( EXIT_FAILURE );
								}
								Real h0, h, d;
								if ( is_double( args[ 0 ] ) ) {
									h0 = double_of( args[ 0 ] ); // Constant start value
								} else {
									std::cerr << " Error: Input function spec toggle[h0,h,d] argument h0 is not a valid double: " << fxn_spec << std::endl;
									std::exit( EXIT_FAILURE );
								}
								if ( is_double( args[ 1 ] ) ) {
									h = double_of( args[ 1 ] ); // Constant start value
								} else {
									std::cerr << " Error: Input function spec toggle[h0,h,d] argument h is not a valid double: " << fxn_spec << std::endl;
									std::exit( EXIT_FAILURE );
								}
								if ( is_double( args[ 2 ] ) ) {
									d = double_of( args[ 2 ] ); // Constant start value
								} else {
									std::cerr << " Error: Input function spec toggle[h0,h,d] argument d is not a valid double: " << fxn_spec << std::endl;
									std::exit( EXIT_FAILURE );
								}
								inp_fxn = Function_Inp_toggle( h0, h, d ); // h0 + h * ( floor( t / d ) % 2 )
							} else {
								std::cerr << " Error: Input function spec function name unrecognized: " << fxn_spec << std::endl;
								std::exit( EXIT_FAILURE );
							}
							std::cout << " Type: Real: Continuous: Input: Function" << std::endl;
						} else if ( i_con_var != options::con.end() ) { // Input connection specified
							std::string const & con_name( i_con_var->second );
							std::cout << " Type: Real: Continuous: Input: Connection: " << con_name << std::endl;
						} else { // Use hard-coded default function
							// inp_fxn = Function_Inp_constant( var_has_xml_start ? xml_start : 1.0 ); // Constant start value
							inp_fxn = Function_Inp_step( ( var_has_xml_start ? xml_start : 0.0 ), 1.0, 1.0 ); // Step up by 1 every 1 s via discrete events
							// inp_fxn = Function_Inp_toggle( ( var_has_xml_start ? xml_start : 0.0 ), 1.0, 1.0 ); // Toggle by 1 every 1 s via discrete events
							// inp_fxn = Function_Inp_sin( 2.0, 10.0, ( var_has_xml_start ? xml_start : 1.0 ) ); // 2 * sin( 10 * t ) + 1
							std::cout << " Type: Real: Continuous: Input: Function" << std::endl;
						}
						if ( inp_fxn ) {
							if ( var_has_xml_start && xml_start != inp_fxn( 0.0 ).x0 ) {
								std::cerr << " Error: Specified start value does not match function value at t=0 for " << var_name << std::endl;
								std::exit( EXIT_FAILURE );
							}
						}
						Variable * qss_var( nullptr );
						if ( inp_fxn || !options::perfect ) { // Use input variables for connections
							Real const var_aTol( std::max( options::specified::aTol ? options::aTol : options::rTol * options::aFac * var_nominal, std::numeric_limits< Real >::min() ) ); // Use variable nominal value to set the absolute tolerance unless aTol specified
							if ( ( options::qss == options::QSS::QSS1 ) || ( options::qss == options::QSS::LIQSS1 ) ) {
								qss_var = new Variable_Inp1( this, var_name, options::rTol, var_aTol, var_start, fmu_var, inp_fxn );
							} else if ( ( options::qss == options::QSS::QSS2 ) || ( options::qss == options::QSS::LIQSS2 ) ) {
								qss_var = new Variable_Inp2( this, var_name, options::rTol, var_aTol, var_start, fmu_var, inp_fxn );
							} else if ( ( options::qss == options::QSS::QSS3 ) || ( options::qss == options::QSS::LIQSS3 ) ) {
								qss_var = new Variable_Inp3( this, var_name, options::rTol, var_aTol, var_start, fmu_var, inp_fxn );
							} else if ( options::qss == options::QSS::xQSS1 ) {
								qss_var = new Variable_xInp1( this, var_name, options::rTol, var_aTol, var_start, fmu_var, inp_fxn );
							} else if ( options::qss == options::QSS::xQSS2 ) {
								qss_var = new Variable_xInp2( this, var_name, options::rTol, var_aTol, var_start, fmu_var, inp_fxn );
							} else if ( options::qss == options::QSS::xQSS3 ) {
								qss_var = new Variable_xInp3( this, var_name, options::rTol, var_aTol, var_start, fmu_var, inp_fxn );
							} else {
								std::cerr << " Error: Specified QSS method is not yet supported for FMUs" << std::endl;
								std::exit( EXIT_FAILURE );
							}
						} else { // Use connection variables for connections
							if ( ( options::qss == options::QSS::QSS1 ) || ( options::qss == options::QSS::LIQSS1 ) || ( options::qss == options::QSS::xQSS1 ) ) {
								qss_var = new Variable_Con( this, 1, var_name, var_start, fmu_var );
							} else if ( ( options::qss == options::QSS::QSS2 ) || ( options::qss == options::QSS::LIQSS2 ) || ( options::qss == options::QSS::xQSS2 ) ) {
								qss_var = new Variable_Con( this, 2, var_name, var_start, fmu_var );
							} else if ( ( options::qss == options::QSS::QSS3 ) || ( options::qss == options::QSS::LIQSS3 ) || ( options::qss == options::QSS::xQSS3 ) ) {
								qss_var = new Variable_Con( this, 3, var_name, var_start, fmu_var );
							} else {
								std::cerr << " Error: Specified QSS method is not yet supported for FMUs" << std::endl;
								std::exit( EXIT_FAILURE );
							}
						}
						vars.push_back( qss_var ); // Add to QSS variables
						qss_var_of_ref[ fmu_var.ref() ] = qss_var;
						var_name_var[ var_name ] = qss_var;
						fmu_idxs[ idx ] = qss_var; // Add to map from FMU variable index to QSS variable
					} else if ( fmu_var.is_State() ) { // State
						std::cout << " Type: Real: Continuous: State" << std::endl;
						FMU_Variable const & fmu_der( fmu_variables[ fmu_var.idd - 1 ] );
						Real state_start( states[ fmu_var.isa ] ); // Initial value from fmi2_import_get_continuous_states() // Make this const once time work-around no longer needed
						if ( var_start != state_start ) {
							std::cerr << "  Warning: Start value differs from initial FMU state value: " << state_start << std::endl;
						}
						if ( ( xml_start != state_start ) && ( var_name == "time" ) ) { // Work-around for strange initial time behavior seen in OCT
							state_start = xml_start;
							std::cerr << "  Info: Using specified initial time instead of initial FMU state value as temporary OCT work-around" << std::endl;
						}
						Variable_QSS * qss_var( nullptr );
						Real const var_aTol( std::max( options::specified::aTol ? options::aTol : options::rTol * options::aFac * var_nominal, std::numeric_limits< Real >::min() ) ); // Use variable nominal value to set the absolute tolerance unless aTol specified
						if ( var_name == "time" ) {
							qss_var = new Variable_time( this, options::order, var_name, options::rTol, var_aTol, options::zTol, state_start, fmu_var, fmu_der );
						} else if ( options::qss == options::QSS::QSS1 ) {
							qss_var = new Variable_QSS1( this, var_name, options::rTol, var_aTol, options::zTol, state_start, fmu_var, fmu_der );
						} else if ( options::qss == options::QSS::QSS2 ) {
							if ( options::relax ) { // Relaxation
								qss_var = new Variable_rQSS2( this, var_name, options::rTol, var_aTol, options::zTol, state_start, fmu_var, fmu_der );
							} else {
								qss_var = new Variable_QSS2( this, var_name, options::rTol, var_aTol, options::zTol, state_start, fmu_var, fmu_der );
							}
						} else if ( options::qss == options::QSS::QSS3 ) {
							qss_var = new Variable_QSS3( this, var_name, options::rTol, var_aTol, options::zTol, state_start, fmu_var, fmu_der );
						} else if ( options::qss == options::QSS::LIQSS1 ) {
							qss_var = new Variable_LIQSS1( this, var_name, options::rTol, var_aTol, options::zTol, state_start, fmu_var, fmu_der );
						} else if ( options::qss == options::QSS::LIQSS2 ) {
							if ( options::relax ) { // Relaxation
								qss_var = new Variable_rLIQSS2( this, var_name, options::rTol, var_aTol, options::zTol, state_start, fmu_var, fmu_der );
							} else {
								qss_var = new Variable_LIQSS2( this, var_name, options::rTol, var_aTol, options::zTol, state_start, fmu_var, fmu_der );
							}
						} else if ( options::qss == options::QSS::LIQSS3 ) {
							qss_var = new Variable_LIQSS3( this, var_name, options::rTol, var_aTol, options::zTol, state_start, fmu_var, fmu_der );
						} else if ( options::qss == options::QSS::xQSS1 ) {
							qss_var = new Variable_xQSS1( this, var_name, options::rTol, var_aTol, options::zTol, state_start, fmu_var, fmu_der );
						} else if ( options::qss == options::QSS::xQSS2 ) {
							if ( options::relax ) { // Relaxation
								qss_var = new Variable_rxQSS2( this, var_name, options::rTol, var_aTol, options::zTol, state_start, fmu_var, fmu_der );
							} else {
								qss_var = new Variable_xQSS2( this, var_name, options::rTol, var_aTol, options::zTol, state_start, fmu_var, fmu_der );
							}
						} else if ( options::qss == options::QSS::xQSS3 ) {
							qss_var = new Variable_xQSS3( this, var_name, options::rTol, var_aTol, options::zTol, state_start, fmu_var, fmu_der );
						} else {
							std::cerr << " Error: Specified QSS method is not yet supported for FMUs" << std::endl;
							std::exit( EXIT_FAILURE );
						}
						vars.push_back( qss_var ); // Add to QSS variables
						qss_var_of_ref[ fmu_var.ref() ] = qss_var;
						var_name_var[ var_name ] = qss_var;
						state_vars.push_back( qss_var ); // Add to state variables
						if ( fmu_var.causality_output() || fmu_var.causality_local() ) { // Add to FMU QSS variable outputs
							if ( fmu_var.causality_output() && qss_var->is_Active() ) { // Skip FMU output of local QSS variables for now
								if ( output_filter( var_name ) ) f_outs_vars.push_back( qss_var );
							}
							fmu_outs.del( fmu_var.rvr ); // Remove it from non-QSS FMU outputs
						}
						fmu_idxs[ idx ] = qss_var; // Add to map from FMU variable index to QSS variable
					} else if ( fmu_var.is_EventIndicator() ) { // Event indicator
						std::cout << " Type: Real: Continuous: Event Indicator" << std::endl;
						Variable_ZC * qss_var( nullptr );
						Real const var_rTol( options::rTol * options::zFac * options::zrFac );
						Real const var_aTol( std::max( ( options::specified::aTol ? options::aTol : options::rTol * options::aFac * var_nominal ) * options::zFac * options::zaFac, std::numeric_limits< Real >::min() ) ); // Use variable nominal value to set the absolute tolerance unless aTol specified
						if ( ( options::qss == options::QSS::QSS1 ) || ( options::qss == options::QSS::LIQSS1 ) || ( options::qss == options::QSS::xQSS1 ) ) {
							qss_var = new Variable_ZC1( this, var_name, var_rTol, var_aTol, options::zTol, var_start, fmu_var );
						} else if ( ( options::qss == options::QSS::QSS2 ) || ( options::qss == options::QSS::LIQSS2 ) || ( options::qss == options::QSS::xQSS2 ) ) {
							qss_var = new Variable_ZC2( this, var_name, var_rTol, var_aTol, options::zTol, var_start, fmu_var );
						} else if ( ( options::qss == options::QSS::QSS3 ) || ( options::qss == options::QSS::LIQSS3 ) || ( options::qss == options::QSS::xQSS3 ) ) {
							qss_var = new Variable_ZC3( this, var_name, var_rTol, var_aTol, options::zTol, var_start, fmu_var );
						} else {
							std::cerr << " Error: Specified QSS method is not yet supported for FMUs" << std::endl;
							std::exit( EXIT_FAILURE );
						}
						cons.push_back( new Conditional< Variable_ZC >( var_name, qss_var, eventq ) ); // Create conditional for the zero-crossing variable
						vars.push_back( qss_var ); // Add to QSS variables
						qss_var_of_ref[ fmu_var.ref() ] = qss_var;
						var_name_var[ var_name ] = qss_var;
						if ( fmu_var.causality_output() && qss_var->is_Active() ) { // Add to FMU QSS variable outputs
							if ( output_filter( var_name ) ) f_outs_vars.push_back( qss_var );
							fmu_outs.del( fmu_var.rvr ); // Remove it from non-QSS FMU outputs
						}
						fmu_idxs[ idx ] = qss_var; // Add to map from FMU variable index to QSS variable
					} else if ( fmu_var.is_Derivative() ) { // Derivative
						std::cout << " Type: Real: Continuous: Derivative" << std::endl;
					} else if ( fmu_var.causality_output() || fmu_var.causality_local() ) {
						Variable * qss_var( nullptr );
						if ( fmu_var.is_Discrete() ) { // Continous in theory but actually discrete
							if ( fmu_var.has_upstream_state_or_ei_observer && options::active ) { // Active
								std::cout << " Type: Real: Continuous: De Facto Discrete: Active" << std::endl;
								qss_var = new Variable_D( this, var_name, var_start, fmu_var );
							} else { // Passive
								std::cout << " Type: Real: Continuous: De Facto Discrete: Passive" << std::endl;
								qss_var = new Variable_DP( this, var_name, var_start, fmu_var );
							}
						} else { // Continuous
							if ( ( fmu_var.has_upstream_state_or_ei_observer && options::active ) /* //Do || is connected */ ) { // Active
								std::cout << " Type: Real: Continuous: Non-Discrete: Active" << std::endl;
								Real const var_rTol( options::rTol * options::zFac * options::zrFac );
								Real const var_aTol( std::max( options::specified::aTol ? options::aTol : options::rTol * options::aFac * var_nominal, std::numeric_limits< Real >::min() ) ); // Use variable nominal value to set the absolute tolerance unless aTol specified
								if ( ( options::qss == options::QSS::QSS1 ) || ( options::qss == options::QSS::LIQSS1 ) || ( options::qss == options::QSS::xQSS1 ) ) {
									qss_var = new Variable_R1( this, var_name, var_rTol, var_aTol, var_start, fmu_var );
								} else if ( ( options::qss == options::QSS::QSS2 ) || ( options::qss == options::QSS::LIQSS2 ) || ( options::qss == options::QSS::xQSS2 ) ) {
									qss_var = new Variable_R2( this, var_name, var_rTol, var_aTol, var_start, fmu_var );
								} else if ( ( options::qss == options::QSS::QSS3 ) || ( options::qss == options::QSS::LIQSS3 ) || ( options::qss == options::QSS::xQSS3 ) ) {
									qss_var = new Variable_R3( this, var_name, var_rTol, var_aTol, var_start, fmu_var );
								} else {
									std::cerr << " Error: Specified QSS method is not yet supported for FMUs" << std::endl;
									std::exit( EXIT_FAILURE );
								}
							} else { // Passive
								std::cout << " Type: Real: Continuous: Non-Discrete: Passive" << std::endl;
								qss_var = new Variable_RP( this, var_name, var_start, fmu_var );
							}
						}
						vars.push_back( qss_var ); // Add to QSS variables
						qss_var_of_ref[ fmu_var.ref() ] = qss_var;
						var_name_var[ var_name ] = qss_var;
						if ( fmu_var.causality_output() && qss_var->is_Active() ) { // Add to FMU QSS variable outputs
							if ( output_filter( var_name ) ) f_outs_vars.push_back( qss_var );
							fmu_outs.del( fmu_var.rvr ); // Remove it from non-QSS FMU outputs
						}
						fmu_idxs[ idx ] = qss_var; // Add to map from FMU variable index to QSS variable
					} else {
						std::cout << " Not tracked by QSS: Not state, event indicator, input, output, or local variable" << std::endl;
					}
				} else if ( fmu_var.variability_discrete() ) { // Discrete
					std::cout << " Type: Real: Discrete" << std::endl;
					if ( !SI_unit_check( fmi2_import_get_real_variable_unit( var_real ) ) ) { // May not be necessary: LBL preference
						std::cerr << " Error: Non-SI unit used for real discrete variable: Not currently supported: " << var_name << std::endl;
						//std::exit( EXIT_FAILURE );
					}
					fmu_vars.add( var_real, fmu_variables.back() );
					if ( fmu_var.causality_input() ) { // Input
						std::cout << " Type: Real: Discrete: Input" << std::endl;
						// Function inp_fxn( Function_Inp_constant( var_has_xml_start ? xml_start : 0.0 ) ); // Constant start value
						Function inp_fxn( Function_Inp_step( var_has_xml_start ? xml_start : 0.0, 1.0, 1.0 ) ); // Step up by 1 every 1 s via discrete events
						// Function inp_fxn( Function_Inp_toggle( var_has_xml_start ? xml_start : 0.0, 1.0, 1.0 ) ); // Toggle by 1 every 1 s via discrete events
						Variable_InpD * qss_var( new Variable_InpD( this, var_name, var_start, fmu_var, inp_fxn ) );
						vars.push_back( qss_var ); // Add to QSS variables
						qss_var_of_ref[ fmu_var.ref() ] = qss_var;
						var_name_var[ var_name ] = qss_var;
						fmu_idxs[ idx ] = qss_var; // Add to map from FMU variable index to QSS variable
					} else if ( fmu_var.causality_output() || fmu_var.causality_local() ) { // Output or local
						Variable * qss_var( nullptr );
						if ( fmu_var.has_upstream_state_or_ei_observer && options::active ) { // Active
							std::cout << " Type: Real: Discrete: " << ( fmu_var.causality_output() ? "Output" : "Local" ) << ": Active" << std::endl;
							qss_var = new Variable_D( this, var_name, var_start, fmu_var );
						} else { // Passive
							std::cout << " Type: Real: Discrete: " << ( fmu_var.causality_output() ? "Output" : "Local" ) << ": Passive" << std::endl;
							qss_var = new Variable_DP( this, var_name, var_start, fmu_var );
						}
						vars.push_back( qss_var ); // Add to QSS variables
						qss_var_of_ref[ fmu_var.ref() ] = qss_var;
						var_name_var[ var_name ] = qss_var;
						fmu_idxs[ idx ] = qss_var; // Add to map from FMU variable index to QSS variable
						if ( fmu_var.causality_output() && qss_var->is_Active() ) { // Add to FMU QSS variable outputs
							if ( output_filter( var_name ) ) f_outs_vars.push_back( qss_var );
							fmu_outs.del( fmu_var.rvr ); // Remove it from non-QSS FMU outputs
						}
					}
				} else if ( fmu_var.variability_fixed() ) { // Fixed
					if ( var_name == "_events_default_tol" ) { // OCT/JModelica parameter for setting FMU zero crossing value tolerance
						if ( fmu_var.causality_parameter() ) {
							if ( !options::specified::zTol ) {
								double const zTol( std::abs( var_start ) );
								if ( zTol > 0.0 ) {
									options::specified::zTol = true;
									options::zTol = zTol;
									std::cout << " FMU zero crossing value tolerance set to " << zTol << std::endl;
								}
							}
						}
					}
				}
				if ( fmu_var.is_Derivative() ) { // Derivative
					FMU_Variable const & fmu_var_state( fmu_variables[ fmu_var.ids - 1 ] );
					std::cout << " Derivative of " << ( fmu_var_state.is_State() ? "state: " : ( fmu_var_state.is_EventIndicator() ? "event indicator: " : ": " ) ) << fmu_var_state.name() << std::endl;
				} else { // Check if derivative of non-state variable
					if ( has_prefix( var_name, "der(" ) && has_suffix( var_name, ")" ) ) { // Name looks like derivative
						fmi2_import_real_variable_t * integral_real( fmi2_import_get_real_variable_derivative_of( var_real ) );
						if ( integral_real != nullptr ) { // Marked as a derivative
							fmi2_import_variable_t * integral_var( fmu_real_to_var_map[ integral_real ] );
							std::string const integral_name( fmi2_import_get_variable_name( integral_var ) );
							std::cout << " Derivative of non-state: " << integral_name << std::endl;
						}
					}
				}
				}
				break;
			case FMU_Variable::Typ::Integer:
				std::cout << " Type: Integer" << std::endl;
				{
				fmi2_import_integer_variable_t * var_int( fmu_var.ivr );
				Integer const var_start( get_integer( fmu_var.ref() ) );
				std::cout << " Start: " << var_start << std::endl;
				Integer const xml_start( fmi2_import_get_integer_variable_start( var_int ) );
				if ( var_has_xml_start ) {
					if ( xml_start != var_start ) {
						std::cout << " Start: XML: " << xml_start << std::endl;
						if ( fmu_var.initial_exact() ) {
							std::cerr << "  Error: Specified exact start value differs from initial FMU value" << std::endl;
							//! Suppressed pending OCT fix std::exit( EXIT_FAILURE );
						} else if ( fmu_var.initial_approx() ) {
							std::cerr << "  Info: Specified approximate start value differs from initial FMU value" << std::endl;
						} else if ( fmu_var.initial_calculated() ) {
							std::cerr << "  Info: Specified calculated start value differs from initial FMU value" << std::endl;
						}
					}
				}
				if ( fmu_var.variability_discrete() ) { // Discrete
					std::cout << " Type: Integer: Discrete" << std::endl;
					fmu_vars.add( var_int, fmu_var );
					if ( ( options::output::F && fmu_var.causality_output() ) || ( options::output::L && fmu_var.causality_local() ) ) { // Add to FMU outputs
						if ( output_filter.fmu( var_name ) ) fmu_outs.add( var_int, fmu_var );
					}
					if ( fmu_var.causality_input() ) { // Input
						std::cout << " Type: Integer: Discrete: Input" << std::endl;
						// Function inp_fxn( Function_Inp_constant( var_has_xml_start ? xml_start : 0.0 ) ); // Constant start value
						Function inp_fxn( Function_Inp_step( ( var_has_xml_start ? xml_start : 0.0 ), 1.0, 1.0 ) ); // Step up by 1 every 1 s via discrete events
						// Function inp_fxn( Function_Inp_toggle( ( var_has_xml_start ? xml_start : 0.0 ), 1.0, 1.0 ) ); // Toggle by 1 every 1 s via discrete events
						Variable_InpI * qss_var( new Variable_InpI( this, var_name, var_start, fmu_var, inp_fxn ) );
						vars.push_back( qss_var ); // Add to QSS variables
						qss_var_of_ref[ var_ref ] = qss_var;
						var_name_var[ var_name ] = qss_var;
						fmu_idxs[ idx ] = qss_var; // Add to map from FMU variable index to QSS variable

					} else if ( fmu_var.causality_output() || fmu_var.causality_local() ) { // Output or local
						Variable * qss_var( nullptr );
						if ( fmu_var.has_upstream_state_or_ei_observer && options::active ) { // Active
							std::cout << " Type: Integer: Discrete: " << ( fmu_var.causality_output() ? "Output" : "Local" ) << ": Active" << std::endl;
							qss_var = new Variable_I( this, var_name, var_start, fmu_var );
						} else { // Passive
							std::cout << " Type: Integer: Discrete: " << ( fmu_var.causality_output() ? "Output" : "Local" ) << ": Passive" << std::endl;
							qss_var = new Variable_IP( this, var_name, var_start, fmu_var );
						}
						vars.push_back( qss_var ); // Add to QSS variables
						qss_var_of_ref[ var_ref ] = qss_var;
						var_name_var[ var_name ] = qss_var;
						fmu_idxs[ idx ] = qss_var; // Add to map from FMU variable index to QSS variable
						if ( fmu_var.causality_output() && qss_var->is_Active() ) { // Add to FMU QSS variable outputs
							if ( output_filter( var_name ) ) f_outs_vars.push_back( qss_var );
							fmu_outs.del( var_int ); // Remove it from non-QSS FMU outputs
						}
					}
				} else if ( fmu_var.variability_fixed() ) { // Fixed
					if ( fmu_var.causality_parameter() ) {
						if ( var_name == "_log_level" ) { // Set FMU log level from options::log
							switch ( options::log ) {
							case options::LogLevel::fatal:
								set_integer( var_ref, 1 );
								break;
							case options::LogLevel::error:
								set_integer( var_ref, 2 );
								break;
							case options::LogLevel::warning:
								set_integer( var_ref, 3 );
								break;
							case options::LogLevel::info:
								set_integer( var_ref, 4 );
								break;
							case options::LogLevel::verbose:
								set_integer( var_ref, 5 );
								break;
							case options::LogLevel::debug:
								set_integer( var_ref, 6 );
								break;
							case options::LogLevel::all:
								set_integer( var_ref, 7 );
								break;
							}
						}
					}
				}
				}
				break;
			case FMU_Variable::Typ::Boolean:
				std::cout << " Type: Boolean" << std::endl;
				{
				fmi2_import_bool_variable_t * var_bool( fmi2_import_get_variable_as_boolean( var ) );
				bool const var_start( get_boolean( fmu_var.ref() ) );
				std::cout << " Start: " << var_start << std::endl;
				bool const xml_start( fmi2_import_get_boolean_variable_start( var_bool ) );
				if ( var_has_xml_start ) {
					if ( xml_start != var_start ) {
						std::cout << " Start: XML: " << xml_start << std::endl;
						if ( fmu_var.initial_exact() ) {
							std::cerr << "  Error: Specified start value differs from initial FMU value" << std::endl;
							//! Suppressed pending OCT fix std::exit( EXIT_FAILURE );
						} else if ( fmu_var.initial_approx() ) {
							std::cerr << "  Info: Specified approximate start value differs from initial FMU value" << std::endl;
						} else if ( fmu_var.initial_calculated() ) {
							std::cerr << "  Info: Specified calculated start value differs from initial FMU value" << std::endl;
						}
					}
				}
				if ( fmu_var.variability_discrete() ) { // Discrete
					std::cout << " Type: Boolean: Discrete" << std::endl;
					fmu_vars.add( var_bool, fmu_var );
					if ( ( options::output::F && fmu_var.causality_output() ) || ( options::output::L && fmu_var.causality_local() ) ) { // Add to FMU outputs
						if ( output_filter.fmu( var_name ) ) fmu_outs.add( var_bool, fmu_var );
					}
					if ( fmu_var.causality_input() ) { // Input
						std::cout << " Type: Boolean: Discrete: Input" << std::endl;
						Function inp_fxn( Function_Inp_toggle( 0.0, 1.0, 1.0 ) ); // Toggle 0-1 every 1 s via discrete events
						Variable_InpB * qss_var( new Variable_InpB( this, var_name, var_start, fmu_var, inp_fxn ) );
						vars.push_back( qss_var ); // Add to QSS variables
						qss_var_of_ref[ var_ref ] = qss_var;
						var_name_var[ var_name ] = qss_var;
						fmu_idxs[ idx ] = qss_var; // Add to map from FMU variable index to QSS variable
					} else if ( fmu_var.causality_output() || fmu_var.causality_local() ) { // Output or local
						Variable * qss_var( nullptr );
						if ( fmu_var.has_upstream_state_or_ei_observer && options::active ) { // Active
							std::cout << " Type: Boolean: Discrete: " << ( fmu_var.causality_output() ? "Output" : "Local" ) << ": Active" << std::endl;
							qss_var = new Variable_B( this, var_name, var_start, fmu_var );
						} else { // Passive
							std::cout << " Type: Boolean: Discrete: " << ( fmu_var.causality_output() ? "Output" : "Local" ) << ": Passive" << std::endl;
							qss_var = new Variable_BP( this, var_name, var_start, fmu_var );
						}
						vars.push_back( qss_var ); // Add to QSS variables
						qss_var_of_ref[ var_ref ] = qss_var;
						var_name_var[ var_name ] = qss_var;
						fmu_idxs[ idx ] = qss_var; // Add to map from FMU variable index to QSS variable
						if ( fmu_var.causality_output() && qss_var->is_Active() ) { // Add to FMU QSS variable outputs
							if ( output_filter( var_name ) ) f_outs_vars.push_back( qss_var );
							fmu_outs.del( var_bool ); // Remove it from non-QSS FMU outputs
						}
					}
				}
				}
				break;
			case FMU_Variable::Typ::String:
				std::cout << " Type: String" << std::endl;
				{
				fmi2_import_string_variable_t * var_string( fmu_var.svr );
				if ( var_has_xml_start ) std::cout << " Start: " << fmi2_import_get_string_variable_start( var_string ) << std::endl;
				fmu_vars.add( var_string, fmu_var );
				}
				break;
			case FMU_Variable::Typ::Enum:
				std::cout << " Type: Enum" << std::endl;
				{
				fmi2_import_enum_variable_t * var_enum( fmu_var.evr );
				if ( var_has_xml_start ) std::cout << " Start: " << fmi2_import_get_enum_variable_start( var_enum ) << std::endl;
				fmu_vars.add( var_enum, fmu_var );
				}
				break;
			default:
				std::cout << " Type: Unknown" << std::endl;
				break;
			}
		}
		size_type const n_state_vars( state_vars.size() );

		// Duplicate checks
		if ( var_names.size() > 1u ) { // Check for repeat variable names
			Var_Names sorted_var_names( var_names );
			std::sort( sorted_var_names.begin(), sorted_var_names.end() );
			std::string repeat_name;
			for ( Var_Names::size_type i = 0, l = var_names.size() - 1; i < l; ++i ) {
				if ( ( var_names[ i ] == var_names[ i + 1 ] ) && ( var_names[ i ] != repeat_name ) ) { // New repeat name
					repeat_name = var_names[ i ];
					std::cerr << " Error: Variable name repeats: " << repeat_name << std::endl;
				}
			}
			if ( !repeat_name.empty() ) {
				std::exit( EXIT_FAILURE );
			}
		}
		{ // Check for duplicate value references in state variables
			bool dups( false );
			fmi2_value_reference_t last_ref( std::numeric_limits< fmi2_value_reference_t >::max() );
			FMU_Variable const * last_fmu_var( nullptr );
			for ( auto const & ref_var : fmu_ref_to_state ) {
				fmi2_value_reference_t const ref( ref_var.first );
				FMU_Variable::Index const idx( ref_var.second );
				FMU_Variable const & fmu_var( fmu_variables[ idx - 1 ] );
				if ( ref == last_ref ) {
					std::cerr << "Error: FMU value reference number " << ref << " repeats in state variables: " << last_fmu_var->name() << " and " << fmu_var.name() << std::endl;
					dups = true;
				}
				last_ref = ref;
				last_fmu_var = &fmu_var;
			}
			if ( dups ) std::exit( EXIT_FAILURE );
		}

		// CSV output setup
		if ( options::csv ) {
			csv.init( name );
			Var_Names res_var_names;
			res_var_indexes.clear();
			for ( size_type i = 0; i < n_fmu_vars; ++i ) {
				Name const & var_name( var_names[ i ] );
				if ( output_filter.res( var_name ) ) {
					res_var_names.push_back( var_name );
					res_var_indexes.push_back( i );
				}
			}
			csv.labels( res_var_names );
			res_var_vals.clear();
			res_var_vals.resize( res_var_indexes.size() );
		}

		// QSS Dependency Processing
		std::cout << "\nQSS Dependency Processing =====" << std::endl;
		for ( FMU_Dependencies::value_type const & idx_var : fmu_dependencies.variables ) { // Pair of index and dep::Variable
			FMU_Dependencies::key_type const idx( idx_var.first );
			FMU_Variable const & fmu_var( fmu_variables[ idx - 1 ] ); // FMU variable corresponding to the dep::Variable
			size_type const idv( fmu_var.is_Derivative() ? fmu_var.ids : idx ); // Index of the FMU variable for the QSS variable that has these dependencies
			auto const i_qss_var( fmu_idxs.find( idv ) );
			if ( i_qss_var != fmu_idxs.end() ) { // QSS variable that these dependencies apply to exists
				dep::Variable const & fmu_dependencies_var( idx_var.second );
				dep::Variable::Observees const & fmu_dependencies_var_observees( fmu_dependencies_var.observees );
				if ( !fmu_dependencies_var_observees.empty() ) {
					Variable * qss_var( i_qss_var->second ); // QSS variable that gets these observees
					std::cout << "\n" << qss_var->name() << " observes:" << std::endl;
					// Should not be any temporaries in the XML now
					// if ( qss_var->is_Boolean() && has_prefix( qss_var->name(), "temp_" ) && ( fmu_dependencies_var_observees.size() == 1u ) ) { // Boolean variable with one observee: Check if temporary variable OCT inserts for an event indicator
					// 	dep::Variable::Index const observee_idx( fmu_dependencies_var_observees[ 0 ] );
					// 	if ( fmu_variables[ observee_idx - 1 ].is_EventIndicator() ) continue; // Temporary variable OCT inserts for an event indicator: Will short-circuit out its dependencies
					// }
					for ( dep::Variable::Index const observee_idx : fmu_dependencies_var_observees ) { // Loop over observee indexes
						assert( !fmu_variables[ observee_idx - 1 ].is_Derivative() ); // Derivative dependencies were short-circuited out above
						auto const i_qss_observee_var( fmu_idxs.find( observee_idx ) ); // QSS variable pointer
						if ( i_qss_observee_var != fmu_idxs.end() ) { // Observee is a QSS variable
							Variable * qss_observee_var( i_qss_observee_var->second );
							// Should not be any temporaries in the XML now
							// if ( qss_observee_var->is_Boolean() && has_prefix( qss_observee_var->name(), "temp_" ) ) { // Boolean observee variable with temporary-style name
							// 	FMU_Dependencies::const_iterator const & i_observee_dep( fmu_dependencies.find( i_qss_observee_var->first ) );
							// 	if ( i_observee_dep != fmu_dependencies.end() ) {
							// 		dep::Variable const & observee_dep_var( i_observee_dep->second );
							// 		dep::Variable::Observees const & fmu_dependencies_observee_observees( observee_dep_var.observees );
							// 		if ( fmu_dependencies_observee_observees.size() == 1u ) { // Boolean variable with one observee: Check if temporary variable OCT inserts for an event indicator
							// 			dep::Variable::Index const observee_observee_idx( fmu_dependencies_observee_observees[ 0 ] );
							// 			FMU_Variable const & fmu_observee_observee_var( fmu_variables[ observee_observee_idx - 1 ] );
							// 			if ( fmu_observee_observee_var.is_EventIndicator() ) { // Temporary variable OCT inserts for an event indicator: Short-circuit out its dependencies
							// 				auto const i_qss_observee_observee_var( fmu_idxs.find( observee_observee_idx ) ); // QSS variable pointer
							// 				if ( i_qss_observee_observee_var != fmu_idxs.end() ) { // Observee is a QSS variable
							// 					Variable * qss_observee_observee_var( i_qss_observee_observee_var->second );
							// 					qss_var->observe( qss_observee_observee_var );
							// 					std::cout << "  " << qss_observee_observee_var->name() << std::endl;
							// 					continue; // Short-circuited so loop to next observee
							// 				}
							// 			}
							// 		}
							// 	}
							// }
							qss_var->observe( qss_observee_var );
							std::cout << "  " << qss_observee_var->name() << std::endl;
						}
					}
				}
			}
		}

		//! Merge dependencies from <ModelStructure> for now until OCT <Dependencies> contains the complete dependency graph
		std::cout << "\n<ModelStructure> Dependencies: Merging =======" << std::endl;

		{ // QSS observer setup: Continuous variables: Derivatives
			size_type * startIndex( nullptr );
			size_type * dependency( nullptr );
			char * factorKind( nullptr );
			fmi2_import_get_derivatives_dependencies( fmu, &startIndex, &dependency, &factorKind );
			if ( startIndex != nullptr ) { // Derivatives dependency info present in XML
				std::cout << "\n<Derivatives> =====" << std::endl;
				for ( size_type i = 0; i < n_derivatives; ++i ) {
					// std::cout << "\nDerivative  Ref: " << drs[ i ] << std::endl;
					fmi2_import_variable_t * der( fmi2_import_get_variable( der_list, i ) );
					std::string const der_name( fmi2_import_get_variable_name( der ) );
					// std::cout << " Name: " << der_name << std::endl;
					fmi2_import_real_variable_t * der_real( fmi2_import_get_variable_as_real( der ) );
					assert( fmu_dvrs.find( der_real ) != fmu_dvrs.end() );
					size_type const idx( fmu_dvrs[ der_real ].idx );
					auto const ivar( fmu_idxs.find( idx ) );
					if ( ivar != fmu_idxs.end() ) {
						Variable * var( ivar->second );
						// std::cout << " Var: " << var->name() << "  Index: " << idx << std::endl;
						std::cout << "\n " << der_name << ':' << std::endl;
						for ( size_type j = startIndex[ i ]; j < startIndex[ i + 1 ]; ++j ) {
							size_type const dep_idx( dependency[ j ] );
							// std::cout << "  Dep Index: " << dep_idx << std::endl;
							// if ( dep_idx == 0u ) { // No info: Depends on all (don't support depends on all for now)
							// 	std::cerr << "   Error: No dependency information provided: Depends-on-all not currently supported" << std::endl;
							// 	std::exit( EXIT_FAILURE );
							// } else { // Process based on kind of dependent
							// 	fmi2_dependency_factor_kind_enu_t const kind( (fmi2_dependency_factor_kind_enu_t)( factorKind[ j ] ) );
							// 	if ( kind == fmi2_dependency_factor_kind_dependent ) {
							// 		std::cout << "   Kind: Dependent (" << kind << ')' << std::endl;
							// 	} else if ( kind == fmi2_dependency_factor_kind_constant ) {
							// 		std::cout << "   Kind: Constant (" << kind << ')' << std::endl;
							// 	} else if ( kind == fmi2_dependency_factor_kind_fixed ) {
							// 		std::cout << "   Kind: Fixed (" << kind << ')' << std::endl;
							// 	} else if ( kind == fmi2_dependency_factor_kind_tunable ) {
							// 		std::cout << "   Kind: Tunable (" << kind << ')' << std::endl;
							// 	} else if ( kind == fmi2_dependency_factor_kind_discrete ) {
							// 		std::cout << "   Kind: Discrete (" << kind << ')' << std::endl;
							// 	} else if ( kind == fmi2_dependency_factor_kind_num ) {
							// 		std::cout << "   Kind: Num (" << kind << ')' << std::endl;
							// 	}
							// }
							auto const idep( fmu_idxs.find( dep_idx ) ); //Do Add support for input variable dependents
							if ( idep != fmu_idxs.end() ) {
								Variable * dep( idep->second );
								var->observe( dep );
								std::cout << "  " << dep->name() << std::endl;
							// } else {
								//std::cout << "   Note: FMU-ME derivative " << der_name << " has dependency with index " << dep_idx << " that is not a QSS variable" << std::endl;
							}
						}
					} else {
						std::cerr << "   Error: QSS variable with index " << idx << " referenced in derivative not found" << std::endl;
						std::exit( EXIT_FAILURE );
					}
				}
			// } else { // Assume no observers in model (this may not be true: FMI spec says no dependencies => dependent on all)
			// 	std::cout << "\nNo Derivatives dependency info in FMU-ME XML" << std::endl;
			}
		}

		// { // QSS observer setup: Continuous variables: InitialUnknowns // QSS should not need initialization dependencies since it gets initial values from the FMU
		// 	size_type * startIndex( nullptr );
		// 	size_type * dependency( nullptr );
		// 	char * factorKind( nullptr );
		// 	fmi2_import_variable_list_t * inu_list( fmi2_import_get_initial_unknowns_list( fmu ) ); // InitialUnknowns variables
		// 	size_type const n_inu_vars( fmi2_import_get_variable_list_size( inu_list ) );
		// 	// std::cout << n_inu_vars << " variables found in InitialUnknowns" << std::endl;
		// 	fmi2_import_get_initial_unknowns_dependencies( fmu, &startIndex, &dependency, &factorKind );
		// 	if ( startIndex != nullptr ) { // InitialUnknowns dependency info present in XML
		// 		std::cout << "\n<InitialUnknowns> =====" << std::endl;
		// 		// fmi2_value_reference_t const * inu_vrs( fmi2_import_get_value_referece_list( inu_list ) ); // reference is spelled wrong in FMIL API
		// 		for ( size_type i = 0; i < n_inu_vars; ++i ) {
		// 			// std::cout << "\nInitialUnknown Variable  Ref: " << inu_vrs[ i ] << std::endl;
		// 			fmi2_import_variable_t * inu( fmi2_import_get_variable( inu_list, i ) );
		// 			// if ( fmi2_import_get_variability( inu ) != fmi2_variability_enu_continuous ) {
		// 				// std::cout << " Skipping: Not continuous variable" << std::endl;
		// 				// continue; // Only look at continuous variables
		// 			// }
		// 			std::string const inu_name( fmi2_import_get_variable_name( inu ) );
		// 			// std::cout << " Name: " << inu_name << std::endl;
		// 			std::cout << "\n " << inu_name << ':' << std::endl;
		// 			fmi2_import_real_variable_t * inu_real( fmi2_import_get_variable_as_real( inu ) );
		// 			if ( fmu_vars.has( inu_real ) ) {
		// 				FMU_Variable & fmu_inu( fmu_vars[ inu_real ] );
		// 				size_type const idx( fmu_inu.idx );
		// 				auto const ivar( fmu_idxs.find( idx ) );
		// 				if ( ivar != fmu_idxs.end() ) {
		// 					Variable * var( ivar->second );
		// 					// std::cout << " Var: " << var->name() << "  Index: " << idx << std::endl;
		// 					for ( size_type j = startIndex[ i ]; j < startIndex[ i + 1 ]; ++j ) {
		// 						size_type const dep_idx( dependency[ j ] );
		// 						// std::cout << "  Dep Index: " << dep_idx << std::endl;
		// 						// if ( dep_idx == 0u ) { // No info: Depends on all (don't support depends on all for now)
		// 						// 	std::cerr << "   Error: No dependency information provided: Depends-on-all not currently supported" << std::endl;
		// 						// 	// std::exit( EXIT_FAILURE ); //OCT Let run proceed while waiting for OCT fixes
		// 						// } else { // Process based on kind of dependent
		// 						// 	fmi2_dependency_factor_kind_enu_t const kind( (fmi2_dependency_factor_kind_enu_t)( factorKind[ j ] ) );
		// 						// 	if ( kind == fmi2_dependency_factor_kind_dependent ) {
		// 						// 		std::cout << "   Kind: Dependent (" << kind << ')' << std::endl;
		// 						// 	} else if ( kind == fmi2_dependency_factor_kind_constant ) {
		// 						// 		std::cout << "   Kind: Constant (" << kind << ')' << std::endl;
		// 						// 	} else if ( kind == fmi2_dependency_factor_kind_fixed ) {
		// 						// 		std::cout << "   Kind: Fixed (" << kind << ')' << std::endl;
		// 						// 	} else if ( kind == fmi2_dependency_factor_kind_tunable ) {
		// 						// 		std::cout << "   Kind: Tunable (" << kind << ')' << std::endl;
		// 						// 	} else if ( kind == fmi2_dependency_factor_kind_discrete ) {
		// 						// 		std::cout << "   Kind: Discrete (" << kind << ')' << std::endl;
		// 						// 	} else if ( kind == fmi2_dependency_factor_kind_num ) {
		// 						// 		std::cout << "   Kind: Num (" << kind << ')' << std::endl;
		// 						// 	}
		// 						// }
		// 						auto idep( fmu_idxs.find( dep_idx ) ); //Do Add support for input variable dependents
		// 						if ( idep != fmu_idxs.end() ) {
		// 							Variable * dep( idep->second );
		// 							var->observe( dep );
		// 							std::cout << "  " << dep->name() << std::endl;
		// 						// } else {
		// 							//std::cout << "   Note: FMU-ME InitialUnknown " << inu_name << " has dependency with index " << dep_idx << " that is not a QSS variable" << std::endl;
		// 						}
		// 					}
		// 				// } else {
		// 					//std::cout << "   Note: QSS variable with index " << idx << " for InitialUnknown not found" << std::endl;
		// 				}
		// 			}
		// 		}
		// 	// } else { // Assume no observers in model (this may not be true: FMI spec says no dependencies => dependent on all)
		// 	// 	std::cout << "\nNo InitialUknowns dependency info in FMU-ME XML" << std::endl;
		// 	}
		// }

		{ // QSS observer setup: Discrete variables
			size_type * startIndex( nullptr );
			size_type * dependency( nullptr );
			char * factorKind( nullptr );
			fmi2_import_variable_list_t * dis_list( fmi2_import_get_discrete_states_list( fmu ) ); // Discrete variables
			size_type const n_dis_vars( fmi2_import_get_variable_list_size( dis_list ) );
			// std::cout << n_dis_vars << " discrete variables found in DiscreteStates" << std::endl;
			fmi2_import_get_discrete_states_dependencies( fmu, &startIndex, &dependency, &factorKind );
			if ( startIndex != nullptr ) { // Discrete dependency info present in XML
				std::cout << "\n<DiscreteStates> =====" << std::endl;
				// fmi2_value_reference_t const * dis_vrs( fmi2_import_get_value_referece_list( dis_list ) ); // reference is spelled wrong in FMIL API
				for ( size_type i = 0; i < n_dis_vars; ++i ) {
					// std::cout << "\nDiscrete Variable  Ref: " << dis_vrs[ i ] << std::endl;
					fmi2_import_variable_t * dis( fmi2_import_get_variable( dis_list, i ) );
					assert( fmi2_import_get_variability( dis ) == fmi2_variability_enu_discrete );
					std::string const dis_name( fmi2_import_get_variable_name( dis ) );
					// std::cout << " Name: " << dis_name << std::endl;
					std::cout << "\n " << dis_name << ':' << std::endl;
					FMU_Variable * fmu_dis( nullptr );
					fmi2_base_type_enu_t const dis_base_type( fmi2_import_get_variable_base_type( dis ) );
					switch ( dis_base_type ) {
					case fmi2_base_type_real:
						std::cout << " Type: Real" << std::endl;
						{
						fmi2_import_real_variable_t * dis_real( fmi2_import_get_variable_as_real( dis ) );
						if ( fmu_vars.has( dis_real ) ) fmu_dis = &fmu_vars[ dis_real ];
						// std::cout << " FMU-ME idx: " << fmu_dis->idx << " maps to QSS var: " << fmu_idxs[ fmu_dis->idx ]->name() << std::endl;
						}
						break;
					case fmi2_base_type_int:
						// std::cout << " Type: Integer" << std::endl;
						{
						fmi2_import_integer_variable_t * dis_int( fmi2_import_get_variable_as_integer( dis ) );
						if ( fmu_vars.has( dis_int ) ) fmu_dis = &fmu_vars[ dis_int ];
						// std::cout << " FMU-ME idx: " << fmu_dis->idx << " maps to QSS var: " << fmu_idxs[ fmu_dis->idx ]->name() << std::endl;
						}
						break;
					case fmi2_base_type_bool:
						// std::cout << " Type: Boolean" << std::endl;
						{
						fmi2_import_bool_variable_t * dis_bool( fmi2_import_get_variable_as_boolean( dis ) );
						if ( fmu_vars.has( dis_bool ) ) fmu_dis = &fmu_vars[ dis_bool ];
						// std::cout << " FMU-ME idx: " << fmu_dis->idx << " maps to QSS var: " << fmu_idxs[ fmu_dis->idx ]->name() << std::endl;
						}
						break;
					case fmi2_base_type_str:
						// std::cout << " Type: String" << std::endl;
						break;
					case fmi2_base_type_enum:
						// std::cout << " Type: Enum" << std::endl;
						break;
					default:
						// std::cout << " Type: Unknown" << std::endl;
						break;
					}
					if ( fmu_dis == nullptr ) continue; // Not a variable we care about
					size_type const idx( fmu_dis->idx );
					auto const idis( fmu_idxs.find( idx ) ); //Do Add support for input variable dependents
					if ( idis != fmu_idxs.end() ) {
						Variable * dis_var( idis->second );
						assert( dis_var->is_Discrete() );
						for ( size_type j = startIndex[ i ]; j < startIndex[ i + 1 ]; ++j ) {
							size_type const dep_idx( dependency[ j ] );
							// std::cout << "  Dep Index: " << dep_idx << std::endl;
							// if ( dep_idx == 0u ) { // No info: Depends on all (don't support depends on all for now)
							// 	std::cerr << "   Error: No dependency information provided: Depends-on-all not currently supported" << std::endl;
							// 	std::exit( EXIT_FAILURE );
							// } else { // Process based on kind of dependent
							// 	fmi2_dependency_factor_kind_enu_t const kind( (fmi2_dependency_factor_kind_enu_t)( factorKind[ j ] ) );
							// 	if ( kind == fmi2_dependency_factor_kind_dependent ) {
							// 		std::cout << "   Kind: Dependent (" << kind << ')' << std::endl;
							// 	} else if ( kind == fmi2_dependency_factor_kind_constant ) {
							// 		std::cout << "   Kind: Constant (" << kind << ')' << std::endl;
							// 	} else if ( kind == fmi2_dependency_factor_kind_fixed ) {
							// 		std::cout << "   Kind: Fixed (" << kind << ')' << std::endl;
							// 	} else if ( kind == fmi2_dependency_factor_kind_tunable ) {
							// 		std::cout << "   Kind: Tunable (" << kind << ')' << std::endl;
							// 	} else if ( kind == fmi2_dependency_factor_kind_discrete ) {
							// 		std::cout << "   Kind: Discrete (" << kind << ')' << std::endl;
							// 	} else if ( kind == fmi2_dependency_factor_kind_num ) {
							// 		std::cout << "   Kind: Num (" << kind << ')' << std::endl;
							// 	}
							// }
							auto idep( fmu_idxs.find( dep_idx ) ); //Do Add support for input variable dependents
							if ( idep != fmu_idxs.end() ) {
								Variable * dep( idep->second );
								dis_var->observe( dep );
								std::cout << "  " << dep->name() << std::endl;
							// } else {
								//std::cout << "   Note: FMU-ME discrete variable " << dis_name << " has dependency with index " << dep_idx << " that is not a QSS variable" << std::endl;
							}
						}
					} else {
						std::cerr << "   Error: QSS variable with index " << idx << " for Discrete variable not found" << std::endl;
						std::exit( EXIT_FAILURE );
					}
				}
			// } else { // Assume no discrete variables dependent on ZC variables in model
			// 	std::cout << "\nNo discrete variable dependency info in FMU-ME XML" << std::endl;
			}
		}

		{ // QSS observer setup: Output variables
			size_type * startIndex( nullptr );
			size_type * dependency( nullptr );
			char * factorKind( nullptr );
			fmi2_import_variable_list_t * out_list( fmi2_import_get_outputs_list( fmu ) ); // Output variables
			size_type const n_out_vars( fmi2_import_get_variable_list_size( out_list ) );
			// std::cout << n_out_vars << " output variables found in OutputStates" << std::endl;
			fmi2_import_get_outputs_dependencies( fmu, &startIndex, &dependency, &factorKind );
			if ( startIndex != nullptr ) { // Dependency info present in XML
				std::cout << "\n<Outputs> =====" << std::endl;
				// fmi2_value_reference_t const * out_vrs( fmi2_import_get_value_referece_list( out_list ) ); // reference is spelled wrong in FMIL API
				for ( size_type i = 0; i < n_out_vars; ++i ) {
					// std::cout << "\nOutput Variable  Ref: " << out_vrs[ i ] << std::endl;
					fmi2_import_variable_t * out( fmi2_import_get_variable( out_list, i ) );
					std::string const out_name( fmi2_import_get_variable_name( out ) );
					std::cout << "\n " << out_name << ':' << std::endl;
					if ( fmi2_import_get_causality( out ) != fmi2_causality_enu_output ) {
						std::cerr << "\nError: Variable in Output section of modelDescription.xml is not causality=output: " << out_name << std::endl;
						// std::exit( EXIT_FAILURE );
					}
					// std::cout << " Name: " << out_name << std::endl;
					FMU_Variable * fmu_out( nullptr ); // FMU output variable
					FMU_Variable * fmu_var( nullptr ); // FMU variable that output variable is derivative of, if any
					fmi2_base_type_enu_t const out_base_type( fmi2_import_get_variable_base_type( out ) );
					switch ( out_base_type ) {
					case fmi2_base_type_real:
						// std::cout << " Type: Real" << std::endl;
						{
						fmi2_import_real_variable_t * out_real( fmi2_import_get_variable_as_real( out ) );
						if ( fmu_vars.has( out_real ) ) fmu_out = &fmu_vars[ out_real ];
						auto const ider( fmu_dvrs.find( out_real ) );
						if ( ider != fmu_dvrs.end() ) fmu_var = ider->second;
						}
						break;
					case fmi2_base_type_int:
						// std::cout << " Type: Integer" << std::endl;
						{
						fmi2_import_integer_variable_t * out_int( fmi2_import_get_variable_as_integer( out ) );
						if ( fmu_vars.has( out_int ) ) fmu_out = &fmu_vars[ out_int ];
						}
						break;
					case fmi2_base_type_bool:
						// std::cout << " Type: Boolean" << std::endl;
						{
						fmi2_import_bool_variable_t * out_bool( fmi2_import_get_variable_as_boolean( out ) );
						if ( fmu_vars.has( out_bool ) ) fmu_out = &fmu_vars[ out_bool ];
						}
						break;
					case fmi2_base_type_str:
						// std::cout << " Type: String" << std::endl;
						break;
					case fmi2_base_type_enum:
						// std::cout << " Type: Enum" << std::endl;
						break;
					default:
						// std::cout << " Type: Unknown" << std::endl;
						break;
					}
					if ( fmu_out == nullptr ) continue; // Not a type we care about
					size_type const idx( fmu_out->idx );
					auto iout( fmu_idxs.find( idx ) ); //Do Add support for input variable dependents
					if ( ( iout == fmu_idxs.end() ) && ( fmu_var != nullptr ) ) iout = fmu_idxs.find( fmu_var->idx ); // Use variable that output variable is derivative of
					if ( iout != fmu_idxs.end() ) { // Output variable corresponds to a QSS variable
						Variable * out_var( iout->second );
						// std::cout << " FMU-ME idx: " << fmu_out->idx << " maps to QSS var: " << out_var->name() << std::endl;
//						if ( out_var->not_ZC() ) continue; // Don't worry about dependencies of non-ZC output variables on the QSS side //?
						for ( size_type j = startIndex[ i ]; j < startIndex[ i + 1 ]; ++j ) {
							size_type const dep_idx( dependency[ j ] );
							// std::cout << "  Dep Index: " << dep_idx << std::endl;
// 							if ( dep_idx == 0u ) { // No info: Depends on all (don't support depends on all for now)
// 								std::cerr << "   Error: No dependency information provided: Depends-on-all not currently supported" << std::endl;
// //								std::exit( EXIT_FAILURE ); //OCT Let run proceed while waiting for OCT fixes
// 							} else { // Process based on kind of dependent
// 								fmi2_dependency_factor_kind_enu_t const kind( (fmi2_dependency_factor_kind_enu_t)( factorKind[ j ] ) );
// 								if ( kind == fmi2_dependency_factor_kind_dependent ) {
// 									std::cout << "   Kind: Dependent (" << kind << ')' << std::endl;
// 								} else if ( kind == fmi2_dependency_factor_kind_constant ) {
// 									std::cout << "   Kind: Constant (" << kind << ')' << std::endl;
// 								} else if ( kind == fmi2_dependency_factor_kind_fixed ) {
// 									std::cout << "   Kind: Fixed (" << kind << ')' << std::endl;
// 								} else if ( kind == fmi2_dependency_factor_kind_tunable ) {
// 									std::cout << "   Kind: Tunable (" << kind << ')' << std::endl;
// 								} else if ( kind == fmi2_dependency_factor_kind_discrete ) {
// 									std::cout << "   Kind: Discrete (" << kind << ')' << std::endl;
// 								} else if ( kind == fmi2_dependency_factor_kind_num ) {
// 									std::cout << "   Kind: Num (" << kind << ')' << std::endl;
// 								}
// 							}
							auto const idep( fmu_idxs.find( dep_idx ) ); //Do Add support for input variable dependents
							if ( idep != fmu_idxs.end() ) { // Dependency is a QSS variable
								Variable * dep( idep->second );
								out_var->observe( dep );
								std::cout << "  " << dep->name() << std::endl;
							// } else { // Dependency is a non-QSS variable
							// 	std::cout << "   Note: Output variable " << out_name << " has dependency on non-QSS variable with index " << dep_idx << std::endl;
							}
						}
//					} else {
//						std::cout << "   Output variable is not a QSS variable" << std::endl;
					}
				}
			// } else { // No output variable dependencies
			// 	std::cout << "\nNo output variable dependency info in FMU-ME XML" << std::endl;
			}
		}

		// Dependencies added with --dep on comand line
		if ( options::dep.all() ) {
			for ( Variable * var : vars ) {
				for ( Variable * dep : vars ) { // Add the dependency
					var->observe( dep );
				}
			}
		} else if ( options::dep.any() ) {
			for ( Variable * var : vars ) {
				for ( options::DepSpecs::Dependency const & dependency : options::dep.dependencies() ) {
					if ( std::regex_match( var->name(), dependency.spec ) ) {
						for ( std::regex const & dep_regex : dependency.deps ) {
							for ( Variable * dep : vars ) {
								if ( std::regex_match( dep->name(), dep_regex ) ) { // Add the dependency
									var->observe( dep );
								}
							}
						}
					}
				}
			}
		}

		// Generate Direct Dependency Graph
		if ( options::dot_graph::d ) {
			std::ofstream dependency_graph( name + ".Dependency.gv", std::ios_base::binary | std::ios_base::out );
			dependency_graph << "digraph " << name << " {\n";
			dependency_graph << "  label=\"" << name << " Direct Dependency Graph" << "\"; labelloc=\"t\"\n";
			for ( auto var : sorted_by_name( vars ) ) { // Variable dependencies
				var->uniquify_observees();
				if ( var->self_observer() ) {
					dependency_graph << "  \"" << var->name() << "\" -> \"" << var->name() << "\"\n";
				}
				for ( auto o : sorted_by_name( var->observees() ) ) {
					dependency_graph << "  \"" << var->name() << "\" -> \"" << o->name() << "\"\n";
				}
				if ( var->is_ZC() ) { // Dependencies of variables modified when event indicators fire
					for ( auto o : sorted_by_name( static_cast< Variable_ZC * >( var )->conditional->observers() ) ) {
						dependency_graph << "  \"" << o->name() << "\" -> \"" << var->name() << "\" [ color=\"darkseagreen\", style=\"dashed\"]\n";
					}
				}
			}
			dependency_graph << "}\n";
			dependency_graph.close();
		}

		// Set Computational Self-Observer Status
		for ( auto var : vars ) {
			if ( var->self_observer() ) {
				if ( var->is_ZC() ) {
					std::cerr << "Error: Zero-crossing variable is self-observer: " << var->name() << std::endl;
					std::exit( EXIT_FAILURE );
				} else if ( var->not_State() ) { // Only use self-observer for state variables in QSS
					var->self_observe_off();
				}
			}
		}

		// Size checks
		if ( n_state_vars != n_states ) {
			std::cerr << "\nError: Number of state variables found (" << n_state_vars << ") is not equal to number in FMU-ME (" << n_states << ')' << std::endl;
		}

		// Sizes
		n_vars = vars.size();
		n_f_outs = f_outs_vars.size();
		n_l_outs = fmu_outs.size();

		// Variable output filtering
		for ( auto var : vars ) {
			if ( !output_filter( var->name() ) ) var->out_turn_off();
		}

		// Variable subtype containers and specs
		vars_ZC.clear();
		vars_NZ.clear();
		vars_CI.clear();
		vars_NC.clear();
		order_max_NC = order_max_CI = 0;
		for ( auto var : vars ) {
			if ( var->is_ZC() ) { // ZC variable
				vars_ZC.push_back( var );
			} else { // Non-ZC variable
				vars_NZ.push_back( var );
				if ( var->is_connection() ) { // Connection variable
					vars_CI.push_back( var );
					order_max_CI = std::max( order_max_CI, var->order() );
				} else { // Non-Connection/ZC variable
					vars_NC.push_back( var );
					order_max_NC = std::max( order_max_NC, var->order() );
				}
			}
		}
		sort_by_type_and_order( vars_NC ); // Put state variables first to reduce issue of directional derivatives needing observee derivatives set
		assert( order_max_CI <= max_rep_order );
		assert( order_max_NC <= max_rep_order );
	}

	// Optimize ND Time Step
	void
	FMU_ME::
	dtND_optimize( Time const to )
	{
		// Note: Zero-crossing variables are not currently considered since they aren't integrated but it may be worth adding them

		assert( options::dtND_optimizer );

		if ( ( order_max_NC <= 1 ) || ( vars_NC.size() == 0u ) ) return; // Nothing to optimize

		Time const dtND_ori( options::dtND );
		Time const dtND_min( std::max( std::abs( t0 ), std::abs( tE ) ) * std::numeric_limits< Time >::epsilon() * 2.0 );
		Time const dtND_max( options::dtND_max );
		Time dtND( dtND_max );
		Time dtND_opt( dtND_ori );
		assert( dtND_min < dtND_ori );

		size_type const n_NC( vars_NC.size() );
		using DtVec = std::vector< Time >;
		using DerVec = std::vector< Real >;
		using DerVecs = std::vector< DerVec >;
		DtVec dtNDs;
		DerVecs x2( n_NC ); // Second derivatives
		DerVecs x3( n_NC ); // Third derivatives

		// Derivatives with dtND max
		options::dtND_set( dtND );
		dtNDs.push_back( dtND );
		init_2_1();
		init_3_1();
		for ( size_type i = 0; i < n_NC; ++i ) {
			Real const x2_i( vars_NC[ i ]->x2( to ) );
			x2[ i ].push_back( x2_i );
			if ( vars_NC[ i ]->order() >= 3 ) {
				Real const x3_i( vars_NC[ i ]->x3( to ) );
				x3[ i ].push_back( x3_i );
			}
		}

		// Derivatives with half dtND max
		options::dtND_set( dtND *= 0.5 );
		dtNDs.push_back( dtND );
		init_2_1();
		init_3_1();
		for ( size_type i = 0; i < n_NC; ++i ) {
			Real const x2_i( vars_NC[ i ]->x2( to ) );
			x2[ i ].push_back( x2_i );
			if ( vars_NC[ i ]->order() >= 3 ) {
				Real const x3_i( vars_NC[ i ]->x3( to ) );
				x3[ i ].push_back( x3_i );
			}
		}

		// Derivatives as dtND decreases
		while ( dtND >= dtND_min * 2.0 ) {
			options::dtND_set( dtND *= 0.5 );
			dtNDs.push_back( dtND );
			init_2_1();
			init_3_1();
			for ( size_type i = 0; i < n_NC; ++i ) {
				Real const x2_i( vars_NC[ i ]->x2( to ) );
				x2[ i ].push_back( x2_i );
				Real const x3_i( vars_NC[ i ]->x3( to ) );
				x3[ i ].push_back( x3_i );
			}
		}

		size_type const n_dtND( dtNDs.size() );
		if ( n_dtND >= 2u ) { // Compute and assign the optimal dtND
			using Ranges = std::vector< Range >;
			Ranges ranges;
			size_type n_dtND_vars( 0u );

			for ( size_type i = 0; i < n_NC; ++i ) { // Each variable
				if ( vars_NC[ i ]->order() >= 2 ) {
					size_type l( 0u ), u( 1u );
					Real d2_min( std::abs( x2[ i ][ 1 ] - x2[ i ][ 0 ] ) );
					for ( size_type j = 2; j < n_dtND; ++j ) { // Each dtND interval
						Real const d2( std::abs( x2[ i ][ j ] - x2[ i ][ j-1 ] ) );
						if ( d2 <= d2_min ) {
							u = j;
							if ( d2 < d2_min ) {
								l = j-1;
								d2_min = d2;
							}
						}
					}
					if ( ( d2_min > 0.0 ) || ( u < n_dtND - 1u ) || ( l > 0u ) ) { // Add range
						++n_dtND_vars;
						ranges.emplace_back( l, u + 1 );
					}
				}
			}

			if ( order_max_NC >= 3 ) { // 3rd order
				for ( size_type i = 0; i < n_NC; ++i ) { // Each variable
					if ( vars_NC[ i ]->order() >= 3 ) {
						size_type l( 0u ), u( 1u );
						Real d3_min( std::abs( x3[ i ][ 1 ] - x3[ i ][ 0 ] ) );
						for ( size_type j = 2; j < n_dtND; ++j ) { // Each dtND interval
							Real const d3( std::abs( x3[ i ][ j ] - x3[ i ][ j-1 ] ) );
							if ( d3 <= d3_min ) {
								u = j;
								if ( d3 < d3_min ) {
									l = j-1;
									d3_min = d3;
								}
							}
						}
						if ( ( d3_min > 0.0 ) || ( u < n_dtND - 1u ) || ( l > 0u ) ) { // Add range
							++n_dtND_vars;
							ranges.emplace_back( l, u + 1 );
						}
					}
				}
			}

			if ( n_dtND_vars > 0u ) { // Find range intersection expanding ranges if needed
				assert( !ranges.empty() );
				Range ri; // Intersection range
				while ( ri.empty() ) {
					ri = ranges[ 0 ];
					for ( Range const & r : ranges ) {
						ri.intersect( r );
						if ( ri.empty() ) { // Expand ranges
							for ( Range & re : ranges ) { // Expand range
								if ( re.b() > 0u ) --re.b();
								if ( re.e() < n_dtND ) ++re.e();
							}
							break; // Try with larger ranges
						}
					}
				}
				assert( !ri.empty() );
				dtND_opt = dtND_max * std::pow( 2.0, -static_cast< double >( ri.b() ) ); // Use largest dtND in range intersection for now
				options::dtND_set( dtND_opt );
				std::cout << "\nAutomatic numeric differentiation time step: " << options::dtND << " (s)" << std::endl;
			} else {
				std::cout << "\nNumeric differentiation time step can't be set automatically" << std::endl;
				options::dtND_set( dtND_ori );
			}

		} else {
			std::cout << "\nNumeric differentiation time step can't be set automatically" << std::endl;
			options::dtND_set( dtND_ori );
		}
	}

	// Initialization
	void
	FMU_ME::
	init()
	{
		double const dtND_min( std::max( std::abs( t0 ), std::abs( tE ) ) * std::numeric_limits< double >::epsilon() * 2.0 );
		if ( options::dtND < dtND_min ) {
			options::dtND = dtND_min;
			std::cout << "\nNumeric differentiation time step raised for compatibility with time range and double precision epsilon: " << options::dtND << std::endl;
		}
		init_0_0();
		init_0_1();
		init_0_2();
		init_1_1();
		init_1_2();
		if ( options::dtND_optimizer ) dtND_optimize( t0 );
		init_2_1();
		init_2_2();
		init_3_1();
		init_ZC();
		init_F();
		init_t0();
		init_pre_simulate();
	}

	// Initialization: Stage 0.0
	void
	FMU_ME::
	init_0_0()
	{
		//std::cout << '\n' + name + " Initialization: Stage 0.0 =====" << std::endl;
		for ( auto var : vars ) { // Uniquify variable observees
			var->uniquify_observees();
		}
		for ( auto var : vars_NZ ) { // Uniquify variable observers
			var->uniquify_observers();
		}
	}

	// Initialization: Stage 0.1
	void
	FMU_ME::
	init_0_1()
	{
		std::cout << '\n' + name + " Initialization: Stage 0.1 =====" << std::endl;
		set_time( t0 );
		init_derivatives();
		if ( t0 != Time( 0 ) ) {
			for ( auto var : vars ) {
				var->init_time( t0 );
			}
		}
		for ( auto var : sorted_by_name( vars_NC ) ) {
			var->init_0();
		}
	}

	// Initialization: Stage 0.2
	void
	FMU_ME::
	init_0_2()
	{
		std::cout << '\n' + name + " Initialization: Stage 0.2 =====" << std::endl;
		for ( auto var : sorted_by_name( vars_CI ) ) {
			var->init_0();
		}
		for ( auto var : sorted_by_name( vars_ZC ) ) { // Initialize zero-crossing variable observees
			var->init_observees();
		}
		for ( auto var : vars_NZ ) { // Initialize observers: all variable observees must be initialized first
			var->init_observers();
		}
		for ( auto var : sorted_by_name( vars_NZ ) ) { // Assign computational observers after all are computed and finish initialization
			var->finalize_observers();
		}
	}

	// Initialization: Stage 1.1
	void
	FMU_ME::
	init_1_1()
	{
		std::cout << '\n' + name + " Initialization: Stage 1.1 =====" << std::endl;
		get_derivatives();
		for ( auto var : vars_NC ) {
			var->init_1();
		}
	}

	// Initialization: Stage 1.2
	void
	FMU_ME::
	init_1_2()
	{
		std::cout << '\n' + name + " Initialization: Stage 1.2 =====" << std::endl;
		for ( auto var : vars_CI ) {
			var->init_1();
		}
	}

	// Initialization: Stage 2.1
	void
	FMU_ME::
	init_2_1()
	{
		std::cout << '\n' + name + " Initialization: Stage 2.1 =====" << std::endl;
		if ( order_max_NC >= 2 ) {
			for ( auto var : vars_NC ) {
				var->init_2();
			}
		}
	}

	// Initialization: Stage 2.2
	void
	FMU_ME::
	init_2_2()
	{
		std::cout << '\n' + name + " Initialization: Stage 2.2 =====" << std::endl;
		if ( order_max_CI >= 2 ) {
			for ( auto var : vars_CI ) {
				var->init_2();
			}
		}
	}

	// Initialization: Stage 3.1
	void
	FMU_ME::
	init_3_1()
	{
		std::cout << '\n' + name + " Initialization: Stage 3.1 =====" << std::endl;
		if ( order_max_NC >= 2 ) {
			for ( auto var : vars_NC ) {
				var->init_3();
			}
		}
	}

	// Initialization: Stage ZC
	void
	FMU_ME::
	init_ZC()
	{
		std::cout << '\n' + name + " Initialization: Stage ZC =====" << std::endl;
		for ( auto var : sorted_by_name( vars_ZC ) ) {
			var->init_0();
		}
	}

	// Initialization: Stage Final
	void
	FMU_ME::
	init_F()
	{
		std::cout << '\n' + name + " Initialization: Stage Final =====" << std::endl;
		for ( auto var : vars_NC ) {
			var->init_F();
		}
	}

	// Initialization: Stage Set State to t0
	void
	FMU_ME::
	init_t0()
	{
		// Set variable FMU state to t0 after initialization ND steps before generating FMU local/output variable outputs at t0
		set_time( t0 );
		for ( auto var : state_vars ) { // State variables
			var->fmu_set_x( t0 );
		}
		for ( auto var : vars_CI ) { // Connection input variables
			var->fmu_set_x( t0 );
		}
		for ( auto var : vars_NC ) { // Non-zero-crossing non-connection variables
			if ( var->is_Input() ) var->fmu_set_x( t0 ); // Non-connection input variables
		}
	}

	// Initialization: Stage Pre-Simulate
	void
	FMU_ME::
	init_pre_simulate()
	{
		// Initialize Conditional observers
		for ( Conditional< Variable_ZC > * con : sorted_by_name( cons ) ) {
			con->init_observers();
		}

		// Generate computational observee graph
		if ( options::dot_graph::e ) {
			std::ofstream observee_graph( name + ".Observee.gv", std::ios_base::binary | std::ios_base::out );
			observee_graph << "digraph " << name << " {\n";
			observee_graph << "  label=\"" << name << " Computational Observee Graph" << "\"; labelloc=\"t\"\n";
			for ( auto var : sorted_by_name( vars ) ) { // Variable dependencies
				if ( var->self_observer() ) {
					observee_graph << "  \"" << var->name() << "\" -> \"" << var->name() << "\"\n";
				}
				for ( auto o : sorted_by_name( var->observees() ) ) {
					observee_graph << "  \"" << var->name() << "\" -> \"" << o->name() << "\"\n";
				}
				if ( var->is_ZC() ) { // Dependencies of variables modified when event indicators fire
					for ( auto o : sorted_by_name( static_cast< Variable_ZC * >( var )->conditional->observers() ) ) {
						observee_graph << "  \"" << o->name() << "\" -> \"" << var->name() << "\" [ color=\"darkseagreen\", style=\"dashed\"]\n";
					}
				}
			}
			observee_graph << "}\n";
			observee_graph.close();
		}

		// Generate computational observer graph
		if ( options::dot_graph::r ) {
			std::ofstream observer_graph( name + ".Observer.gv", std::ios_base::binary | std::ios_base::out );
			observer_graph << "digraph " << name << " {\n";
			observer_graph << "  label=\"" << name << " Computational Observer Graph" << "\"; labelloc=\"t\"\n";
			for ( auto var : sorted_by_name( vars ) ) { // Variable dependencies
				// if ( var->self_observer() ) { // Self-observer dependency is not relevant for observers (upstream change signaling)
				// 	observer_graph << "  \"" << var->name() << "\" -> \"" << var->name() << "\"\n";
				// }
				for ( auto o : sorted_by_name( var->observers() ) ) {
					observer_graph << "  \"" << o->name() << "\" -> \"" << var->name() << "\"\n";
				}
				if ( var->is_ZC() ) { // Dependencies of variables modified when event indicators fire
					for ( auto o : sorted_by_name( static_cast< Variable_ZC * >( var )->conditional->observers() ) ) {
						observer_graph << "  \"" << o->name() << "\" -> \"" << var->name() << "\" [ color=\"darkseagreen\", style=\"dashed\"]\n";
					}
				}
			}
			observer_graph << "}\n";
			observer_graph.close();
		}

		// Dependency cycle detection: After observers set up
		if ( options::cycles ) cycles< Variable, Variable_ZC >( vars );

		// Find continuous state variable self-dependency cycles (clusters): After computational observees set up
		if ( options::cluster ) {
			std::cout << '\n' + name + " Clustering =====" << std::endl;
			clusters< Variable, Variable_QSS >( state_vars );
			for ( Variable_QSS * var : state_vars ) {
				var->uniquify_cluster();
			}
		}

		// Output initialization
		if ( options::output::K && ( out_var_refs.size() > 0u ) ) { // FMU t0 smooth token outputs
			for ( auto const & var_ref : out_var_refs ) {
				auto ivar( qss_var_of_ref.find( var_ref ) );
				if ( ivar != qss_var_of_ref.end() ) {
					if ( output_filter.fmu( ivar->second->name() ) ) {
						fmu_qss_qss_outs.push_back( ivar->second );
					}
				}
			}
			n_fmu_qss_qss_outs = fmu_qss_qss_outs.size();
		}
		doROut = ( options::output::R && ( options::output::X || options::output::Q ) );
		doZOut = ( options::output::Z && ( options::output::X || options::output::Q ) );
		doDOut = ( options::output::D && ( options::output::X || options::output::Q ) );
		doTOut = options::output::T;
		doSOut = (
		 ( options::output::S && ( options::output::X || options::output::Q ) ) ||
		 ( options::output::F && ( n_f_outs > 0u ) ) ||
		 ( options::output::L && ( n_l_outs > 0u ) ) ||
		 ( options::output::K && ( n_fmu_qss_qss_outs > 0u ) ) ||
		 options::csv
		);
		doKOut = options::output::K && ( out_var_refs.size() > 0u );
		std::string const output_dir( options::have_multiple_models() ? name : std::string() );
		if ( ( ( options::output::R || options::output::Z || options::output::D || options::output::S ) && ( options::output::X || options::output::Q ) ) || options::output::T ) { // QSS t0 outputs
#ifdef _WIN32
			name_decorate( vars );
#endif
			for ( auto var : vars ) {
				if ( output_filter.fmu( var->name() ) ) {
					var->init_out( output_dir );
					if ( doROut || doZOut || doDOut || doSOut ) var->out( t );
					if ( doTOut ) var->out_t( t );
				}
			}
		}
		if ( options::output::F && ( n_f_outs > 0u ) ) { // FMU QSS variable t0 outputs
			f_outs.reserve( n_f_outs );
			for ( auto const & var : f_outs_vars ) { // FMU QSS variables
				f_outs.emplace_back( output_dir, var->name() + var->decoration(), 'f' );
				Output<> & f_out( f_outs.back() );
				if ( options::output::h ) {
					if ( var->var().is_Real() ) {
						char const * var_type_char( fmi2_import_get_real_variable_quantity( var->var().rvr ) );
						std::string const var_type( var_type_char == nullptr ? "" : var_type_char );
						fmi2_import_unit_t * const var_unit_ptr( fmi2_import_get_real_variable_unit( var->var().rvr ) );
						std::string const var_unit( var_unit_ptr == nullptr ? "" : fmi2_import_get_unit_name( var_unit_ptr ) );
						f_out.header( var_type, var_unit );
					} else if ( var->var().is_Integer() ) {
						char const * var_type_char( fmi2_import_get_integer_variable_quantity( var->var().ivr ) );
						std::string const var_type( var_type_char == nullptr ? "" : var_type_char );
						// Integer variables have no unit
						f_out.header( var_type );
					} else { // Modelica Boolean variables can have a quantity but there is no FMIL API for getting it
						f_out.header();
					}
				}
				f_out.append( t, var->x( t ) );
			}
		}
		if ( options::output::L && ( n_l_outs > 0u ) ) { // FMU local variable t0 outputs
#ifdef _WIN32
			std::vector< std::string > names;
			for ( auto const & e : fmu_outs ) {
				FMU_Variable const & var( *(e.second) );
				names.emplace_back( fmi2_import_get_variable_name( var.var ) );
			}
			std::unordered_map< std::string, std::string > decs;
			name_decorations( names, decs );
#endif
			l_outs.reserve( n_l_outs );
			for ( auto const & e : fmu_outs ) {
				FMU_Variable const & var( *(e.second) );
				std::string const var_name( fmi2_import_get_variable_name( var.var ) );
#ifdef _WIN32
				l_outs.emplace_back( output_dir, var_name, 'f', decs[ var_name ] );
#else
				l_outs.emplace_back( output_dir, var_name, 'f' );
#endif
				Output<> & l_out( l_outs.back() );
				if ( options::output::h ) {
					if ( var.is_Real() ) {
						char const * var_type_char( fmi2_import_get_real_variable_quantity( var.rvr ) );
						std::string const var_type( var_type_char == nullptr ? "" : var_type_char );
						fmi2_import_unit_t * const var_unit_ptr( fmi2_import_get_real_variable_unit( var.rvr ) );
						std::string const var_unit( var_unit_ptr == nullptr ? "" : fmi2_import_get_unit_name( var_unit_ptr ) );
						l_out.header( var_type, var_unit );
					} else if ( var.is_Integer() ) {
						char const * var_type_char( fmi2_import_get_integer_variable_quantity( var.ivr ) );
						std::string const var_type( var_type_char == nullptr ? "" : var_type_char );
						// Integer variables have no unit
						l_out.header( var_type );
					} else { // Modelica Boolean variables can have a quantity but there is no FMIL API for getting it
						l_out.header();
					}
				}
				l_out.append( t, get_as_real( var ) );
			}
		}
		if ( doKOut ) { // FMU-QSS t0 smooth token outputs
			for ( Variable * var : fmu_qss_qss_outs ) {
				k_qss_outs.emplace_back( output_dir, var->name() + var->decoration(), 'k' );
				k_qss_outs.back().append( t, var->k( t ) );
			}
//			for ( FMU_Variable const & fmu_var : fmu_qss_fmu_outs ) {
//				k_fmu_outs.emplace_back( output_dir, fmi2_import_get_variable_name( fmu_var.var, 'k' ) );
//				k_fmu_outs.back().append( t, get_real( fmu_var.ref() ) ); //Do SmoothToken once we can get derivatives
//			}
		}

		// Simulation loop initialization
		tPer = 0;
		n_discrete_events = 0;
		n_QSS_events = 0;
		n_QSS_simultaneous_events = 0;
		n_ZC_events = 0;
		sim_dtMin = options::dtMin;
		pass_warned = false;
		enterEventMode = fmi2_false;
		terminateSimulation = fmi2_false;
		sim_cpu_time = 0; // CPU time
#ifdef _OPENMP
		sim_wall_time = 0.0; // Wall time
#endif

		std::cout << '\n' + name + " Simulation Starting =====" << std::endl;
	}

	// Simulation Pass
	void
	FMU_ME::
	simulate( fmi2_event_info_t * eventInfoMaster, bool const connected )
	{
		// Types
		using Variables = Variable::Variables;
		using Variable_ZCs = std::vector< Variable_ZC * >;
		using size_type = Variables::size_type;
		using Time = Variable::Time;

		// I/o setup
		std::cout << std::setprecision( 16 );
		std::cerr << std::setprecision( 16 );

		if ( options::output::d ) std::cout << '\n' + name + " Simulation Loop =====" << std::endl;

		// Timing setup
		Time const tSim( tE - t0 ); // Simulation time span expected
		Time const tPass( eventq->top_time() ); // Pass start time
		Time tNext( eventInfoMaster->nextEventTimeDefined ? std::min( eventInfoMaster->nextEventTime, tE ) : tE );
		Time t_bump( t ); // Bump time for FMU crossing detection
		double const cpu_time_beg( cpu_time() ); // CPU time
#ifdef _OPENMP
		double const wall_time_beg( omp_get_wtime() ); // Wall time
#endif

		// Binning setup
		size_type max_bin_size( 1u ); // Max bin size used since last bin optimizer pass
		size_type bin_size( std::min( options::bin_size, max( state_vars.size(), vars_ZC.size(), vars.size() - state_vars.size() - vars_ZC.size() ) ) ); // Initial bin size: Bin optimizer will adjust it during the run in auto mode
		Real const bin_frac( options::bin_frac ); // Min time step fraction for a binned variable
		std::pair< size_type, size_type > bin_size_auto( 0u, 0u ); // Automatic bin size total and count for reporting average
		Time const bin_performance_dt_max( tSim / 5.0 ); // Max solution time span for checking performance
		Time bin_performance_dt( 0.0 ); // Min solution time span for checking performance: adjusted on the fly
		timers::Performance bin_performance( tPass ); // Solution performance "stopwatch"
		BinOptimizer bin_optimizer( state_vars.size() ); // Bin size optimizer
		bool const bin_auto( options::specified::bin && options::bin_auto );
		if ( bin_auto ) bin_performance.start( t ); // Initialize solution performance metric

		// Simulation loop
		Variables triggers; // Reusable triggers container
		Variables cluster_triggers; // Reusable cluster triggers container
		Variables handlers; // Reusable handlers container
		Variable_ZCs var_ZCs; // Last zero-crossing trigger variables
		Handlers< Variable > handlers_s( this ); // Simultaneous handlers
		Triggers< Variable > triggers_s( this ); // Binned/simultaneous triggers
		Triggers_ZC< Variable > triggers_zc_s( this ); // Binned/simultaneous triggers
		Triggers_R< Variable > triggers_r_s( this ); // Binned/simultaneous triggers
		Observers< Variable > observers_s( this ); // Binned/simultaneous observers
		bool connected_output_event( false );
		while ( t <= tNext ) {
			t = eventq->top_time();
			if ( doSOut ) { // QSS and/or FMU sampled outputs
				Time const tStop( std::min( t, tNext ) );
				while ( tOut < tStop ) {
					set_time( tOut );
					if ( options::output::S ) { // QSS outputs
						for ( auto var : vars ) {
							var->out( tOut );
						}
					}
					if ( options::output::F ) { // FMU QSS variable outputs
						if ( n_f_outs > 0u ) { // FMU QSS variables
							for ( size_type i = 0; i < n_f_outs; ++i ) {
								Variable * var( f_outs_vars[ i ] );
								f_outs[ i ].append( tOut, var->x( tOut ) );
							}
						}
					}
					if ( options::output::L ) { // FMU local variable outputs
						if ( n_l_outs > 0u ) { // FMU local variables
							set_time( tOut );
//							for ( size_type i = 0; i < n_states; ++i ) {
//								if ( state_vars[ i ] != nullptr ) states[ i ] = state_vars[ i ]->x( tOut );
//							}
//							fmi2_import_set_continuous_states( fmu, states, n_states );
							for ( auto var : vars_NC ) {
								var->fmu_set_x( tOut );
							}
							size_type i( 0u );
							for ( auto const & e : fmu_outs ) {
								FMU_Variable const & var( *(e.second) );
								l_outs[ i ].append( tOut, get_as_real( var ) );
								++i;
							}
						}
					}
					if ( options::output::K ) { // FMU-QSS smooth token outputs
						if ( n_fmu_qss_qss_outs > 0u ) {
							for ( size_type i = 0; i < n_fmu_qss_qss_outs; ++i ) {
								Variable * var( fmu_qss_qss_outs[ i ] );
								k_qss_outs[ i ].append( tOut, var->k( tOut ) );
							}
						}
//						if ( fmu_qss_fmu_outs.size() > 0u ) {
//							set_time( tOut );
//							for ( size_type i = 0; i < n_states; ++i ) {
//								if ( state_vars[ i ] != nullptr ) states[ i ] = state_vars[ i ]->x( tOut );
//							}
//							fmi2_import_set_continuous_states( fmu, states, n_states );
//							size_type i( n_fmu_qss_qss_outs );
//							for ( FMU_Variable const & fmu_var : fmu_qss_qss_outs ) {
//								k_fmu_outs[ i++ ].append( tOut, get_real( fmu_var.ref() ) ); //Do SmoothToken once we can get derivatives
//							}
//						}
					}
					if ( options::csv ) {
						for ( auto var : vars ) {
							var->fmu_set_x( tOut );
						}
						for ( std::size_t i = 0, n = res_var_indexes.size(); i < n; ++i ) {
							res_var_vals[ i ] = get_as_real( fmu_variables[ res_var_indexes[ i ] ] );
						}
						csv.values( res_var_vals );
					}
					assert( iOut < std::numeric_limits< size_type >::max() );
					tOut = t0 + ( ++iOut ) * options::dtOut;
				}
			}
			if ( t <= tNext ) { // Perform event(s)

				if ( connected ) { // Check if next event(s) will modify a connected output
					if ( options::perfect ) { // Flag whether next event(s) will modify a connected output
						connected_output_event = false;
						auto const tops( eventq->tops() );
						for ( auto i = tops.first; i != tops.second; ++i ) {
							Target const * target( i->second.target() );
							if ( target->connected_output || target->connected_output_observer ) {
								connected_output_event = true;
								break;
							}
						}
					} else if ( eventq->top_time() > tPass ) { // Stop if beyond pass start time and next event(s) will modify a connected output
						bool connected_output_next( false );
						auto const tops( eventq->tops() );
						for ( auto i = tops.first; i != tops.second; ++i ) {
							Target const * target( i->second.target() );
							if ( target->connected_output || target->connected_output_observer ) {
								connected_output_next = true;
								break;
							}
						}
						if ( connected_output_next ) break; // Exit t loop
					}
				}

				set_time( t );
				Event< Target > & event( eventq->top() );
				SuperdenseTime const s( eventq->top_superdense_time() );
				if ( s.i >= options::pass ) { // Pass count limit reached
					if ( s.i <= max_pass_count_multiplier * options::pass ) { // Use time step controls
						if ( sim_dtMin > 0.0 ) { // Double dtMin
							if ( sim_dtMin < std::min( 0.5 * infinity, 0.25 * options::dtMax ) ) {
								sim_dtMin = std::min( 2.0 * sim_dtMin, 0.5 * options::dtMax );
							} else {
								std::cerr << "\nError: Pass count limit exceeded at time: " << t << "  Min time step limit reached: Terminating simulation" << std::endl;
								tNext = t; // To avoid tNext outputs beyond actual simulation
								break;
							}
						} else { // Set dtMin
							sim_dtMin = std::min( 2.0 * std::numeric_limits< Time >::epsilon() * std::max( std::abs( t0 ), std::abs( tE ) ), 0.5 * options::dtMax );
						}
						for ( auto var : vars ) {
							var->dt_min = sim_dtMin;
						}
						if ( !pass_warned ) {
							std::cerr << "\nWarning: Pass count limit reached at time: " << t << "  Min time step control activated" << std::endl;
							pass_warned = true;
						}
					} else { // Time step control doesn't seem to be working: Abort
						std::cerr << "\nError: " << max_pass_count_multiplier << " x pass count limit exceeded at time: " << t << "  Terminating simulation" << std::endl;
						tNext = t; // To avoid tNext outputs beyond actual simulation
						break;
					}
				}
				eventq->set_active_time();
				if ( event.is_discrete() ) { // Discrete event(s)
					++n_discrete_events;
					if ( eventq->single() ) { // Single trigger
						Variable * trigger( event.sub< Variable >() );
						assert( trigger->tD == t );

						trigger->st = s; // Set trigger superdense time

						if ( doDOut ) { // Discrete event output: pre
							trigger->out( t );
							trigger->observers_out_pre( t );
						}

						trigger->advance_discrete();

						if ( doDOut ) { // Discrete event output: post
							if ( options::output::A ) { // All variables
								for ( auto var : vars ) {
									var->out( t );
								}
							} else { // Trigger and observers
								trigger->out( t );
								trigger->observers_out_post( t );
							}
						}
					} else { // Simultaneous triggers
						eventq->top_subs< Variable >( triggers );
						observers_s.assign( triggers );
						sort_by_order( triggers );

						if ( doDOut ) { // Discrete event output: pre
							for ( Variable * trigger : triggers ) { // Triggers
								trigger->out( t );
							}
							if ( options::output::O ) { // Observers
								for ( Variable * observer : observers_s ) {
									observer->observer_out_pre( t );
								}
							}
						}

						for ( Variable * trigger : triggers ) {
							assert( trigger->tD == t );
							trigger->st = s; // Set trigger superdense time
							trigger->advance_discrete_simultaneous();
						}
						if ( observers_s.have() ) observers_s.advance( t ); // Advance observers

						if ( doDOut ) { // Discrete event output: post
							if ( options::output::A ) { // All variables
								for ( auto var : vars ) {
									var->out( t );
								}
							} else { // Triggers and observers
								for ( Variable * trigger : triggers ) { // Triggers
									trigger->out( t );
								}
								if ( options::output::O ) { // Observers
									for ( Variable * observer : observers_s ) {
										observer->observer_out_post( t );
									}
								}
							}
						}
					}
				} else if ( event.is_ZC() ) { // Zero-crossing event(s)
					++n_ZC_events;
					var_ZCs.clear();
					t_bump = t; // Bump time for FMU zero-crossing detection
					while ( eventq->top_superdense_time() == s ) { // Set bump time and do zero-crossing outputs
						Variable_ZC * trigger( eventq->top_sub< Variable_ZC >() );
						var_ZCs.push_back( trigger );
						assert( eq_tol( trigger->tZ, t, 1e-15 ) );
						trigger->st = s; // Set trigger superdense time
						trigger->advance_ZC();
						++c_ZC_events[ trigger ];
						t_bump = std::max( t_bump, trigger->tZC_bump( t ) );
						if ( doZOut ) { // Zero-crossing event output
							if ( options::output::A ) { // All variables
								for ( auto var : vars ) {
									var->out( t );
								}
							} else { // Trigger
								trigger->out( t );
							}
						}
					}
				} else if ( event.is_conditional() ) { // Conditional event(s)
					while ( eventq->top_superdense_time() == s ) {
						Conditional< Variable_ZC > * trigger( eventq->top_sub< Conditional< Variable_ZC > >() );
						trigger->st = s; // Set trigger superdense time
						trigger->advance_conditional(); // Set handler observee state before FMU event detection and shift conditional's next event to t=infinity
					}
				} else if ( event.is_handler() ) { // Zero-crossing handler event(s)

					// Pre-zero-crossing time bump to set event indicator state before the crossing so FMU can detect relevant crossings

					Time const t_pre_bump( ( 2.0 * t ) - t_bump ); // Pre-bump time for FMU crossing detection
					if ( options::output::d ) std::cout << "Zero-crossing handler event(s): Pre-bump time = " << t_pre_bump << std::endl;
					set_time( t_pre_bump ); // Set FMU to pre-bump time
					for ( Variable_ZC const * trigger : var_ZCs ) { // Advance zero-crossing variables observees to pre-bump time
						trigger->bump_time( t_pre_bump );
					}

					// Get event indicators
					fmi2_import_get_event_indicators( fmu, event_indicators, n_event_indicators );
					// if ( options::output::d ) {
					// 	std::cout << "FMU event indicators: Handler event processing start @ t=" << t_pre_bump << std::endl;
					// 	for ( size_type k = 0; k < n_event_indicators; ++k ) std::cout << event_indicators[ k ] << std::endl;
					// }
					fmi2_import_enter_event_mode( fmu );
					do_event_iteration();
					fmi2_import_enter_continuous_time_mode( fmu );
					fmi2_import_get_continuous_states( fmu, states, n_states );
					fmi2_import_get_event_indicators( fmu, event_indicators, n_event_indicators );

					// Zero-crossing time bump to try and get the FMU to detect relevant crossings

					if ( options::output::d ) std::cout << "Zero-crossing handler event(s): Bump time = " << t_bump << std::endl;
					set_time( t_bump ); // Set FMU to bump time
					for ( Variable_ZC const * trigger : var_ZCs ) { // Advance zero-crossing variables observees to bump time
						trigger->bump_time( t_bump );
						if ( options::output::d ) std::cout << "  " << trigger->name() << " bump value = " << trigger->fmu_get_real() << std::endl;
					}

					// Get event indicators
					std::swap( event_indicators, event_indicators_last ); // Save previous event indicators for zero crossing check
					fmi2_import_get_event_indicators( fmu, event_indicators, n_event_indicators );
					// if ( options::output::d ) {
					// 	std::cout << "FMU event indicators: Handler event processing start @ t=" << t_bump << std::endl;
					// 	for ( size_type k = 0; k < n_event_indicators; ++k ) std::cout << event_indicators[ k ] << std::endl;
					// }

					// Check if an event indicator has triggered
					bool zero_crossing_event( false );
					for ( size_type k = 0; k < n_event_indicators; ++k ) {
						if ( ( event_indicators[ k ] >= 0.0 ) != ( event_indicators_last[ k ] > 0.0 ) ) {
							zero_crossing_event = true;
							break;
						}
					}

					// FMU zero-crossing event processing
					if ( enterEventMode || zero_crossing_event ) {
						if ( options::output::d ) std::cout << "Zero-crossing triggers FMU-ME event at t=" << t << std::endl;
						fmi2_import_enter_event_mode( fmu );
						do_event_iteration();
						fmi2_import_enter_continuous_time_mode( fmu );
						fmi2_import_get_continuous_states( fmu, states, n_states );
						fmi2_import_get_event_indicators( fmu, event_indicators, n_event_indicators );
						// if ( options::output::d ) {
						// 	std::cout << "FMU event indicators: Handler event processing after event iteration @ t=" << t << std::endl;
						// 	for ( size_type k = 0; k < n_event_indicators; ++k ) std::cout << event_indicators[ k ] << std::endl;
						// }
					} else {
						if ( options::output::d ) std::cout << "Zero-crossing does not trigger FMU-ME event at t=" << t << std::endl;
					}

					// Perform handler operations on QSS side
					if ( enterEventMode || zero_crossing_event ) {
						set_time( t ); // Reset FMU to event time
						if ( eventq->single() ) { // Single handler
							Variable * handler( event.sub< Variable >() );

							for ( Variable_ZC const * trigger : var_ZCs ) { // Un-bump time
								trigger->un_bump_time( t, handler );
							}

							if ( doROut ) { // Handler output: pre
								handler->out( t );
								handler->observers_out_pre( t );
							}

							handler->advance_handler( t );

							if ( doROut ) { // Handler output: post
								if ( options::output::A ) { // All variables
									for ( auto var : vars ) {
										var->out( t );
									}
								} else { // Handlers and observers
									handler->out( t );
									handler->observers_out_post( t );
								}
							}
						} else { // Simultaneous handlers
							eventq->top_subs< Variable >( handlers );
							observers_s.assign( handlers );

							for ( Variable_ZC const * trigger : var_ZCs ) { // Un-bump time
								trigger->un_bump_time( t, handlers );
							}

							if ( doROut ) { // Handler output: pre
								for ( Variable * handler : handlers ) { // Handlers
									handler->out( t );
								}
								if ( options::output::O ) { // Observers
									for ( Variable * observer : observers_s ) {
										observer->observer_out_pre( t );
									}
								}
							}

							handlers_s.assign( handlers );
							handlers_s.advance( t );
							if ( observers_s.have() ) observers_s.advance( t ); // Advance observers

							if ( doROut ) { // Handler output: post
								if ( options::output::A ) { // All variables
									for ( auto var : vars ) {
										var->out( t );
									}
								} else { // Handlers and observers
									for ( Variable * handler : handlers ) { // Handlers
										handler->out( t );
									}
									if ( options::output::O ) { // Observers
										for ( Variable * observer : observers_s ) {
											observer->observer_out_post( t );
										}
									}
								}
							}
						}

						// Re-run FMU event processing after handlers run since event indicator signs may have changed (such as in "bounce" events)

						// Re-bump zero-crossing state
						t_bump = t; // Bump time for FMU zero crossing detection
						for ( Variable_ZC const * trigger : var_ZCs ) {
							t_bump = std::max( t_bump, trigger->tZC_bump( t ) );
						}
						if ( options::output::d ) std::cout << "Zero-crossing handler event(s): Re-bump time = " << t_bump << std::endl;
						set_time( t_bump ); // Advance FMU to bump time
						for ( Variable_ZC const * trigger : var_ZCs ) {
							trigger->re_bump_time( t_bump );
							if ( options::output::d ) std::cout << "  " << trigger->name() << " re-bump value = " << trigger->fmu_get_real() << std::endl;
						}

						// Get event indicators
						std::swap( event_indicators, event_indicators_last ); // Save previous event indicators for zero crossing check
						fmi2_import_get_event_indicators( fmu, event_indicators, n_event_indicators );
//						if ( options::output::d ) {
//							std::cout << "FMU event indicators: Handler event processing after re-bump @ t=" << t << std::endl;
//							for ( size_type k = 0; k < n_event_indicators; ++k ) std::cout << event_indicators[ k ] << std::endl;
//						}

						// Check if an event indicator has triggered
						zero_crossing_event = false;
						for ( size_type k = 0; k < n_event_indicators; ++k ) {
							if ( ( event_indicators[ k ] >= 0.0 ) != ( event_indicators_last[ k ] > 0.0 ) ) {
								zero_crossing_event = true;
								break;
							}
						}

						// FMU zero-crossing event processing
						if ( zero_crossing_event ) {
							if ( options::output::d ) std::cout << "Zero-crossing handler re-bump triggers FMU-ME event at t=" << t << std::endl;
							fmi2_import_enter_event_mode( fmu );
							do_event_iteration();
							fmi2_import_enter_continuous_time_mode( fmu );
							fmi2_import_get_continuous_states( fmu, states, n_states );
							fmi2_import_get_event_indicators( fmu, event_indicators, n_event_indicators );
//							if ( options::output::d ) {
//								std::cout << "FMU event indicators: Handler event processing after re-bump event iteration @ t=" << t << std::endl;
//								for ( size_type k = 0; k < n_event_indicators; ++k ) std::cout << event_indicators[ k ] << std::endl;
//							}
						} else {
							if ( options::output::d ) std::cout << "Zero-crossing handler re-bump does not trigger FMU-ME event at t=" << t << std::endl;
						}

					} else { // Update event queue entries for no-action handler event
						if ( eventq->single() ) { // Single handler
							Variable * handler( event.sub< Variable >() );
							handler->no_advance_handler();
						} else { // Simultaneous handlers
							eventq->top_subs< Variable >( handlers );
							for ( Variable * handler : handlers ) {
								handler->no_advance_handler();
							}
						}
					}

					// Restore FMU simulation time
					set_time( t );

				} else if ( event.is_QSS() ) { // QSS requantization event(s)
					++n_QSS_events;

					// Trigger(s) setup: Single, simultaneous, or binned
					Variable * trigger1( nullptr );
					if ( bin_size > 1u ) {
						eventq->bin_QSS< Variable >( bin_size, bin_frac, triggers );
						if ( options::output::d ) {
							std::cout << "\nBin @ " << t << " trigger(s):" << '\n';
							for ( Variable * trigger : triggers ) std::cout << "   " << trigger->name() << "  tQ-tE: " << trigger->tQ << '-' << trigger->tE << '\n';
							std::cout << std::endl;
						}
						if ( triggers.size() == 1u ) trigger1 = triggers[ 0 ]; // Use single trigger processing
					} else if ( eventq->single() ) { // Single trigger
						trigger1 = event.sub< Variable >();
						triggers.clear();
						triggers.push_back( trigger1 ); // For coding convenience below
					} else { // Simultaneous triggers
						eventq->top_subs< Variable >( triggers );
					}

#ifndef NDEBUG
					// Assert that triggers are continuous state (QSS) variables
					for ( Variable const * trigger : triggers ) {
						assert( trigger->is_QSS() );
					}
#endif

					// Clustering
					if ( options::cluster ) {
						cluster_triggers.clear();
						for ( Variable * trigger : triggers ) {
							if ( trigger->yoyoing() ) { // Yo-yoing
								Variable_QSS * trigger_QSS( dynamic_cast< Variable_QSS * >( trigger ) );
								if ( trigger_QSS->has_cluster() ) { // Use cluster
									for ( Variable_QSS * var : trigger_QSS->cluster ) {
										cluster_triggers.push_back( var );
								}
								}
							}
						}
						if ( !cluster_triggers.empty() ) {
							uniquify( cluster_triggers );
							triggers.insert( triggers.end(), cluster_triggers.begin(), cluster_triggers.end() ); // Merge in the cluster triggers
							uniquify( triggers );
							if ( triggers.size() > 1u ) trigger1 = nullptr;
						}
					}

					// Connected
					if ( connected ) { // Check if next event(s) will modify a connected output
						if ( options::perfect ) { // Flag whether next event(s) will modify a connected output
							if ( !connected_output_event ) {
								for ( Variable const * trigger : triggers ) {
									if ( trigger->connected_output || trigger->connected_output_observer ) {
										connected_output_event = true;
										break;
									}
								}
							}
						} else if ( t > tPass ) { // Stop if beyond pass start time and next event(s) will modify a connected output
							bool connected_output_next( false );
							for ( Variable const * trigger : triggers ) {
								if ( trigger->connected_output || trigger->connected_output_observer ) {
									connected_output_next = true;
									break;
								}
							}
							if ( connected_output_next ) break; // Exit t loop
						}
					}

					// Requantize
					if ( trigger1 != nullptr ) { // Single trigger
						Variable * trigger( trigger1 );
						assert( trigger->tE == t );
						trigger->st = s; // Set trigger superdense time
						++c_QSS_events[ trigger ];

						if ( doROut ) { // Requantization output: pre
							trigger->out_q( t ); // Quantized-only: State requantization has no x discontinuity
							trigger->observers_out_pre( t );
						}

						trigger->advance_QSS();

						if ( doROut ) { // Requantization output: post
							if ( options::output::A ) { // All variables
								for ( auto var : vars ) {
									var->out( t );
								}
							} else { // Trigger and observers
								trigger->out( t );
								trigger->observers_out_post( t );
							}
						}
						if ( doTOut ) { // Time step output
							trigger->out_t( t );
						}
					} else { // Simultaneous/binned triggers
						if ( options::output::s || options::steps ) { // Statistics or steps file
							for ( Variable * trigger : triggers ) {
								++c_QSS_events[ trigger ];
							}
						}
						++n_QSS_simultaneous_events;
						observers_s.assign( triggers );

						if ( doROut ) { // Requantization output: pre
							for ( Variable * trigger : triggers ) { // Triggers
								trigger->out_q( t ); // Quantized-only: State requantization has no x discontinuity
							}
							if ( options::output::O ) { // Observers
								for ( Variable * observer : observers_s ) {
									observer->observer_out_pre( t );
								}
							}
						}

						triggers_s.assign( triggers );
						triggers_s.advance_QSS( t, s );
						if ( observers_s.have() ) observers_s.advance( t ); // Advance observers

						if ( doROut ) { // Requantization output: post
							if ( options::output::A ) { // All variables
								for ( auto var : vars ) {
									var->out( t );
								}
							} else { // Triggers and observers
								for ( Variable * trigger : triggers ) { // Triggers
									trigger->out( t );
								}
								if ( options::output::O ) { // Observers
									for ( Variable * observer : observers_s ) {
										observer->observer_out_post( t );
									}
								}
							}
						}
						if ( doTOut ) { // Time step output
							for ( Variable * trigger : triggers ) { // Triggers
								trigger->out_t( t );
							}
						}
						max_bin_size = std::max( max_bin_size, triggers.size() );
					}
				} else if ( event.is_QSS_ZC() ) { // QSS ZC requantization event(s)
					++n_QSS_events;

					// Trigger(s) setup: Single, simultaneous, or binned
					Variable * trigger1( nullptr );
					if ( bin_size > 1u ) {
						eventq->bin_QSS_ZC< Variable >( bin_size, bin_frac, triggers );
						if ( options::output::d ) {
							std::cout << "\nBin @ " << t << " trigger(s):" << '\n';
							for ( Variable * trigger : triggers ) std::cout << "   " << trigger->name() << "  tQ-tE: " << trigger->tQ << '-' << trigger->tE << '\n';
							std::cout << std::endl;
						}
						if ( triggers.size() == 1u ) trigger1 = triggers[ 0 ]; // Use single trigger processing
					} else if ( eventq->single() ) { // Single trigger
						trigger1 = event.sub< Variable >();
						triggers.clear();
						triggers.push_back( trigger1 ); // For coding convenience below
					} else { // Simultaneous triggers
						eventq->top_subs< Variable >( triggers );
					}

#ifndef NDEBUG
					// Assert that triggers are zero-crossing variables
					for ( Variable const * trigger : triggers ) {
						assert( trigger->is_ZC() );
					}
#endif

					// Connected
					if ( connected ) { // Check if next event(s) will modify a connected output
						if ( options::perfect ) { // Flag whether next event(s) will modify a connected output
							if ( !connected_output_event ) {
								for ( Variable const * trigger : triggers ) {
									if ( trigger->connected_output || trigger->connected_output_observer ) {
										connected_output_event = true;
										break;
									}
								}
							}
						} else if ( t > tPass ) { // Stop if beyond pass start time and next event(s) will modify a connected output
							bool connected_output_next( false );
							for ( Variable const * trigger : triggers ) {
								if ( trigger->connected_output || trigger->connected_output_observer ) {
									connected_output_next = true;
									break;
								}
							}
							if ( connected_output_next ) break; // Exit t loop
						}
					}

					// Requantize
					if ( trigger1 != nullptr ) { // Single trigger
						Variable * trigger( trigger1 );
						assert( trigger->tE == t );
						assert( trigger->is_ZC() ); // ZC trigger
						trigger->st = s; // Set trigger superdense time
						++c_QSS_events[ trigger ];

						if ( doROut ) { // Requantization output: pre
							trigger->out( t );
						}

						trigger->advance_QSS();

						if ( doROut ) { // Requantization output: post
							if ( options::output::A ) { // All variables
								for ( auto var : vars ) {
									var->out( t );
								}
							} else { // Trigger
								trigger->out( t );
							}
						}
						if ( doTOut ) { // Time step output
							trigger->out_t( t );
						}
					} else { // Simultaneous/binned triggers
						if ( options::output::s || options::steps ) { // Statistics or steps file
							for ( Variable * trigger : triggers ) {
								++c_QSS_events[ trigger ];
							}
						}
						++n_QSS_simultaneous_events;

						if ( doROut ) { // Requantization output: pre
							for ( Variable * trigger : triggers ) { // Triggers
								trigger->out( t );
							}
						}

						triggers_zc_s.assign( triggers );
						triggers_zc_s.advance_QSS( t, s );

						if ( doROut ) { // Requantization output: post
							if ( options::output::A ) { // All variables
								for ( auto var : vars ) {
									var->out( t );
								}
							} else { // Triggers
								for ( Variable * trigger : triggers ) {
									trigger->out( t );
								}
							}
						}
						if ( doTOut ) { // Time step output
							for ( Variable * trigger : triggers ) { // Triggers
								trigger->out_t( t );
							}
						}
						max_bin_size = std::max( max_bin_size, triggers.size() );
					}

				} else if ( event.is_QSS_R() ) { // QSS R requantization event(s)
					++n_QSS_events;

					// Trigger(s) setup: Single, simultaneous, or binned
					Variable * trigger1( nullptr );
					if ( bin_size > 1u ) {
						eventq->bin_QSS_R< Variable >( bin_size, bin_frac, triggers );
						if ( options::output::d ) {
							std::cout << "\nBin @ " << t << " trigger(s):" << '\n';
							for ( Variable * trigger : triggers ) std::cout << "   " << trigger->name() << "  tQ-tE: " << trigger->tQ << '-' << trigger->tE << '\n';
							std::cout << std::endl;
						}
						if ( triggers.size() == 1u ) trigger1 = triggers[ 0 ]; // Use single trigger processing
					} else if ( eventq->single() ) { // Single trigger
						trigger1 = event.sub< Variable >();
					} else { // Simultaneous triggers
						eventq->top_subs< Variable >( triggers );
					}

#ifndef NDEBUG
					// Assert that triggers are zero-crossing variables
					for ( Variable const * trigger : triggers ) {
						assert( trigger->is_R() );
					}
#endif

					// Connected
					if ( connected ) { // Check if next event(s) will modify a connected output
						if ( options::perfect ) { // Flag whether next event(s) will modify a connected output
							if ( !connected_output_event ) {
								for ( Variable const * trigger : triggers ) {
									if ( trigger->connected_output || trigger->connected_output_observer ) {
										connected_output_event = true;
										break;
									}
								}
							}
						} else if ( t > tPass ) { // Stop if beyond pass start time and next event(s) will modify a connected output
							bool connected_output_next( false );
							for ( Variable const * trigger : triggers ) {
								if ( trigger->connected_output || trigger->connected_output_observer ) {
									connected_output_next = true;
									break;
								}
							}
							if ( connected_output_next ) break; // Exit t loop
						}
					}

					// Requantize
					if ( trigger1 != nullptr ) { // Single trigger
						Variable * trigger( trigger1 );
						assert( trigger->tE == t );
						assert( trigger->is_R() ); // R trigger
						trigger->st = s; // Set trigger superdense time
						++c_QSS_events[ trigger ];

						if ( doROut ) { // Requantization output: pre
							trigger->out( t );
							trigger->observers_out_pre( t );
						}

						trigger->advance_QSS();

						if ( doROut ) { // Requantization output: post
							if ( options::output::A ) { // All variables
								for ( auto var : vars ) {
									var->out( t );
								}
							} else { // Trigger and observers
								trigger->out( t );
								trigger->observers_out_post( t );
							}
						}
						if ( doTOut ) { // Time step output
							trigger->out_t( t );
						}
					} else { // Simultaneous/binned triggers
						if ( options::output::s || options::steps ) { // Statistics or steps file
							for ( Variable * trigger : triggers ) {
								++c_QSS_events[ trigger ];
							}
						}
						++n_QSS_simultaneous_events;
						observers_s.assign( triggers );

						if ( doROut ) { // Requantization output: pre
							for ( Variable * trigger : triggers ) { // Triggers
								trigger->out( t );
							}
							if ( options::output::O ) { // Observers
								for ( Variable * observer : observers_s ) {
									observer->observer_out_pre( t );
								}
							}
						}

						triggers_r_s.assign( triggers );
						triggers_r_s.advance_QSS( t, s );
						if ( observers_s.have() ) observers_s.advance( t ); // Advance observers

						if ( doROut ) { // Requantization output: post
							if ( options::output::A ) { // All variables
								for ( auto var : vars ) {
									var->out( t );
								}
							} else { // Triggers and observers
								for ( Variable * trigger : triggers ) { // Triggers
									trigger->out( t );
								}
								if ( options::output::O ) { // Observers
									for ( Variable * observer : observers_s ) {
										observer->observer_out_post( t );
									}
								}
							}
						}
						if ( doTOut ) { // Time step output
							for ( Variable * trigger : triggers ) { // Triggers
								trigger->out_t( t );
							}
						}
						max_bin_size = std::max( max_bin_size, triggers.size() );
					}

				} else if ( event.is_QSS_Inp() ) { // QSS Input requantization event(s)
					++n_QSS_events;
					Variable * trigger( event.sub< Variable >() );
					assert( trigger->tE == t );
					assert( trigger->is_Input() );
					trigger->st = s; // Set trigger superdense time
					++c_QSS_events[ trigger ];

					if ( doROut ) { // Requantization output: pre
						trigger->out( t );
						trigger->observers_out_pre( t );
					}

					trigger->advance_QSS();

					if ( doROut ) { // Requantization output: post
						if ( options::output::A ) { // All variables
							for ( auto var : vars ) {
								var->out( t );
							}
						} else { // Trigger
							trigger->out( t );
							trigger->observers_out_post( t );
						}
					}
					if ( doTOut ) { // Time step output
						trigger->out_t( t );
					}
				} else { // Unsupported event
					assert( false );
				}

				// Local variable event outputs
				if ( options::output::L && ( n_l_outs > 0u ) && ( options::specified::tLoc ) && ( options::tLoc.first <= t ) && ( t <= options::tLoc.second ) ) {
					for ( auto var : vars_NC ) {
						var->fmu_set_x( t );
					}
					size_type i( 0u );
					for ( auto const & e : fmu_outs ) {
						FMU_Variable const & var( *(e.second) );
						if ( var.causality_local() ) l_outs[ i ].append( t, get_as_real( var ) );
						++i;
					}
				}

				tProc = t;

				// Bin optimization
				if ( bin_auto ) { // Bin optimization active
					if ( t >= bin_performance.tb() + bin_performance_dt ) { // Enough simulation time to check elapsed CPU time
						Time const cpu_time_elapsed( bin_performance.elapsed() );
						if ( ( cpu_time_elapsed >= 1.0 ) || ( t >= bin_performance.tb() + bin_performance_dt_max ) ) { // Compute bin size metrics
							timers::Performance::Velocity const bin_velocity( bin_performance( t, cpu_time_elapsed ) );
							bin_performance_dt = std::max( bin_performance_dt, t - bin_performance.tb() ); // Tune simulation time until next check
							// std::cerr << "\nBining Performance: " << t << ' ' << cpu_time_elapsed << ' ' << bin_size << ' ' << max_bin_size << ' ' << bin_velocity << ' ' << bin_performance_dt << std::endl; //Diagnostic
							size_type const bin_size_old( bin_size );
							bin_optimizer.add( max_bin_size, bin_velocity );
							bin_size = bin_optimizer.rec_bin_size();
							bin_size_auto.first += bin_size;
							++bin_size_auto.second;
							if ( options::output::d ) {
								if ( bin_size != bin_size_old ) {
									std::cout << "\nBin size adjusted to: " << bin_size << std::endl;
								}
							}
							bin_performance.start( t );
							max_bin_size = 1u;
						}
					}
				}

			}

			// Report % complete
			if ( !options::output::d ) {
				int const tPerNow( static_cast< int >( 100 * ( tProc - t0 ) / tSim ) );
				if ( tPerNow > tPer ) { // Report % complete
					tPer = tPerNow;
					std::cout << '\r' + name + " Simulation " << std::setw( 3 ) << tPer << "% =====" << std::flush;
				}
			}

			// FMU end of step processing
// Not sure we need to set continuous states: It would be a performance hit
//ZC and this wipes out ZC bump values between ZC and Handler event calls
//			set_time( tProc );
//			for ( size_type i = 0; i < n_states; ++i ) {
//				if ( state_vars[ i ] != nullptr ) states[ i ] = state_vars[ i ]->x( tProc );
//			}
//			fmi2_import_set_continuous_states( fmu, states, n_states );
			fmi2_import_completed_integrator_step( fmu, fmi2_true, &enterEventMode, &terminateSimulation );
			if ( eventInfo.terminateSimulation || terminateSimulation ) {
				eventInfoMaster->terminateSimulation = fmi2_true;
				break;
			}

			// Stop if perfect sync and this event modified a connected output
			if ( connected_output_event ) {
				t = eventq->top_time(); // To give master loop event queue the next event time
				break;
			}
		}
		eventInfoMaster->nextEventTimeDefined = fmi2_true;
		eventInfoMaster->nextEventTime = t; // For master loop event queue

		sim_cpu_time += cpu_time() - cpu_time_beg; // CPU time
#ifdef _OPENMP
		sim_wall_time += omp_get_wtime() - wall_time_beg; // Wall time
#endif

		// Reporting
		if ( t >= tE ) {
			if ( !options::output::d ) std::cout << '\r' + name + " Simulation 100% =====" << std::endl;
			std::cout << '\n' + name + " Simulation Complete =====" << std::endl;
			if ( n_discrete_events > 0 ) std::cout << n_discrete_events << " discrete event passes" << std::endl;
			if ( n_QSS_events > 0 ) std::cout << n_QSS_events << " requantization event passes" << std::endl;
			if ( n_QSS_simultaneous_events > 0 ) std::cout << n_QSS_simultaneous_events << " simultaneous/binned requantization event passes" << std::endl;
			if ( n_ZC_events > 0 ) std::cout << n_ZC_events << " zero-crossing event passes" << std::endl;
			std::cout << "Simulation CPU time:  " << sim_cpu_time << " (s)" << std::endl; // CPU time
#ifdef _OPENMP
			std::cout << "Simulation wall time: " << sim_wall_time << " (s)" << std::endl; // Wall time
#endif
			if ( bin_auto && ( bin_size_auto.second > 0u ) ) {
				std::cout << "\nAverage optimized bin size: " << static_cast< size_type >( std::round( double( bin_size_auto.first ) / bin_size_auto.second ) ) << std::endl;
			}
			if ( options::output::s ) { // Statistics
				if ( n_QSS_events > 0 ) {
					std::cout << "\nQSS Requantization Events: By Name" << std::endl;
					for ( Variable const * var : vars ) {
						if ( c_QSS_events[ var ] > 0u ) std::cout << ' ' << var->name() << ' ' << c_QSS_events[ var ] << " (" <<  100u * c_QSS_events[ var ] / n_QSS_events << "%)" << std::endl;
					}
					std::cout << "\nQSS Requantization Events: By Count" << std::endl;
					Variables vars_by_requants( vars );
					std::stable_sort( vars_by_requants.begin(), vars_by_requants.end(), [this]( Variable const * v1, Variable const * v2 ){ return c_QSS_events[ v1 ] > c_QSS_events[ v2 ]; } );
					for ( Variable const * var : vars_by_requants ) {
						if ( c_QSS_events[ var ] > 0u ) std::cout << ' ' << var->name() << ' ' << c_QSS_events[ var ] << " (" <<  100u * c_QSS_events[ var ] / n_QSS_events << "%)" << std::endl;
					}
				}
				if ( n_ZC_events > 0 ) {
					std::cout << "\nQSS Zero-Crossing Events:" << std::endl;
					bool any_detected_crossings( false );
					for ( Variable const * var : vars_ZC ) {
						if ( c_ZC_events[ var ] > 0u ) std::cout << ' ' << var->name() << ' ' << c_ZC_events[ var ] << " (" <<  100u * c_ZC_events[ var ] / n_ZC_events << "%)" << std::endl;
						if ( var->detected_crossing() ) any_detected_crossings = true;
					}
					if ( any_detected_crossings ) {
						std::cout << "\nQSS Zero-Crossing Variables with Unpredicted Zero Crossings Detected:" << std::endl;
						for ( Variable const * var : vars_ZC ) {
							if ( var->detected_crossing() ) std::cout << ' ' << var->name() << std::endl;
						}
					}
				}
			}
			if ( options::stiff ) { // Stiffness report
				std::cout << "\nQSS State Variables Average LIQSS/QSS Step Size Ratio:" << std::endl;
				for ( Variable const * var : state_vars ) {
					if ( ( !var->is_LIQSS() ) && ( !var->is_time() ) ) {
						size_type const avg_ratio( static_cast< size_type >( std::round( static_cast< Variable_QSS const * >( var )->liqss_qss_step_ratio.ratio() ) ) );
						size_type const percent_finite( static_cast< size_type >( std::round( 100.0 - static_cast< Variable_QSS const * >( var )->liqss_qss_step_ratio.ratio_inf_percent() ) ) );
						std::cout << ' ' << var->name() << ' ' << avg_ratio;
						if ( percent_finite < 100u ) std::cout << " (" << percent_finite << "% steps finite)";
						std::cout << std::endl;
					}
				}
			}
			if ( options::steps ) { // Steps file
				std::ofstream step_stream( name + ".stp", std::ios_base::binary | std::ios_base::out );
				if ( step_stream.is_open() ) {
					OutputFilter const steps_filter;
					step_stream << n_QSS_events << '\n';
					for ( Variable const * var : vars ) {
						if ( steps_filter( var->name() ) ) step_stream << var->name() << ' ' << c_QSS_events[ var ] << '\n';
					}
				}
				step_stream.close();
			}
		}
	}

	// Simulation Pass
	void
	FMU_ME::
	simulate()
	{
		fmi2_event_info_t eventInfoMaster;
		eventInfoMaster.newDiscreteStatesNeeded = fmi2_false;
		eventInfoMaster.terminateSimulation = fmi2_false;
		eventInfoMaster.nominalsOfContinuousStatesChanged = fmi2_false;
		eventInfoMaster.valuesOfContinuousStatesChanged = fmi2_false;
		eventInfoMaster.nextEventTimeDefined = fmi2_false;
		simulate( &eventInfoMaster );
	}

	// Post-Simulation Actions
	void
	FMU_ME::
	post_simulate()
	{
		// End time outputs
		set_time( tE );
		if ( ( ( options::output::R || options::output::Z || options::output::D || options::output::S ) && ( options::output::X || options::output::Q ) ) || options::output::T ) { // QSS tE outputs
			for ( auto var : vars ) {
				if ( var->tQ < tE ) {
					if ( doROut || doZOut || doDOut || doSOut ) var->out( tE );
					if ( doTOut ) var->out_t( tE );
				}
				var->flush_out();
			}
		}
		if ( options::output::F ) { // FMU QSS variable tE outputs
			if ( n_f_outs > 0u ) { // FMU QSS variables
				for ( size_type i = 0; i < n_f_outs; ++i ) {
					Variable * var( f_outs_vars[ i ] );
					f_outs[ i ].append( tE, var->x( tE ) );
					f_outs[ i ].flush();
				}
			}
		}
		if ( options::output::L ) { // FMU local variable tE outputs
			if ( n_l_outs > 0u ) { // FMU local variables
				set_time( tE );
//				for ( size_type i = 0; i < n_states; ++i ) {
//					if ( state_vars[ i ] != nullptr ) states[ i ] = state_vars[ i ]->x( tE );
//				}
//				fmi2_import_set_continuous_states( fmu, states, n_states );
				for ( auto var : vars_NC ) {
					var->fmu_set_x( tE );
				}
				size_type i( 0u );
				for ( auto const & e : fmu_outs ) {
					FMU_Variable const & var( *(e.second) );
					l_outs[ i ].append( tE, get_as_real( var ) );
					l_outs[ i ].flush();
					++i;
				}
			}
		}
		if ( options::output::K ) { // FMU-QSS smooth token outputs
			if ( n_fmu_qss_qss_outs > 0u ) {
				for ( size_type i = 0; i < n_fmu_qss_qss_outs; ++i ) {
					Variable * var( fmu_qss_qss_outs[ i ] );
					k_qss_outs[ i ].append( tE, var->k( tE ) );
					k_qss_outs[ i ].flush();
				}
			}
//			if ( fmu_qss_fmu_outs.size() > 0u ) {
//				set_time( tE );
//				for ( size_type i = 0; i < n_states; ++i ) {
//					if ( state_vars[ i ] != nullptr ) states[ i ] = state_vars[ i ]->x( tE );
//				}
//				fmi2_import_set_continuous_states( fmu, states, n_states );
//				size_type i( n_fmu_qss_qss_outs );
//				for ( FMU_Variable const & fmu_var : fmu_qss_qss_outs ) {
//					k_fmu_outs[ i++ ].append( tE, get_real( fmu_var.ref() ) ); //Do SmoothToken once we can get derivatives
//				}
//			}
		}
		if ( options::csv ) {
			for ( auto var : vars ) {
				var->fmu_set_x( tE );
			}
			for ( std::size_t i = 0, n = res_var_indexes.size(); i < n; ++i ) {
				res_var_vals[ i ] = get_as_real( fmu_variables[ res_var_indexes[ i ] ] );
			}
			csv.values( res_var_vals );
		}
	}

	// Find Event Indicator and Non-Event Indicator Observees in Observee Subgraph
	void
	FMU_ME::
	subgraph_ei_observees( FMU_Dependencies const & fmu_dependencies, dep::Variable::Observees const & observees, DepIdxSet & nei_observees, DepIdxSet & ei_observees ) const
	{
		for ( dep::Variable::Index const observee : observees ) {
			FMU_Variable const & observee_fmu_var( fmu_variables[ observee - 1 ] ); // FMU variable corresponding to the observee index
			if ( observee_fmu_var.is_EventIndicator() ) { // Observee is an event indicator
				if ( ei_observees.insert( observee ).second ) { // Observee was added to event indicator observees
					subgraph_ei_observees( fmu_dependencies, fmu_dependencies[ observee ].observees, nei_observees, ei_observees ); // Recurse
				}
			} else { // Observee is not an event indicator
				nei_observees.insert( observee );
			}
		}
	}

	// Mark FMU Variables with Upstream State or Event Indicator Observees
	void
	FMU_ME::
	mark_downstream_observees( FMU_Dependencies const & fmu_dependencies, dep::Variable const & dep_var )
	{
		for ( dep::Variable::Index const observee : dep_var.observees ) {
			FMU_Variable & observee_fmu_var( fmu_variables[ observee - 1 ] ); // FMU variable corresponding to the observee index
			if ( !observee_fmu_var.has_upstream_state_or_ei_observer ) { // Not yet marked
				observee_fmu_var.has_upstream_state_or_ei_observer = true; // Mark it
				if ( !( observee_fmu_var.is_State() || observee_fmu_var.is_Derivative() || observee_fmu_var.is_EventIndicator() ) ) { // State/Derivative or Event indicator sub-graph will be root of another marking pass
					if ( fmu_dependencies.has( observee ) ) {
						mark_downstream_observees( fmu_dependencies, fmu_dependencies[ observee ] ); // Recurse
					}
				}
			}
		}
	}

	// FMI Status Check/Report
	bool
	FMU_ME::
	status_check( fmi2_status_t const status, std::string const & fxn_name )
	{
		switch ( status ) {
		case fmi2_status_ok:
			return true;
		case fmi2_status_warning:
			if ( !fxn_name.empty() ) std::cerr << '\n' << fxn_name << " FMI status = warning" << std::endl;
			return false;
		case fmi2_status_discard:
			if ( !fxn_name.empty() ) std::cerr << '\n' << fxn_name << " FMI status = discard" << std::endl;
			return false;
		case fmi2_status_error:
			if ( !fxn_name.empty() ) std::cerr << '\n' << fxn_name << " FMI status = error" << std::endl;
			return false;
		case fmi2_status_fatal:
			if ( !fxn_name.empty() ) std::cerr << '\n' << fxn_name << " FMI status = fatal" << std::endl;
			return false;
		case fmi2_status_pending:
			if ( !fxn_name.empty() ) std::cerr << '\n' << fxn_name << " FMI status = pending" << std::endl;
			return false;
		default:
			return false;
		}
	}

	// FMI SI Unit Check
	bool
	FMU_ME::
	SI_unit_check( fmi2_import_unit_t * unit, bool const msg )
	{
		if ( unit != nullptr ) {
			double const unit_scl( fmi2_import_get_SI_unit_factor( unit ) );
			double const unit_del( fmi2_import_get_SI_unit_offset( unit ) );
			if ( ( unit_scl != 1.0 ) || ( unit_del != 0.0 ) ) {
				if ( msg ) std::cerr << " Non-SI unit: " << fmi2_import_get_unit_name( unit ) << "  Scale: " << unit_scl << "  Offset: " << unit_del << std::endl;
				return false;
			}
		}
		return true;
	}

} // QSS
