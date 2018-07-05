// FMU-QSS FMI 2.0 API Wrappers
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
#include <QSS/fmu/FMI.hh>
#include <QSS/fmu/FMU_ME.hh>
#include <QSS/fmu/FMU_QSS.hh>
#include <QSS/fmu/FMU_QSS_GUID_ok.hh>
#include <QSS/fmu/FMU_QSS_defines.hh>
#include <QSS/options.hh> // Before FMU_QSS_options.hh
#include <QSS/fmu/FMU_QSS_options.hh>

// FMIL Headers
#include <FMI2/fmi2_import_impl.h>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <iostream>

// Types
using namespace QSS;
using namespace QSS::fmu;

namespace {

struct component_t
{
	/*************** FMI ME 2.0 ****************/
	fmi2Real states [1]; // Actually 0
	fmi2Real states_nom [1]; // Actually 0
	fmi2Real states_der [1]; // Actually 0
	fmi2Real event_indicators[1]; // Actually 0
	fmi2Real reals [N_REAL];
	fmi2Integer integers [N_INTEGER];
	fmi2Boolean booleans [N_BOOLEAN];
	fmi2String strings [N_STRING];

	/* fmiInstantiateModel */
	fmi2Boolean loggingOn;
	char instanceName	[BUFFER];
	char GUID [BUFFER];
	fmi2CallbackFunctions const * functions;

	/* fmiSetTime */
	fmi2Real fmitime;

	/* fmiInitializeModel */
	fmi2Boolean toleranceControlled;
	fmi2Real relativeTolerance;
	fmi2EventInfo eventInfo;

	/*************** FMI CS 2.0. Depends on the ME fields above and functions ****************/
	fmi2Real states_prev [1]; // Actually 0

	/* fmiInstantiateSlave */
	char fmuLocation [BUFFER];
	fmi2Boolean visible;

	/* fmiInitializeSlave */
	fmi2Real tStart;
	fmi2Boolean StopTimeDefined;
	fmi2Real tStop;

	/* fmiSetRealInputDerivatives */
	fmi2Real input_real [N_INPUT_REAL][N_INPUT_REAL_MAX_ORDER + 1];

	/* fmiGetRealOutputDerivatives */
	fmi2Real output_real [N_OUTPUT_REAL][N_OUTPUT_REAL_MAX_ORDER + 1];
};
using component_ptr_t = component_t *;

} // <unnamed>

inline
void
simulate()
{
	fmi2Component c( fmu_qss.fmu->capi->c );
	fmi2Real const fmu_time( ((component_ptr_t)(fmu_me.fmu->capi->c))->fmitime );
	if ( fmu_me.t < fmu_time ) { // Advance simulation to FMU time
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
		Time time( fmu_me.t );
		Time const tNext( fmu_time ); // This can be a varying next step stop time to do output to another FMU
		while ( time <= fmu_time ) {
			while ( time <= fmu_time ) {
				while ( ( eventInfo.newDiscreteStatesNeeded == fmi2_true ) && ( eventInfo.terminateSimulation == fmi2_false ) && ( eventInfo.nextEventTime < tNext ) ) {
					eventInfo.nextEventTime = tNext; // Signal QSS simulation pass when to stop
					fmu_me.simulate( (fmi2_event_info_t *)(&eventInfo) );
					if ( ( fmu_me.t >= fmu_me.tE ) || ( eventInfo.terminateSimulation ) ) {
						eventInfo.terminateSimulation = fmi2_true;
						eventInfo.newDiscreteStatesNeeded = fmi2_false;
						fmu_me.post_simulate();
					}
				}
				time = eventInfo.nextEventTime;
				if ( eventInfo.terminateSimulation ) break;
			}
		}
	}
}

char const *
fmi2GetVersion()
{
	return "2.0";
}

char const *
fmi2GetTypesPlatform()
{
	return fmi2_get_types_platform();
}

fmi2Component
fmu_qss_fmi_instantiate(
 fmi2String instanceName,
 fmi2Type fmuType, // Unused
 fmi2String fmuGUID,
 fmi2String fmuLocation,
 fmi2CallbackFunctions const * functions,
 fmi2Boolean visible,
 fmi2Boolean loggingOn
)
{
	component_ptr_t comp( (component_ptr_t)functions->allocateMemory( 1, sizeof( component_t ) ) );
	if ( comp == nullptr ) return nullptr;
	std::snprintf( comp->instanceName, BUFFER, "%s", instanceName );
	std::snprintf( comp->GUID, BUFFER, "%s", fmuGUID );
	std::snprintf( comp->fmuLocation, BUFFER, "%s", fmuLocation );
	comp->functions = functions;
	comp->loggingOn = loggingOn;
	comp->visible = visible;
	for ( int k = 0; k < N_REAL; ++k ) comp->reals[k] = 0.0;
	for ( int k = 0; k < N_INTEGER; ++k ) comp->integers[k] = 0;
	for ( int k = 0; k < N_BOOLEAN; ++k ) comp->booleans[k] = fmi2False;
	for ( int k = 0; k < N_STRING; ++k ) comp->strings[k] = NULL;
	return comp;
}

fmi2Component
fmi2Instantiate(
 fmi2String instanceName,
 fmi2Type fmuType,
 fmi2String GUID,
 fmi2String fmuResourceLocation,
 fmi2CallbackFunctions const * functions,
 fmi2Boolean visible,
 fmi2Boolean loggingOn
)
{
	if ( fmu_qss.fmu == nullptr ) { // 3rd-party master algorithm
		assert( fmuResourceLocation != nullptr );
		fmu_qss.pre_instantiate( FMU_QSS::unzip_loc( fmuResourceLocation ) );
	}
	if ( ! FMU_QSS_GUID_ok( GUID ) ) {
		std::cerr << "\nError: Incorrect GUID found in FMU-QSS modelDescription.xml" << std::endl;
		std::exit( EXIT_FAILURE );
	}
	fmu_qss.instantiate();
	fmi2Component c( fmu_qss.fmu->capi->c = fmu_qss_fmi_instantiate( instanceName, fmuType, GUID, fmuResourceLocation, functions, visible, loggingOn ) );
	if ( c == nullptr ) {
		std::cerr << "\nError: FMU-QSS instantiate failed" << std::endl;
		std::exit( EXIT_FAILURE );
	}
	if ( fmi2_import_instantiate( fmu_me.fmu, "FMU-ME model instance", (fmi2_type_t)fmuType, 0, visible ) == jm_status_error ) {
		std::cerr << "\nError: FMU-ME fmi2_import_instantiate failed" << std::endl;
		std::exit( EXIT_FAILURE );
	}
	fmi2_import_set_debug_logging( fmu_me.fmu, loggingOn, 0, 0 );
	return c;
}

fmi2Status
fmi2SetDebugLogging(
 fmi2Component c,
 fmi2Boolean loggingOn,
 size_t n ,
 fmi2String const cat[]
)
{
	assert( c == fmu_qss.fmu->capi->c );
	if ( c == nullptr ) {
		return fmi2Fatal;
	} else {
		((component_ptr_t)c)->loggingOn = loggingOn;
	}
	return (fmi2Status)fmi2_import_set_debug_logging( fmu_me.fmu, loggingOn, n, (fmi2String *)cat );
}

fmi2Status
fmi2SetupExperiment(
 fmi2Component c,
 fmi2Boolean toleranceDefined,
 fmi2Real tolerance,
 fmi2Real startTime,
 fmi2Boolean stopTimeDefined,
 fmi2Real stopTime
)
{
	assert( c == fmu_qss.fmu->capi->c );

	// Merge any command line options specified with FMU-QSS QSS options
	if ( ! options::specified::qss ) {
		options::qss = fmu_qss_qss;
		options::specified::qss = true;
	}
	if ( toleranceDefined ) {
		options::rTol = tolerance;
		options::specified::rTol = true;
	} else if ( ( ! options::specified::rTol ) && ( fmu_qss_rTol >= 0.0 ) ) {
		options::rTol = fmu_qss_rTol;
		options::specified::rTol = true;
		toleranceDefined = true;
	}
	if ( ! options::specified::aTol ) {
		options::aTol = fmu_qss_aTol;
		options::specified::aTol = true;
	}
	if ( stopTimeDefined ) {
		options::tEnd = stopTime;
		options::specified::tEnd = true;
	} else if ( ( ! options::specified::tEnd ) && ( fmu_qss_tEnd >= 0.0 ) ) {
		options::tEnd = fmu_qss_tEnd;
		options::specified::tEnd = true;
		stopTimeDefined = true;
	}

	// FMU-ME setup
	fmi2_import_t * fmu( fmu_me.fmu );
	fmi2_real_t const tstart( fmi2_import_get_default_experiment_start( fmu ) ); // [0.0]
	assert( startTime == tstart ); // No control for overriding this
	fmi2_real_t const tstop( stopTimeDefined ? stopTime : fmi2_import_get_default_experiment_stop( fmu ) ); // [1.0]
	std::cout << "\nSimulation Time Range (s):  Start: " << tstart << "  Stop: " << tstop << std::endl;
	fmi2_real_t const rTolerance( toleranceDefined ? tolerance : fmi2_import_get_default_experiment_tolerance( fmu ) ); // [0.0001]
	fmu_me.set_options( tstart, tstop, rTolerance );
	return (fmi2Status)fmi2_import_setup_experiment( fmu, toleranceDefined, tolerance, startTime, stopTimeDefined, stopTime );
}

fmi2Status
fmi2EnterInitializationMode( fmi2Component c )
{
	assert( c == fmu_qss.fmu->capi->c );
	return (fmi2Status)fmi2_import_enter_initialization_mode( fmu_me.fmu );
}

fmi2Status
fmi2ExitInitializationMode( fmi2Component c )
{
	assert( c == fmu_qss.fmu->capi->c );
	fmi2Status const fmi_status( (fmi2Status)fmi2_import_exit_initialization_mode( fmu_me.fmu ) );
	fmu_qss.pre_simulate();
	fmu_me.pre_simulate( fmu_qss.out_var_refs );
	return fmi_status;
}

fmi2Status
fmi2EnterEventMode( fmi2Component c )
{
	assert( c == fmu_qss.fmu->capi->c );
	return (fmi2Status)fmi2_import_enter_event_mode( fmu_me.fmu );
}

fmi2Status
fmi2EnterContinuousTimeMode( fmi2Component c )
{
	assert( c == fmu_qss.fmu->capi->c );
	fmi2_import_enter_event_mode( fmu_me.fmu ); //? Keep FMUChecker happy
	return (fmi2Status)fmi2_import_enter_continuous_time_mode( fmu_me.fmu );
}

fmi2Status
fmi2NewDiscreteStates(
 fmi2Component c,
 fmi2EventInfo * eventInfo
)
{
	assert( c == fmu_qss.fmu->capi->c );
	fmu_me.simulate( (fmi2_event_info_t *)eventInfo );
	if ( ( fmu_me.t >= fmu_me.tE ) || ( eventInfo->terminateSimulation ) ) {
		eventInfo->terminateSimulation = fmi2_true;
		eventInfo->newDiscreteStatesNeeded = fmi2_false;
		fmu_me.post_simulate();
	}
	((component_ptr_t)c)->fmitime = fmu_me.t;
	return fmi2OK;
}

fmi2Status
fmi2SetRealInputDerivatives(
 fmi2Component c,
 fmi2ValueReference const vr[],
 size_t nvr,
 fmi2Integer const order[],
 fmi2Real const value[]
)
{
	assert( c == fmu_qss.fmu->capi->c );
	return (fmi2Status)fmi2_import_set_real_input_derivatives( fmu_me.fmu, vr, nvr, order, value );
}

fmi2Status
fmi2GetRealOutputDerivatives(
 fmi2Component c,
 fmi2ValueReference const vr[],
 size_t nvr,
 fmi2Integer const order[],
 fmi2Real value[]
)
{
	assert( c == fmu_qss.fmu->capi->c );
	return (fmi2Status)fmi2_import_get_real_output_derivatives( fmu_me.fmu, vr, nvr, order, value );
}

fmi2Status
fmi2SetTime(
 fmi2Component c,
 fmi2Real fmitime
)
{
	assert( c == fmu_qss.fmu->capi->c );
	((component_ptr_t)c)->fmitime = fmitime;
	return (fmi2Status)fmi2_import_set_time( fmu_me.fmu, fmitime );
}

fmi2Status
fmi2GetReal(
 fmi2Component c,
 fmi2ValueReference const vr[],
 size_t nvr,
 fmi2Real value[]
)
{
	assert( c == fmu_qss.fmu->capi->c );
	simulate(); // Advance simulation to FMU time
	return (fmi2Status)fmi2_import_get_real( fmu_me.fmu, vr, nvr, value );
}

fmi2Status
fmi2SetReal(
 fmi2Component c,
 fmi2ValueReference const vr[],
 size_t nvr,
 fmi2Real const value[]
)
{
	assert( c == fmu_qss.fmu->capi->c );
	return (fmi2Status)fmi2_import_set_real( fmu_me.fmu, vr, nvr, value );
}

fmi2Status
fmi2GetInteger(
 fmi2Component c,
 fmi2ValueReference const vr[],
 size_t nvr,
 fmi2Integer value[]
)
{
	assert( c == fmu_qss.fmu->capi->c );
	simulate(); // Advance simulation to FMU time
	return (fmi2Status)fmi2_import_get_integer( fmu_me.fmu, vr, nvr, value );
}

fmi2Status
fmi2SetInteger(
 fmi2Component c,
 fmi2ValueReference const vr[],
 size_t nvr,
 fmi2Integer const value[]
)
{
	assert( c == fmu_qss.fmu->capi->c );
	return (fmi2Status)fmi2_import_set_integer( fmu_me.fmu, vr, nvr, value );
}

fmi2Status
fmi2GetBoolean(
 fmi2Component c,
 fmi2ValueReference const vr[],
 size_t nvr,
 fmi2Boolean value[]
)
{
	assert( c == fmu_qss.fmu->capi->c );
	simulate(); // Advance simulation to FMU time
	return (fmi2Status)fmi2_import_get_boolean( fmu_me.fmu, vr, nvr, value );
}

fmi2Status
fmi2SetBoolean(
 fmi2Component c,
 fmi2ValueReference const vr[],
 size_t nvr,
 fmi2Boolean const value[]
)
{
	assert( c == fmu_qss.fmu->capi->c );
	return (fmi2Status)fmi2_import_set_boolean( fmu_me.fmu, vr, nvr, value );
}

fmi2Status
fmi2GetString(
 fmi2Component c,
 fmi2ValueReference const vr[],
 size_t nvr,
 fmi2String value[]
)
{
	assert( c == fmu_qss.fmu->capi->c );
	simulate(); // Advance simulation to FMU time
	return (fmi2Status)fmi2_import_get_string( fmu_me.fmu, vr, nvr, value );
}

fmi2Status
fmi2SetString(
 fmi2Component c,
 fmi2ValueReference const vr[],
 size_t nvr,
 fmi2String const value[]
)
{
	assert( c == fmu_qss.fmu->capi->c );
	return (fmi2Status)fmi2_import_set_string( fmu_me.fmu, vr, nvr, value );
}

fmi2Status
fmi2CompletedIntegratorStep(
 fmi2Component c,
 fmi2Boolean noSetFMUStatePriorToCurrentPoint,
 fmi2Boolean * enterEventMode,
 fmi2Boolean * terminateSimulation
)
{
	assert( c == fmu_qss.fmu->capi->c );
	return (fmi2Status)fmi2_import_completed_integrator_step( fmu_me.fmu, fmi2_true, enterEventMode, terminateSimulation );
	((component_ptr_t)c)->fmitime = fmu_me.t;
}

fmi2Status
fmi2GetContinuousStates(
 fmi2Component c,
 fmi2Real x[],
 size_t nx
)
{
	assert( c == fmu_qss.fmu->capi->c );
	assert( nx == 0u ); // No continuous states in FMU-QSS
	return fmi2OK;
}

fmi2Status
fmi2SetContinuousStates(
 fmi2Component c,
 fmi2Real const x[],
 size_t nx
)
{
	assert( c == fmu_qss.fmu->capi->c );
	assert( nx == 0u ); // No continuous states in FMU-QSS
	return fmi2OK;
}

fmi2Status
fmi2GetDerivatives(
 fmi2Component c,
 fmi2Real derivatives[],
 size_t nx
)
{
	assert( c == fmu_qss.fmu->capi->c );
	assert( nx == 0u ); // No continuous states in FMU-QSS
	return fmi2OK;
}

fmi2Status
fmi2GetDirectionalDerivative(
 fmi2Component c,
 fmi2ValueReference const vUnknown_ref[],
 size_t nUnknown,
 fmi2ValueReference const vKnown_ref[],
 size_t nKnown,
 fmi2Real const dvKnown[],
 fmi2Real dvUnknown[]
)
{
	assert( c == fmu_qss.fmu->capi->c );
	return (fmi2Status)fmi2_import_get_directional_derivative( fmu_me.fmu, vKnown_ref, nKnown, vUnknown_ref, nUnknown, dvKnown, dvUnknown ); // Note arg order switch
}

fmi2Status
fmi2GetEventIndicators(
 fmi2Component c,
 fmi2Real eventIndicators[],
 size_t ni
)
{
	assert( c == fmu_qss.fmu->capi->c );
	assert( ni == 0u );
	return fmi2OK;
}

fmi2Status
fmi2GetNominalsOfContinuousStates(
 fmi2Component c,
 fmi2Real x_nominal[],
 size_t nx
)
{
	assert( c == fmu_qss.fmu->capi->c );
	assert( nx == 0u ); // No continuous states in FMU-QSS
	return fmi2OK;
}

fmi2Status
fmi2GetFMUstate(
 fmi2Component c,
 fmi2FMUstate * FMUstate
)
{
	assert( c == fmu_qss.fmu->capi->c );
	return (fmi2Status)fmi2_import_get_fmu_state( fmu_me.fmu, FMUstate );
}

fmi2Status
fmi2SetFMUstate(
 fmi2Component c,
 fmi2FMUstate FMUstate
)
{
	assert( c == fmu_qss.fmu->capi->c );
	return (fmi2Status)fmi2_import_set_fmu_state( fmu_me.fmu, FMUstate );
}

fmi2Status
fmi2SerializeFMUstate(
 fmi2Component c,
 fmi2FMUstate FMUstate,
 fmi2Byte serializedState[],
 size_t size
)
{
	assert( c == fmu_qss.fmu->capi->c );
	return (fmi2Status)fmi2_import_serialize_fmu_state( fmu_me.fmu, FMUstate, serializedState, size );
}

fmi2Status
fmi2SerializedFMUstateSize(
 fmi2Component c,
 fmi2FMUstate FMUstate,
 size_t * size
)
{
	assert( c == fmu_qss.fmu->capi->c );
	return (fmi2Status)fmi2_import_serialized_fmu_state_size( fmu_me.fmu, FMUstate, size );
}

fmi2Status
fmi2DeSerializeFMUstate(
 fmi2Component c,
 fmi2Byte const serializedState[],
 size_t size,
 fmi2FMUstate * FMUstate
)
{
	assert( c == fmu_qss.fmu->capi->c );
	return (fmi2Status)fmi2_import_de_serialize_fmu_state( fmu_me.fmu, serializedState, size, FMUstate );
}

fmi2Status
fmi2Reset( fmi2Component c )
{
	assert( c == fmu_qss.fmu->capi->c );
	return (fmi2Status)fmi2_import_reset( fmu_me.fmu );
}

fmi2Status
fmi2Terminate( fmi2Component c )
{
	assert( c == fmu_qss.fmu->capi->c );
	return (fmi2Status)fmi2_import_terminate( fmu_me.fmu );
}

void
fmi2FreeInstance( fmi2Component c )
{
	assert( c == fmu_qss.fmu->capi->c );
	if ( ( fmu_qss.fmu != nullptr ) && ( fmu_qss.fmu->capi != nullptr ) ) fmu_qss.fmu->capi->c = nullptr;
	fmi2_import_free_instance( fmu_me.fmu );
}

fmi2Status
fmi2FreeFMUstate(
 fmi2Component c,
 fmi2FMUstate * FMUstate
)
{
	assert( c == fmu_qss.fmu->capi->c );
//	return (fmi2Status)fmi2_import_free_fmu_state( fmu_me.fmu, FMUstate ) : fmi2OK );
	return fmi2OK; //? Don't have state pointer for FMU-ME
}
