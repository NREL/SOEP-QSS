// FMU-ME Class
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
#include <QSS/fmu/FMU_Variable.hh>
#include <QSS/EventQueue.hh>
#include <QSS/Output.hh>
#include <QSS/SmoothToken.hh>

// FMI Library Headers
#include <fmilib.h>

// C++ Headers
#include <cassert>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>

namespace QSS {

// Forward
class Target;
class Conditional;

namespace fmu {

// Forward
class Variable;

// FMU-ME Class
struct FMU_ME final
{

public: // Types

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

	enum class FMU_Generator { JModelica, Dymola, Other };

	using size_type = std::size_t;
	using Time = double;
	using Real = double;
	using Integer = int;
	using Boolean = bool;
	using Events = EventQueue< Target >;
	using Variables = std::vector< Variable * >;
	using FMU_Variables = std::vector< FMU_Variable >;
	using Var_Idx = std::unordered_map< Variable const *, size_type >; // Map from Variables to their indexes
	using Var_Name_Ref = std::unordered_map< std::string, fmi2_value_reference_t >; // Map from variable names to FMU variable value references
	using Conditionals = std::vector< Conditional * >;
	using FMU_Vars = std::unordered_map< FMUVarPtr, FMU_Variable, FMUVarPtrHash >; // Map from FMU variables to FMU_Variable objects
	using FMU_Idxs = std::unordered_map< size_type, Variable * >; // Map from FMU variable indexes to QSS Variables
	using SmoothTokenOutput = Output_T< SmoothToken >;

public: // Creation

	// Default Constructor
	FMU_ME();

	// FMU-ME Path Constructor
	FMU_ME( std::string const & path );

	// Destructor
	~FMU_ME();

public: // Simulation Methods

	// Initialize
	void
	init( std::string const & path, bool const in_place = true );

	// Instantiate FMU
	void
	instantiate();

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

	// Pre-Simulation Setup
	void
	pre_simulate()
	{
		pre_simulate( std::vector< fmi2_value_reference_t >() ); // Pass empty output variable references
	}

	// Reinitialize
	void
	reinitialize();

	// Simulation Pass
	void
	simulate( fmi2_event_info_t * eventInfoMaster );

	// Simulation Pass
	void
	simulate();

	// Post-Simulation Actions
	void
	post_simulate();

public: // FMU Methods

	// Get FMU Time
	Time
	get_time()
	{
		return t_fmu;
	}

	// Set FMU Time
	void
	set_time( Time const t )
	{
		assert( fmu != nullptr );
		fmi2_import_set_time( fmu, t_fmu = t );
//Do Use below instead when not doing forward time bumps for numeric differentiation or zero crossing
//		fmi2_status_t const fmi_status = fmi2_import_set_time( fmu, t_fmu = t );
//		assert( status_check( fmi_status, "set_time" );
	}

	// Initialize Derivatives Array Size
	void
	init_derivatives( std::size_t const n_der )
	{
		if ( derivatives != nullptr ) delete derivatives;
		derivatives = new fmi2_real_t[ n_der ];
		n_derivatives = n_der;
	}

	// Get a Real FMU Variable Value
	Real
	get_real( fmi2_value_reference_t const ref )
	{
		assert( fmu != nullptr );
		Real val;
		fmi2_status_t const fmi_status = fmi2_import_get_real( fmu, &ref, std::size_t( 1u ), &val );
		assert( status_check( fmi_status, "get_real" ) );
		return val;
	}

	// Get Real FMU Variable Values
	void
	get_reals( std::size_t const n, fmi2_value_reference_t const refs[], Real vals[] )
	{
		assert( fmu != nullptr );
		fmi2_status_t const fmi_status = fmi2_import_get_real( fmu, refs, n, vals );
		assert( status_check( fmi_status, "get_reals" ) );
	}

	// Set a Real FMU Variable Value
	void
	set_real( fmi2_value_reference_t const ref, Real const val )
	{
		assert( fmu != nullptr );
		fmi2_status_t const fmi_status = fmi2_import_set_real( fmu, &ref, std::size_t( 1u ), &val );
		assert( status_check( fmi_status, "set_real" ) );
	}

	// Set a Real FMU Variable Value
	void
	set_reals( std::size_t const n, fmi2_value_reference_t const refs[], Real const vals[] )
	{
		assert( fmu != nullptr );
		fmi2_status_t const fmi_status = fmi2_import_set_real( fmu, refs, n, vals );
		assert( status_check( fmi_status, "set_reals" ) );
	}

	// Get All Derivatives Array: FMU Time and Variable Values Must be Set First
	void
	get_derivatives()
	{
		assert( derivatives != nullptr );
		fmi2_status_t const fmi_status = fmi2_import_get_derivatives( fmu, derivatives, n_derivatives );
		assert( status_check( fmi_status, "get_derivatives" ) );
	}

	// Get a Derivative: First call get_derivatives
	Real
	get_derivative( std::size_t const der_idx )
	{
		assert( der_idx - 1 < n_derivatives );
		return derivatives[ der_idx - 1 ];
	}

	// Get an Integer FMU Variable Value
	Integer
	get_integer( fmi2_value_reference_t const ref )
	{
		assert( fmu != nullptr );
		Integer val;
		fmi2_status_t const fmi_status = fmi2_import_get_integer( fmu, &ref, std::size_t( 1u ), &val );
		assert( status_check( fmi_status, "get_integer" ) );
		return val;
	}

	// Set an Integer FMU Variable Value
	void
	set_integer( fmi2_value_reference_t const ref, Integer const val )
	{
		assert( fmu != nullptr );
		fmi2_status_t const fmi_status = fmi2_import_set_integer( fmu, &ref, std::size_t( 1u ), &val );
		assert( status_check( fmi_status, "set_integer" ) );
	}

	// Get an Boolean FMU Variable Value
	bool
	get_boolean( fmi2_value_reference_t const ref )
	{
		assert( fmu != nullptr );
		fmi2_boolean_t fbt;
		fmi2_status_t const fmi_status = fmi2_import_get_boolean( fmu, &ref, std::size_t( 1u ), &fbt );
		assert( status_check( fmi_status, "get_boolean" ) );
		return ( fbt != 0 );
	}

	// Set an Boolean FMU Variable Value
	void
	set_boolean( fmi2_value_reference_t const ref, bool const val )
	{
		assert( fmu != nullptr );
		fmi2_boolean_t const fbt( static_cast< fmi2_boolean_t >( val ) );
		fmi2_status_t const fmi_status = fmi2_import_set_boolean( fmu, &ref, std::size_t( 1u ), &fbt );
		assert( status_check( fmi_status, "set_boolean" ) );
	}

	// Discrete Event Processing
	void
	do_event_iteration( fmi2_import_t * fmu, fmi2_event_info_t * eventInfo )
	{
		eventInfo->newDiscreteStatesNeeded = fmi2_true;
		eventInfo->terminateSimulation = fmi2_false;
		while ( eventInfo->newDiscreteStatesNeeded && !eventInfo->terminateSimulation ) {
			fmi2_import_new_discrete_states( fmu, eventInfo );
		}
	}

	// Cleanup Allocations
	void
	cleanup()
	{
		delete derivatives;
		derivatives = nullptr;
		n_derivatives = 0;
	}

private: // Static Methods

	// FMI Status Check/Report
	static
	bool
	status_check( fmi2_status_t const status, std::string const & fxn_name = std::string() );

public: // Data

	// Model name and unzip directory
	std::string name;
	std::string unzip_dir;

	// FMU
	fmi2_import_t * fmu{ nullptr }; // FMU pointer
	fmi2_real_t * states{ nullptr };
	fmi2_real_t * derivatives{ nullptr };
	fmi2_real_t * event_indicators{ nullptr };
	fmi2_real_t * event_indicators_last{ nullptr };
	fmi_import_context_t * context{ nullptr };
	fmi2_import_variable_list_t * var_list{ nullptr };
	fmi2_import_variable_list_t * der_list{ nullptr };
	fmi2_event_info_t eventInfo;
	fmi2_callback_functions_t callBackFunctions;
	jm_callbacks callbacks;
	FMU_Generator fmu_generator;

	// FMU counts
	size_type n_vars{ 0 };
	size_type n_states{ 0 };
	size_type n_derivatives{ 0 };
	size_type n_event_indicators{ 0 };
	size_type n_outs{ 0 };
	size_type n_fmu_outs{ 0 };
	size_type n_all_outs{ 0 };
	size_type n_fmu_qss_qss_outs{ 0 };

	// Timing
	Time t0{ 0.0 }; // Simulation start time
	Time tE{ 0.0 }; // Simulation end time
	Time t{ 0.0 }; // Simulation current time
	Time t_fmu{ 0.0 }; // FMU time
	Time tOut{ 0.0 }; // Simulation output time
	size_type iOut{ 1u }; // Output step index

	// Event queue
	Events * events; // Event queue

	// Tolerances
	Real rTol; // Relative tolerance
	Real aTol; // Absolute tolerance

	// Collections
	Variables vars; // QSS variables
	Variables vars_NZ; // Non-zero-crossing variables
	Variables vars_ZC; // Zero-crossing variables
	Variables state_vars; // FMU state QSS variables
	Variables outs; // FMU output QSS variables
	Variables fmu_qss_qss_outs; // FMU-QSS output QSS variables
	Var_Idx var_idx; // Map from Variables to their indexes
	Var_Name_Ref var_name_ref; // Map from variable names to FMU variable value references
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
	int order_max_NZ{ 0 }; // Non-zero-crossing QSS variable max order
	int order_max_ZC{ 0 }; // Zero-crossing QSS variable max order

	// Output controls
	bool doSOut{ false };
	bool doTOut{ false };
	bool doROut{ false };

	// Simulation
	size_type const max_pass_count_multiplier{ 2 };
	size_type n_discrete_events{ 0 };
	size_type n_QSS_events{ 0 };
	size_type n_QSS_simultaneous_events{ 0 };
	size_type n_ZC_events{ 0 };
	double sim_dtMin{ 0.0 };
	bool pass_warned{ false };
	Variables observers;
	fmi2_boolean_t enterEventMode{ fmi2_false };
	fmi2_boolean_t terminateSimulation{ fmi2_false };

};

} // fmu
} // QSS

#endif
