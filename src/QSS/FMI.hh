// FMU-QSS FMI 2.0 API Wrappers
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2024 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_FMI_hh_INCLUDED
#define QSS_FMI_hh_INCLUDED

// C++ Headers
#include <unordered_map>

// FMIL Headers
#include <fmilib.h>
#include <FMI2/fmi2FunctionTypes.h>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <string>
#include <utility>

namespace QSS {

// Forward
struct FMU_QSS;

// Globals
extern std::unordered_map< std::string, FMU_QSS * > guid_to_fmu_qss; // FMU-QSS lookup from FMU-ME GUID
extern std::unordered_map< fmi2Component, FMU_QSS * > c_to_fmu_qss; // FMU-QSS lookup from context

// Register FMU-QSS by its FMU-ME GUID
inline
void
reg( std::string const & guid, FMU_QSS * fmu_qss )
{
	guid_to_fmu_qss[ guid ] = fmu_qss;
}

// Register FMU-QSS by its fmi2Component
inline
void
reg( fmi2Component c, FMU_QSS * fmu_qss )
{
	c_to_fmu_qss[ c ] = fmu_qss;
}

// Unregister FMU-QSS
inline
void
unreg( FMU_QSS * fmu_qss )
{
	auto ig( std::find_if( guid_to_fmu_qss.begin(), guid_to_fmu_qss.end(), [fmu_qss]( std::pair< std::string, FMU_QSS * > const & p ){ return p.second == fmu_qss; } ) );
	if ( ig != guid_to_fmu_qss.end() ) guid_to_fmu_qss.erase( ig );
	assert( std::find_if( guid_to_fmu_qss.begin(), guid_to_fmu_qss.end(), [fmu_qss]( std::pair< std::string, FMU_QSS * > const & p ){ return p.second == fmu_qss; } ) == guid_to_fmu_qss.end() ); // Each FMU-QSS should only appear once

	auto ic( std::find_if( c_to_fmu_qss.begin(), c_to_fmu_qss.end(), [fmu_qss]( std::pair< fmi2Component, FMU_QSS * > const & p ){ return p.second == fmu_qss; } ) );
	if ( ic != c_to_fmu_qss.end() ) c_to_fmu_qss.erase( ic );
	assert( std::find_if( c_to_fmu_qss.begin(), c_to_fmu_qss.end(), [fmu_qss]( std::pair< fmi2Component, FMU_QSS * > const & p ){ return p.second == fmu_qss; } ) == c_to_fmu_qss.end() ); // Each FMU-QSS should only appear once
}

// Lookup FMU-QSS by its FMU-ME GUID
inline
FMU_QSS &
fmu_qss_of( std::string const & guid )
{
	auto ig( guid_to_fmu_qss.find( guid ) );
	assert( ig != guid_to_fmu_qss.end() );
	return *(ig->second);
}

// Lookup FMU-QSS by its fmi2Component
inline
FMU_QSS &
fmu_qss_of( fmi2Component c )
{
	auto ic( c_to_fmu_qss.find( c ) );
	assert( ic != c_to_fmu_qss.end() );
	return *(ic->second);
}

} // QSS

#ifdef _MSC_VER
#define DllExport __declspec( dllexport )
#else
#define DllExport
#endif

#ifndef FMI2_Export
#define FMI2_Export DllExport
#endif

#ifdef __cplusplus
extern "C" {
#endif

FMI2_Export
char const *
fmi2GetVersion();

FMI2_Export
char const *
fmi2GetTypesPlatform();

FMI2_Export
fmi2Component
fmi2Instantiate(
 fmi2String instanceName,
 fmi2Type fmuType,
 fmi2String GUID,
 fmi2String fmuResourceLocation,
 fmi2CallbackFunctions const * functions,
 fmi2Boolean visible,
 fmi2Boolean loggingOn
);

FMI2_Export
fmi2Status
fmi2SetDebugLogging(
 fmi2Component c,
 fmi2Boolean loggingOn,
 std::size_t n ,
 fmi2String const cat[]
);

FMI2_Export
fmi2Status
fmi2SetupExperiment(
 fmi2Component c,
 fmi2Boolean toleranceDefined,
 fmi2Real tolerance,
 fmi2Real startTime,
 fmi2Boolean stopTimeDefined,
 fmi2Real stopTime
);

FMI2_Export
fmi2Status
fmi2EnterInitializationMode( fmi2Component c );

FMI2_Export
fmi2Status
fmi2ExitInitializationMode( fmi2Component c );

FMI2_Export
fmi2Status
fmi2EnterEventMode( fmi2Component c );

FMI2_Export
fmi2Status
fmi2EnterContinuousTimeMode( fmi2Component c );

FMI2_Export
fmi2Status
fmi2NewDiscreteStates(
 fmi2Component c,
 fmi2EventInfo * eventInfo
);

FMI2_Export
fmi2Status
fmi2SetRealInputDerivatives(
 fmi2Component c,
 fmi2ValueReference const vr[],
 std::size_t nvr,
 fmi2Integer const order[],
 fmi2Real const value[]
);

FMI2_Export
fmi2Status
fmi2GetRealOutputDerivatives(
 fmi2Component c,
 fmi2ValueReference const vr[],
 std::size_t nvr,
 fmi2Integer const order[],
 fmi2Real value[]
);

FMI2_Export
fmi2Status
fmi2SetTime(
 fmi2Component c,
 fmi2Real fmitime
);

FMI2_Export
fmi2Status
fmi2GetReal(
 fmi2Component c,
 fmi2ValueReference const vr[],
 std::size_t nvr,
 fmi2Real value[]
);

FMI2_Export
fmi2Status
fmi2SetReal(
 fmi2Component c,
 fmi2ValueReference const vr[],
 std::size_t nvr,
 fmi2Real const value[]
);

FMI2_Export
fmi2Status
fmi2GetInteger(
 fmi2Component c,
 fmi2ValueReference const vr[],
 std::size_t nvr,
 fmi2Integer value[]
);

FMI2_Export
fmi2Status
fmi2SetInteger(
 fmi2Component c,
 fmi2ValueReference const vr[],
 std::size_t nvr,
 fmi2Integer const value[]
);

FMI2_Export
fmi2Status
fmi2GetBoolean(
 fmi2Component c,
 fmi2ValueReference const vr[],
 std::size_t nvr,
 fmi2Boolean value[]
);

FMI2_Export
fmi2Status
fmi2SetBoolean(
 fmi2Component c,
 fmi2ValueReference const vr[],
 std::size_t nvr,
 fmi2Boolean const value[]
);

FMI2_Export
fmi2Status
fmi2GetString(
 fmi2Component c,
 fmi2ValueReference const vr[],
 std::size_t nvr,
 fmi2String value[]
);

FMI2_Export
fmi2Status
fmi2SetString(
 fmi2Component c,
 fmi2ValueReference const vr[],
 std::size_t nvr,
 fmi2String const value[]
);

FMI2_Export
fmi2Status
fmi2CompletedIntegratorStep(
 fmi2Component c,
 fmi2Boolean noSetFMUStatePriorToCurrentPoint,
 fmi2Boolean * enterEventMode,
 fmi2Boolean * terminateSimulation
);

FMI2_Export
fmi2Status
fmi2GetContinuousStates(
 fmi2Component c,
 fmi2Real x[],
 std::size_t nx
);

FMI2_Export
fmi2Status
fmi2SetContinuousStates(
 fmi2Component c,
 fmi2Real const x[],
 std::size_t nx
);

FMI2_Export
fmi2Status
fmi2GetDerivatives(
 fmi2Component c,
 fmi2Real derivatives[],
 std::size_t nx
);

FMI2_Export
fmi2Status
fmi2GetDirectionalDerivative(
 fmi2Component c,
 fmi2ValueReference const vUnknown_ref[],
 std::size_t nUnknown,
 fmi2ValueReference const vKnown_ref[],
 std::size_t nKnown,
 fmi2Real const dvKnown[],
 fmi2Real dvUnknown[]
);

FMI2_Export
fmi2Status
fmi2GetEventIndicators(
 fmi2Component c,
 fmi2Real eventIndicators[],
 std::size_t ni
);

FMI2_Export
fmi2Status
fmi2GetNominalsOfContinuousStates(
 fmi2Component c,
 fmi2Real x_nominal[],
 std::size_t nx
);

FMI2_Export
fmi2Status
fmi2GetFMUstate(
 fmi2Component c,
 fmi2FMUstate * FMUstate
);

FMI2_Export
fmi2Status
fmi2SetFMUstate(
 fmi2Component c,
 fmi2FMUstate FMUstate
);

FMI2_Export
fmi2Status
fmi2SerializeFMUstate(
 fmi2Component c,
 fmi2FMUstate FMUstate,
 fmi2Byte serializedState[],
 std::size_t size
);

FMI2_Export
fmi2Status
fmi2SerializedFMUstateSize(
 fmi2Component c,
 fmi2FMUstate FMUstate,
 std::size_t * size
);

FMI2_Export
fmi2Status
fmi2DeSerializeFMUstate(
 fmi2Component c,
 fmi2Byte const serializedState[],
 std::size_t size,
 fmi2FMUstate * FMUstate
);

FMI2_Export
fmi2Status
fmi2Reset( fmi2Component c );

FMI2_Export
fmi2Status
fmi2Terminate( fmi2Component c );

FMI2_Export
void
fmi2FreeInstance( fmi2Component c );

FMI2_Export
fmi2Status
fmi2FreeFMUstate(
 fmi2Component c,
 fmi2FMUstate * FMUstate
);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
