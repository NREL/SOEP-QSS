// Multiple FMU-QSS Simulation Runner
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
#include <QSS/fmu/simulate_fmu_qss_mul.hh>
#include <QSS/fmu/FMU_QSS.hh>
#include <QSS/fmu/FMI.hh>
#include <QSS/options.hh>
#include <QSS/string.hh>

// FMI Library Headers
#include <fmilib.h>
#include <FMI2/fmi2FunctionTypes.h>

// C++ Headers
#include <cstdlib>
#include <iostream>
#include <map>
#include <utility>

namespace QSS {
namespace fmu {

// Simulate multiple FMU-QSS with QSS
void
simulate_fmu_qss_mul( std::vector< std::string > const & paths )
{
	// Types
	using size_type = std::size_t;
	using Time = double;
	using Real = double;
	using FMU_QSSs = std::vector< FMU_QSS >;
	using Contexts = std::vector< fmi2Component >;

	// Locals
	size_type n_models( paths.size() );
	FMU_QSSs fmu_qsss;
	fmu_qsss.reserve( n_models );
	Contexts contexts;
	contexts.reserve( n_models );
	Time tStart( 0.0 );
	Time tEnd( 0.0 );

	// Instantiate models
	for ( size_type i = 0; i < n_models; ++i ) {
		std::string const & path( paths[ i ] );
		fmu_qsss.emplace_back( path );
		FMU_QSS & fmu_qss( fmu_qsss[ i ] );

		// Initialize the FMUs
		reg( fmi2_import_get_GUID( fmu_qss.fmu ), &fmu_qss );

		// Instantiation
		fmi2Component c( fmi2Instantiate( "FMU-QSS model instance", fmi2ModelExchange, fmi2_import_get_GUID( fmu_qss.fmu ), fmu_qss.fmuResourceLocation().c_str(), (fmi2CallbackFunctions*)&fmu_qss.callBackFunctions, 0, 0 ) );
		if ( c == nullptr ) {
			std::cerr << "\nError: fmi2Instantiate failed: " << std::endl;
			std::exit( EXIT_FAILURE );
		}
		reg( c, &fmu_qss );
		contexts.push_back( c );

		// Time initialization
		if ( i == 0 ) {
			tStart = fmi2_import_get_default_experiment_start( fmu_qss.fmu );
		} else {
			if ( tStart != fmi2_import_get_default_experiment_start( fmu_qss.fmu ) ) {
				std::cerr << "\nError: Start times of FMU-QSS differ" << std::endl;
				std::exit( EXIT_FAILURE );
			}
		}
		tEnd = std::max( tEnd, fmi2_import_get_default_experiment_stop( fmu_qss.fmu ) ); // Use max of specified end times
	}
	tEnd = ( options::specified::tEnd ? options::tEnd : tEnd );

	// Initialize models
	for ( size_type i = 0; i < n_models; ++i ) {
		fmi2Component c( contexts[ i ] );

		// Initialization
		if ( fmi2SetupExperiment( c, options::specified::rTol, options::rTol, tStart, options::specified::tEnd, tEnd ) != fmi2OK ) {
			std::cerr << "\nError: fmi2SetupExperiment failed: " << std::endl;
			std::exit( EXIT_FAILURE );
		}
		if ( fmi2EnterInitializationMode( c ) != fmi2OK ) {
			std::cerr << "\nError: fmi2EnterInitializationMode failed: " << std::endl;
			std::exit( EXIT_FAILURE );
		}
		if ( fmi2ExitInitializationMode( c ) != fmi2OK ) {
			std::cerr << "\nError: fmi2ExitInitializationMode failed: " << std::endl;
			std::exit( EXIT_FAILURE );
		}
	}

	// Connection setup
	using ModelRef = std::pair< size_type, fmi2_value_reference_t >;
	using ConMap = std::map< ModelRef, ModelRef >;
	ConMap con_map;
	for ( auto j = options::con.begin(), ej = options::con.end(); j != ej; ++j ) {
		std::string const & inp( j->first );
		std::string const & out( j->second );
		ModelRef inp_ref( 0u, 0u );
		ModelRef out_ref( 0u, 0u );
		bool inp_found( false );
		bool out_found( false );
		for ( size_type i = 0; i < n_models; ++i ) {
			FMU_QSS & fmu_qss( fmu_qsss[ i ] );
			FMU_ME & fmu_me( fmu_qss.fmu_me );
			std::string const & model( fmu_me.name );
			if ( has_prefix( inp, model + '.' ) ) {
				std::string const var_name( inp.substr( model.length() + 1u ) );
				auto const ivr( fmu_me.var_name_ref.find( var_name ) );
				if ( ivr == fmu_me.var_name_ref.end() ) {
					std::cerr << "\nError: Connection input variable not found: " << inp << std::endl;
					std::exit( EXIT_FAILURE );
				} else if ( inp_found ) {
					std::cerr << "\nError: Connection input variable spec is not unique: " << out << std::endl;
					std::exit( EXIT_FAILURE );
				} else {
					inp_found = true;
					inp_ref = ModelRef( i, ivr->second );
				}
			}
			if ( has_prefix( out, model + '.' ) ) {
				std::string const var_name( inp.substr( model.length() + 1u ) );
				auto const ivr( fmu_me.var_name_ref.find( var_name ) );
				if ( ivr == fmu_me.var_name_ref.end() ) {
					std::cerr << "\nError: Connection output variable not found: " << out << std::endl;
					std::exit( EXIT_FAILURE );
				} else if ( out_found ) {
					std::cerr << "\nError: Connection output variable spec is not unique: " << out << std::endl;
					std::exit( EXIT_FAILURE );
				} else {
					out_found = true;
					out_ref = ModelRef( i, ivr->second );
				}
			}
		}
		if ( inp_found && out_found ) {
			con_map[ inp_ref ] = out_ref;
		} else {
			if ( ! inp_found ) std::cerr << "\nError: Connection input variable not found: " << inp << std::endl;
			if ( ! out_found ) std::cerr << "\nError: Connection output variable not found: " << out << std::endl;
			std::exit( EXIT_FAILURE );
		}
	}
	for ( auto icon = con_map.begin(), icon_end = con_map.end(); icon != icon_end; ++icon ) { // Update the connection input variables
		ModelRef const & inp_ref( icon->first );
		ModelRef const & out_ref( icon->second );
		Real const out_val( fmu_qsss[ out_ref.first ].fmu_me.get_real( out_ref.second ) );
		fmu_qsss[ inp_ref.first ].fmu_me.set_real( inp_ref.second, out_val );
	}

	// Reinitialize models
	for ( size_type i = 0; i < n_models; ++i ) {
		FMU_ME & fmu_me( fmu_qsss[ i ].fmu_me );
		fmu_me.reinitialize();
	}

	// EventInfo setup
	std::vector< fmi2EventInfo > eventInfos;
	eventInfos.reserve( n_models );
	for ( size_type i = 0; i < n_models; ++i ) {
		fmi2EventInfo eventInfo;
		eventInfo.newDiscreteStatesNeeded = fmi2_true;
		eventInfo.terminateSimulation = fmi2_false;
		eventInfo.nominalsOfContinuousStatesChanged = fmi2_false;
		eventInfo.valuesOfContinuousStatesChanged = fmi2_false;
		eventInfo.nextEventTimeDefined = fmi2_false;
		eventInfo.nextEventTime = -0.0; // We are using this to signal time in/out of FMU-ME!!!
		eventInfos.push_back( eventInfo );
	}

	// Simulation loop
	for ( size_type i = 0; i < n_models; ++i ) {
		fmi2Component c( contexts[ i ] );
		fmi2EnterEventMode( c );
		fmi2EnterContinuousTimeMode( c );
	}
	Time const dt( 0.001 );
	Time time( tStart );
	Time tNext( tStart + dt );
	while ( time <= tEnd ) {
		for ( auto icon = con_map.begin(), icon_end = con_map.end(); icon != icon_end; ++icon ) { // Update the connection input variables
			ModelRef const & inp_ref( icon->first );
			ModelRef const & out_ref( icon->second );
			Real const out_val( fmu_qsss[ out_ref.first ].fmu_me.get_real( out_ref.second ) );
			fmu_qsss[ inp_ref.first ].fmu_me.set_real( inp_ref.second, out_val );
		}
		bool terminateSimulation( false );
		for ( size_type i = 0; i < n_models; ++i ) {
			Time t( time );
			while ( t < tNext ) {
				fmi2Component c( contexts[ i ] );
				fmi2EventInfo & eventInfo( eventInfos[ i ] );
				eventInfo.nextEventTimeDefined = fmi2_true;
				while ( ( eventInfo.newDiscreteStatesNeeded == fmi2_true ) && ( eventInfo.terminateSimulation == fmi2_false ) && ( eventInfo.nextEventTime < tNext ) ) {
					eventInfo.nextEventTime = tNext; // Signal QSS simulation pass when to stop
					if ( fmi2NewDiscreteStates( c, &eventInfo ) != fmi2OK ) {
						std::cerr << "\nError: fmi2NewDiscreteStates failed: " << std::endl;
						std::exit( EXIT_FAILURE );
					}
				}
				t = eventInfo.nextEventTime;
				if ( eventInfo.terminateSimulation ) {
					terminateSimulation = true;
					break;
				}
			}
		}
		if ( terminateSimulation ) break;
		time += dt;
		tNext += dt;
	}

	// Cleanup loop
	for ( size_type i = 0; i < n_models; ++i ) {
		FMU_QSS & fmu_qss( fmu_qsss[ i ] );
		fmi2Component c( contexts[ i ] );
		fmi2Terminate( c );
		fmi2FreeInstance( c );
		unreg( &fmu_qss );
	}
}

} // fmu
} // QSS
