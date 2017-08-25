// QSS FMU Model Simulation Runner
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

// QSS Headers
#include <QSS/fmu/simulate_fmu.hh>
#include <QSS/fmu/FMI.hh>
#include <QSS/fmu/FMU_Variable.hh>
#include <QSS/fmu/Function_Inp_constant.hh>
#include <QSS/fmu/Function_Inp_sin.hh>
#include <QSS/fmu/Function_Inp_step.hh>
#include <QSS/fmu/globals_fmu.hh>
#include <QSS/fmu/Variable_D.hh>
#include <QSS/fmu/Variable_Inp1.hh>
#include <QSS/fmu/Variable_Inp2.hh>
#include <QSS/fmu/Variable_InpD.hh>
#include <QSS/fmu/Variable_InpI.hh>
#include <QSS/fmu/Variable_LIQSS1.hh>
#include <QSS/fmu/Variable_LIQSS2.hh>
#include <QSS/fmu/Variable_QSS1.hh>
#include <QSS/fmu/Variable_QSS2.hh>
#include <QSS/fmu/Variable_ZC1.hh>
#include <QSS/fmu/Variable_ZC2.hh>
#include <QSS/math.hh>
#include <QSS/options.hh>

// FMI Library Headers
#include <fmilib.h>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace QSS {
namespace fmu {

// Forward
void
do_event_iteration( fmi2_import_t * fmu, fmi2_event_info_t * eventInfo );

// FMU Variable Pointer Union
union FMUVarPtr { // Support FMU real and integer variables
	fmi2_import_real_variable_t * rvr; // FMU real variable pointer
	fmi2_import_integer_variable_t * ivr; // FMU integer variable pointer

	FMUVarPtr( fmi2_import_real_variable_t * rvr ) :
	 rvr( rvr )
	{}

	FMUVarPtr( fmi2_import_integer_variable_t * ivr ) :
	 ivr( ivr )
	{}

	friend
	bool
	operator ==( FMUVarPtr const & p1, FMUVarPtr const & p2 )
	{
		return p1.rvr == p2.rvr;
	}
};

// Hash for FMUVarPtr
struct FMUVarPtrHash
{
	std::size_t
	operator ()( FMUVarPtr const & p ) const
	{
		return std::hash< fmi2_import_real_variable_t * >{}( p.rvr );
	}
};

// Simulate an FMU Model
void
simulate_fmu()
{
	// Types
	using Variables = Variable::Variables;
	using size_type = Variables::size_type;
	using Events = EventQueue< Variable >::Events;
	using Time = Variable::Time;
	using Value = Variable::Value;
	using Var_Idx = std::unordered_map< Variable const *, size_type >; // Map from Variables to their indexes
	using VariableLookup = std::unordered_set< Variable * >; // Fast Variable lookup container
	using ObserversSet = std::unordered_set< Variable * >; // Simultaneous trigger observers collection
	using FMU_Vars = std::unordered_map< FMUVarPtr, FMU_Variable, FMUVarPtrHash >; // Map from FMU variables to FMU_Variable objects
	using FMU_Idxs = std::unordered_map< size_type, Variable * >; // Map from FMU variable indexes to QSS Variables
	using Function = std::function< SmoothToken const &( Time const ) >;

	// I/o setup
	std::cout << std::setprecision( 16 );
	std::cerr << std::setprecision( 16 );
	std::vector< std::ofstream > x_streams; // Continuous output streams
	std::vector< std::ofstream > q_streams; // Quantized output streams
	std::vector< std::ofstream > f_streams; // FMU output streams

	// FMI Library setup /////

#ifdef _WIN32
	char const * TEMP( std::getenv( "TEMP" ) );
	char const * tmpPath( TEMP != nullptr ? TEMP : "." );
#else
	char const * tmpPath( "/tmp" );
#endif

	jm_callbacks callbacks;
	callbacks.malloc = std::malloc;
	callbacks.calloc = std::calloc;
	callbacks.realloc = std::realloc;
	callbacks.free = std::free;
	callbacks.logger = jm_default_logger;
	callbacks.log_level = jm_log_level_warning;
	callbacks.context = 0;

	fmi_import_context_t * const context( fmi_import_allocate_context( &callbacks ) );
	fmi_version_enu_t const fmi_version( fmi_import_get_fmi_version( context, options::model.c_str(), tmpPath ) );
	if ( fmi_version != fmi_version_2_0_enu ) {
		std::cerr << "Error: Only FMI version 2.0 is supported" << std::endl;
		std::exit( EXIT_FAILURE );
	}
	fmi2_xml_callbacks_t * xml_callbacks( nullptr );
	fmi2_import_t * const fmu( fmi2_import_parse_xml( context, tmpPath, xml_callbacks ) );
	if ( !fmu ) {
		std::cerr << "Error: FMU XML parsing error" << std::endl;
		std::exit( EXIT_FAILURE );
	}
	fmu::fmu = fmu;
	if ( fmi2_import_get_fmu_kind( fmu ) == fmi2_fmu_kind_cs ) {
		std::cerr << "Error: Only FMU ME is supported: Supplied FMU is CS" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	fmi2_callback_functions_t callBackFunctions;
	callBackFunctions.logger = fmi2_log_forwarding;
	callBackFunctions.allocateMemory = std::calloc;
	callBackFunctions.freeMemory = std::free;
	callBackFunctions.componentEnvironment = fmu;

	if ( fmi2_import_create_dllfmu( fmu, fmi2_fmu_kind_me, &callBackFunctions ) == jm_status_error ) {
		std::cerr << "Error: Could not create the FMU library loading mechanism" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Check SI units
	fmi2_import_unit_definitions_t * unit_defs( fmi2_import_get_unit_definitions( fmu ) );
	if ( unit_defs != nullptr ) {
		size_type const n_units( fmi2_import_get_unit_definitions_number( unit_defs ) );
		std::cout << n_units << " units defined" << std::endl;
		bool units_error( false );
		for ( size_type i = 0; i < n_units; ++i ) {
			fmi2_import_unit_t * unit( fmi2_import_get_unit( unit_defs, static_cast< unsigned >( i ) ) );
			if ( unit != nullptr ) {
				double const scl( fmi2_import_get_SI_unit_factor( unit ) );
				double const del( fmi2_import_get_SI_unit_offset( unit ) );
				if ( ( scl != 1.0 ) || ( del != 0.0 ) ) {
					std::cerr << "Error: Non-SI unit present: " << fmi2_import_get_unit_name( unit ) << std::endl;
					units_error = true;
				}
			}
		}
		if ( units_error ) std::exit( EXIT_FAILURE );
	}

	size_type const n_states( fmi2_import_get_number_of_continuous_states( fmu ) );
	std::cout << n_states << " continuous variables" << std::endl;
	size_type const n_event_indicators( fmi2_import_get_number_of_event_indicators( fmu ) );
	std::cout << n_event_indicators << " event indicators" << std::endl;

	fmi2_real_t * const states( (fmi2_real_t*)std::calloc( n_states, sizeof( double ) ) );
	fmi2_real_t * const states_der( (fmi2_real_t*)std::calloc( n_states, sizeof( double ) ) );
	fmi2_real_t * event_indicators( (fmi2_real_t*)std::calloc( n_event_indicators, sizeof( double ) ) );
	fmi2_real_t * event_indicators_prev( (fmi2_real_t*)std::calloc( n_event_indicators, sizeof( double ) ) );

	if ( fmi2_import_instantiate( fmu, "FMU ME model instance", fmi2_model_exchange, 0, 0 ) == jm_status_error ) {
		std::cerr << "Error: fmi2_import_instantiate failed" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	fmi2_import_set_debug_logging( fmu, fmi2_false, 0, 0 );

	// Don't see an FMIL call to see if DefaultExperiment is present
	//  The defaults for these 3 values are: 0, 1.0, and 0.0001
	//  Should provide the user a way to override them along with other controls
	fmi2_real_t const tstart( fmi2_import_get_default_experiment_start( fmu ) ); // [0.0]
	fmi2_real_t const tstop( fmi2_import_get_default_experiment_stop( fmu ) ); // [1.0]
	std::cout << "\nSimulation Time Range:  Start: " << tstart << "  Stop: " << tstop << std::endl;
	fmi2_real_t const relativeTolerance( fmi2_import_get_default_experiment_tolerance( fmu ) ); // [0.0001]
	std::cout << "\nRelative Tolerance in FMU: " << relativeTolerance << std::endl;
	fmi2_boolean_t callEventUpdate( fmi2_false );
	fmi2_boolean_t terminateSimulation( fmi2_false );
	fmi2_boolean_t const toleranceControlled( fmi2_false ); // FMIL says tolerance control not supported for ME
	fmi2_boolean_t const stopTimeDefined( fmi2_true );
	fmi2_import_setup_experiment( fmu, toleranceControlled, relativeTolerance, tstart, stopTimeDefined, tstop );

	// QSS time and tolerance run controls
	Time const t0( tstart ); // Simulation start time
	Time const tE( options::tEnd_set ? options::tEnd : tstop ); // Simulation end time
	Time t( t0 ); // Simulation current time
	Time tOut( t0 + options::dtOut ); // Sampling time
	size_type iOut( 1u ); // Output step index
	if ( options::rTol_set ) {
	} else {
		options::rTol = relativeTolerance; // Quantization relative tolerance (FMU doesn't have an absolute tolerance)
	}
	std::cout << "Relative Tolerance: " << options::rTol << std::endl;
	std::cout << "Absolute Tolerance: " << options::aTol << std::endl;

	fmi2_import_enter_initialization_mode( fmu );
	fmi2_import_exit_initialization_mode( fmu );

	fmi2_event_info_t eventInfo;
	eventInfo.newDiscreteStatesNeeded           = fmi2_false;
	eventInfo.terminateSimulation               = fmi2_false;
	eventInfo.nominalsOfContinuousStatesChanged = fmi2_false;
	eventInfo.valuesOfContinuousStatesChanged   = fmi2_true;
	eventInfo.nextEventTimeDefined              = fmi2_false;
	eventInfo.nextEventTime                     = -0.0;

	do_event_iteration( fmu, &eventInfo );
	fmi2_import_enter_continuous_time_mode( fmu );
	fmi2_import_get_continuous_states( fmu, states, n_states ); // Should get initial values
	fmi2_import_get_event_indicators( fmu, event_indicators, n_event_indicators );

	// FMU Query: Model
	std::cout << "\nModel name: " << fmi2_import_get_model_name( fmu ) << std::endl;
	std::cout << "Model identifier: " << fmi2_import_get_model_identifier_ME( fmu ) << std::endl;

	// Collections
	Variable::Variables vars; // QSS variables collection
	Variable::Variables outs; // FMU output QSS variables collection
	FMU_Vars fmu_vars;
	FMU_Vars fmu_outs;
	FMU_Vars fmu_ders; // FMU variable to derivative map
	FMU_Vars fmu_dvrs; // FMU derivative to variable map
	FMU_Idxs fmu_idxs; // Map from FMU variable index to QSS variable

	// Process FMU variables
	fmi2_import_variable_list_t * var_list( fmi2_import_get_variable_list( fmu, 0 ) ); // sort order = 0 for original order
	size_type const n_fmu_vars( fmi2_import_get_variable_list_size( var_list ) );
	std::cout << "\nFMU Variable Processing: Num FMU Variables: " << n_fmu_vars << " =====" << std::endl;
	fmi2_value_reference_t const * vrs( fmi2_import_get_value_referece_list( var_list ) ); // reference is misspelled in FMIL API
	for ( size_type i = 0; i < n_fmu_vars; ++i ) {
		std::cout << "\nVariable  Index: " << i+1 << " Ref: " << vrs[ i ] << std::endl;
		fmi2_import_variable_t * var( fmi2_import_get_variable( var_list, i ) );
		std::string const var_name( fmi2_import_get_variable_name( var ) );
		std::cout << " Name: " << var_name << std::endl;
		std::cout << " Desc: " << ( fmi2_import_get_variable_description( var ) ? fmi2_import_get_variable_description( var ) : "" ) << std::endl;
		std::cout << " Ref: " << fmi2_import_get_variable_vr( var ) << std::endl;
		bool const var_has_start( fmi2_import_get_variable_has_start( var ) == 1 );
		std::cout << " Start? " << var_has_start << std::endl;
		fmi2_base_type_enu_t var_base_type( fmi2_import_get_variable_base_type( var ) );
		fmi2_variability_enu_t const var_variability( fmi2_import_get_variability( var ) );
		fmi2_causality_enu_t const var_causality( fmi2_import_get_causality( var ) );
		switch ( var_base_type ) {
		case fmi2_base_type_real:
			std::cout << " Type: Real" << std::endl;
			{
			fmi2_import_real_variable_t * var_real( fmi2_import_get_variable_as_real( var ) );
			fmi2_real_t const var_start( var_has_start ? fmi2_import_get_real_variable_start( var_real ) : 0.0 );
			if ( var_has_start ) std::cout << " Start: " << var_start << std::endl;
			if ( var_causality == fmi2_causality_enu_output ) {
				std::cout << " Type: Real: Output" << std::endl;
				fmu_outs[ var_real ] = FMU_Variable( var, var_real, fmi2_import_get_variable_vr( var ), i+1 );
			}
			if ( var_variability == fmi2_variability_enu_continuous ) {
				std::cout << " Type: Real: Continuous" << std::endl;
				FMU_Variable const fmu_var( var, var_real, fmi2_import_get_variable_vr( var ), i+1 );
				fmu_vars[ var_real ] = fmu_var;
				if ( var_causality == fmi2_causality_enu_input ) {
					std::cout << " Type: Real: Continuous: Input" << std::endl;
//					Function inp_fxn = Function_Inp_constant( var_start ); // Constant start value
					Function inp_fxn = Function_Inp_step( 1.0, 1.0, 0.1 ); // Step up by 1 every 0.1 s via discrete events
//					Function inp_fxn = Function_Inp_sin( 2.0, 10.0, 1.0 ); // 2 * sin( 10 * t ) + 1
					if ( var_has_start && var_start != inp_fxn( 0.0 ).x_0 ) {
						std::cerr << "Error: Specified start value does not match function value at t=0 for " << var_name << std::endl;
						std::exit( EXIT_FAILURE );
					}
					Variable_Inp * qss_var( nullptr );
					if ( ( options::qss == options::QSS::QSS1 ) || ( options::qss == options::QSS::LIQSS1 ) ) {
						qss_var = new Variable_Inp1( var_name, options::rTol, options::aTol, fmu_var, inp_fxn );
					} else if ( ( options::qss == options::QSS::QSS2 ) || ( options::qss == options::QSS::LIQSS2 ) ) {
						qss_var = new Variable_Inp2( var_name, options::rTol, options::aTol, fmu_var, inp_fxn );
					} else {
						std::cerr << "Error: Specified QSS method is not yet supported for FMUs" << std::endl;
						std::exit( EXIT_FAILURE );
					}
					vars.push_back( qss_var ); // Add to QSS variables
					fmu_idxs[ i+1 ] = qss_var; // Add to map from FMU variable index to QSS variable
					std::cout << " FMU idx: " << i+1 << " maps to QSS var: " << qss_var->name << std::endl;
				}
			} else if ( var_variability == fmi2_variability_enu_discrete ) {
				std::cout << " Type: Real: Discrete" << std::endl;
				FMU_Variable const fmu_var( var, var_real, fmi2_import_get_variable_vr( var ), i+1 );
				fmu_vars[ var_real ] = fmu_var;
				if ( var_causality == fmi2_causality_enu_input ) {
					std::cout << " Type: Real: Discrete: Input" << std::endl;
//					Function inp_fxn = Function_Inp_constant( var_start ); // Constant start value
					Function inp_fxn = Function_Inp_step( 1.0, 1.0, 0.1 ); // Step up by 1 every 0.1 s via discrete events
					Variable_InpD * qss_var( new Variable_InpD( var_name, options::rTol, options::aTol, fmu_var, inp_fxn ) );
					vars.push_back( qss_var ); // Add to QSS variables
					fmu_idxs[ i+1 ] = qss_var; // Add to map from FMU variable index to QSS variable
					std::cout << " FMU idx: " << i+1 << " maps to QSS var: " << qss_var->name << std::endl;
				} else {
					Variable_D * qss_var( new Variable_D( var_name, var_start, fmu_var ) );
					vars.push_back( qss_var ); // Add to QSS variables
					if ( var_causality == fmi2_causality_enu_output ) { // Add to FMU QSS variable outputs
						outs.push_back( qss_var );
						fmu_outs.erase( var_real ); // Remove it from non-QSS FMU outputs
					}
					fmu_idxs[ i+1 ] = qss_var; // Add to map from FMU variable index to QSS variable
					std::cout << " FMU idx: " << i+1 << " maps to QSS var: " << qss_var->name << std::endl;
				}
			}
			}
			break;
		case fmi2_base_type_int:
			std::cout << " Type: Integer" << std::endl;
			{
			fmi2_import_integer_variable_t * var_int( fmi2_import_get_variable_as_integer( var ) );
			int const var_start( var_has_start ? fmi2_import_get_integer_variable_start( var_int ) : 0 );
			if ( var_has_start ) std::cout << " Start: " << var_start << std::endl;
			if ( var_variability == fmi2_variability_enu_discrete ) {
				if ( var_causality == fmi2_causality_enu_input ) {
					std::cout << " Type: Integer: Discrete: Input" << std::endl;
					FMU_Variable const fmu_var( var, var_int, fmi2_import_get_variable_vr( var ), i+1 );
					fmu_vars[ var_int ] = fmu_var;
//					Function inp_fxn = Function_Inp_constant( var_start ); // Constant start value
					Function inp_fxn = Function_Inp_step( 1.0, 1.0, 0.1 ); // Step up by 1 every 0.1 s via discrete events
					Variable_InpI * qss_var( new Variable_InpI( var_name, options::rTol, options::aTol, fmu_var, inp_fxn ) );
					vars.push_back( qss_var ); // Add to QSS variables
					fmu_idxs[ i+1 ] = qss_var; // Add to map from FMU variable index to QSS variable
					std::cout << " FMU idx: " << i+1 << " maps to QSS var: " << qss_var->name << std::endl;
				}
			}
			}
			break;
		case fmi2_base_type_bool:
			std::cout << " Type: Boolean" << std::endl;
			if ( var_has_start ) std::cout << " Start: " << fmi2_import_get_boolean_variable_start( fmi2_import_get_variable_as_boolean( var ) ) << std::endl;
			break;
		case fmi2_base_type_str:
			std::cout << " Type: String" << std::endl;
			if ( var_has_start ) std::cout << " Start: " << fmi2_import_get_string_variable_start( fmi2_import_get_variable_as_string( var ) ) << std::endl;
			break;
		case fmi2_base_type_enum:
			std::cout << " Type: Enum" << std::endl;
			if ( var_has_start ) std::cout << " Start: " << fmi2_import_get_enum_variable_start( fmi2_import_get_variable_as_enum( var ) ) << std::endl;
			break;
		default:
			std::cout << " Type: Unknown" << std::endl;
			break;
		}
		if ( var_variability == fmi2_variability_enu_constant ) {
			std::cout << " Variability: Constant" << std::endl;
		} else if ( var_variability == fmi2_variability_enu_fixed ) {
			std::cout << " Variability: Fixed" << std::endl;
		} else if ( var_variability == fmi2_variability_enu_tunable ) {
			std::cout << " Variability: Tunable" << std::endl;
		} else if ( var_variability == fmi2_variability_enu_discrete ) {
			std::cout << " Variability: Discrete" << std::endl;
		} else if ( var_variability == fmi2_variability_enu_continuous ) {
			std::cout << " Variability: Continuous" << std::endl;
		} else if ( var_variability == fmi2_variability_enu_unknown ) {
			std::cout << " Variability: Unknown" << std::endl;
		}
		if ( var_causality == fmi2_causality_enu_parameter ) {
			std::cout << " Causality: Parameter" << std::endl;
		} else if ( var_causality == fmi2_causality_enu_calculated_parameter ) {
			std::cout << " Causality: Calculated Parameter" << std::endl;
		} else if ( var_causality == fmi2_causality_enu_input ) {
			std::cout << " Causality: Input" << std::endl;
		} else if ( var_causality == fmi2_causality_enu_output ) {
			std::cout << " Causality: Output" << std::endl;
		} else if ( var_causality == fmi2_causality_enu_local ) {
			std::cout << " Causality: Local" << std::endl;
		} else if ( var_causality == fmi2_causality_enu_independent ) {
			std::cout << " Causality: Independent" << std::endl;
		} else if ( var_causality == fmi2_causality_enu_unknown ) {
			std::cout << " Causality: Unknown" << std::endl;
		}
		fmi2_initial_enu_t const var_initial( fmi2_import_get_initial( var ) );
		if ( var_initial == fmi2_initial_enu_exact ) {
			std::cout << " Initial: Exact" << std::endl;
		} else if ( var_initial == fmi2_initial_enu_approx ) {
			std::cout << " Initial: Approx" << std::endl;
		} else if ( var_initial == fmi2_initial_enu_calculated ) {
			std::cout << " Initial: Calculated" << std::endl;
		} else if ( var_initial == fmi2_initial_enu_unknown ) {
			std::cout << " Initial: Unknown" << std::endl;
		}
	}

	// Process FMU derivatives
	fmi2_import_variable_list_t * der_list( fmi2_import_get_derivatives_list( fmu ) );
	size_type const n_ders( fmi2_import_get_variable_list_size( der_list ) );
	std::cout << "\nFMU Derivative Processing: Num FMU Derivatives: " << n_ders << " =====" << std::endl;
	fmi2_value_reference_t const * drs( fmi2_import_get_value_referece_list( der_list ) ); // reference is spelled wrong in FMIL API
	for ( size_type i = 0, ics = 0; i < n_ders; ++i ) {
		std::cout << "\nDerivative  Ref: " << drs[ i ] << std::endl;
		fmi2_import_variable_t * der( fmi2_import_get_variable( der_list, i ) );
		std::string const der_name( fmi2_import_get_variable_name( der ) );
		std::cout << " Name: " << der_name << std::endl;
		std::cout << " Desc: " << ( fmi2_import_get_variable_description( der ) ? fmi2_import_get_variable_description( der ) : "" ) << std::endl;
		std::cout << " Ref: " << fmi2_import_get_variable_vr( der ) << std::endl;
		fmi2_base_type_enu_t der_base_type( fmi2_import_get_variable_base_type( der ) );
		bool const der_start( fmi2_import_get_variable_has_start( der ) == 1 );
		std::cout << " Start? " << der_start << std::endl;
		switch ( der_base_type ) {
		case fmi2_base_type_real:
			{
			std::cout << " Type: Real" << std::endl;
			fmi2_import_real_variable_t * der_real( fmi2_import_get_variable_as_real( der ) );
			if ( der_start ) std::cout << " Start: " << fmi2_import_get_real_variable_start( der_real ) << std::endl;
			fmi2_import_real_variable_t * var_real( fmi2_import_get_real_variable_derivative_of( der_real ) );
			if ( var_real != nullptr ) { // Add to Variable to Derivative Map
				FMU_Variable & fmu_der( fmu_vars[ der_real ] );
				FMU_Variable & fmu_var( fmu_vars[ var_real ] );
				Value const states_initial( states[ ics ] ); // Initial value from fmi2_import_get_continuous_states()
				fmu_der.ics = fmu_var.ics = ++ics;
				fmu_ders[ var_real ] = fmu_der;
				fmu_dvrs[ der_real ] = fmu_var;
				std::string const var_name( fmi2_import_get_variable_name( fmu_var.var ) );
				std::cout << " Initial value of " << var_name << " = " << states_initial << std::endl;
				bool const start( fmi2_import_get_variable_has_start( fmu_var.var ) == 1 );
				if ( start ) {
					Value const var_initial( fmi2_import_get_real_variable_start( var_real ) );
					if ( var_initial != states_initial ) {
						std::cerr << "Warning: Initial value from xml specs: " << var_initial << " is not equal to initial value from fmi2GetContinuousStates(): " << states_initial << '\n';
						std::cerr << "         Using initial value from fmi2GetContinuousStates()" << std::endl;
					}
				}
				Variable_QSS * qss_var( nullptr );
				if ( options::qss == options::QSS::QSS1 ) {
					qss_var = new Variable_QSS1( var_name, options::rTol, options::aTol, states_initial, fmu_var, fmu_der );
				} else if ( options::qss == options::QSS::QSS2 ) {
					qss_var = new Variable_QSS2( var_name, options::rTol, options::aTol, states_initial, fmu_var, fmu_der );
				} else if ( options::qss == options::QSS::LIQSS1 ) {
					qss_var = new Variable_LIQSS1( var_name, options::rTol, options::aTol, states_initial, fmu_var, fmu_der );
				} else if ( options::qss == options::QSS::LIQSS2 ) {
					qss_var = new Variable_LIQSS2( var_name, options::rTol, options::aTol, states_initial, fmu_var, fmu_der );
				} else {
					std::cerr << "Error: Specified QSS method is not yet supported for FMUs" << std::endl;
					std::exit( EXIT_FAILURE );
				}
				vars.push_back( qss_var ); // Add to QSS variables
				if ( fmi2_import_get_causality( fmu_var.var ) == fmi2_causality_enu_output ) { // Add to FMU QSS variable outputs
					outs.push_back( qss_var );
					fmu_outs.erase( fmu_var.rvr ); // Remove it from non-QSS FMU outputs
				}
				fmu_idxs[ fmu_var.idx ] = qss_var; // Add to map from FMU variable index to QSS variable
				std::cout << " FMU idx: " << fmu_var.idx << " maps to QSS var: " << qss_var->name << std::endl;
			} else {
				std::cerr << "Error: Derivative missing associated variable: " << der_name << std::endl;
				std::exit( EXIT_FAILURE );
			}
			}
			break;
		case fmi2_base_type_int:
			std::cout << " Type: Integer" << std::endl;
			if ( der_start ) std::cout << " Start: " << fmi2_import_get_integer_variable_start( fmi2_import_get_variable_as_integer( der ) ) << std::endl;
			break;
		case fmi2_base_type_bool:
			std::cout << " Type: Boolean" << std::endl;
			if ( der_start ) std::cout << " Start: " << fmi2_import_get_boolean_variable_start( fmi2_import_get_variable_as_boolean( der ) ) << std::endl;
			break;
		case fmi2_base_type_str:
			std::cout << " Type: String" << std::endl;
			if ( der_start ) std::cout << " Start: " << fmi2_import_get_string_variable_start( fmi2_import_get_variable_as_string( der ) ) << std::endl;
			break;
		case fmi2_base_type_enum:
			std::cout << " Type: Enum" << std::endl;
			if ( der_start ) std::cout << " Start: " << fmi2_import_get_enum_variable_start( fmi2_import_get_variable_as_enum( der ) ) << std::endl;
			break;
		default:
			std::cout << " Type: Unknown" << std::endl;
			break;
		}
	}

	// Process FMU zero-crossing variables
	std::cout << "\nFMU Zero Crossing Processing =====" << std::endl;
	for ( size_type i = 0; i < n_fmu_vars; ++i ) {
		fmi2_import_variable_t * var( fmi2_import_get_variable( var_list, i ) );
		fmi2_base_type_enu_t var_base_type( fmi2_import_get_variable_base_type( var ) );
		if ( ( fmi2_import_get_variability( var ) == fmi2_variability_enu_continuous ) && ( fmi2_import_get_variable_base_type( var ) == fmi2_base_type_real ) ) {
			std::string const var_name( fmi2_import_get_variable_name( var ) );
			if ( ( var_name.find( "__zc_" ) == 0 ) && ( var_name.length() > 5 ) ) { // Zero-crossing variable by convention (temporary work-around)
				std::string const der_name( "__zc_der_" + var_name.substr( 5 ) );
				for ( size_type j = 0; j < n_fmu_vars; ++j ) { // Scan FMU variables for matching derivative
					fmi2_import_variable_t * der( fmi2_import_get_variable( var_list, j ) );
					fmi2_base_type_enu_t der_base_type( fmi2_import_get_variable_base_type( der ) );
					if ( ( fmi2_import_get_variability( der ) == fmi2_variability_enu_continuous ) && ( fmi2_import_get_variable_base_type( der ) == fmi2_base_type_real ) ) {
						if ( fmi2_import_get_variable_name( der ) == der_name ) { // Found derivative
							fmi2_import_real_variable_t * var_real( fmi2_import_get_variable_as_real( var ) );
							fmi2_import_real_variable_t * der_real( fmi2_import_get_variable_as_real( der ) );
							FMU_Variable & fmu_var( fmu_vars[ var_real ] );
							FMU_Variable & fmu_der( fmu_vars[ der_real ] );
							if ( ( fmu_ders.find( var_real ) == fmu_ders.end() ) && ( fmu_dvrs.find( der_real ) == fmu_dvrs.end() ) ) { // Not processed above
								std::cout << "\nZero Crossing Der: " << der_name << " of Var: " << var_name << std::endl;
								fmu_ders[ var_real ] = fmu_der;
								fmu_dvrs[ der_real ] = fmu_var;
								Variable_ZC * qss_var( nullptr );
								if ( ( options::qss == options::QSS::QSS1 ) || ( options::qss == options::QSS::LIQSS1 ) ) {
									qss_var = new Variable_ZC1( var_name, options::rTol, options::aTol, fmu_var, fmu_der );
								} else if ( ( options::qss == options::QSS::QSS2 ) || ( options::qss == options::QSS::LIQSS2 ) ) {
									qss_var = new Variable_ZC2( var_name, options::rTol, options::aTol, fmu_var, fmu_der );
								} else {
									std::cerr << "Error: Specified QSS method is not yet supported for FMUs" << std::endl;
									std::exit( EXIT_FAILURE );
								}
								vars.push_back( qss_var ); // Add to QSS variables
								if ( fmi2_import_get_causality( fmu_var.var ) == fmi2_causality_enu_output ) { // Add to FMU QSS variable outputs
									outs.push_back( qss_var );
									fmu_outs.erase( fmu_var.rvr ); // Remove it from non-QSS FMU outputs
								}
								fmu_idxs[ fmu_var.idx ] = qss_var; // Add to map from FMU variable index to QSS variable
								std::cout << " FMU idx: " << fmu_var.idx << " maps to QSS var: " << qss_var->name << std::endl;
							}
							break; // Found derivative so stop scanning
						}
					}
				}
			}
		}
	}

	{ // QSS observer setup: Continuous variables
		std::cout << "\nObserver Setup: Continuous Variables =====" << std::endl;
		size_type * startIndex( nullptr );
		size_type * dependency( nullptr );
		char * factorKind( nullptr );
		fmi2_import_get_derivatives_dependencies( fmu, &startIndex, &dependency, &factorKind );
		if ( startIndex != nullptr ) { // Derivative dependency info present in XML
			for ( size_type i = 0; i < n_ders; ++i ) {
				std::cout << "\nDerivative  Ref: " << drs[ i ] << std::endl;
				fmi2_import_variable_t * der( fmi2_import_get_variable( der_list, i ) );
				std::string const der_name( fmi2_import_get_variable_name( der ) );
				std::cout << " Name: " << der_name << std::endl;
				fmi2_import_real_variable_t * der_real( fmi2_import_get_variable_as_real( der ) );
				size_type const idx( fmu_dvrs[ der_real ].idx );
				std::cout << " Var Index: " << idx << std::endl;
				Variable * var( fmu_idxs[ idx ] );
				std::cout << " Var: " << var->name << std::endl;
				for ( size_type j = startIndex[ i ]; j < startIndex[ i + 1 ]; ++j ) {
					size_type const dep_idx( dependency[ j ] );
					fmi2_dependency_factor_kind_enu_t const kind( (fmi2_dependency_factor_kind_enu_t)( factorKind[ j ] ) );
					std::cout << "  Dep Index: " << dep_idx << "  Kind: " << kind << std::endl;
					if ( dep_idx == 0 ) { // No info: Depends on all (don't support depends on all for now)
						std::cerr << "   Error: No dependency information provided: Depends-on-all not currently supported" << std::endl;
					} else { // Process based on kind of dependent
						if ( kind == fmi2_dependency_factor_kind_dependent ) {
							std::cout << "  Kind: Dependent" << std::endl;
						} else if ( kind == fmi2_dependency_factor_kind_constant ) {
							std::cout << "  Kind: Constant" << std::endl;
						} else if ( kind == fmi2_dependency_factor_kind_fixed ) {
							std::cout << "  Kind: Fixed" << std::endl;
						} else if ( kind == fmi2_dependency_factor_kind_tunable ) {
							std::cout << "  Kind: Tunable" << std::endl;
						} else if ( kind == fmi2_dependency_factor_kind_discrete ) {
							std::cout << "  Kind: Discrete" << std::endl;
						} else if ( kind == fmi2_dependency_factor_kind_num ) {
							std::cout << "  Kind: Num" << std::endl;
						}
					}
					auto idep( fmu_idxs.find( dep_idx ) ); //Do Add support for input variable dependents
					if ( idep != fmu_idxs.end() ) {
						Variable * dep( idep->second );
						if ( dep == var ) {
							std::cout << "  Var: " << dep->name << " is self-observer" << std::endl;
							var->self_observer = true;
						} else {
							std::cout << "  Var: " << dep->name << " has observer " << var->name << std::endl;
							dep->add_observer( var );
							if ( ! dep->is_ZC() ) var->add_observee( dep );
						}
					} else {
						//std::cout << "FMU derivative " << der_name << " has dependency with index " << dep_idx << " that is not a QSS variable" << std::endl;
					}
				}
			}
		} else { // Assume no observers in model (this may not be true: FMI spec says no dependencies => dependent on all)
			std::cout << "No derivative dependency info in FMU XML" << std::endl;
		}
	}

	{ // QSS observer setup: Discrete variables
		std::cout << "\nObserver Setup: Discrete Variables =====" << std::endl;
		size_type * startIndex( nullptr );
		size_type * dependency( nullptr );
		char * factorKind( nullptr );
		fmi2_import_variable_list_t * dis_list( fmi2_import_get_discrete_states_list( fmu ) ); // Discrete variables
		size_type const n_dis_vars( fmi2_import_get_variable_list_size( dis_list ) );
		std::cout << n_dis_vars << " discrete variables found in DiscreteStates" << std::endl;
		fmi2_value_reference_t const * dis_vrs( fmi2_import_get_value_referece_list( dis_list ) ); // reference is spelled wrong in FMIL API
		fmi2_import_get_discrete_states_dependencies( fmu, &startIndex, &dependency, &factorKind );
		if ( startIndex != nullptr ) { // Discrete dependency info present in XML
			for ( size_type i = 0; i < n_dis_vars; ++i ) {
				std::cout << "\nDiscrete Variable  Index: " << i+1 << " Ref: " << dis_vrs[ i ] << std::endl;
				fmi2_import_variable_t * dis( fmi2_import_get_variable( dis_list, i ) );
				assert( fmi2_import_get_variability( dis ) == fmi2_variability_enu_discrete );
				std::string const dis_name( fmi2_import_get_variable_name( dis ) );
				std::cout << " Name: " << dis_name << std::endl;
				FMU_Variable * fmu_dis( nullptr );
				fmi2_base_type_enu_t dis_base_type( fmi2_import_get_variable_base_type( dis ) );
				switch ( dis_base_type ) {
				case fmi2_base_type_real:
					std::cout << " Type: Real" << std::endl;
					{
					fmi2_import_real_variable_t * dis_real( fmi2_import_get_variable_as_real( dis ) );
					fmu_dis = &fmu_vars[ dis_real ];
					std::cout << " FMU idx: " << fmu_dis->idx << " maps to QSS var: " << fmu_idxs[ fmu_dis->idx ]->name << std::endl;
					}
					break;
				case fmi2_base_type_int:
					std::cout << " Type: Integer" << std::endl;
					break;
				case fmi2_base_type_bool:
					std::cout << " Type: Boolean" << std::endl;
					break;
				case fmi2_base_type_str:
					std::cout << " Type: String" << std::endl;
					break;
				case fmi2_base_type_enum:
					std::cout << " Type: Enum" << std::endl;
					break;
				default:
					std::cout << " Type: Unknown" << std::endl;
					break;
				}
				auto idis( fmu_idxs.find( fmu_dis->idx ) ); //Do Add support for input variable dependents
				if ( idis != fmu_idxs.end() ) {
					Variable * dis_var( idis->second );
					assert( dis_var->is_Discrete() );
					for ( size_type j = startIndex[ i ]; j < startIndex[ i + 1 ]; ++j ) {
						size_type const dep_idx( dependency[ j ] );
						fmi2_dependency_factor_kind_enu_t const kind( (fmi2_dependency_factor_kind_enu_t)( factorKind[ j ] ) );
						std::cout << "  Dep Index: " << dep_idx << "  Kind: " << kind << std::endl;
						if ( dep_idx == 0 ) { // No info: Depends on all (don't support depends on all for now)
							std::cerr << "   Error: No dependency information provided: Depends-on-all not currently supported" << std::endl;
						} else { // Process based on kind of dependent
							if ( kind == fmi2_dependency_factor_kind_dependent ) {
								std::cout << "  Kind: Dependent" << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_constant ) {
								std::cout << "  Kind: Constant" << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_fixed ) {
								std::cout << "  Kind: Fixed" << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_tunable ) {
								std::cout << "  Kind: Tunable" << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_discrete ) {
								std::cout << "  Kind: Discrete" << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_num ) {
								std::cout << "  Kind: Num" << std::endl;
							}
						}
						auto idep( fmu_idxs.find( dep_idx ) ); //Do Add support for input variable dependents
						if ( idep != fmu_idxs.end() ) {
							Variable * dep( idep->second );
							std::cout << "  Var: " << dep->name << " has observer " << dis_name << std::endl;
							if ( ! dep->is_ZC() ) {
								std::cerr << "Error: Discrete variable " << dis_name << " has dependency on non-zero-crossing variable " << dep->name << std::endl;
								std::exit( EXIT_FAILURE );
							}
							if ( dep == dis_var ) {
								assert( false ); // If dep is ZC it can't be a discrete variable
							} else {
								dep->add_observer( dis_var );
							}
						} else {
							//std::cout << "FMU discrete variable " << dis_name << " has dependency with index " << dep_idx << " that is not a QSS variable" << std::endl;
						}
					}
				}
			}
		} else { // Assume no discrete variables dependent on ZC variables in model
			std::cout << "No discrete variable dependency info in FMU XML" << std::endl;
		}
	}

	{ // QSS observer setup: Output variables
		std::cout << "\nObserver Setup: Output Variables =====" << std::endl;
		size_type * startIndex( nullptr );
		size_type * dependency( nullptr );
		char * factorKind( nullptr );
		fmi2_import_variable_list_t * out_list( fmi2_import_get_outputs_list( fmu ) ); // Output variables
		size_type const n_out_vars( fmi2_import_get_variable_list_size( out_list ) );
		std::cout << n_out_vars << " output variables found in OutputStates" << std::endl;
		fmi2_value_reference_t const * out_vrs( fmi2_import_get_value_referece_list( out_list ) ); // reference is spelled wrong in FMIL API
		fmi2_import_get_outputs_dependencies( fmu, &startIndex, &dependency, &factorKind );
		if ( startIndex != nullptr ) { // Dependency info present in XML
			for ( size_type i = 0; i < n_out_vars; ++i ) {
				std::cout << "\nOutput Variable  Index: " << i+1 << " Ref: " << out_vrs[ i ] << std::endl;
				fmi2_import_variable_t * out( fmi2_import_get_variable( out_list, i ) );
				assert( fmi2_import_get_causality( out ) == fmi2_causality_enu_output );
				std::string const out_name( fmi2_import_get_variable_name( out ) );
				std::cout << " Name: " << out_name << std::endl;
				FMU_Variable * fmu_out( nullptr );
				fmi2_base_type_enu_t out_base_type( fmi2_import_get_variable_base_type( out ) );
				switch ( out_base_type ) {
				case fmi2_base_type_real:
					std::cout << " Type: Real" << std::endl;
					{
					fmi2_import_real_variable_t * out_real( fmi2_import_get_variable_as_real( out ) );
					fmu_out = &fmu_vars[ out_real ];
					}
					break;
				case fmi2_base_type_int:
					std::cout << " Type: Integer" << std::endl;
					break;
				case fmi2_base_type_bool:
					std::cout << " Type: Boolean" << std::endl;
					break;
				case fmi2_base_type_str:
					std::cout << " Type: String" << std::endl;
					break;
				case fmi2_base_type_enum:
					std::cout << " Type: Enum" << std::endl;
					break;
				default:
					std::cout << " Type: Unknown" << std::endl;
					break;
				}
				auto iout( fmu_idxs.find( fmu_out->idx ) ); //Do Add support for input variable dependents
				if ( iout != fmu_idxs.end() ) {
					std::cout << " FMU idx: " << fmu_out->idx << " maps to QSS var: " << fmu_idxs[ fmu_out->idx ]->name << std::endl;
					Variable * out_var( iout->second );
					if ( ! out_var->is_ZC() ) continue; // Don't worry about dependencies of non-ZC output variables on the QSS side
					for ( size_type j = startIndex[ i ]; j < startIndex[ i + 1 ]; ++j ) {
						size_type const dep_idx( dependency[ j ] );
						fmi2_dependency_factor_kind_enu_t const kind( (fmi2_dependency_factor_kind_enu_t)( factorKind[ j ] ) );
						std::cout << "  Dep Index: " << dep_idx << "  Kind: " << kind << std::endl;
						if ( dep_idx == 0 ) { // No info: Depends on all (don't support depends on all for now)
							std::cerr << "   Error: No dependency information provided: Depends-on-all not currently supported" << std::endl;
						} else { // Process based on kind of dependent
							if ( kind == fmi2_dependency_factor_kind_dependent ) {
								std::cout << "  Kind: Dependent" << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_constant ) {
								std::cout << "  Kind: Constant" << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_fixed ) {
								std::cout << "  Kind: Fixed" << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_tunable ) {
								std::cout << "  Kind: Tunable" << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_discrete ) {
								std::cout << "  Kind: Discrete" << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_num ) {
								std::cout << "  Kind: Num" << std::endl;
							}
						}
						auto idep( fmu_idxs.find( dep_idx ) ); //Do Add support for input variable dependents
						if ( idep != fmu_idxs.end() ) {
							Variable * dep( idep->second );
							std::cout << "  Var: " << dep->name << " has observer " << out_name << std::endl;
							if ( dep == out_var ) {
								assert( false ); // Output variables can't be self-observers
							} else {
								dep->add_observer( out_var );
								out_var->add_observee( dep );
							}
						} else {
							//std::cout << "FMU output variable " << out_name << " has dependency with index " << dep_idx << " that is not a QSS variable" << std::endl;
						}
					}
				}
			}
		} else { // Assume no output variables dependent on ZC variables in model
			std::cout << "No output variable dependency info in FMU XML" << std::endl;
		}
	}

	// Size setup
	size_type const n_vars( vars.size() );
	size_type const n_outs( outs.size() );
	size_type const n_fmu_outs( fmu_outs.size() );

	// Variable-index map setup
	Var_Idx var_idx;
	for ( size_type i = 0; i < n_vars; ++i ) {
		var_idx[ vars[ i ] ] = i;
	}

	// Containers of ZC and non-ZC variables
	Variables vars_ZC;
	Variables vars_nonZC;
	int max_QSS_order( 0 );
	for ( auto var : vars ) {
		if ( var->is_ZC() ) { // ZC variable
			vars_ZC.push_back( var );
		} else { // Non-ZC variable
			vars_nonZC.push_back( var );
			max_QSS_order = std::max( max_QSS_order, var->order() ); // Max QSS order of non-ZC variables to avoid unnec loop stages
		}
	}
	int const QSS_order_max( max_QSS_order ); // Highest QSS order in use
	assert( QSS_order_max <= 3 );

	// Variable initialization
	std::cout << "\nInitialization =====" << std::endl;
	fmu::set_time( t0 );
	fmu::init_derivatives( n_ders );
	for ( auto var : vars_nonZC ) {
		var->init_0();
	}
	for ( auto var : vars_nonZC ) {
		var->init_1();
	}
	if ( QSS_order_max >= 2 ) {
		fmu::set_time( t = t0 + options::dtNum );
		for ( auto var : vars_nonZC ) {
			if ( ! var->is_Discrete() ) var->fmu_set_sn( t );
		}
		for ( auto var : vars_nonZC ) {
			var->init_2();
		}
	}
	if ( ! vars_ZC.empty() ) { // ZC variables after to get actual LIQSS2+ quantized reps
		fmu::set_time( t0 );
		for ( auto var : vars_ZC ) {
			var->init_0();
		}
		for ( auto var : vars_ZC ) {
			var->init_1();
		}
		if ( QSS_order_max >= 2 ) {
			fmu::set_time( t0 + options::dtNum );
			for ( auto var : vars_ZC ) {
				var->init_2();
			}
		}
	}
	fmu::set_time( t = t0 ); // Probably don't need this

	// Output stream initialization
	bool const doSOut( ( options::output::s && ( options::output::x || options::output::q ) ) || ( options::output::f && ( n_outs + n_fmu_outs > 0u ) ) );
	bool const doTOut( options::output::t && ( options::output::x || options::output::q ) );
	bool const doROut( options::output::r && ( options::output::x || options::output::q ) );
	if ( ( options::output::t || options::output::r || options::output::s ) && ( options::output::x || options::output::q ) ) { // t0 QSS outputs
		for ( auto var : vars ) { // QSS outputs
			if ( options::output::x ) {
				x_streams.push_back( std::ofstream( var->name + ".x.out", std::ios_base::binary | std::ios_base::out ) );
				x_streams.back() << std::setprecision( 16 ) << t << '\t' << var->x( t ) << '\n';
			}
			if ( options::output::q ) {
				q_streams.push_back( std::ofstream( var->name + ".q.out", std::ios_base::binary | std::ios_base::out ) );
				q_streams.back() << std::setprecision( 16 ) << t << '\t' << var->q( t ) << '\n';
			}
		}
	}
	if ( options::output::f && ( n_outs + n_fmu_outs > 0u ) ) { // t0 FMU outputs
		for ( auto const & var : outs ) { // FMU QSS variable outputs
			f_streams.push_back( std::ofstream( std::string( fmi2_import_get_variable_name( var->var.var ) ) + ".f.out", std::ios_base::binary | std::ios_base::out ) );
			f_streams.back() << std::setprecision( 16 ) << t << '\t' << var->x( t ) << '\n';
		}
		for ( auto const & e : fmu_outs ) { // FMU (non-QSS) variable (non-QSS) outputs
			FMU_Variable const & var( e.second );
			f_streams.push_back( std::ofstream( std::string( fmi2_import_get_variable_name( var.var ) ) + ".f.out", std::ios_base::binary | std::ios_base::out ) );
			f_streams.back() << std::setprecision( 16 ) << t << '\t' << fmu::get_real( var.ref ) << '\n';
		}
	}

	// Simulation loop
	std::cout << "\nSimulation Loop =====" << std::endl;
	size_type n_discrete_events( 0 );
	size_type n_QSS_events( 0 );
	size_type n_QSS_simultaneous_events( 0 );
	size_type n_ZC_events( 0 );
	while ( t <= tE ) {
		t = events.top_time();
		if ( doSOut ) { // Sampled and/or FMU outputs
			Time const tStop( std::min( t, tE ) );
			while ( tOut < tStop ) {
				if ( options::output::s ) { // QSS variable outputs
					for ( size_type i = 0; i < n_vars; ++i ) {
						if ( options::output::x ) x_streams[ i ] << tOut << '\t' << vars[ i ]->x( tOut ) << '\n';
						if ( options::output::q ) q_streams[ i ] << tOut << '\t' << vars[ i ]->q( tOut ) << '\n';
					}
				}
				if ( options::output::f ) {	// FMU variable outputs
					if ( n_outs > 0u ) { // FMU QSS variables
						for ( size_type i = 0; i < n_outs; ++i ) {
							Variable * var( outs[ i ] );
							f_streams[ i ] << tOut << '\t' << var->x( tOut ) << '\n';
						}
					}
					if ( n_fmu_outs > 0u ) { // FMU (non-QSS) variables
						fmu::set_time( tOut );
						for ( size_type i = 0; i < n_states; ++i ) {
							states[ i ] = vars[ i ]->x( tOut );
						}
						fmi2_import_set_continuous_states( fmu, states, n_states );
						size_type i( n_outs );
						for ( auto const & e : fmu_outs ) {
							FMU_Variable const & var( e.second );
							f_streams[ i++ ] << tOut << '\t' << fmu::get_real( var.ref ) << '\n';
						}
					}
				}
				assert( iOut < std::numeric_limits< size_type >::max() );
				tOut = t0 + ( ++iOut ) * options::dtOut;
			}
		}
		if ( t <= tE ) { // Perform event(s)
			fmu::set_time( t );
			Event< Variable > & event( events.top() );
			SuperdenseTime const & s( events.top_superdense_time() );
			events.set_active_time();
			if ( event.is_discrete() ) { // Discrete event
				++n_discrete_events;
				if ( events.single() ) { // Single trigger
					Variable * trigger( events.top_var() );
					assert( trigger->tD == t );
					trigger->advance_discrete();
					if ( doTOut ) { // Time event variable output
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
								if ( options::output::q ) q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
							}
						} else { // Time event and/or observer variable output
							if ( options::output::t ) { // Time event variable output
								size_type const i( var_idx[ trigger ] );
								if ( options::output::x ) x_streams[ i ] << t << '\t' << trigger->x( t ) << '\n';
								if ( options::output::q ) q_streams[ i ] << t << '\t' << trigger->q( t ) << '\n';
								for ( Variable const * observer : trigger->observers() ) {
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										size_type const i( var_idx[ observer ] );
										if ( options::output::x ) x_streams[ i ] << t << '\t' << observer->x( t ) << '\n';
										if ( options::output::q ) q_streams[ i ] << t << '\t' << observer->q( t ) << '\n';
									}
								}
							}
							if ( ( options::output::o ) && ( options::output::x ) ) { // Observer variable output
								for ( Variable const * observer : trigger->observers() ) { // Zero-crossing observer output
									if ( ( ! options::output::t ) || ( ! observer->is_ZC() ) ) { // ZC observers output above if options::output::t
										size_type const i( var_idx[ observer ] );
										x_streams[ i ] << t << '\t' << observer->x( t ) << '\n';
									}
								}
							}
						}
					}
				} else { // Simultaneous triggers
					Variables triggers( events.top_vars() );
					std::sort( triggers.begin(), triggers.end(), []( Variable * v1, Variable * v2 ){ return v1->order() < v2->order(); } ); // Sort triggers by order
					for ( Variable * trigger : triggers ) {
						trigger->sT = s; // Set trigger superdense time
					}
					size_type const iBeg_triggers_2( static_cast< size_type >( std::distance( triggers.begin(), std::find_if( triggers.begin(), triggers.end(), []( Variable * v ){ return v->order() >= 2; } ) ) ) );
					int const triggers_order_max( triggers.empty() ? 0 : triggers.back()->order() );
					VariableLookup const var_lookup( triggers.begin(), triggers.end() );
					ObserversSet observers_set;
					for ( Variable * trigger : triggers ) { // Collect observers to avoid duplicate advance calls
						for ( Variable * observer : trigger->observers() ) {
							if ( var_lookup.find( observer ) == var_lookup.end() ) observers_set.insert( observer ); // Skip triggers
						}
					}
					Variables observers( observers_set.begin(), observers_set.end() );
					std::sort( observers.begin(), observers.end(), []( Variable * v1, Variable * v2 ){ return v1->order() < v2->order(); } ); // Sort observers by order
					size_type const iBeg_observers_2( static_cast< size_type >( std::distance( observers.begin(), std::find_if( observers.begin(), observers.end(), []( Variable * v ){ return v->order() >= 2; } ) ) ) );
					int const order_max( observers.empty() ? triggers_order_max : std::max( triggers_order_max, observers.back()->order() ) );
					for ( Variable * trigger : triggers ) {
						assert( trigger->tD == t );
						trigger->advance_discrete_0_1();
					}
					if ( order_max >= 2 ) { // 2nd order pass
						fmu::set_time( t + options::dtNum ); // Set time to t + delta for numeric differentiation
						for ( size_type i = iBeg_triggers_2, n = triggers.size(); i < n; ++i ) {
							triggers[ i ]->advance_discrete_2();
						}
					}
					if ( ! observers.empty() ) { // Observer advance
						if ( order_max >= 2 ) fmu::set_time( t );
						for ( Variable * observer : observers ) {
							observer->advance_observer_simultaneous_1( t );
						}
						if ( order_max >= 2 ) { // 2nd order pass
							Time const tN( t + options::dtNum ); // Set time to t + delta for numeric differentiation
							fmu::set_time( tN );
							for ( size_type i = iBeg_observers_2, n = observers.size(); i < n; ++i ) {
								observers[ i ]->advance_observer_simultaneous_2( tN );
							}
						}
						if ( options::output::d ) {
							for ( Variable * observer : observers ) {
								observer->advance_observer_d();
							}
						}
					}
					if ( doTOut ) { // Time event output
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
								if ( options::output::q ) q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
							}
						} else { // Time event and/or observer variable output
							if ( options::output::t ) { // Time event variable output
								for ( Variable const * trigger : triggers ) { // Triggers
									size_type const i( var_idx[ trigger ] );
									if ( options::output::x ) x_streams[ i ] << t << '\t' << trigger->x( t ) << '\n';
									if ( options::output::q ) q_streams[ i ] << t << '\t' << trigger->q( t ) << '\n';
								}
								for ( Variable const * observer : observers ) { // Zero-crossing observer output
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										size_type const i( var_idx[ observer ] );
										if ( options::output::x ) x_streams[ i ] << t << '\t' << observer->x( t ) << '\n';
										if ( options::output::q ) q_streams[ i ] << t << '\t' << observer->q( t ) << '\n';
									}
								}
							}
							if ( ( options::output::o ) && ( options::output::x ) ) { // Observer variable output
								for ( Variable const * observer : observers ) { // Zero-crossing observer output
									if ( ( ! options::output::t ) || ( ! observer->is_ZC() ) ) { // ZC observers output above if options::output::t
										size_type const i( var_idx[ observer ] );
										x_streams[ i ] << t << '\t' << observer->x( t ) << '\n';
									}
								}
							}
						}
					}
				}
			} else if ( event.is_QSS() ) { // QSS requantization event
				++n_QSS_events;
				if ( events.single() ) { // Single trigger
					Variable * trigger( events.top_var() );
					assert( trigger->tE == t );
					trigger->advance_QSS();
					if ( doROut ) { // Requantization output
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
								if ( options::output::q ) q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
							}
						} else { // Requantization and/or observer variable output
							if ( options::output::r ) { // Requantization variable output
								size_type const i( var_idx[ trigger ] );
								if ( options::output::x ) x_streams[ i ] << t << '\t' << trigger->x( t ) << '\n';
								if ( options::output::q ) q_streams[ i ] << t << '\t' << trigger->q( t ) << '\n';
								for ( Variable const * observer : trigger->observers() ) {
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										size_type const i( var_idx[ observer ] );
										if ( options::output::x ) x_streams[ i ] << t << '\t' << observer->x( t ) << '\n';
										if ( options::output::q ) q_streams[ i ] << t << '\t' << observer->q( t ) << '\n';
									}
								}
							}
							if ( ( options::output::o ) && ( options::output::x ) ) { // Observer variable output
								for ( Variable const * observer : trigger->observers() ) { // Zero-crossing observer output
									if ( ( ! options::output::r ) || ( ! observer->is_ZC() ) ) { // ZC observers output above if options::output::r
										size_type const i( var_idx[ observer ] );
										x_streams[ i ] << t << '\t' << observer->x( t ) << '\n';
									}
								}
							}
						}
					}
				} else { // Simultaneous triggers
					++n_QSS_simultaneous_events;
					Variables triggers( events.top_vars() );
					std::sort( triggers.begin(), triggers.end(), []( Variable * v1, Variable * v2 ){ return v1->order() < v2->order(); } ); // Sort triggers by order
					Variables triggers_ZC;
					Variables triggers_nonZC;
					for ( Variable * trigger : triggers ) {
						if ( trigger->is_ZC() ) { // ZC variable
							triggers_ZC.push_back( trigger );
						} else { // Non-ZC variable
							triggers_nonZC.push_back( trigger );
						}
						trigger->sT = s; // Set trigger superdense time
					}
					size_type const iBeg_triggers_nonZC_2( static_cast< size_type >( std::distance( triggers_nonZC.begin(), std::find_if( triggers_nonZC.begin(), triggers_nonZC.end(), []( Variable * v ){ return v->order() >= 2; } ) ) ) );
					int const triggers_ZC_order_max( triggers_ZC.empty() ? 0 : triggers_ZC.back()->order() );
					int const triggers_nonZC_order_max( triggers_nonZC.empty() ? 0 : triggers_nonZC.back()->order() );
					VariableLookup const var_lookup( triggers_nonZC.begin(), triggers_nonZC.end() );
					ObserversSet observers_set;
					for ( Variable * trigger : triggers_nonZC ) { // Collect observers to avoid duplicate advance calls
						for ( Variable * observer : trigger->observers() ) {
							if ( var_lookup.find( observer ) == var_lookup.end() ) observers_set.insert( observer ); // Skip triggers
						}
					}
					Variables observers( observers_set.begin(), observers_set.end() );
					std::sort( observers.begin(), observers.end(), []( Variable * v1, Variable * v2 ){ return v1->order() < v2->order(); } ); // Sort observers by order
					size_type const iBeg_observers_2( static_cast< size_type >( std::distance( observers.begin(), std::find_if( observers.begin(), observers.end(), []( Variable * v ){ return v->order() >= 2; } ) ) ) );
					int const nonZC_order_max( observers.empty() ? triggers_nonZC_order_max : std::max( triggers_nonZC_order_max, observers.back()->order() ) );
					for ( Variable * trigger : triggers_nonZC ) {
						assert( trigger->tE == t );
						trigger->advance_QSS_0();
					}
					for ( Variable * trigger : triggers_nonZC ) {
						trigger->advance_QSS_1();
					}
					if ( nonZC_order_max >= 2 ) { // 2nd order pass
						fmu::set_time( t + options::dtNum ); // Set time to t + delta for numeric differentiation
						for ( size_type i = iBeg_triggers_nonZC_2, n = triggers_nonZC.size(); i < n; ++i ) {
							triggers_nonZC[ i ]->advance_QSS_2();
						}
					}
					if ( ! triggers_ZC.empty() ) { // ZC variables after to get actual LIQSS2+ quantized reps
						if ( nonZC_order_max >= 2 ) fmu::set_time( t );
						for ( Variable * trigger : triggers_ZC ) {
							assert( trigger->tE == t );
							trigger->advance_QSS_0();
						}
						fmu::set_time( t + options::dtNum ); // Set time to t + delta for numeric differentiation
						for ( Variable * trigger : triggers_ZC ) {
							trigger->advance_QSS_1();
						}
						if ( triggers_ZC_order_max >= 2 ) {
							fmu::set_time( t - options::dtNum ); // Set time to t - delta for numeric differentiation
							for ( Variable * trigger : triggers_ZC ) {
								trigger->advance_QSS_2();
							}
						}
					}
					if ( ! observers.empty() ) { // Observer advance
						if ( ( nonZC_order_max >= 2 ) || ( ! triggers_ZC.empty() ) ) fmu::set_time( t );
						for ( Variable * observer : observers ) {
							observer->advance_observer_simultaneous_1( t );
						}
						if ( nonZC_order_max >= 2 ) { // 2nd order pass
							Time const tN( t + options::dtNum ); // Set time to t + delta for numeric differentiation
							fmu::set_time( tN );
							for ( size_type i = iBeg_observers_2, n = observers.size(); i < n; ++i ) {
								observers[ i ]->advance_observer_simultaneous_2( tN );
							}
						}
						if ( options::output::d ) {
							for ( Variable * observer : observers ) {
								observer->advance_observer_d();
							}
						}
					}
					if ( doROut ) { // Requantization output
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
								if ( options::output::q ) q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
							}
						} else { // Requantization and/or observer variable output
							if ( options::output::r ) { // Requantization variable output
								for ( Variable const * trigger : triggers ) { // Triggers
									size_type const i( var_idx[ trigger ] );
									if ( options::output::x ) x_streams[ i ] << t << '\t' << trigger->x( t ) << '\n';
									if ( options::output::q ) q_streams[ i ] << t << '\t' << trigger->q( t ) << '\n';
								}
								for ( Variable const * observer : observers ) { // Zero-crossing observer output
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										size_type const i( var_idx[ observer ] );
										if ( options::output::x ) x_streams[ i ] << t << '\t' << observer->x( t ) << '\n';
										if ( options::output::q ) q_streams[ i ] << t << '\t' << observer->q( t ) << '\n';
									}
								}
							}
							if ( ( options::output::o ) && ( options::output::x ) ) { // Observer variable output
								for ( Variable const * observer : observers ) { // Zero-crossing observer output
									if ( ( ! options::output::r ) || ( ! observer->is_ZC() ) ) { // ZC observers output above if options::output::r
										size_type const i( var_idx[ observer ] );
										x_streams[ i ] << t << '\t' << observer->x( t ) << '\n';
									}
								}
							}
						}
					}
				}
			} else if ( event.is_ZC() ) { // Zero-crossing event
				++n_ZC_events;
				while ( events.top_superdense_time() == s ) {
					Variable * trigger( events.top_var() );
					assert( trigger->tZC() == t );
					trigger->advance_ZC();
				}
			} else if ( event.is_handler() ) { // Zero-crossing handler event

				// Perform FMU event mode handler processing
				fmi2_status_t fmistatus;

				{ // Swap event_indicators and event_indicators_prev so that we can get new indicators
					fmi2_real_t * temp = event_indicators;
					event_indicators = event_indicators_prev;
					event_indicators_prev = temp;
				}
				fmistatus = fmi2_import_get_event_indicators( fmu, event_indicators, n_event_indicators );

				// Check if an event indicator has triggered
				bool zero_crossing_event( false );
				for ( size_type k = 0; k < n_event_indicators; ++k ) {
					if ( ( event_indicators[ k ] > 0 ) != ( event_indicators_prev[ k ] > 0 ) ) {
						zero_crossing_event = true;
						break;
					}
				}

				// Handle zero-crossing events
				if ( callEventUpdate || zero_crossing_event ) {
					fmistatus = fmi2_import_enter_event_mode( fmu );
					do_event_iteration( fmu, &eventInfo );
					fmistatus = fmi2_import_enter_continuous_time_mode( fmu );
					fmistatus = fmi2_import_get_continuous_states( fmu, states, n_states );
					fmistatus = fmi2_import_get_event_indicators( fmu, event_indicators, n_event_indicators );
				} else {
					std::cerr << "Warning: Expected zero-crossing event(s) not detected by FMU at t=" << t << std::endl;
				}

				// Perform handler operations on QSS side
				if ( events.single() ) { // Single handler
					if ( doROut ) { // Requantization output: Before discontinuous handler changes
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
								if ( options::output::q ) q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
							}
						} else { // Requantization and/or observer variable output
							Variable const * handler( event.var() );
							if ( options::output::r ) { // Requantization variable output
								size_type const i( var_idx[ handler ] );
								if ( options::output::x ) x_streams[ i ] << t << '\t' << handler->x( t ) << '\n';
								if ( options::output::q ) q_streams[ i ] << t << '\t' << handler->q( t ) << '\n';
								for ( Variable const * observer : handler->observers() ) {
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										size_type const i( var_idx[ observer ] );
										if ( options::output::x ) x_streams[ i ] << t << '\t' << observer->x( t ) << '\n';
										if ( options::output::q ) q_streams[ i ] << t << '\t' << observer->q( t ) << '\n';
									}
								}
							}
							if ( ( options::output::o ) && ( options::output::x ) ) { // Observer variable output
								for ( Variable const * observer : handler->observers() ) { // Zero-crossing observer output
									if ( ( ! options::output::r ) || ( ! observer->is_ZC() ) ) { // ZC observers output above if options::output::r
										size_type const i( var_idx[ observer ] );
										x_streams[ i ] << t << '\t' << observer->x( t ) << '\n';
									}
								}
							}
						}
					}
					event.var()->advance_handler( t );
					if ( doROut ) { // Requantization output
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
								if ( options::output::q ) q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
							}
						} else { // Requantization and/or observer variable output
							Variable const * handler( event.var() );
							if ( options::output::r ) { // Requantization variable output
								size_type const i( var_idx[ handler ] );
								if ( options::output::x ) x_streams[ i ] << t << '\t' << handler->x( t ) << '\n';
								if ( options::output::q ) q_streams[ i ] << t << '\t' << handler->q( t ) << '\n';
								for ( Variable const * observer : handler->observers() ) {
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										size_type const i( var_idx[ observer ] );
										if ( options::output::x ) x_streams[ i ] << t << '\t' << observer->x( t ) << '\n';
										if ( options::output::q ) q_streams[ i ] << t << '\t' << observer->q( t ) << '\n';
									}
								}
							}
							if ( ( options::output::o ) && ( options::output::x ) ) { // Observer variable output
								for ( Variable const * observer : handler->observers() ) { // Zero-crossing observer output
									if ( ( ! options::output::r ) || ( ! observer->is_ZC() ) ) { // ZC observers output above if options::output::r
										size_type const i( var_idx[ observer ] );
										x_streams[ i ] << t << '\t' << observer->x( t ) << '\n';
									}
								}
							}
						}
					}
				} else { // Simultaneous handlers
					Events tops( events.top_events() );
					Variables handlers;
					handlers.reserve( tops.size() );
					for ( auto & e : tops ) handlers.push_back( e.var() );
					std::sort( handlers.begin(), handlers.end(), []( Variable * v1, Variable * v2 ){ return v1->order() < v2->order(); } ); // Sort handlers by order
					size_type const iBeg_handlers_1( static_cast< size_type >( std::distance( handlers.begin(), std::find_if( handlers.begin(), handlers.end(), []( Variable * v ){ return v->order() >= 1; } ) ) ) );
					size_type const iBeg_handlers_2( static_cast< size_type >( std::distance( handlers.begin(), std::find_if( handlers.begin(), handlers.end(), []( Variable * v ){ return v->order() >= 2; } ) ) ) );
					int const handlers_order_max( handlers.empty() ? 0 : handlers.back()->order() );
					VariableLookup const var_lookup( handlers.begin(), handlers.end() );
					ObserversSet observers_set;
					for ( Variable * handler : handlers ) { // Collect observers to avoid duplicate advance calls
						for ( Variable * observer : handler->observers() ) {
							if ( var_lookup.find( observer ) == var_lookup.end() ) observers_set.insert( observer ); // Skip handlers
						}
					}
					Variables observers( observers_set.begin(), observers_set.end() );
					std::sort( observers.begin(), observers.end(), []( Variable * v1, Variable * v2 ){ return v1->order() < v2->order(); } ); // Sort observers by order
					size_type const iBeg_observers_2( static_cast< size_type >( std::distance( observers.begin(), std::find_if( observers.begin(), observers.end(), []( Variable * v ){ return v->order() >= 2; } ) ) ) );
					int const ho_order_max( observers.empty() ? handlers_order_max : std::max( handlers_order_max, observers.back()->order() ) );
					if ( doROut ) { // Requantization output: Before discontinuous handler changes
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
								if ( options::output::q ) q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
							}
						} else { // Requantization and/or observer variable output
							if ( options::output::r ) { // Requantization variable output
								for ( Variable const * handler : handlers ) {
									size_type const i( var_idx[ handler ] );
									if ( options::output::x ) x_streams[ i ] << t << '\t' << handler->x( t ) << '\n';
									if ( options::output::q ) q_streams[ i ] << t << '\t' << handler->q( t ) << '\n';
								}
								for ( Variable const * observer : observers ) { // Zero-crossing observer output
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										size_type const i( var_idx[ observer ] );
										if ( options::output::x ) x_streams[ i ] << t << '\t' << observer->x( t ) << '\n';
										if ( options::output::q ) q_streams[ i ] << t << '\t' << observer->q( t ) << '\n';
									}
								}
							}
							if ( ( options::output::o ) && ( options::output::x ) ) { // Observer variable output
								for ( Variable const * observer : observers ) { // Zero-crossing observer output
									if ( ( ! options::output::r ) || ( ! observer->is_ZC() ) ) { // ZC observers output above if options::output::r
										size_type const i( var_idx[ observer ] );
										x_streams[ i ] << t << '\t' << observer->x( t ) << '\n';
									}
								}
							}
						}
					}
					for ( auto & e : tops ) {
						e.var()->advance_handler_0( t );
					}
					for ( Variable * observer : observers ) {
						observer->advance_observer_simultaneous_1( t );
					}
					for ( size_type i = iBeg_handlers_1, n = handlers.size(); i < n; ++i ) {
						handlers[ i ]->advance_handler_1();
					}
					if ( ho_order_max >= 2 ) { // 2nd order pass
						Time const tN( t + options::dtNum ); // Advance time to t + delta for numeric differentiation
						fmu::set_time( tN );
						for ( size_type i = iBeg_observers_2, n = observers.size(); i < n; ++i ) {
							observers[ i ]->advance_observer_simultaneous_2( tN );
						}
						for ( size_type i = iBeg_handlers_2, n = handlers.size(); i < n; ++i ) {
							handlers[ i ]->advance_handler_2();
						}
					}
					if ( options::output::d ) {
						for ( Variable * observer : observers ) {
							observer->advance_observer_d();
						}
					}
					if ( doROut ) { // Requantization output
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
								if ( options::output::q ) q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
							}
						} else { // Requantization and/or observer variable output
							if ( options::output::r ) { // Requantization variable output
								for ( Variable const * handler : handlers ) {
									size_type const i( var_idx[ handler ] );
									if ( options::output::x ) x_streams[ i ] << t << '\t' << handler->x( t ) << '\n';
									if ( options::output::q ) q_streams[ i ] << t << '\t' << handler->q( t ) << '\n';
								}
								for ( Variable const * observer : observers ) { // Zero-crossing observer output
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										size_type const i( var_idx[ observer ] );
										if ( options::output::x ) x_streams[ i ] << t << '\t' << observer->x( t ) << '\n';
										if ( options::output::q ) q_streams[ i ] << t << '\t' << observer->q( t ) << '\n';
									}
								}
							}
							if ( ( options::output::o ) && ( options::output::x ) ) { // Observer variable output
								for ( Variable const * observer : observers ) { // Zero-crossing observer output
									if ( ( ! options::output::r ) || ( ! observer->is_ZC() ) ) { // ZC observers output above if options::output::r
										size_type const i( var_idx[ observer ] );
										x_streams[ i ] << t << '\t' << observer->x( t ) << '\n';
									}
								}
							}
						}
					}
				}
			} else { // Unsupported event
				assert( false );
			}
		}

		// FMU end of step processing
// Not sure we need to set continuous states: It would be a performance hit
//		fmu::set_time( t );
//		for ( size_type i = 0; i < n_states; ++i ) {
//			states[ i ] = vars[ i ]->x( t );
//		}
//		fmi2_import_set_continuous_states( fmu, states, n_states );
		fmi2_import_completed_integrator_step( fmu, fmi2_true, &callEventUpdate, &terminateSimulation );
		if ( eventInfo.terminateSimulation || terminateSimulation ) break;
	}

	// End time outputs and streams close
	if ( ( options::output::r || options::output::s ) && ( options::output::x || options::output::q ) ) {
		for ( size_type i = 0; i < n_vars; ++i ) {
			Variable const * var( vars[ i ] );
			if ( var->tQ < tE ) {
				if ( options::output::x ) {
					x_streams[ i ] << tE << '\t' << var->x( tE ) << '\n';
					x_streams[ i ].close();
				}
				if ( options::output::q ) {
					q_streams[ i ] << tE << '\t' << var->q( tE ) << '\n';
					q_streams[ i ].close();
				}
			}
		}
	}

	// tE FMU outputs and streams close
	if ( options::output::f ) {
		if ( n_outs > 0u ) { // FMU QSS variable outputs
			for ( size_type i = 0; i < n_outs; ++i ) {
				Variable * var( outs[ i ] );
				f_streams[ i ] << tE << '\t' << var->x( tE ) << '\n';
				f_streams[ i ].close();
			}
		}
		if ( n_fmu_outs > 0u ) { // FMU (non-QSS) variable outputs
			fmu::set_time( tE );
			for ( size_type i = 0; i < n_states; ++i ) {
				states[ i ] = vars[ i ]->x( tE );
			}
			fmi2_import_set_continuous_states( fmu, states, n_states );
			size_type i( n_outs );
			for ( auto const & e : fmu_outs ) {
				FMU_Variable const & var( e.second );
				f_streams[ i ] << tE << '\t' << fmu::get_real( var.ref ) << '\n';
				f_streams[ i++ ].close();
			}
		}
	}

	// Reporting
	std::cout << "\nSimulation Complete =====" << std::endl;
	if ( n_discrete_events > 0 ) std::cout << n_discrete_events << " discrete event passes" << std::endl;
	if ( n_QSS_events > 0 ) std::cout << n_QSS_events << " requantization event passes" << std::endl;
	if ( n_QSS_simultaneous_events > 0 ) std::cout << n_QSS_simultaneous_events << " simultaneous requantization event passes" << std::endl;
	if ( n_ZC_events > 0 ) std::cout << n_ZC_events << " zero-crossing event passes" << std::endl;

	// QSS cleanup
	for ( auto & var : vars ) delete var;

	// FMU cleanup
	fmu::cleanup();
	fmi2_import_terminate( fmu );
	fmi2_import_free_instance( fmu );
	std::free( states );
	std::free( states_der );
	std::free( event_indicators );
	std::free( event_indicators_prev );
	std::free( var_list );
	std::free( der_list );
	fmi2_import_destroy_dllfmu( fmu );
	fmi2_import_free( fmu );
	fmi_import_free_context( context );
}

// Discrete Event Processing
void
do_event_iteration( fmi2_import_t * fmu, fmi2_event_info_t * eventInfo )
{
	eventInfo->newDiscreteStatesNeeded = fmi2_true;
	eventInfo->terminateSimulation     = fmi2_false;
	while ( eventInfo->newDiscreteStatesNeeded && !eventInfo->terminateSimulation ) {
		fmi2_import_new_discrete_states( fmu, eventInfo );
	}
}

} // fmu
} // QSS
