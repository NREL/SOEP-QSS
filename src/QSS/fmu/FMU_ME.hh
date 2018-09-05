// QSS FMU-ME Class
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

#ifndef QSS_fmu_FMU_ME_hh_INCLUDED
#define QSS_fmu_FMU_ME_hh_INCLUDED

// QSS Headers
#include <QSS/fmu/Variable.hh>
#include <QSS/fmu/FMU_Variable.hh>
#include <QSS/Output.hh>
#include <QSS/SmoothToken.hh>

// FMI Library Headers
#include <fmilib.h>

// C++ Headers
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>

namespace QSS {

// Forward
class Conditional;

namespace fmu {

// Forward
class Variable;

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

// FMU-ME Class
struct FMU_ME final
{

public: // Types

	using size_type = std::size_t;
	using Time = double;
	using Real = double;
	using Variables = std::vector< Variable * >;
	using FMU_Variables = std::vector< FMU_Variable >;
	using Var_Idx = std::unordered_map< Variable const *, size_type >; // Map from Variables to their indexes
	using Conditionals = std::vector< Conditional * >;
	using FMU_Vars = std::unordered_map< FMUVarPtr, FMU_Variable, FMUVarPtrHash >; // Map from FMU variables to FMU_Variable objects
	using FMU_Idxs = std::unordered_map< size_type, Variable * >; // Map from FMU variable indexes to QSS Variables
	using SmoothTokenOutput = Output_T< SmoothToken >;

	enum class FMU_Generator { JModelica, Dymola, Other };

public: // Creation

	// Destructor
	~FMU_ME();

public: // Methods

	// Initialize
	void
	init( std::string const & path );

	// Options Setup
	void
	set_options(
	 Real const tBeg,
	 Real const tEnd,
	 Real const rTolerance
	);

	// Pre-Simulation Setup
	void
	pre_simulate( std::vector< fmi2_value_reference_t > const & fmu_qss_out_var_refs );

	// Simulation Pass
	void
	simulate( fmi2_event_info_t * eventInfoMaster );

	// Post-Simulation Actions
	void
	post_simulate();

public: // Data

	std::string name;
	std::string unzip_dir;

	fmi2_import_t * fmu = nullptr; // FMU pointer
	fmi2_event_info_t eventInfo;
	fmi2_real_t * states = nullptr;
	fmi2_real_t * states_der = nullptr;
	fmi2_real_t * event_indicators = nullptr;
	fmi2_real_t * event_indicators_last = nullptr;
	fmi_import_context_t * context = nullptr;
	fmi2_import_variable_list_t * var_list = nullptr;
	fmi2_import_variable_list_t * der_list = nullptr;

	FMU_Generator fmu_generator;

	jm_callbacks callbacks;
	fmi2_callback_functions_t callBackFunctions;

	// Timing
	Time t0; // Simulation start time
	Time tE; // Simulation end time
	Time t; // Simulation current time
	Time tOut; // Simulation output time
	size_type iOut = 1u; // Output step index

	// Tolerances
	Real rTol; // Relative tolerance
	Real aTol; // Absolute tolerance

	// FMU counts
	size_type n_states = 0;
	size_type n_event_indicators = 0;
	size_type n_vars = 0;
	size_type n_outs = 0;
	size_type n_fmu_outs = 0;
	size_type n_all_outs = 0;
	size_type n_fmu_qss_qss_outs = 0;

	// Collections
	Variables vars; // QSS variables
	Variables state_vars; // FMU state QSS variables
	Variables outs; // FMU output QSS variables
	Variables fmu_qss_qss_outs; // FMU-QSS output QSS variables
	Var_Idx var_idx;
	Conditionals cons; // Conditionals
	FMU_Vars fmu_vars; // FMU variables
	FMU_Vars fmu_outs; // FMU output variables
	FMU_Vars fmu_ders; // FMU variable to derivative map
	FMU_Vars fmu_dvrs; // FMU derivative to variable map
	FMU_Idxs fmu_idxs; // Map from FMU variable index to QSS variable
	std::vector< Output > x_outs; // Continuous rep outputs
	std::vector< Output > q_outs; // Quantized rep outputs
	std::vector< Output > f_outs; // FMU outputs
	std::vector< SmoothTokenOutput > k_qss_outs; // FMU-QSS QSS variable smooth token outputs

	// Output controls
	bool doSOut = false;
	bool doTOut = false;
	bool doROut = false;

	// Simulation
	size_type const max_pass_count_multiplier =2;
	size_type n_discrete_events = 0;
	size_type n_QSS_events = 0;
	size_type n_QSS_simultaneous_events = 0;
	size_type n_ZC_events = 0;
	double sim_dtMin = 0.0;
	bool pass_warned = false;
	Variables observers;
	fmi2_boolean_t enterEventMode = fmi2_false;
	fmi2_boolean_t terminateSimulation = fmi2_false;

};

// Globals
extern FMU_ME fmu_me;

} // fmu
} // QSS

#endif
