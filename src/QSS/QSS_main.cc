// QSS Solver Main Implementation
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2025 Objexx Engineering, Inc. All rights reserved.
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
#include <QSS/QSS_main.hh>
#include <QSS/options.hh>
#include <QSS/path.hh>
#include <QSS/version.hh>
#include <QSS/simulate_fmu_me.hh>
#include <QSS/simulate_fmu_me_con.hh>
#include <QSS/simulate_fmu_me_con_perfect.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>

namespace QSS {

// Model Type from Name
ModelType
model_type_of( std::string const & model )
{
	if ( model.rfind( ".fmu" ) == model.length() - 4u ) { // FMU
		if ( model.length() >= 5 ) { // FMU-ME
			return ModelType::FMU_ME;
		} else {
			std::cerr << "Error: FMU model file name invalid: " + model << std::endl;
			std::exit( EXIT_FAILURE );
		}
	} else {
		return ModelType::UNK;
	}
}

// QSS Main Implementation
void
QSS_main( std::vector< std::string > const & args )
{
	using namespace QSS;

	// I/o setup
	std::cout << std::setprecision( 16 ) << std::boolalpha;
	std::cerr << std::setprecision( 16 ) << std::boolalpha;
	std::clog << std::setprecision( 16 ) << std::boolalpha;

	// Startup banner
	std::cout << "LBNL/DOE  SOEP-QSS  Quantized State System Solver  Version: " << version() << '\n';

	// Process command line arguments
	options::process_args( args );
	if ( options::models.empty() ) {
		std::cerr << "Error: No model name or FMU file specified" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Check model names/types
	ModelType model_type( ModelType::UNK );
	if ( options::have_multiple_models() ) {

		// Check for mix of model types
		for ( std::string const & model : options::models ) {
			ModelType const model_type_loop( model_type_of( model ) );
			if ( model_type == ModelType::UNK ) {
				model_type = model_type_loop;
			} else if ( model_type != model_type_loop ) {
				std::cerr << "Error: Models must all FMU-ME" << std::endl;
				std::exit( EXIT_FAILURE );
			}
		}

		// Check for repeat model names
		options::Models sorted_models( options::models );
		std::sort( sorted_models.begin(), sorted_models.end() );
		if ( std::adjacent_find( sorted_models.begin(), sorted_models.end() ) != sorted_models.end() ) {
			std::cerr << "Error: Repeat model name" << std::endl;
			std::exit( EXIT_FAILURE );
		}

	} else { // Single model
		assert( options::models.size() == 1u );
		model_type = model_type_of( options::models[ 0 ] );
	}

	// Run FMU-ME model simulation
	if ( options::have_multiple_models() && options::connected() ) { // Synched simulations
		if ( model_type == ModelType::FMU_ME ) { // FMU-ME
			if ( options::perfect ) {
				simulate_fmu_me_con_perfect( options::models );
			} else {
				simulate_fmu_me_con( options::models );
			}
		} else {
			assert( false );
		}
	} else { // Independent simulations
		std::int64_t const n_models( options::models.size() );
		if ( model_type == ModelType::FMU_ME ) { // FMU-ME
			#pragma omp parallel for schedule(dynamic) if ( n_models > 1 )
			for ( std::int64_t i = 0; i < n_models; ++i ) {
				std::string const & model( options::models[ i ] );
				std::cout << '\n' + path::base( model ) + " Simulation =====" << std::endl;
				simulate_fmu_me( model );
			}
		} else {
			assert( false );
		}
	}
}

} // QSS
