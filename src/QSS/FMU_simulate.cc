// QSS FMU Simulation Runner
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/FMU_simulate.hh>
#include <QSS/FMU.hh>
#include <QSS/FMU_Variable.hh>
#include <QSS/globals.hh>
#include <QSS/math.hh>
#include <QSS/options.hh>
#include <QSS/Variable_FMU_QSS1.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

// FMI Library Headers
#include <fmilib.h>

namespace FMU {

// Forward
void
do_event_iteration( fmi2_import_t * fmu, fmi2_event_info_t * eventInfo );

// Simulate an FMU Model
void
simulate()
{
	// Types
	using size_type = std::size_t;
	using Time = Variable::Time;
	using Value = Variable::Value;
	using Variable_QSS = Variable_FMU_QSS1; // Default QSS variable type to use for FMU variables (later XML annotation can override)
	using FMU_Vars = std::unordered_map< fmi2_import_real_variable_t *, FMU_Variable >; // Map from FMU variables to FMU_Variable objects
	using FMU_Idxs = std::unordered_map< size_type, Variable_QSS * >; // Map from FMU variable indexes to QSS Variables
	using QSS_Vars = std::unordered_map< Variable *, size_type >; // Map from QSS variables to their indexes

	// I/o setup
	std::cout << std::setprecision( 16 );
	std::cerr << std::setprecision( 16 );
	std::vector< std::ofstream > x_streams; // Continuous output streams
	std::vector< std::ofstream > q_streams; // Quantized output streams
	std::vector< std::ofstream > f_streams; // FMU output streams

	// Controls
	int const QSS_order_max( 3 ); // Highest QSS order in use or 3 to handle all supported orders

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
	FMU::fmu = fmu;
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

	size_type const n_states( fmi2_import_get_number_of_continuous_states( fmu ) );
	std::cout << n_states << " continuous variables" << std::endl;
	size_type const n_event_indicators( fmi2_import_get_number_of_event_indicators( fmu ) );
	std::cout << n_event_indicators << " event indicators" << std::endl;

	fmi2_real_t * const states( (fmi2_real_t*)std::calloc( n_states, sizeof( double ) ) );
	fmi2_real_t * const states_der( (fmi2_real_t*)std::calloc( n_states, sizeof( double ) ) );
	fmi2_real_t * const event_indicators( (fmi2_real_t*)std::calloc( n_event_indicators, sizeof( double ) ) );
	fmi2_real_t * const event_indicators_prev( (fmi2_real_t*)std::calloc( n_event_indicators, sizeof( double ) ) );

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
	std::cout << "\nRelative Tolerance: " << relativeTolerance << std::endl;
	fmi2_boolean_t callEventUpdate( fmi2_false );
	fmi2_boolean_t terminateSimulation( fmi2_false );
	fmi2_boolean_t const toleranceControlled( fmi2_true );
	fmi2_boolean_t const stopTimeDefined( fmi2_true );
	fmi2_import_setup_experiment( fmu, toleranceControlled, relativeTolerance, tstart, stopTimeDefined, tstop );

	// QSS time and tolerance run controls
	Time const t0( tstart ); // Simulation start time
	Time const tE( options::tEnd_set ? options::tEnd : tstop ); // Simulation end time
	Time t( t0 ); // Simulation current time
	Time tOut( t0 + options::dtOut ); // Sampling time
	size_type iOut( 1u ); // Output step index
	if ( !options::rTol_set ) options::rTol = relativeTolerance; // Quantization relative tolerance (FMU doesn't have an absolute tolerance)

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

	// Process FMU variables
	FMU_Vars fmu_vars;
	FMU_Vars fmu_outs;
	fmi2_import_variable_list_t * var_list( fmi2_import_get_variable_list( fmu, 0 ) ); // sort order = 0 for original order
	size_type const n_fmu_vars( fmi2_import_get_variable_list_size( var_list ) );
	std::cout << "\nNum FMU Variables: " << n_fmu_vars << std::endl;
	fmi2_value_reference_t const * vrs( fmi2_import_get_value_referece_list( var_list ) );
	for ( size_type i = 0; i < n_fmu_vars; ++i ) {
		std::cout << "\nVariable  Index: " << i+1 << " Ref: " << vrs[ i ] << std::endl;
		fmi2_import_variable_t * var( fmi2_import_get_variable( var_list, i ) );
		std::cout << " Name: " << fmi2_import_get_variable_name( var ) << std::endl;
		std::cout << " Desc: " << ( fmi2_import_get_variable_description( var ) ? fmi2_import_get_variable_description( var ) : "" ) << std::endl;
		std::cout << " Ref: " << fmi2_import_get_variable_vr( var ) << std::endl;
		bool const var_start( fmi2_import_get_variable_has_start( var ) == 1 );
		std::cout << " Start? " << var_start << std::endl;
		fmi2_base_type_enu_t var_base_type( fmi2_import_get_variable_base_type( var ) );
		switch ( var_base_type ) {
		case fmi2_base_type_real:
			std::cout << " Type: Real" << std::endl;
			{
			fmi2_import_real_variable_t * var_real( fmi2_import_get_variable_as_real( var ) );
			if ( var_start ) std::cout << " Start: " << fmi2_import_get_real_variable_start( var_real ) << std::endl;
			if ( fmi2_import_get_variability( var ) == fmi2_variability_enu_continuous ) {
				fmu_vars[ var_real ] = FMU_Variable( var, var_real, fmi2_import_get_variable_vr( var ), i+1 );
			}
			if ( fmi2_import_get_causality( var ) == fmi2_causality_enu_output ) {
				fmu_outs[ var_real ] = FMU_Variable( var, var_real, fmi2_import_get_variable_vr( var ), i+1 );
			}
			}
			break;
		case fmi2_base_type_int:
			std::cout << " Type: Integer" << std::endl;
			if ( var_start ) std::cout << " Start: " << fmi2_import_get_integer_variable_start( fmi2_import_get_variable_as_integer( var ) ) << std::endl;
			break;
		case fmi2_base_type_bool:
			std::cout << " Type: Boolean" << std::endl;
			if ( var_start ) std::cout << " Start: " << fmi2_import_get_boolean_variable_start( fmi2_import_get_variable_as_boolean( var ) ) << std::endl;
			break;
		case fmi2_base_type_str:
			std::cout << " Type: String" << std::endl;
			if ( var_start ) std::cout << " Start: " << fmi2_import_get_string_variable_start( fmi2_import_get_variable_as_string( var ) ) << std::endl;
			break;
		case fmi2_base_type_enum:
			std::cout << " Type: Enum" << std::endl;
			if ( var_start ) std::cout << " Start: " << fmi2_import_get_enum_variable_start( fmi2_import_get_variable_as_enum( var ) ) << std::endl;
			break;
		default:
			std::cout << " Type: Unknown" << std::endl;
			break;
		}
		fmi2_variability_enu_t const var_variability( fmi2_import_get_variability( var ) );
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
		fmi2_causality_enu_t const var_causality( fmi2_import_get_causality( var ) );
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
	Variable_FMU::Variables_FMU vars; // QSS variables collection
	Variable_FMU::Variables_FMU outs; // FMU output QSS variables collection
	vars.reserve( n_states );
	FMU_Vars fmu_ders; // FMU variable to derivative map
	FMU_Vars fmu_dvrs; // FMU derivative to variable map
	FMU_Idxs fmu_idxs;
	QSS_Vars qss_vars; // Map from QSS variables to their indexes
	fmi2_import_variable_list_t * der_list( fmi2_import_get_derivatives_list( fmu ) );
	size_type const n_ders( fmi2_import_get_variable_list_size( der_list ) );
	std::cout << "\nNum FMU Derivatives: " << n_ders << std::endl;
	fmi2_value_reference_t const * drs( fmi2_import_get_value_referece_list( der_list ) );
	for ( size_type i = 0, ics = 0; i < n_ders; ++i ) {
		std::cout << "\nDerivative  Ref: " << drs[ i ] << std::endl;
		fmi2_import_variable_t * der( fmi2_import_get_variable( der_list, i ) );
		std::cout << " Name: " << fmi2_import_get_variable_name( der ) << std::endl;
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
				std::cout << " Initial value of " << fmi2_import_get_variable_name( fmu_var.var ) << " = " << states_initial << std::endl;
				bool const start( fmi2_import_get_variable_has_start( fmu_var.var ) == 1 );
				if ( start ) {
					Value const var_initial( fmi2_import_get_real_variable_start( var_real ) );
					if ( var_initial != states_initial ) {
						std::cerr << "Warning: Initial value from xml specs: " << var_initial << " is not equal to initial value from fmi2GetContinuousStates(): " << states_initial << std::endl;
						std::cerr << "         Using initial value from fmi2GetContinuousStates()" << std::endl;
					}
				}
				Variable_QSS * qss_var( new Variable_QSS( fmi2_import_get_variable_name( fmu_var.var ), options::rTol, options::aTol, states_initial, fmu_var, fmu_der ) ); // Create QSS variable
				qss_vars[ qss_var ] = ics - 1;
				vars.push_back( qss_var ); // Add to QSS variables
				if ( fmi2_import_get_causality( fmu_var.var ) == fmi2_causality_enu_output ) { // Add to FMU QSS variable outputs
					outs.push_back( qss_var );
					fmu_outs.erase( fmu_var.rvr ); // Remove it from non-QSS FMU outputs
				}
				fmu_idxs[ fmu_var.idx ] = qss_var; // Add to map from FMU variable index to QSS variable
				std::cout << " FMU idx: " << fmu_var.idx << " maps to QSS var: " << qss_var->name << std::endl;
			} else {
				std::cerr << "Error: Derivative missing associated Variable: " << fmi2_import_get_variable_name( der ) << std::endl;
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
	if ( vars.size() != n_states ) {
		std::cout << "Warning: Number of QSS variables " << vars.size() << " is not equal to the number of FMU continuous states " << n_states << std::endl;
	}

	// QSS observer setup
	size_type * startIndex( nullptr );
	size_type * dependency( nullptr );
	char * factorKind( nullptr );
	fmi2_import_get_derivatives_dependencies( fmu, &startIndex, &dependency, &factorKind );
	if ( startIndex != nullptr ) { // Dependency info present in XML
		for ( size_type i = 0; i < n_ders; ++i ) {
			std::cout << "\nDerivative  Ref: " << drs[ i ] << std::endl;
			fmi2_import_variable_t * der( fmi2_import_get_variable( der_list, i ) );
			std::cout << " Name: " << fmi2_import_get_variable_name( der ) << std::endl;
			fmi2_import_real_variable_t * der_real( fmi2_import_get_variable_as_real( der ) );
			size_type const idx( fmu_dvrs[ der_real ].idx );
			std::cout << " Var Index: " << idx << std::endl;
			Variable_QSS * var( fmu_idxs[ idx ] );
			std::cout << " QSS Variable: " << var->name << std::endl;
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
					Variable_QSS * dep( idep->second );
					std::cout << "  QSS var: " << dep->name << " has observer " << var->name << std::endl;
					if ( dep == var ) {
						var->self_observer = true;
					} else {
						dep->add_observer( var );
						var->add_observee( dep );
					}
				} else {
					//std::cout << "FMU derivative " << fmi2_import_get_variable_name( der ) << " has dependency with index " << dep_idx << " that is not a QSS variable" << std::endl;
				}
			}
		}
	} else { // Assume no observers in model (this may not be true: FMI spec says no dependencies => dependent on all)
		std::cout << "No dependency info in FMU XML" << std::endl;
	}

	// Solver master logic
	fmi2_import_set_time( fmu, t0 );
	FMU::init_derivatives( n_ders );
	for ( auto var : vars ) {
		var->init1_LIQSS();
	}
	for ( auto var : vars ) {
		var->init1();
	}
	FMU::get_derivatives();
	for ( auto var : vars ) {
		var->init1_fmu();
	}
	if ( QSS_order_max >= 2 ) {
		for ( auto var : vars ) {
			var->init2_LIQSS();
		}
		for ( auto var : vars ) {
			var->init2();
		}
		if ( QSS_order_max >= 3 ) {
			for ( auto var : vars ) {
				var->init3();
			}
		}
	}
	for ( auto var : vars ) {
		var->init_event();
	}
	size_type const n_vars( vars.size() );
	size_type const n_outs( outs.size() );
	size_type const n_fmu_outs( fmu_outs.size() );
	bool const doSOut( ( options::output::s && ( options::output::x || options::output::q ) ) || ( options::output::f && ( n_outs + n_fmu_outs > 0u ) ) );
	bool const doROut( options::output::r && ( options::output::x || options::output::q ) );
	size_type n_requant_events( 0 );
	if ( ( options::output::r || options::output::s ) && ( options::output::x || options::output::q ) ) { // t0 QSS outputs
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
			f_streams.back() << std::setprecision( 16 ) << t << '\t' << FMU::get_real( var.ref ) << '\n';
		}
	}
	while ( t <= tE ) {
		t = events.top_time();
		if ( doSOut ) { // Sampled and/or FMU outputs
			Time const tStop( std::min( t, tE ) );
			while ( tOut < tStop ) {
				//std::cout << "Output time: " << tOut << std::endl;/////////////////////////////////////
				if ( options::output::s ) { // QSS variable outputs
					for ( size_type i = 0; i < n_vars; ++i ) {
						if ( options::output::x ) x_streams[ i ] << tOut << '\t' << vars[ i ]->x( tOut ) << '\n';
						if ( options::output::q ) q_streams[ i ] << tOut << '\t' << vars[ i ]->q( tOut ) << '\n';
					}
				}
				if ( options::output::f ) {
					if ( n_outs > 0u ) { // FMU QSS variable outputs
						for ( size_type i = 0; i < n_outs; ++i ) {
							Variable * var( outs[ i ] );
							f_streams[ i ] << tOut << '\t' << var->x( tOut ) << '\n';
						}
					}
					if ( n_fmu_outs > 0u ) { // FMU (non-QSS) variable outputs
						fmi2_import_set_time( fmu, tOut );
						for ( size_type i = 0; i < n_states; ++i ) {
							states[ i ] = vars[ i ]->x( tOut );
						}
						fmi2_import_set_continuous_states( fmu, states, n_states );
						size_type i( n_outs );
						for ( auto const & e : fmu_outs ) {
							FMU_Variable const & var( e.second );
							f_streams[ i++ ] << tOut << '\t' << FMU::get_real( var.ref ) << '\n';
						}
					}
				}
				assert( iOut < std::numeric_limits< size_type >::max() );
				tOut = t0 + ( ++iOut ) * options::dtOut;
			}
		}
		if ( t <= tE ) { // Perform event
			//std::cout << "Event time: " << t << std::endl;/////////////////////////////////////
			++n_requant_events;
			fmi2_import_set_time( fmu, t );
			if ( events.simultaneous() ) { // Simultaneous trigger
				if ( options::output::d ) std::cout << "Simultaneous trigger event at t = " << t << std::endl;
				EventQueue< Variable >::Variables triggers( events.simultaneous_variables() ); // Chg to generator approach to avoid heap hit // Sort/ptn by QSS order to save unnec loops/calls below
				for ( Variable * trigger : triggers ) {
					assert( trigger->tE == t );
					trigger->advance0();
				}
				for ( Variable * trigger : triggers ) {
					trigger->advance_fmu();
				}
				FMU::get_derivatives();
				for ( Variable * trigger : triggers ) {
					trigger->advance1_LIQSS();
				}
				for ( Variable * trigger : triggers ) {
					trigger->advance1();
				}
				if ( QSS_order_max >= 2 ) {
					for ( Variable * trigger : triggers ) {
						trigger->advance2_LIQSS();
					}
					for ( Variable * trigger : triggers ) {
						trigger->advance2();
					}
					if ( QSS_order_max >= 3 ) {
						for ( Variable * trigger : triggers ) {
							trigger->advance3();
						}
					}
				}
				for ( Variable * trigger : triggers ) {
					trigger->advance_observers();
				}
				if ( doROut ) { // Requantization output
					for ( Variable * trigger : triggers ) {
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
								if ( options::output::q ) q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
							}
						} else { // Trigger variable output
							size_type const i( qss_vars[ trigger ] );
							if ( options::output::x ) x_streams[ i ] << t << '\t' << trigger->x( t ) << '\n';
							if ( options::output::q ) q_streams[ i ] << t << '\t' << trigger->q( t ) << '\n';
						}
					}
				}
			} else { // Single trigger
				Variable * trigger( events.top() );
				assert( trigger->tE == t );
				trigger->advance();
				if ( doROut ) { // Requantization output
					if ( options::output::a ) { // All variables output
						for ( size_type i = 0; i < n_vars; ++i ) {
							if ( options::output::x ) x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
							if ( options::output::q ) q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
						}
					} else { // Trigger variable output
						size_type const i( qss_vars[ trigger ] );
						if ( options::output::x ) x_streams[ i ] << t << '\t' << trigger->x( t ) << '\n';
						if ( options::output::q ) q_streams[ i ] << t << '\t' << trigger->q( t ) << '\n';
					}
				}
			}
		}

		// FMU end of step processing
// Not sure we need to set continuous states: It would be a performance hit
//		fmi2_import_set_time( fmu, t );
//		for ( size_type i = 0; i < n_states; ++i ) {
//			states[ i ] = vars[ i ]->x( t );
//		}
//		fmi2_import_set_continuous_states( fmu, states, n_states );
		fmi2_import_completed_integrator_step( fmu, fmi2_true, &callEventUpdate, &terminateSimulation );
		if ( eventInfo.terminateSimulation || terminateSimulation ) break;
	}

	// tE QSS outputs and streams close
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
			fmi2_import_set_time( fmu, tE );
			for ( size_type i = 0; i < n_states; ++i ) {
				states[ i ] = vars[ i ]->x( tE );
			}
			fmi2_import_set_continuous_states( fmu, states, n_states );
			size_type i( n_outs );
			for ( auto const & e : fmu_outs ) {
				FMU_Variable const & var( e.second );
				f_streams[ i ] << tE << '\t' << FMU::get_real( var.ref ) << '\n';
				f_streams[ i++ ].close();
			}
		}
	}

	// Reporting
	std::cout << "Simulation complete" << std::endl;
	std::cout << n_requant_events << " total requantization events occurred" << std::endl;

	// QSS cleanup
	for ( auto & var : vars ) delete var;
	FMU::cleanup();

	// FMI Library cleanup
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

} // FMU
