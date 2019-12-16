// QSS Solver Main
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
#include <QSS/cod/simulate_cod.hh>
#ifdef QSS_FMU
#include <QSS/fmu/simulate_fmu_me.hh>
#include <QSS/fmu/simulate_fmu_me_con.hh>
#include <QSS/fmu/simulate_fmu_me_con_perfect.hh>
#include <QSS/fmu/simulate_fmu_qss.hh>
#include <QSS/fmu/simulate_fmu_qss_con.hh>
#endif
#include <QSS/options.hh>
#include <QSS/path.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>

// Types
enum class ModelType { UNK, COD, FMU_ME, FMU_QSS };

// Model Type from Name
ModelType
model_type_of( std::string const & model )
{
	if ( model.rfind( ".fmu" ) == model.length() - 4u ) { // FMU
		if ( ( model.length() >= 9 ) && ( model.rfind( "_QSS.fmu" ) == model.length() - 8u ) ) { // FMU-QSS
			return ModelType::FMU_QSS;
		} else if ( model.length() >= 5 ) { // FMU-ME
			return ModelType::FMU_ME;
		} else {
			std::cerr << "Error: FMU model file name invalid: " + model << std::endl;
			std::exit( EXIT_FAILURE );
		}
	} else { // Code-defined model
		return ModelType::COD;
	}
}

// QSS Solver Main
int
main( int argc, char * argv[] )
{
	using namespace QSS;

	// Process command line arguments
	options::process_args( argc, argv );
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
				std::cerr << "Error: Models must all be code-defined, FMU-ME, or FMU-QSS" << std::endl;
				std::exit( EXIT_FAILURE );
			}
		}

		// Check for repeat model names
		options::Models sorted_models( options::models );
		std::sort( sorted_models.begin(), sorted_models.end() );
		if ( std::adjacent_find( sorted_models.begin(), sorted_models.end() ) != sorted_models.end() )  {
			std::cerr << "Error: Repeat model name" << std::endl;
			std::exit( EXIT_FAILURE );
		}

	} else { // Single model
		assert( options::models.size() == 1u );
		model_type = model_type_of( options::models[ 0 ] );
	}

#ifndef QSS_FMU
	// Check for FMU models if not supported in this build
	if ( ( model_type == ModelType::FMU_ME ) || ( model_type == ModelType::FMU_QSS ) ) {
		std::cerr << "Error: FMU models not supported in this build" << std::endl;
		std::exit( EXIT_FAILURE );
	}
#endif

	// Run FMU-QSS, FMU-ME, or code-defined model simulation
	if ( options::have_multiple_models() && options::connected() ) { // Synched simulations
		if ( model_type == ModelType::COD ) { // Code-defined model
			std::cerr << "Error: Code-defined models with input:output connections not yet supported" << std::endl;
			std::exit( EXIT_FAILURE );
		} else if ( model_type == ModelType::FMU_ME ) { // FMU-ME
#ifdef QSS_FMU
			if ( options::perfect ) {
				fmu::simulate_fmu_me_con_perfect( options::models );
			} else {
				fmu::simulate_fmu_me_con( options::models );
			}
#endif
		} else if ( model_type == ModelType::FMU_QSS ) { // FMU-QSS
#ifdef QSS_FMU
			if ( options::perfect ) {
				std::cerr << "Error: Perfect sync only supported for FMU-ME, not FMU-QSS" << std::endl;
				std::exit( EXIT_FAILURE );
			}
			fmu::simulate_fmu_qss_con( options::models );
#endif
		} else {
			assert( false );
		}
	} else { // Independent simulations
		std::int64_t const n_models( options::models.size() );
		if ( model_type == ModelType::COD ) { // Code-defined model
			// Can parallelize if global event queue is eliminated
			for ( std::int64_t i = 0; i < n_models; ++i ) {
				std::string const & model( options::models[ i ] );
				std::cout << '\n' + path::base( model ) + " Simulation =====" << std::endl;
				cod::simulate( model );
			}
		} else if ( model_type == ModelType::FMU_ME ) { // FMU-ME
#ifdef QSS_FMU
			#pragma omp parallel for schedule(dynamic) if ( n_models > 1 )
			for ( std::int64_t i = 0; i < n_models; ++i ) {
				std::string const & model( options::models[ i ] );
				std::cout << '\n' + path::base( model ) + " Simulation =====" << std::endl;
				fmu::simulate_fmu_me( model );
			}
#endif
		} else if ( model_type == ModelType::FMU_QSS ) { // FMU-QSS
#ifdef QSS_FMU
			#pragma omp parallel for schedule(dynamic) if ( n_models > 1 )
			for ( std::int64_t i = 0; i < n_models; ++i ) {
				std::string const & model( options::models[ i ] );
				std::cout << '\n' + path::base( model ) + " Simulation =====" << std::endl;
				fmu::simulate_fmu_qss( model );
			}
#endif
		} else {
			assert( false );
		}
	}
}
