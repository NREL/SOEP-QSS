// Connected FMU-ME Perfect Sync Simulation Runner
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
#include <QSS/fmu/simulate_fmu_me_con_perfect.hh>
#include <QSS/fmu/FMU_ME.hh>
#include <QSS/fmu/Variable_Con.hh>
#include <QSS/options.hh>
#include <QSS/string.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <utility>

namespace QSS {
namespace fmu {

// Simulate Connected FMU-ME with Perfect Sync
void
simulate_fmu_me_con_perfect( std::vector< std::string > const & paths )
{
	// Types
	using size_type = std::size_t;
	using Time = FMU_ME::Time;
	using FMU_MEs = std::vector< FMU_ME * >;

	// Locals
	size_type const n_models( paths.size() );
	assert( n_models > 1u );
	FMU_MEs fmu_mes;
	fmu_mes.reserve( n_models );
	Time tStart( 0.0 );
	Time tEnd( 0.0 );

	// Instantiate models
	for ( size_type i = 0; i < n_models; ++i ) {
		std::string const & path( paths[ i ] );
		fmu_mes.emplace_back( new FMU_ME( path ) );
		FMU_ME & fmu_me( *fmu_mes[ i ] );
		std::cout << "\n\nFMU-ME Instantiation: " << fmu_me.name << std::endl;
		fmu_me.instantiate();

		// Time initialization
		if ( i == 0 ) {
			tStart = fmu_me.t0;
		} else {
			if ( tStart != fmu_me.t0 ) {
				std::cerr << "\nError: Start times of FMU-ME differ" << std::endl;
				std::exit( EXIT_FAILURE );
			}
		}
		tEnd = std::max( tEnd, fmu_me.tE ); // Use max of specified end times

		// Pre-simulation setup
		fmu_me.pre_simulate();
	}

	// Set uniform end time
	tEnd = ( options::specified::tEnd ? options::tEnd : tEnd );
	for ( size_type i = 0; i < n_models; ++i ) {
		fmu_mes[ i ]->tE = tEnd;
	}

	// Connect model inputs to outputs
	std::cout << "\nConnection Setup =====" << std::endl;
	using ModelRef = std::pair< size_type, Variable * >;
	for ( auto j = options::con.begin(), ej = options::con.end(); j != ej; ++j ) {
		std::string const & inp( j->first );
		std::string const & out( j->second );
		ModelRef inp_ref( 0u, nullptr );
		ModelRef out_ref( 0u, nullptr );
		bool inp_found( false );
		bool out_found( false );
		for ( size_type i = 0; i < n_models; ++i ) {
			FMU_ME & fmu_me( *fmu_mes[ i ] );
			std::string const & model( fmu_me.name );
			if ( has_prefix( inp, model + '.' ) ) {
				std::string const var_name( inp.substr( model.length() + 1u ) );
				auto const ivr( fmu_me.var_name_var.find( var_name ) );
				if ( ivr == fmu_me.var_name_var.end() ) {
					std::cerr << "\nError: Connection input variable not found: " << inp << std::endl;
					std::exit( EXIT_FAILURE );
				} else if ( inp_found ) {
					std::cerr << "\nError: Connection input variable spec is not unique: " << inp << std::endl;
					std::exit( EXIT_FAILURE );
				} else {
					inp_found = true;
					inp_ref = ModelRef( i, ivr->second );
				}
			}
			if ( has_prefix( out, model + '.' ) ) {
				std::string const var_name( out.substr( model.length() + 1u ) );
				auto const ivr( fmu_me.var_name_var.find( var_name ) );
				if ( ivr == fmu_me.var_name_var.end() ) {
					std::cerr << "\nError: Connection output variable not found: " << out << std::endl;
					std::exit( EXIT_FAILURE );
				} else if ( out_found ) {
					std::cerr << "\nError: Connection output variable spec is not unique: " << out << std::endl;
					std::exit( EXIT_FAILURE );
				} else {
					out_found = true;
					out_ref = ModelRef( i, ivr->second );
					ivr->second->connected_output = true;
				}
			}
		}
		if ( inp_found && out_found ) {
			std::cerr << "Connection: " << fmu_mes[ inp_ref.first ]->name << '.' << inp_ref.second->name() << " <= " << fmu_mes[ out_ref.first ]->name << '.' << out_ref.second->name() << std::endl;
			Variable_Con * const inp_var( dynamic_cast< Variable_Con * >( inp_ref.second ) );
			if ( inp_var == nullptr ) {
				std::cerr << "\nError: Connection input variable is not a Modelica input variable: " << fmu_mes[ inp_ref.first ]->name << '.' << inp_ref.second->name() << std::endl;
				std::exit( EXIT_FAILURE );
			}
			if ( out_ref.second->is_ZC() ) { // Don't allow zero-crossing output connections to avoid processing order complexities
				std::cerr << "\nError: Connection output variable is a zero-crossing variable: " << inp_ref.second->name() << std::endl;
				std::exit( EXIT_FAILURE );
			}
			Variable * out_var( out_ref.second );
			out_var->connect();
			out_var->connections().push_back( inp_var );
			inp_var->out_var() = out_var; // Set connection variable's output variable
		} else {
			if ( ! inp_found ) std::cerr << "\nError: Connection input variable not found: " << inp << std::endl;
			if ( ! out_found ) std::cerr << "\nError: Connection output variable not found: " << out << std::endl;
			std::exit( EXIT_FAILURE );
		}
	}

	// Initialize models
	for ( size_type i = 0; i < n_models; ++i ) {
		fmu_mes[ i ]->init_0_1();
	}
	for ( size_type i = 0; i < n_models; ++i ) {
		fmu_mes[ i ]->init_0_2();
	}
	for ( size_type i = 0; i < n_models; ++i ) {
		fmu_mes[ i ]->init_1_1();
	}
	for ( size_type i = 0; i < n_models; ++i ) {
		fmu_mes[ i ]->init_1_2();
	}
	for ( size_type i = 0; i < n_models; ++i ) {
		fmu_mes[ i ]->init_2_1();
	}
	for ( size_type i = 0; i < n_models; ++i ) {
		fmu_mes[ i ]->init_2_2();
	}
	for ( size_type i = 0; i < n_models; ++i ) {
		fmu_mes[ i ]->init_3_1();
	}
	for ( size_type i = 0; i < n_models; ++i ) {
		fmu_mes[ i ]->init_F();
	}
	for ( size_type i = 0; i < n_models; ++i ) {
		fmu_mes[ i ]->init_ZC();
	}
	for ( size_type i = 0; i < n_models; ++i ) {
		fmu_mes[ i ]->init_pre_simulate();
	}

	// EventInfo setup
	std::vector< fmi2_event_info_t > eventInfos;
	eventInfos.reserve( n_models );
	for ( size_type i = 0; i < n_models; ++i ) {
		fmi2_event_info_t eventInfo;
		eventInfo.newDiscreteStatesNeeded = fmi2_true;
		eventInfo.terminateSimulation = fmi2_false;
		eventInfo.nominalsOfContinuousStatesChanged = fmi2_false;
		eventInfo.valuesOfContinuousStatesChanged = fmi2_false;
		eventInfo.nextEventTimeDefined = fmi2_false;
		eventInfo.nextEventTime = -0.0; // We are using this to signal time in/out of FMU-ME!!!
		eventInfos.push_back( eventInfo );
	}

	// Top event time initialization
	Time top_time( fmu_mes[ 0u ]->eventq->top_time() );
	size_type top_model( 0u );
	for ( size_type i = 1; i < n_models; ++i ) {
		Time const model_top_time( fmu_mes[ i ]->eventq->top_time() );
		if ( model_top_time < top_time ) {
			top_time = model_top_time;
			top_model = i;
		}
	}

	// Simulation loop
	Time time( tStart );
	while ( time <= tEnd ) {
		fmi2_event_info_t & eventInfo( eventInfos[ top_model ] );
		eventInfo.newDiscreteStatesNeeded = fmi2_true;
		eventInfo.nextEventTimeDefined = fmi2_false;
		fmu_mes[ top_model ]->simulate( &eventInfo, true );

		// Top event time update: Any model might have event queue changes due to connections
		top_time = fmu_mes[ 0u ]->eventq->top_time();
		top_model = 0u;
		for ( size_type i = 1; i < n_models; ++i ) {
			Time const model_top_time( fmu_mes[ i ]->eventq->top_time() );
			if ( model_top_time < top_time ) {
				top_time = model_top_time;
				top_model = i;
			}
		}
		time = top_time;
	}

	// Post-simulate
	for ( size_type i = 0; i < n_models; ++i ) {
		fmu_mes[ i ]->post_simulate();
	}

	// Cleanup
	for ( size_type i = 0; i < n_models; ++i ) {
		delete fmu_mes[ i ];
	}
}

} // fmu
} // QSS
