// FMU-ME Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2021 Objexx Engineering, Inc. All rights reserved.
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

namespace fmu {

// Forward
class Variable;
template< typename V > class Conditional;

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

	enum class FMU_Generator { JModelica, Optimica, Dymola, Other };

	using size_type = std::size_t;
	using Time = double;
	using Real = double;
	using Integer = int;
	using Boolean = bool;
	using EventQ = EventQueue< Target >;
	using Variables = std::vector< Variable * >;
	using Var_Idx = std::unordered_map< Variable const *, size_type >; // Map from Variables to their indexes
	using Var_Name_Var = std::unordered_map< std::string, Variable * >; // Map from variable names to variables
	using Var_Name_Ref = std::unordered_map< std::string, fmi2_value_reference_t >; // Map from variable names to FMU variable value references
	using Ref_Var = std::unordered_map< fmi2_value_reference_t, Variable * >;
	using Var_Refs = std::vector< fmi2_value_reference_t >;
	using Conditionals = std::vector< Conditional< Variable > * >;
	using FMU_Vars = std::unordered_map< FMUVarPtr, FMU_Variable, FMUVarPtrHash >; // Map from FMU variables to FMU_Variable objects
	using FMU_Idxs = std::unordered_map< size_type, Variable * >; // Map from FMU variable indexes to QSS Variables
	using SmoothTokenOutput = Output< SmoothToken >;
	using Counts = std::unordered_map< Variable const *, size_type >; // Map from Variables to counters

public: // Creation

	// Default Constructor
	FMU_ME();

	// Copy Constructor
	FMU_ME( FMU_ME const & ) = delete;

	// Move Constructor
	FMU_ME( FMU_ME && ) = delete;

	// FMU-ME Path Constructor
	explicit
	FMU_ME( std::string const & path );

	// FMU-ME Path + Event Queue Constructor
	FMU_ME( std::string const & path, EventQ * eventq );

	// Destructor
	~FMU_ME();

public: // Assignment

	// Copy Assignment
	FMU_ME &
	operator =( FMU_ME const & ) = delete;

	// Move Assignment
	FMU_ME &
	operator =( FMU_ME && ) = delete;

public: // Predicate

	// Forward Time?
	bool
	fwd_time( Time const time ) const
	{
		return time >= t0;
	}

public: // Property

	// Variable Lookup by Name (for Testing)
	Variable const *
	var_named( std::string const & var_name ) const;

	// Variable Lookup by Name (for Testing)
	Variable *
	var_named( std::string const & var_name );

public: // Simulation Methods

	// Initialize
	void
	initialize( std::string const & path, bool const in_place = true );

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
	pre_simulate();

	// Optimize ND Time Step
	void
	dtND_optimize( Time const to );

	// Initialization
	void
	init();

	// Initialization: Stage 0.1
	void
	init_0_1();

	// Initialization: Stage 0.2
	void
	init_0_2();

	// Initialization: Stage 1.1
	void
	init_1_1();

	// Initialization: Stage 1.2
	void
	init_1_2();

	// Initialization: Stage 2.1
	void
	init_2_1();

	// Initialization: Stage 2.2
	void
	init_2_2();

	// Initialization: Stage 3.1
	void
	init_3_1();

	// Initialization: Stage Final
	void
	init_F();

	// Initialization: Stage XB
	void
	init_XB();

	// Initialization: Stage Set State to t0
	void
	init_t0();

	// Initialization: Stage Pre-Simulate
	void
	init_pre_simulate();

	// Simulation
	void
	simulate( fmi2_event_info_t * eventInfoMaster, bool const connected = false );

	// Simulation
	void
	simulate();

	// Post-Simulation Actions
	void
	post_simulate();

public: // FMU Methods

	// Get FMU Time
	Time
	get_time() const
	{
		return t_fmu;
	}

	// Set FMU Time
	void
	set_time( Time const t_fmu_new )
	{
		assert( fmu != nullptr );
		fmi2_import_set_time( fmu, t_fmu = t_fmu_new );
//Do Use below instead when not doing forward time bumps for numeric differentiation or zero crossing
//		fmi2_status_t const fmi_status = fmi2_import_set_time( fmu, t_fmu = t_fmu_new );
//		assert( status_check( fmi_status, "set_time" );
//		(void)fmi_status; // Suppress unused warning
	}

	// Initialize Derivatives Array Size
	void
	init_derivatives()
	{
		if ( derivatives != nullptr ) delete derivatives;
		derivatives = ( n_derivatives > 0u ? new fmi2_real_t[ n_derivatives ] : nullptr );
	}

	// Get a Real FMU Variable Value
	Real
	get_real( fmi2_value_reference_t const ref ) const
	{
		assert( fmu != nullptr );
		Real val;
		fmi2_status_t const fmi_status = fmi2_import_get_real( fmu, &ref, std::size_t( 1u ), &val );
		assert( status_check( fmi_status, "get_real" ) );
		(void)fmi_status; // Suppress unused warning
		return val;
	}

	// Set a Real FMU Variable Value
	void
	set_real( fmi2_value_reference_t const ref, Real const val )
	{
		assert( fmu != nullptr );
		fmi2_status_t const fmi_status = fmi2_import_set_real( fmu, &ref, std::size_t( 1u ), &val );
		assert( status_check( fmi_status, "set_real" ) );
		(void)fmi_status; // Suppress unused warning
	}

	// Get Real FMU Variable Values
	void
	get_reals( std::size_t const n, fmi2_value_reference_t const refs[], Real vals[] ) const
	{
		assert( fmu != nullptr );
		fmi2_status_t const fmi_status = fmi2_import_get_real( fmu, refs, n, vals );
		assert( status_check( fmi_status, "get_reals" ) );
		(void)fmi_status; // Suppress unused warning
	}

	// Set a Real FMU Variable Value
	void
	set_reals( std::size_t const n, fmi2_value_reference_t const refs[], Real const vals[] )
	{
		assert( fmu != nullptr );
		fmi2_status_t const fmi_status = fmi2_import_set_real( fmu, refs, n, vals );
		assert( status_check( fmi_status, "set_reals" ) );
		(void)fmi_status; // Suppress unused warning
	}

	// Get a Derivative: First call get_derivatives
	Real
	get_derivative( std::size_t const der_idx ) const
	{
		assert( der_idx - 1 < n_derivatives );
		return derivatives[ der_idx - 1 ];
	}

	// Get All Derivatives Array: FMU Time and Variable Values Must be Set First
	void
	get_derivatives() const
	{
		assert( derivatives != nullptr );
		fmi2_status_t const fmi_status = fmi2_import_get_derivatives( fmu, derivatives, n_derivatives );
		assert( status_check( fmi_status, "get_derivatives" ) );
		(void)fmi_status; // Suppress unused warning
	}

	// Get a Directional Derivative
	Real
	get_directional_derivative(
	 fmi2_value_reference_t const v_ref[], // Seed value references
	 std::size_t const nv, // Seed size
	 fmi2_value_reference_t const z_ref, // Derivative value reference
	 fmi2_real_t const dv[] // Seed values
	) const
	{
		assert( fmu != nullptr );
		fmi2_real_t dz;
		fmi2_status_t const fmi_status = fmi2_import_get_directional_derivative( fmu, v_ref, nv, &z_ref, std::size_t( 1u ), dv, &dz );
		assert( status_check( fmi_status, "get_directional_derivative" ) );
		(void)fmi_status; // Suppress unused warning
		return dz;
	}

	// Get Directional Derivatives
	void
	get_directional_derivatives(
	 fmi2_value_reference_t const v_ref[], // Seed value references
	 std::size_t const nv, // Seed size
	 fmi2_value_reference_t const z_ref[], // Derivative value references
	 std::size_t const nz, // Derivative size
	 fmi2_real_t const dv[], // Seed values
	 fmi2_real_t dz[] // Derivatives
	) const
	{
		assert( fmu != nullptr );
		fmi2_status_t const fmi_status = fmi2_import_get_directional_derivative( fmu, v_ref, nv, z_ref, nz, dv, dz );
		assert( status_check( fmi_status, "get_directional_derivatives" ) );
		(void)fmi_status; // Suppress unused warning
	}

	// Get an Integer FMU Variable Value
	Integer
	get_integer( fmi2_value_reference_t const ref ) const
	{
		assert( fmu != nullptr );
		Integer val;
		fmi2_status_t const fmi_status = fmi2_import_get_integer( fmu, &ref, std::size_t( 1u ), &val );
		assert( status_check( fmi_status, "get_integer" ) );
		(void)fmi_status; // Suppress unused warning
		return val;
	}

	// Set an Integer FMU Variable Value
	void
	set_integer( fmi2_value_reference_t const ref, Integer const val )
	{
		assert( fmu != nullptr );
		fmi2_status_t const fmi_status = fmi2_import_set_integer( fmu, &ref, std::size_t( 1u ), &val );
		assert( status_check( fmi_status, "set_integer" ) );
		(void)fmi_status; // Suppress unused warning
	}

	// Get an Boolean FMU Variable Value
	bool
	get_boolean( fmi2_value_reference_t const ref ) const
	{
		assert( fmu != nullptr );
		fmi2_boolean_t fbt;
		fmi2_status_t const fmi_status = fmi2_import_get_boolean( fmu, &ref, std::size_t( 1u ), &fbt );
		assert( status_check( fmi_status, "get_boolean" ) );
		(void)fmi_status; // Suppress unused warning
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
		(void)fmi_status; // Suppress unused warning
	}

	// Get a Real FMU Variable Value
	Real
	get_as_real( FMU_Variable const & var ) const
	{
		return ( var.is_Real() ? get_real( var.ref ) : ( var.is_Integer() ? Real( get_integer( var.ref ) ) : ( var.is_Boolean() ? Real( get_boolean( var.ref ) ) : 0.0 ) ) );
	}

	// Discrete Event Processing
	void
	do_event_iteration()
	{
		eventInfo.newDiscreteStatesNeeded = fmi2_true;
		eventInfo.terminateSimulation = fmi2_false;
		while ( eventInfo.newDiscreteStatesNeeded && !eventInfo.terminateSimulation ) {
			fmi2_import_new_discrete_states( fmu, &eventInfo );
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

	// FMI SI Unit Check
	static
	bool
	SI_unit_check( fmi2_import_unit_t * unit, bool const msg = true );

public: // Data

	// Model name and unzip directory
	std::string name;
	std::string unzip_dir;

	// FMU
	fmi2_import_t * fmu{ nullptr }; // FMU pointer
	fmi2_real_t * states{ nullptr };
	fmi2_real_t * derivatives{ nullptr };
	fmi2_real_t * x_nominal{ nullptr };
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
	size_type n_vars{ 0u };
	size_type n_states{ 0u };
	size_type n_derivatives{ 0u };
	size_type n_event_indicators{ 0u };
	size_type n_f_outs{ 0u };
	size_type n_l_outs{ 0u };
	size_type n_fmu_qss_qss_outs{ 0u };

	// Timing
	Time t0{ 0.0 }; // Simulation start time
	Time tE{ 0.0 }; // Simulation end time
	Time t{ 0.0 }; // Simulation current time
	Time tProc{ 0.0 }; // Time of last processed event
	Time t_fmu{ 0.0 }; // FMU time
	Time tOut{ 0.0 }; // Simulation output time
	size_type iOut{ 1u }; // Output step index

	// Event queue
	EventQ * eventq{ nullptr }; // Event queue
	bool eventq_own{ true }; // Own the event queue?

	// Tolerances
	Real rTol{ 1.0e-4 }; // Relative tolerance

	// Variables
	Variables vars; // QSS variables
	Variables vars_ZC; // Zero-crossing variables
	Variables vars_NZ; // Non-zero-crossing variables
	Variables vars_CI; // Connection input variables
	Variables vars_NC; // Non-zero-crossing non-connection variables
	Variables vars_XB; // X-Based (ZC+BIDR) state variables
	Variables state_vars; // FMU state QSS variables
	Variables f_outs_vars; // FMU output QSS variables
	Variables fmu_qss_qss_outs; // FMU-QSS output QSS variables
	Var_Idx var_idx; // Map from Variables to their indexes
	Var_Name_Ref var_name_ref; // Map from variable names to FMU variable value references
	Var_Name_Var var_name_var; // Map from variable names to variables
	Conditionals cons; // Conditionals
	FMU_Vars fmu_vars; // FMU variables
	FMU_Vars fmu_outs; // FMU output variables
	FMU_Vars fmu_ders; // FMU variable to derivative map
	FMU_Vars fmu_dvrs; // FMU derivative to variable map
	FMU_Idxs fmu_idxs; // Map from FMU variable index to QSS variable
	Ref_Var qss_var_of_ref;
	Var_Refs out_var_refs;
	std::vector< Output<> > f_outs; // FMU QSS variable outputs
	std::vector< Output<> > l_outs; // FMU local variable outputs
	std::vector< SmoothTokenOutput > k_qss_outs; // FMU-QSS QSS variable smooth token outputs
	int order_max_CI{ 0 }; // Connection input QSS variable max order
	int order_max_NC{ 0 }; // Non-zero-crossing non-connection QSS variable max order
	bool has_event_indicators{ false };
	bool has_explicit_ZCs{ false };

	// Output controls
	bool doROut{ false }; // Requantizations
	bool doZOut{ false }; // Zero crossings
	bool doDOut{ false }; // Discrete events
	bool doSOut{ false }; // Sampled
	bool doKOut{ false }; // Smooth token

	// Simulation
	size_type const max_pass_count_multiplier{ 2 };
	size_type n_discrete_events{ 0u };
	size_type n_QSS_events{ 0u };
	size_type n_QSS_simultaneous_events{ 0u };
	size_type n_ZC_events{ 0u };
	double sim_dtMin{ 0.0 };
	bool pass_warned{ false };
	Variables observers;
	fmi2_boolean_t enterEventMode{ fmi2_false };
	fmi2_boolean_t terminateSimulation{ fmi2_false };
	int tPer{ 0 }; // Percent of simulation time completed
	double sim_cpu_time{ 0.0 }; // Simulation CPU time
	double sim_wall_time{ 0.0 }; // Simulation wall time
	Counts c_QSS_events;
	Counts c_ZC_events;

}; // FMU_ME

} // fmu
} // QSS

#endif
