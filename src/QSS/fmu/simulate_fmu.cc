// QSS FMU Model Simulation Runner
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

// QSS Headers
#include <QSS/fmu/simulate_fmu.hh>
#include <QSS/fmu/cycles_fmu.hh>
#include <QSS/fmu/Conditional.hh>
#include <QSS/fmu/container.hh>
#include <QSS/fmu/FMI.hh>
#include <QSS/fmu/FMU_Variable.hh>
#include <QSS/fmu/Function_Inp_constant.hh>
#include <QSS/fmu/Function_Inp_sin.hh>
#include <QSS/fmu/Function_Inp_step.hh>
#include <QSS/fmu/Function_Inp_toggle.hh>
#include <QSS/fmu/Variable_B.hh>
#include <QSS/fmu/Variable_D.hh>
#include <QSS/fmu/Variable_I.hh>
#include <QSS/fmu/Variable_Inp1.hh>
#include <QSS/fmu/Variable_Inp2.hh>
#include <QSS/fmu/Variable_InpB.hh>
#include <QSS/fmu/Variable_InpD.hh>
#include <QSS/fmu/Variable_InpI.hh>
#include <QSS/fmu/Variable_LIQSS1.hh>
#include <QSS/fmu/Variable_LIQSS2.hh>
#include <QSS/fmu/Variable_QSS1.hh>
#include <QSS/fmu/Variable_QSS2.hh>
#include <QSS/fmu/Variable_ZC1.hh>
#include <QSS/fmu/Variable_ZC2.hh>
#include <QSS/globals.hh>
#include <QSS/options.hh>
#include <QSS/Output.hh>

// FMI Library Headers
#include <fmilib.h>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace QSS {
namespace fmu {

// Forward
void
do_event_iteration( fmi2_import_t * fmu, fmi2_event_info_t * eventInfo );

// FMU Variable Pointer Union
union FMUVarPtr { // Support FMU real, integer, and boolean variables
	fmi2_import_real_variable_t * rvr; // FMU real variable pointer
	fmi2_import_integer_variable_t * ivr; // FMU integer variable pointer
	fmi2_import_bool_variable_t * bvr; // FMU boolean variable pointer

	FMUVarPtr( fmi2_import_real_variable_t * rvr ) :
	 rvr( rvr )
	{}

	FMUVarPtr( fmi2_import_integer_variable_t * ivr ) :
	 ivr( ivr )
	{}

	FMUVarPtr( fmi2_import_bool_variable_t * bvr ) :
	 bvr( bvr )
	{}

	friend
	bool
	operator ==( FMUVarPtr const & p1, FMUVarPtr const & p2 )
	{
		return p1.rvr == p2.rvr;
	}
};

// Hash for FMUVarPtr
struct FMUVarPtrHash final
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
	using Time = Variable::Time;
	using Value = Variable::Value;
	using Var_Idx = std::unordered_map< Variable const *, size_type >; // Map from Variables to their indexes
	using Conditionals = std::vector< Conditional * >;
	using FMU_Vars = std::unordered_map< FMUVarPtr, FMU_Variable, FMUVarPtrHash >; // Map from FMU variables to FMU_Variable objects
	using FMU_Idxs = std::unordered_map< size_type, Variable * >; // Map from FMU variable indexes to QSS Variables
	using Function = std::function< SmoothToken const &( Time const ) >;

	// I/o setup
	std::cout << std::setprecision( 16 );
	std::cerr << std::setprecision( 16 );
	std::vector< Output > x_outs; // Continuous rep outputs
	std::vector< Output > q_outs; // Quantized rep outputs
	std::vector< Output > f_outs; // FMU outputs

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
		std::cerr << "\nError: Only FMI version 2.0 is supported" << std::endl;
		std::exit( EXIT_FAILURE );
	}
	fmi2_xml_callbacks_t * xml_callbacks( nullptr );
	fmi2_import_t * const fmu( fmi2_import_parse_xml( context, tmpPath, xml_callbacks ) );
	if ( !fmu ) {
		std::cerr << "\nError: FMU XML parsing error" << std::endl;
		std::exit( EXIT_FAILURE );
	}
	fmu::fmu = fmu;
	if ( fmi2_import_get_fmu_kind( fmu ) == fmi2_fmu_kind_cs ) {
		std::cerr << "\nError: Only FMU ME is supported: Supplied FMU is CS" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	fmi2_callback_functions_t callBackFunctions;
	callBackFunctions.logger = fmi2_log_forwarding;
	callBackFunctions.allocateMemory = std::calloc;
	callBackFunctions.freeMemory = std::free;
	callBackFunctions.componentEnvironment = fmu;

	if ( fmi2_import_create_dllfmu( fmu, fmi2_fmu_kind_me, &callBackFunctions ) == jm_status_error ) {
		std::cerr << "\nError: Could not create the FMU library loading mechanism" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Get generation tool
	std::string const fmu_generation_tool( fmi2_import_get_generation_tool( fmu ) );
	std::cout << "FMU generated by: " << fmu_generation_tool << std::endl;
	enum class FMU_Generator { JModelica, Dymola, Other };
	FMU_Generator const fmu_generator(
	 fmu_generation_tool.find( "JModelica" ) == 0u ? FMU_Generator::JModelica : (
	 fmu_generation_tool.find( "Dymola" ) == 0u ? FMU_Generator::Dymola : FMU_Generator::Other )
	);

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
					std::cerr << "\nError: Non-SI unit present: " << fmi2_import_get_unit_name( unit ) << std::endl;
					units_error = true;
				}
			}
		}
		//if ( units_error ) std::exit( EXIT_FAILURE ); // Not a fatal error since some non-SI units don't affect integration
	}

	size_type const n_states( fmi2_import_get_number_of_continuous_states( fmu ) );
	std::cout << n_states << " continuous state variables" << std::endl;
	size_type const n_event_indicators( fmi2_import_get_number_of_event_indicators( fmu ) );
	std::cout << n_event_indicators << " event indicators" << std::endl;

	fmi2_real_t * const states( (fmi2_real_t*)std::calloc( n_states, sizeof( double ) ) );
	fmi2_real_t * const states_der( (fmi2_real_t*)std::calloc( n_states, sizeof( double ) ) );
	fmi2_real_t * event_indicators( (fmi2_real_t*)std::calloc( n_event_indicators, sizeof( double ) ) );
	fmi2_real_t * event_indicators_last( (fmi2_real_t*)std::calloc( n_event_indicators, sizeof( double ) ) );

	if ( fmi2_import_instantiate( fmu, "FMU ME model instance", fmi2_model_exchange, 0, 0 ) == jm_status_error ) {
		std::cerr << "\nError: fmi2_import_instantiate failed" << std::endl;
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
	Time tE( options::tEnd_set ? options::tEnd : tstop ); // Simulation end time
	Time t( t0 ); // Simulation current time
	Time tOut( t0 + options::dtOut ); // Sampling time
	size_type iOut( 1u ); // Output step index
	if ( ! options::rTol_set ) options::rTol = relativeTolerance; // Quantization relative tolerance (FMU doesn't have an absolute tolerance)
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
	Variables vars; // QSS variables
	Variables state_vars; // FMU state QSS variables
	Variables outs; // FMU output QSS variables
	Conditionals cons;
	FMU_Vars fmu_vars; // FMU variables
	FMU_Vars fmu_outs; // FMU output variables
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
						std::cerr << "\n Error: Specified start value does not match function value at t=0 for " << var_name << std::endl;
						std::exit( EXIT_FAILURE );
					}
					Variable_Inp * qss_var( nullptr );
					if ( ( options::qss == options::QSS::QSS1 ) || ( options::qss == options::QSS::LIQSS1 ) ) {
						qss_var = new Variable_Inp1( var_name, options::rTol, options::aTol, fmu_var, inp_fxn );
					} else if ( ( options::qss == options::QSS::QSS2 ) || ( options::qss == options::QSS::LIQSS2 ) ) {
						qss_var = new Variable_Inp2( var_name, options::rTol, options::aTol, fmu_var, inp_fxn );
					} else {
						std::cerr << "\n Error: Specified QSS method is not yet supported for FMUs" << std::endl;
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
					Variable_InpD * qss_var( new Variable_InpD( var_name, fmu_var, inp_fxn ) );
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
				FMU_Variable const fmu_var( var, var_int, fmi2_import_get_variable_vr( var ), i+1 );
				fmu_vars[ var_int ] = fmu_var;
				if ( var_causality == fmi2_causality_enu_input ) {
					std::cout << " Type: Integer: Discrete: Input" << std::endl;
//					Function inp_fxn = Function_Inp_constant( var_start ); // Constant start value
					Function inp_fxn = Function_Inp_step( 1.0, 1.0, 0.1 ); // Step up by 1 every 0.1 s via discrete events
					Variable_InpI * qss_var( new Variable_InpI( var_name, fmu_var, inp_fxn ) );
					vars.push_back( qss_var ); // Add to QSS variables
					fmu_idxs[ i+1 ] = qss_var; // Add to map from FMU variable index to QSS variable
					std::cout << " FMU idx: " << i+1 << " maps to QSS var: " << qss_var->name << std::endl;
				} else {
					std::cout << " Type: Integer: Discrete" << std::endl;
					Variable_I * qss_var( new Variable_I( var_name, var_start, fmu_var ) );
					vars.push_back( qss_var ); // Add to QSS variables
					if ( var_causality == fmi2_causality_enu_output ) { // Add to FMU QSS variable outputs
						outs.push_back( qss_var );
						fmu_outs.erase( var_int ); // Remove it from non-QSS FMU outputs
					}
					fmu_idxs[ i+1 ] = qss_var; // Add to map from FMU variable index to QSS variable
					std::cout << " FMU idx: " << i+1 << " maps to QSS var: " << qss_var->name << std::endl;
				}
			}
			}
			break;
		case fmi2_base_type_bool:
			std::cout << " Type: Boolean" << std::endl;
			{
			fmi2_import_bool_variable_t * var_bool( fmi2_import_get_variable_as_boolean( var ) );
			bool const var_start( var_has_start ? fmi2_import_get_boolean_variable_start( var_bool ) != 0 : 0 );
			if ( var_has_start ) std::cout << " Start: " << var_start << std::endl;
			if ( var_variability == fmi2_variability_enu_discrete ) {
				FMU_Variable const fmu_var( var, var_bool, fmi2_import_get_variable_vr( var ), i+1 );
				fmu_vars[ var_bool ] = fmu_var;
				if ( var_causality == fmi2_causality_enu_input ) {
					std::cout << " Type: Boolean: Discrete: Input" << std::endl;
					Function inp_fxn = Function_Inp_toggle( 1.0, 1.0, 0.1 ); // Toggle 0-1 every 0.1 s via discrete events
					Variable_InpB * qss_var( new Variable_InpB( var_name, fmu_var, inp_fxn ) );
					vars.push_back( qss_var ); // Add to QSS variables
					fmu_idxs[ i+1 ] = qss_var; // Add to map from FMU variable index to QSS variable
					std::cout << " FMU idx: " << i+1 << " maps to QSS var: " << qss_var->name << std::endl;
				} else {
					std::cout << " Type: Boolean: Discrete" << std::endl;
					Variable_B * qss_var( new Variable_B( var_name, var_start, fmu_var ) );
					vars.push_back( qss_var ); // Add to QSS variables
					if ( var_causality == fmi2_causality_enu_output ) { // Add to FMU QSS variable outputs
						outs.push_back( qss_var );
						fmu_outs.erase( var_bool ); // Remove it from non-QSS FMU outputs
					}
					fmu_idxs[ i+1 ] = qss_var; // Add to map from FMU variable index to QSS variable
					std::cout << " FMU idx: " << i+1 << " maps to QSS var: " << qss_var->name << std::endl;
				}
			}
			}
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
						std::cerr << "\n Warning: Initial value from xml specs: " << var_initial << " is not equal to initial value from fmi2GetContinuousStates(): " << states_initial << '\n';
						std::cerr << "          Using initial value from fmi2GetContinuousStates()" << std::endl;
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
					std::cerr << "\n Error: Specified QSS method is not yet supported for FMUs" << std::endl;
					std::exit( EXIT_FAILURE );
				}
				vars.push_back( qss_var ); // Add to QSS variables
				state_vars.push_back( qss_var ); // Add to state variables
				if ( fmi2_import_get_causality( fmu_var.var ) == fmi2_causality_enu_output ) { // Add to FMU QSS variable outputs
					outs.push_back( qss_var );
					fmu_outs.erase( fmu_var.rvr ); // Remove it from non-QSS FMU outputs
				}
				fmu_idxs[ fmu_var.idx ] = qss_var; // Add to map from FMU variable index to QSS variable
				std::cout << " FMU idx: " << fmu_var.idx << " maps to QSS var: " << qss_var->name << std::endl;
			} else {
				std::cerr << "\n Error: Derivative missing associated variable: " << der_name << std::endl;
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
	size_type const n_state_vars( state_vars.size() );

	// Process FMU zero-crossing variables
	std::cout << "\nFMU Zero Crossing Processing =====" << std::endl;
	size_type n_ZC_vars( 0 );
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
									qss_var = new Variable_ZC1( var_name, options::rTol, options::aTol, options::zTol, fmu_var, fmu_der );
								} else if ( ( options::qss == options::QSS::QSS2 ) || ( options::qss == options::QSS::LIQSS2 ) ) {
									qss_var = new Variable_ZC2( var_name, options::rTol, options::aTol, options::zTol, fmu_var, fmu_der );
								} else {
									std::cerr << "\n Error: Specified QSS method is not yet supported for FMUs" << std::endl;
									std::exit( EXIT_FAILURE );
								}
								vars.push_back( qss_var ); // Add to QSS variables
								if ( fmi2_import_get_causality( fmu_var.var ) == fmi2_causality_enu_output ) { // Add to FMU QSS variable outputs
									outs.push_back( qss_var );
									fmu_outs.erase( fmu_var.rvr ); // Remove it from non-QSS FMU outputs
								}
								fmu_idxs[ fmu_var.idx ] = qss_var; // Add to map from FMU variable index to QSS variable
								std::cout << " FMU idx: " << fmu_var.idx << " maps to QSS var: " << qss_var->name << std::endl;
								++n_ZC_vars;

								// Create single clause when block for the zero-crossing variable for now: FMU conditional block info would allow us to do more
								using When = WhenV< Variable >;
								When * when( new When() );
								cons.push_back( when );
								When::Clause * when_clause( when->add_clause() );
								when_clause->add( qss_var );
							}
							break; // Found derivative so stop scanning
						}
					}
				}
			}
		}
	}
	size_type const n_vars( vars.size() );
	size_type const n_outs( outs.size() );
	size_type const n_fmu_outs( fmu_outs.size() );
	size_type const n_all_outs( n_outs + n_fmu_outs );
	if ( fmu_generator == FMU_Generator::Dymola ) {
		if ( n_event_indicators != 2 * n_ZC_vars ) { // Dymola has 2x as many event indicators
			std::cerr << "\nWarning: Number of FMU event indicators (" << n_event_indicators << ") is not equal to twice the number of zero-crossing variables found (" << n_ZC_vars << ") as expected for Dymola FMUs" << std::endl;
		}
	} else if ( n_event_indicators != n_ZC_vars ) {
		std::cerr << "\nWarning: Number of FMU event indicators (" << n_event_indicators << ") is not equal to the number of zero-crossing variables found (" << n_ZC_vars << ')' << std::endl;
	}

	{ // QSS observer setup: Continuous variables: Derivatives
		std::cout << "\nObserver Setup: Continuous Variables: Derivatives =====" << std::endl;
		size_type * startIndex( nullptr );
		size_type * dependency( nullptr );
		char * factorKind( nullptr );
		fmi2_import_get_derivatives_dependencies( fmu, &startIndex, &dependency, &factorKind );
		if ( startIndex != nullptr ) { // Derivatives dependency info present in XML
			for ( size_type i = 0; i < n_ders; ++i ) {
				std::cout << "\nDerivative  Ref: " << drs[ i ] << std::endl;
				fmi2_import_variable_t * der( fmi2_import_get_variable( der_list, i ) );
				std::string const der_name( fmi2_import_get_variable_name( der ) );
				std::cout << " Name: " << der_name << std::endl;
				fmi2_import_real_variable_t * der_real( fmi2_import_get_variable_as_real( der ) );
				size_type const idx( fmu_dvrs[ der_real ].idx );
				Variable * var( fmu_idxs[ idx ] );
				std::cout << " Var: " << var->name << "  Index: " << idx << std::endl;
				for ( size_type j = startIndex[ i ]; j < startIndex[ i + 1 ]; ++j ) {
					size_type const dep_idx( dependency[ j ] );
					std::cout << "  Dep Index: " << dep_idx << std::endl;
					if ( dep_idx == 0 ) { // No info: Depends on all (don't support depends on all for now)
						std::cerr << "\n   Error: No dependency information provided: Depends-on-all not currently supported" << std::endl;
					} else { // Process based on kind of dependent
						fmi2_dependency_factor_kind_enu_t const kind( (fmi2_dependency_factor_kind_enu_t)( factorKind[ j ] ) );
						if ( kind == fmi2_dependency_factor_kind_dependent ) {
							std::cout << "  Kind: Dependent (" << kind << ')' << std::endl;
						} else if ( kind == fmi2_dependency_factor_kind_constant ) {
							std::cout << "  Kind: Constant (" << kind << ')' << std::endl;
						} else if ( kind == fmi2_dependency_factor_kind_fixed ) {
							std::cout << "  Kind: Fixed (" << kind << ')' << std::endl;
						} else if ( kind == fmi2_dependency_factor_kind_tunable ) {
							std::cout << "  Kind: Tunable (" << kind << ')' << std::endl;
						} else if ( kind == fmi2_dependency_factor_kind_discrete ) {
							std::cout << "  Kind: Discrete (" << kind << ')' << std::endl;
						} else if ( kind == fmi2_dependency_factor_kind_num ) {
							std::cout << "  Kind: Num (" << kind << ')' << std::endl;
						}
					}
					auto idep( fmu_idxs.find( dep_idx ) ); //Do Add support for input variable dependents
					if ( idep != fmu_idxs.end() ) {
						Variable * dep( idep->second );
						if ( dep == var ) {
							std::cout << "  Var: " << dep->name << " is self-observer" << std::endl;
							var->self_observer = true;
						} else if ( dep->is_ZC() ) {
							std::cout << "  Zero Crossing Var: " << dep->name << " handler modifies " << var->name << std::endl;
							assert( dep->when_clauses.size() == 1u ); // Should just be one clause for now
							for ( auto * when_clause : dep->when_clauses ) when_clause->add_observer( var );
						} else {
							std::cout << "  Var: " << dep->name << " has observer " << var->name << std::endl;
							var->observe( dep );
						}
					} else {
						//std::cout << "FMU derivative " << der_name << " has dependency with index " << dep_idx << " that is not a QSS variable" << std::endl;
					}
				}
			}
		} else { // Assume no observers in model (this may not be true: FMI spec says no dependencies => dependent on all)
			std::cout << "No Derivatives dependency info in FMU XML" << std::endl;
		}
	}

	{ // QSS observer setup: Continuous variables: InitialUnknowns
		std::cout << "\nObserver Setup: Continuous Variables: InitialUnknowns =====" << std::endl;
		size_type * startIndex( nullptr );
		size_type * dependency( nullptr );
		char * factorKind( nullptr );
		fmi2_import_variable_list_t * inu_list( fmi2_import_get_initial_unknowns_list( fmu ) ); // InitialUnknowns variables
		size_type const n_inu_vars( fmi2_import_get_variable_list_size( inu_list ) );
		std::cout << n_inu_vars << " variables found in InitialUnknowns" << std::endl;
		fmi2_value_reference_t const * inu_vrs( fmi2_import_get_value_referece_list( inu_list ) ); // reference is spelled wrong in FMIL API
		fmi2_import_get_initial_unknowns_dependencies( fmu, &startIndex, &dependency, &factorKind );
		if ( startIndex != nullptr ) { // InitialUnknowns dependency info present in XML
			for ( size_type i = 0; i < n_inu_vars; ++i ) {
				std::cout << "\nInitialUnknown Variable  Ref: " << inu_vrs[ i ] << std::endl;
				fmi2_import_variable_t * inu( fmi2_import_get_variable( inu_list, i ) );
				if ( fmi2_import_get_variability( inu ) != fmi2_variability_enu_continuous ) {
					std::cout << " Skipping: Not continuous variable" << std::endl;
					continue; // Only look at continuous variables
				}
				std::string const inu_name( fmi2_import_get_variable_name( inu ) );
				std::cout << " Name: " << inu_name << std::endl;
				fmi2_import_real_variable_t * inu_real( fmi2_import_get_variable_as_real( inu ) );
				FMU_Variable & fmu_inu( fmu_vars[ inu_real ] );
				size_type const idx( fmu_inu.idx );
				auto ivar( fmu_idxs.find( idx ) );
				if ( ivar != fmu_idxs.end() ) {
					Variable * var( fmu_idxs[ idx ] );
					std::cout << " Var: " << var->name << "  Index: " << idx << std::endl;
					for ( size_type j = startIndex[ i ]; j < startIndex[ i + 1 ]; ++j ) {
						size_type const dep_idx( dependency[ j ] );
						std::cout << "  Dep Index: " << dep_idx << std::endl;
						if ( dep_idx == 0 ) { // No info: Depends on all (don't support depends on all for now)
							std::cerr << "\n   Error: No dependency information provided: Depends-on-all not currently supported" << std::endl;
						} else { // Process based on kind of dependent
							fmi2_dependency_factor_kind_enu_t const kind( (fmi2_dependency_factor_kind_enu_t)( factorKind[ j ] ) );
							if ( kind == fmi2_dependency_factor_kind_dependent ) {
								std::cout << "  Kind: Dependent (" << kind << ')' << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_constant ) {
								std::cout << "  Kind: Constant (" << kind << ')' << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_fixed ) {
								std::cout << "  Kind: Fixed (" << kind << ')' << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_tunable ) {
								std::cout << "  Kind: Tunable (" << kind << ')' << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_discrete ) {
								std::cout << "  Kind: Discrete (" << kind << ')' << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_num ) {
								std::cout << "  Kind: Num (" << kind << ')' << std::endl;
							}
						}
						auto idep( fmu_idxs.find( dep_idx ) ); //Do Add support for input variable dependents
						if ( idep != fmu_idxs.end() ) {
							Variable * dep( idep->second );
							if ( dep == var ) {
								std::cout << "  Var: " << dep->name << " is self-observer" << std::endl;
								var->self_observer = true;
							} else if ( dep->is_ZC() ) {
								std::cout << "  Zero Crossing Var: " << dep->name << " handler modifies " << var->name << std::endl;
								assert( dep->when_clauses.size() == 1u ); // Should just be one clause for now
								for ( auto * when_clause : dep->when_clauses ) when_clause->add_observer( var );
							} else {
								std::cout << "  Var: " << dep->name << " has observer " << var->name << std::endl;
								var->observe( dep );
							}
						} else {
							//std::cout << "FMU variable " << var_name << " has dependency with index " << dep_idx << " that is not a QSS variable" << std::endl;
						}
					}
				}
			}
		} else { // Assume no observers in model (this may not be true: FMI spec says no dependencies => dependent on all)
			std::cout << "No InitialUknowns dependency info in FMU XML" << std::endl;
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
				std::cout << "\nDiscrete Variable  Ref: " << dis_vrs[ i ] << std::endl;
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
					{
					fmi2_import_integer_variable_t * dis_int( fmi2_import_get_variable_as_integer( dis ) );
					fmu_dis = &fmu_vars[ dis_int ];
					std::cout << " FMU idx: " << fmu_dis->idx << " maps to QSS var: " << fmu_idxs[ fmu_dis->idx ]->name << std::endl;
					}
					break;
				case fmi2_base_type_bool:
					std::cout << " Type: Boolean" << std::endl;
					{
					fmi2_import_bool_variable_t * dis_bool( fmi2_import_get_variable_as_boolean( dis ) );
					fmu_dis = &fmu_vars[ dis_bool ];
					std::cout << " FMU idx: " << fmu_dis->idx << " maps to QSS var: " << fmu_idxs[ fmu_dis->idx ]->name << std::endl;
					}
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
						std::cout << "  Dep Index: " << dep_idx << std::endl;
						if ( dep_idx == 0 ) { // No info: Depends on all (don't support depends on all for now)
							std::cerr << "\n   Error: No dependency information provided: Depends-on-all not currently supported" << std::endl;
						} else { // Process based on kind of dependent
							fmi2_dependency_factor_kind_enu_t const kind( (fmi2_dependency_factor_kind_enu_t)( factorKind[ j ] ) );
							if ( kind == fmi2_dependency_factor_kind_dependent ) {
								std::cout << "  Kind: Dependent (" << kind << ')' << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_constant ) {
								std::cout << "  Kind: Constant (" << kind << ')' << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_fixed ) {
								std::cout << "  Kind: Fixed (" << kind << ')' << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_tunable ) {
								std::cout << "  Kind: Tunable (" << kind << ')' << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_discrete ) {
								std::cout << "  Kind: Discrete (" << kind << ')' << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_num ) {
								std::cout << "  Kind: Num (" << kind << ')' << std::endl;
							}
						}
						auto idep( fmu_idxs.find( dep_idx ) ); //Do Add support for input variable dependents
						if ( idep != fmu_idxs.end() ) {
							Variable * dep( idep->second );
							if ( dep == dis_var ) {
								std::cerr << "\n   Error: Discrete variable " << dis_name << " has self-dependency" << std::endl;
								std::exit( EXIT_FAILURE );
							} else if ( dep->is_ZC() ) {
								std::cout << "  Zero Crossing Var: " << dep->name << " handler modifies discrete variable " << dis_name << std::endl;
								assert( dep->when_clauses.size() == 1u ); // Should just be one clause for now
								for ( auto * when_clause : dep->when_clauses ) when_clause->add_observer( dis_var );
							} else {
								std::cerr << "\n   Error: Discrete variable " << dis_name << " has dependency on non-zero-crossing variable " << dep->name << std::endl;
								std::exit( EXIT_FAILURE );
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
				std::cout << "\nOutput Variable  Ref: " << out_vrs[ i ] << std::endl;
				fmi2_import_variable_t * out( fmi2_import_get_variable( out_list, i ) );
				assert( fmi2_import_get_causality( out ) == fmi2_causality_enu_output );
				std::string const out_name( fmi2_import_get_variable_name( out ) );
				std::cout << " Name: " << out_name << std::endl;
				FMU_Variable * fmu_out( nullptr ); // Output FMU variable
				FMU_Variable * fmu_var( nullptr ); // FMU variable that output variable is derivative of
				fmi2_base_type_enu_t out_base_type( fmi2_import_get_variable_base_type( out ) );
				switch ( out_base_type ) {
				case fmi2_base_type_real:
					std::cout << " Type: Real" << std::endl;
					{
					fmi2_import_real_variable_t * out_real( fmi2_import_get_variable_as_real( out ) );
					fmu_out = &fmu_vars[ out_real ];
					auto const ider( fmu_dvrs.find( out_real ) );
					if ( ider != fmu_dvrs.end() ) fmu_var = &(ider->second);
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
				if ( ( iout == fmu_idxs.end() ) && ( fmu_var != nullptr ) ) iout = fmu_idxs.find( fmu_var->idx ); // Use variable that output variable is derivative of
				if ( iout != fmu_idxs.end() ) {
					Variable * out_var( iout->second );
					std::cout << " FMU idx: " << fmu_out->idx << " -> QSS var: " << out_var->name << std::endl;
					if ( ! out_var->is_ZC() ) continue; // Don't worry about dependencies of non-ZC output variables on the QSS side
					for ( size_type j = startIndex[ i ]; j < startIndex[ i + 1 ]; ++j ) {
						size_type const dep_idx( dependency[ j ] );
						std::cout << "  Dep Index: " << dep_idx << std::endl;
						if ( dep_idx == 0 ) { // No info: Depends on all (don't support depends on all for now)
							std::cerr << "\n   Error: No dependency information provided: Depends-on-all not currently supported" << std::endl;
						} else { // Process based on kind of dependent
							fmi2_dependency_factor_kind_enu_t const kind( (fmi2_dependency_factor_kind_enu_t)( factorKind[ j ] ) );
							if ( kind == fmi2_dependency_factor_kind_dependent ) {
								std::cout << "  Kind: Dependent (" << kind << ')' << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_constant ) {
								std::cout << "  Kind: Constant (" << kind << ')' << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_fixed ) {
								std::cout << "  Kind: Fixed (" << kind << ')' << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_tunable ) {
								std::cout << "  Kind: Tunable (" << kind << ')' << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_discrete ) {
								std::cout << "  Kind: Discrete (" << kind << ')' << std::endl;
							} else if ( kind == fmi2_dependency_factor_kind_num ) {
								std::cout << "  Kind: Num (" << kind << ')' << std::endl;
							}
						}
						auto idep( fmu_idxs.find( dep_idx ) ); //Do Add support for input variable dependents
						if ( idep != fmu_idxs.end() ) {
							Variable * dep( idep->second );
							if ( dep == out_var ) {
								std::cerr << "\n   Error: Output variable " << out_name << " has self-dependency" << std::endl;
								std::exit( EXIT_FAILURE );
							} else if ( dep->is_ZC() ) {
								std::cout << "  Zero Crossing Var: " << dep->name << " handler modifies output variable " << out_name << std::endl;
								assert( dep->when_clauses.size() == 1u ); // Should just be one clause for now
								for ( auto * when_clause : dep->when_clauses ) when_clause->add_observer( out_var );
							} else {
								std::cout << "  Var: " << dep->name << " has observer " << out_name << std::endl;
								out_var->observe( dep );
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

	// Size checks
	if ( n_state_vars != n_states ) {
		std::cerr << "\nError: Number of state variables found (" << n_state_vars << ") is not equal to number in FMU (" << n_states << ')' << std::endl;
	}

	// Variable-index map setup
	Var_Idx var_idx;
	for ( size_type i = 0; i < n_vars; ++i ) {
		var_idx[ vars[ i ] ] = i;
	}

	// Containers of ZC and non-ZC variables
	Variables vars_ZC;
	Variables vars_NZ;
	int max_QSS_order( 0 );
	for ( auto var : vars ) {
		if ( var->is_ZC() ) { // ZC variable
			vars_ZC.push_back( var );
		} else { // Non-ZC variable
			vars_NZ.push_back( var );
			max_QSS_order = std::max( max_QSS_order, var->order() ); // Max QSS order of non-ZC variables to avoid unnec loop stages
		}
	}
	int const QSS_order_max( max_QSS_order ); // Highest QSS order in use
	assert( QSS_order_max <= 3 );

	// Variable initialization
	std::cout << "\nInitialization =====" << std::endl;
	fmu::set_time( t0 );
	fmu::init_derivatives( n_ders );
	if ( t0 != Time( 0 ) ) {
		for ( auto var : vars ) {
			var->init_time( t0 );
		}
	}
	for ( auto var : vars_ZC ) {
		var->init_0_ZC(); // Adds drill-through observees
	}
	for ( auto var : vars_NZ ) {
		var->init_0();
	}
	for ( auto var : vars_NZ ) {
		var->init_1();
	}
	if ( QSS_order_max >= 2 ) {
		fmu::set_time( t = t0 + options::dtNum ); // Set time to t0 + delta for numeric differentiation
		for ( auto var : vars_NZ ) {
			if ( ! var->is_Discrete() ) var->fmu_set_sn( t );
		}
		for ( auto var : vars_NZ ) {
			var->init_2();
		}
	}
	if ( ! vars_ZC.empty() ) {
		if ( QSS_order_max >= 2 ) fmu::set_time( t0 );
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
	fmu::set_time( t = t0 );

	// Dependency cycle detection: After init sets up observers
	if ( options::cycles ) cycles( vars );

	// Output initialization
	bool const doSOut( ( options::output::s && ( options::output::x || options::output::q ) ) || ( options::output::f && ( n_all_outs > 0u ) ) );
	bool const doTOut( options::output::t && ( options::output::x || options::output::q ) );
	bool const doROut( options::output::r && ( options::output::x || options::output::q ) );
	if ( ( options::output::t || options::output::r || options::output::s ) && ( options::output::x || options::output::q ) ) { // t0 outputs
		if ( options::output::x ) x_outs.reserve( vars.size() );
		if ( options::output::q ) q_outs.reserve( vars.size() );
		for ( auto var : vars ) {
			if ( options::output::x ) {
				x_outs.push_back( Output( var->name, 'x' ) );
				x_outs.back().append( t, var->x( t ) );
			}
			if ( options::output::q ) {
				q_outs.push_back( Output( var->name, 'q' ) );
				q_outs.back().append( t, var->q( t ) );
			}
		}
	}
	if ( options::output::f && ( n_all_outs > 0u ) ) {
		f_outs.reserve( n_all_outs );
		for ( auto const & var : outs ) { // FMU QSS variable outputs
			f_outs.push_back( Output( fmi2_import_get_variable_name( var->var.var ), 'f' ) );
			f_outs.back().append( t, var->x( t ) );
		}
		for ( auto const & e : fmu_outs ) { // FMU (non-QSS) variable (non-QSS) outputs
			FMU_Variable const & var( e.second );
			f_outs.push_back( Output( fmi2_import_get_variable_name( var.var ), 'f' ) );
			f_outs.back().append( t, fmu::get_real( var.ref ) );
		}
	}

	// Simulation loop
	std::cout << "\nSimulation Loop =====" << std::endl;
	size_type const max_pass_count_multiplier( 2 );
	size_type n_discrete_events( 0 );
	size_type n_QSS_events( 0 );
	size_type n_QSS_simultaneous_events( 0 );
	size_type n_ZC_events( 0 );
	double sim_dtMin( options::dtMin );
	bool pass_warned( false );
	Variables observers;
	while ( t <= tE ) {
		t = events.top_time();
		if ( doSOut ) { // Sampled and/or FMU outputs
			Time const tStop( std::min( t, tE ) );
			while ( tOut < tStop ) {
				if ( options::output::s ) { // QSS variable outputs
					for ( size_type i = 0; i < n_vars; ++i ) {
						if ( options::output::x ) x_outs[ i ].append( tOut, vars[ i ]->x( tOut ) );
						if ( options::output::q ) q_outs[ i ].append( tOut, vars[ i ]->q( tOut ) );
					}
				}
				if ( options::output::f ) {	// FMU variable outputs
					if ( n_outs > 0u ) { // FMU QSS variables
						for ( size_type i = 0; i < n_outs; ++i ) {
							Variable * var( outs[ i ] );
							f_outs[ i ].append( tOut, var->x( tOut ) );
						}
					}
					if ( n_fmu_outs > 0u ) { // FMU (non-QSS) variables
						fmu::set_time( tOut );
						for ( size_type i = 0; i < n_states; ++i ) {
							states[ i ] = state_vars[ i ]->x( tOut );
						}
						fmi2_import_set_continuous_states( fmu, states, n_states );
						size_type i( n_outs );
						for ( auto const & e : fmu_outs ) {
							FMU_Variable const & var( e.second );
							f_outs[ i++ ].append( tOut, fmu::get_real( var.ref ) );
						}
					}
				}
				assert( iOut < std::numeric_limits< size_type >::max() );
				tOut = t0 + ( ++iOut ) * options::dtOut;
			}
		}
		if ( t <= tE ) { // Perform event(s)
			fmu::set_time( t );
			Event< Target > & event( events.top() );
			SuperdenseTime const s( events.top_superdense_time() );
			if ( s.i >= options::pass ) { // Pass count limit reached
				if ( s.i <= max_pass_count_multiplier * options::pass ) { // Use time step controls
					if ( sim_dtMin > 0.0 ) { // Double dtMin
						if ( sim_dtMin < std::min( 0.5 * infinity, 0.25 * options::dtMax ) ) {
							sim_dtMin = std::min( 2.0 * sim_dtMin, 0.5 * options::dtMax );
						} else {
							std::cerr << "\nError: Pass count limit exceeded at time: " << t << "  Min time step limit reached: Terminating simulation" << std::endl;
							tE = t; // To avoid tE outputs beyond actual simulation
							break;
						}
					} else { // Set dtMin
						sim_dtMin = std::min( std::max( 1.0e-9, tE * 1.0e-12 ), 0.5 * options::dtMax );
					}
					for ( auto var : vars ) {
						var->dt_min = sim_dtMin;
					}
					if ( ! pass_warned ) {
						std::cerr << "\nWarning: Pass count limit reached at time: " << t << "  Min time step control activated" << std::endl;
						pass_warned = true;
					}
				} else { // Time step control doesn't seem to be working: Abort
					std::cerr << "\nError: " << max_pass_count_multiplier << " x pass count limit exceeded at time: " << t << "  Terminating simulation" << std::endl;
					tE = t; // To avoid tE outputs beyond actual simulation
					break;
				}
			}
			events.set_active_time();
			if ( event.is_discrete() ) { // Discrete event
				++n_discrete_events;
				if ( events.single() ) { // Single trigger
					Variable * trigger( event.sub< Variable >() );
					assert( trigger->tD == t );

					trigger->st = s; // Set trigger superdense time

					if ( doTOut ) { // Time event output: before discrete changes
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
							}
						} else { // Time event and observer output
							if ( options::output::t ) { // Time event output
								size_type const i( var_idx[ trigger ] );
								if ( options::output::x ) x_outs[ i ].append( t, trigger->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, trigger->q( t ) );
								for ( Variable const * observer : trigger->observers() ) { // Observer output
									size_type const i( var_idx[ observer ] );
									if ( options::output::x ) x_outs[ i ].append( t, observer->x( t ) );
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										if ( options::output::q ) q_outs[ i ].append( t, observer->q( t ) );
									}
								}
							}
						}
					}

					trigger->advance_discrete();

					if ( doTOut ) { // Time event output: after discrete changes
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
							}
						} else { // Time event and observer output
							if ( options::output::t ) { // Time event output
								size_type const i( var_idx[ trigger ] );
								if ( options::output::x ) x_outs[ i ].append( t, trigger->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, trigger->q( t ) );
								for ( Variable const * observer : trigger->observers() ) { // Observer output
									size_type const i( var_idx[ observer ] );
									if ( options::output::x ) x_outs[ i ].append( t, observer->x( t ) );
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										if ( options::output::q ) q_outs[ i ].append( t, observer->q( t ) );
									}
								}
							}
						}
					}
				} else { // Simultaneous triggers
					Variables triggers( events.top_subs< Variable >() );
					variables_observers( triggers, observers );
					size_type const iBeg_triggers_2( begin_order_index( triggers, 2 ) );
					int const triggers_order_max( triggers.back()->order() );

					if ( doTOut ) { // Time event output: before discrete changes
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
							}
						} else { // Time event and observer output
							if ( options::output::t ) { // Time event output
								for ( Variable const * trigger : triggers ) { // Triggers
									size_type const i( var_idx[ trigger ] );
									if ( options::output::x ) x_outs[ i ].append( t, trigger->x( t ) );
									if ( options::output::q ) q_outs[ i ].append( t, trigger->q( t ) );
								}
								for ( Variable const * observer : observers ) { // Observer output
									size_type const i( var_idx[ observer ] );
									if ( options::output::x ) x_outs[ i ].append( t, observer->x( t ) );
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										if ( options::output::q ) q_outs[ i ].append( t, observer->q( t ) );
									}
								}
							}
						}
					}

					for ( Variable * trigger : triggers ) {
						assert( trigger->tD == t );
						trigger->st = s; // Set trigger superdense time
						trigger->advance_discrete_0_1();
					}
					if ( triggers_order_max >= 2 ) { // 2nd order pass
						// fmu::set_time( t + options::dtNum ); // Set time to t + delta for numeric differentiation // Need this if we enable discrete events on QSS variables
						for ( size_type i = iBeg_triggers_2, n = triggers.size(); i < n; ++i ) {
							triggers[ i ]->advance_discrete_2();
						}
					}

					if ( ! observers.empty() ) { // Advance observers
						// if ( triggers_order_max >= 2 ) fmu::set_time( t ); // Need this if we enable discrete events on QSS variables
						Variable::advance_observers( observers, t );
					}

					if ( doTOut ) { // Time event output: after discrete changes
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
							}
						} else { // Time event and observer output
							if ( options::output::t ) { // Time event output
								for ( Variable const * trigger : triggers ) { // Triggers
									size_type const i( var_idx[ trigger ] );
									if ( options::output::x ) x_outs[ i ].append( t, trigger->x( t ) );
									if ( options::output::q ) q_outs[ i ].append( t, trigger->q( t ) );
								}
								for ( Variable const * observer : observers ) { // Observer output
									size_type const i( var_idx[ observer ] );
									if ( options::output::x ) x_outs[ i ].append( t, observer->x( t ) );
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										if ( options::output::q ) q_outs[ i ].append( t, observer->q( t ) );
									}
								}
							}
						}
					}
				}
			} else if ( event.is_ZC() ) { // Zero-crossing event
				++n_ZC_events;
				while ( events.top_superdense_time() == s ) {
					Variable * trigger( events.top_sub< Variable >() );
					assert( trigger->tZC() == t );
					trigger->st = s; // Set trigger superdense time
					trigger->advance_ZC();
					if ( doTOut ) { // Time event output: after discrete changes
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
							}
						} else { // Time event and observer output
							if ( options::output::t ) { // Time event output
								size_type const i( var_idx[ trigger ] );
								if ( options::output::x ) x_outs[ i ].append( t, trigger->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, trigger->q( t ) );
							}
						}
					}
				}
			} else if ( event.is_conditional() ) { // Conditional event
				while ( events.top_superdense_time() == s ) {
					Conditional * trigger( events.top_sub< Conditional >() );
					trigger->st = s; // Set trigger superdense time
					trigger->advance_conditional();
				}
			} else if ( event.is_handler() ) { // Zero-crossing handler event

				// Perform FMU event mode handler processing /////

				// Advance FMU time to help it detect zero crossing event
				fmu::set_time( t + options::dtZC );

				{ // Swap event_indicators and event_indicators_last so that we can get new indicators
					fmi2_real_t * temp = event_indicators;
					event_indicators = event_indicators_last;
					event_indicators_last = temp;
				}
				fmi2_status_t fmi_status = fmi2_import_get_event_indicators( fmu, event_indicators, n_event_indicators );

				// Check if an event indicator has triggered
				bool zero_crossing_event( false );
				for ( size_type k = 0; k < n_event_indicators; ++k ) {
					if ( ( event_indicators[ k ] > 0 ) != ( event_indicators_last[ k ] > 0 ) ) {
						zero_crossing_event = true;
						break;
					}
				}

				// Handle zero-crossing events
				if ( callEventUpdate || zero_crossing_event ) {
					fmi_status = fmi2_import_enter_event_mode( fmu );
					do_event_iteration( fmu, &eventInfo );
					fmi_status = fmi2_import_enter_continuous_time_mode( fmu );
					fmi_status = fmi2_import_get_continuous_states( fmu, states, n_states );
					fmi_status = fmi2_import_get_event_indicators( fmu, event_indicators, n_event_indicators );
					if ( options::output::d ) std::cout << "Zero-crossing triggers FMU event at t=" << t << std::endl;
				} else {
					if ( options::output::d ) std::cout << "Zero-crossing does not trigger FMU event at t=" << t << std::endl;
				}

				// Restore FMU simulation time
				fmu::set_time( t );

				// Perform handler operations on QSS side
				if ( callEventUpdate || zero_crossing_event ) {
					if ( events.single() ) { // Single handler
						Variable * handler( event.sub< Variable >() );

						if ( doROut ) { // Requantization output: before handler changes
							if ( options::output::a ) { // All variables output
								for ( size_type i = 0; i < n_vars; ++i ) {
									if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
									if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
								}
							} else { // Requantization and observer output
								if ( options::output::r ) { // Requantization output
									size_type const i( var_idx[ handler ] );
									if ( options::output::x ) x_outs[ i ].append( t, handler->x( t ) );
									if ( options::output::q ) q_outs[ i ].append( t, handler->q( t ) );
									for ( Variable const * observer : handler->observers() ) { // Observer output
										size_type const i( var_idx[ observer ] );
										if ( options::output::x ) x_outs[ i ].append( t, observer->x( t ) );
										if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
											if ( options::output::q ) q_outs[ i ].append( t, observer->q( t ) );
										}
									}
								}
							}
						}

						handler->advance_handler( t );

						if ( doROut ) { // Requantization output: after handler changes
							if ( options::output::a ) { // All variables output
								for ( size_type i = 0; i < n_vars; ++i ) {
									if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
									if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
								}
							} else { // Requantization and observer output
								if ( options::output::r ) { // Requantization output
									size_type const i( var_idx[ handler ] );
									if ( options::output::x ) x_outs[ i ].append( t, handler->x( t ) );
									if ( options::output::q ) q_outs[ i ].append( t, handler->q( t ) );
									for ( Variable const * observer : handler->observers() ) { // Observer output
										size_type const i( var_idx[ observer ] );
										if ( options::output::x ) x_outs[ i ].append( t, observer->x( t ) );
										if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
											if ( options::output::q ) q_outs[ i ].append( t, observer->q( t ) );
										}
									}
								}
							}
						}
					} else { // Simultaneous handlers
						Variables handlers( events.top_subs< Variable >() );
						variables_observers( handlers, observers );
						size_type const iBeg_handlers_1( begin_order_index( handlers, 1 ) );
						size_type const iBeg_handlers_2( begin_order_index( handlers, 2 ) );
						int const handlers_order_max( handlers.back()->order() );

						if ( doROut ) { // Requantization output: before handler changes
							if ( options::output::a ) { // All variables output
								for ( size_type i = 0; i < n_vars; ++i ) {
									if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
									if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
								}
							} else { // Requantization and observer output
								if ( options::output::r ) { // Requantization output
									for ( Variable const * handler : handlers ) { // Handlers
										size_type const i( var_idx[ handler ] );
										if ( options::output::x ) x_outs[ i ].append( t, handler->x( t ) );
										if ( options::output::q ) q_outs[ i ].append( t, handler->q( t ) );
									}
									for ( Variable const * observer : observers ) { // Observer output
										size_type const i( var_idx[ observer ] );
										if ( options::output::x ) x_outs[ i ].append( t, observer->x( t ) );
										if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
											if ( options::output::q ) q_outs[ i ].append( t, observer->q( t ) );
										}
									}
								}
							}
						}

						for ( Variable * handler : handlers ) {
							handler->advance_handler_0( t );
						}
						for ( size_type i = iBeg_handlers_1, n = handlers.size(); i < n; ++i ) {
							handlers[ i ]->advance_handler_1();
						}
						if ( handlers_order_max >= 2 ) { // 2nd order pass
							fmu::set_time( t + options::dtNum ); // Advance time to t + delta for numeric differentiation
							for ( size_type i = iBeg_handlers_2, n = handlers.size(); i < n; ++i ) {
								handlers[ i ]->advance_handler_2();
							}
						}

						if ( ! observers.empty() ) { // Advance observers
							if ( handlers_order_max >= 2 ) fmu::set_time( t );
							Variable::advance_observers( observers, t );
						}

						if ( doROut ) { // Requantization output: after handler changes
							if ( options::output::a ) { // All variables output
								for ( size_type i = 0; i < n_vars; ++i ) {
									if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
									if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
								}
							} else { // Requantization and observer output
								if ( options::output::r ) { // Requantization output
									for ( Variable const * handler : handlers ) {
										size_type const i( var_idx[ handler ] );
										if ( options::output::x ) x_outs[ i ].append( t, handler->x( t ) );
										if ( options::output::q ) q_outs[ i ].append( t, handler->q( t ) );
									}
									for ( Variable const * observer : observers ) { // Observer output
										size_type const i( var_idx[ observer ] );
										if ( options::output::x ) x_outs[ i ].append( t, observer->x( t ) );
										if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
											if ( options::output::q ) q_outs[ i ].append( t, observer->q( t ) );
										}
									}
								}
							}
						}
					}
				} else { // Update event queue entries for no-action handler event
					if ( events.single() ) { // Single handler
						Variable * handler( event.sub< Variable >() );
						handler->no_advance_handler();
					} else { // Simultaneous handlers
						for ( Variable * handler : events.top_subs< Variable >() ) {
							handler->no_advance_handler();
						}
					}
				}
			} else if ( event.is_QSS() ) { // QSS requantization event
				++n_QSS_events;
				if ( events.single() ) { // Single trigger
					Variable * trigger( event.sub< Variable >() );
					assert( trigger->tE == t );
					assert( ! trigger->is_ZC() ); // ZC variable requantizations are QSS_ZC events
					trigger->st = s; // Set trigger superdense time

					trigger->advance_QSS();

					if ( doROut ) { // Requantization output
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
							}
						} else { // Requantization and observer output
							if ( options::output::r ) { // Requantization output
								size_type const i( var_idx[ trigger ] );
								if ( options::output::x ) x_outs[ i ].append( t, trigger->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, trigger->q( t ) );
								for ( Variable const * observer : trigger->observers() ) { // Observer output
									size_type const i( var_idx[ observer ] );
									if ( options::output::x ) x_outs[ i ].append( t, observer->x( t ) );
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										if ( options::output::q ) q_outs[ i ].append( t, observer->q( t ) );
									}
								}
							}
						}
					}
				} else { // Simultaneous triggers
					++n_QSS_simultaneous_events;
					Variables triggers( events.top_subs< Variable >() );
					variables_observers( triggers, observers );
					size_type const iBeg_triggers_2( begin_order_index( triggers, 2 ) );
					int const triggers_order_max( triggers.back()->order() );

					for ( Variable * trigger : triggers ) {
						assert( trigger->tE == t );
						assert( ! trigger->is_ZC() ); // ZC variable requantizations are QSS_ZC events
						trigger->st = s; // Set trigger superdense time
						trigger->advance_QSS_0();
					}
					for ( Variable * trigger : triggers ) {
						trigger->advance_QSS_1();
					}
					if ( triggers_order_max >= 2 ) { // 2nd order pass
						fmu::set_time( t + options::dtNum ); // Set time to t + delta for numeric differentiation
						for ( size_type i = iBeg_triggers_2, n = triggers.size(); i < n; ++i ) {
							triggers[ i ]->advance_QSS_2();
						}
					}

					if ( ! observers.empty() ) { // Advance observers
						if ( triggers_order_max >= 2 ) fmu::set_time( t );
						Variable::advance_observers( observers, t );
					}

					if ( doROut ) { // Requantization output
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
							}
						} else { // Requantization and observer output
							if ( options::output::r ) { // Requantization output
								for ( Variable const * trigger : triggers ) { // Triggers
									size_type const i( var_idx[ trigger ] );
									if ( options::output::x ) x_outs[ i ].append( t, trigger->x( t ) );
									if ( options::output::q ) q_outs[ i ].append( t, trigger->q( t ) );
								}
								for ( Variable const * observer : observers ) { // Observer output
									size_type const i( var_idx[ observer ] );
									if ( options::output::x ) x_outs[ i ].append( t, observer->x( t ) );
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										if ( options::output::q ) q_outs[ i ].append( t, observer->q( t ) );
									}
								}
							}
						}
					}
				}
			} else if ( event.is_QSS_ZC() ) { // QSS ZC requantization event
				++n_QSS_events;
				Variable * trigger( event.sub< Variable >() );
				assert( trigger->tE == t );
				assert( trigger->is_ZC() ); // ZC variable requantizations are QSS_ZC events
				trigger->st = s; // Set trigger superdense time

				trigger->advance_QSS();

				if ( doROut ) { // Requantization output
					if ( options::output::a ) { // All variables output
						for ( size_type i = 0; i < n_vars; ++i ) {
							if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
							if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
						}
					} else { // Requantization output
						if ( options::output::r ) { // Requantization output
							size_type const i( var_idx[ trigger ] );
							if ( options::output::x ) x_outs[ i ].append( t, trigger->x( t ) );
							if ( options::output::q ) q_outs[ i ].append( t, trigger->q( t ) );
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
//			states[ i ] = state_vars[ i ]->x( t );
//		}
//		fmi2_import_set_continuous_states( fmu, states, n_states );
		fmi2_import_completed_integrator_step( fmu, fmi2_true, &callEventUpdate, &terminateSimulation );
		if ( eventInfo.terminateSimulation || terminateSimulation ) break;
	}

	// End time outputs
	if ( ( options::output::r || options::output::s ) && ( options::output::x || options::output::q ) ) {
		for ( size_type i = 0; i < n_vars; ++i ) {
			Variable const * var( vars[ i ] );
			if ( var->tQ < tE ) {
				if ( options::output::x ) {
					x_outs[ i ].append( tE, var->x( tE ) );
				}
				if ( options::output::q ) {
					q_outs[ i ].append( tE, var->q( tE ) );
				}
			}
		}
	}
	if ( options::output::f ) {
		if ( n_outs > 0u ) { // FMU QSS variable outputs
			for ( size_type i = 0; i < n_outs; ++i ) {
				Variable * var( outs[ i ] );
				f_outs[ i ].append( tE, var->x( tE ) );
			}
		}
		if ( n_fmu_outs > 0u ) { // FMU (non-QSS) variable outputs
			fmu::set_time( tE );
			for ( size_type i = 0; i < n_states; ++i ) {
				states[ i ] = state_vars[ i ]->x( tE );
			}
			fmi2_import_set_continuous_states( fmu, states, n_states );
			size_type i( n_outs );
			for ( auto const & e : fmu_outs ) {
				FMU_Variable const & var( e.second );
				f_outs[ i++ ].append( tE, fmu::get_real( var.ref ) );
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
	for ( auto & con : cons ) delete con;

	// FMU cleanup
	fmu::cleanup();
	fmi2_import_terminate( fmu );
	fmi2_import_free_instance( fmu );
	std::free( states );
	std::free( states_der );
	std::free( event_indicators );
	std::free( event_indicators_last );
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
