// FMU-QSS Simulation Runner
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
#include <QSS/fmu/simulate_fmu_qss.hh>
#include <QSS/fmu/FMU_QSS.hh>
#include <QSS/fmu/FMI.hh>
#include <QSS/options.hh>

// FMI Library Headers
#include <fmilib.h>
#include <FMI2/fmi2FunctionTypes.h>

// C++ Headers
#include <cstdlib>
#include <iostream>

namespace QSS {
namespace fmu {

// Simulate an FMU-QSS with QSS
void
simulate_fmu_qss( std::string const & path )
{
	// Types
	using Time = double;

	// Initialize the FMUs
	FMU_QSS fmu_qss( path );
	reg( fmi2_import_get_GUID( fmu_qss.fmu ), &fmu_qss );

	// Instantiation
	fmi2Component c( fmi2Instantiate( "FMU-QSS model instance", fmi2ModelExchange, fmi2_import_get_GUID( fmu_qss.fmu ), fmu_qss.fmuResourceLocation().c_str(), (fmi2CallbackFunctions*)&fmu_qss.callBackFunctions, 0, 0 ) );
	if ( c == nullptr ) {
		std::cerr << "\nError: fmi2Instantiate failed: " << std::endl;
		std::exit( EXIT_FAILURE );
	}
	reg( c, &fmu_qss );

	// Time initialization
	Time const tStart( fmi2_import_get_default_experiment_start( fmu_qss.fmu ) );
	Time const tEnd( options::specified::tEnd ? options::tEnd : fmi2_import_get_default_experiment_stop( fmu_qss.fmu ) ); // No FMI API for getting stop time from FMU
	Time const tNext( tEnd ); // This can be a varying next step stop time to do output to another FMU
	Time time( tStart );

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

	// Simulation
	fmi2EventInfo eventInfo;
	eventInfo.newDiscreteStatesNeeded = fmi2_true;
	eventInfo.terminateSimulation = fmi2_false;
	eventInfo.nominalsOfContinuousStatesChanged = fmi2_false;
	eventInfo.valuesOfContinuousStatesChanged = fmi2_false;
	eventInfo.nextEventTimeDefined = fmi2_false;
	eventInfo.nextEventTime = -0.0; // We are using this to signal time in/out of FMU-ME!!!
	fmi2EnterEventMode( c );
	fmi2EnterContinuousTimeMode( c );
	eventInfo.nextEventTimeDefined = fmi2_true;
	while ( time <= tEnd ) {
		while ( time <= tNext ) {
			while ( ( eventInfo.newDiscreteStatesNeeded == fmi2_true ) && ( eventInfo.terminateSimulation == fmi2_false ) && ( eventInfo.nextEventTime < tNext ) ) {
				eventInfo.nextEventTime = tNext; // Signal QSS simulation pass when to stop
				if ( fmi2NewDiscreteStates( c, &eventInfo ) != fmi2OK ) {
					std::cerr << "\nError: fmi2NewDiscreteStates failed: " << std::endl;
					std::exit( EXIT_FAILURE );
				}
			}
			time = eventInfo.nextEventTime;
			if ( eventInfo.terminateSimulation ) break;
		}
	}
	fmi2Terminate( c );
	fmi2FreeInstance( c );
	unreg( &fmu_qss );
}

} // fmu
} // QSS
