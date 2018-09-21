// QSS Defined Model Simulation Runner
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
#include <QSS/cod/simulate_cod.hh>
#include <QSS/cod/Conditional.hh>
#include <QSS/cod/Variable.hh>
#include <QSS/cod/mdl/achilles.hh>
#include <QSS/cod/mdl/achilles2.hh>
#include <QSS/cod/mdl/achillesc.hh>
#include <QSS/cod/mdl/achilles_ND.hh>
#include <QSS/cod/mdl/bball.hh>
#include <QSS/cod/mdl/exponential_decay.hh>
#include <QSS/cod/mdl/exponential_decay_sine.hh>
#include <QSS/cod/mdl/exponential_decay_sine_ND.hh>
#include <QSS/cod/mdl/exponential_decay_step.hh>
#include <QSS/cod/mdl/gen.hh>
#include <QSS/cod/mdl/nonlinear.hh>
#include <QSS/cod/mdl/nonlinear_ND.hh>
#include <QSS/cod/mdl/StateEvent6.hh>
#include <QSS/cod/mdl/stiff.hh>
#include <QSS/cod/mdl/xy.hh>
#include <QSS/cod/mdl/xyz.hh>
#include <QSS/container.hh>
#include <QSS/cycles.hh>
#include <QSS/globals.hh>
#include <QSS/math.hh>
#include <QSS/options.hh>
#include <QSS/Output.hh>
#include <QSS/string.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <ctime> // Simulation time
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace QSS {
namespace cod {

// Simulate a Defined Model
void
simulate()
{
	// Types
	using Variables = Variable::Variables;
	using size_type = Variables::size_type;
	using Time = Variable::Time;
	using Var_Idx = std::unordered_map< Variable const *, size_type >; // Map from Variables to their indexes
	using Conditionals = std::vector< Conditional * >;

	// I/o setup
	std::cout << std::setprecision( 15 );
	std::cerr << std::setprecision( 15 );
	std::vector< Output > x_outs; // Continuous rep outputs
	std::vector< Output > q_outs; // Quantized rep outputs

	// Report QSS method
	if ( options::qss == options::QSS::QSS1 ) {
		std::cout << "\nQSS Method: QSS1" << std::endl;
	} else if ( options::qss == options::QSS::QSS2 ) {
		std::cout << "\nQSS Method: QSS2" << std::endl;
	} else if ( options::qss == options::QSS::QSS3 ) {
		std::cout << "\nQSS Method: QSS3" << std::endl;
	} else if ( options::qss == options::QSS::LIQSS1 ) {
		std::cout << "\nQSS Method: LIQSS1" << std::endl;
	} else if ( options::qss == options::QSS::LIQSS2 ) {
		std::cout << "\nQSS Method: LIQSS2" << std::endl;
	} else if ( options::qss == options::QSS::LIQSS3 ) {
		std::cout << "\nQSS Method: LIQSS3" << std::endl;
	} else if ( options::qss == options::QSS::xQSS1 ) {
		std::cout << "\nQSS Method: xQSS1" << std::endl;
	} else if ( options::qss == options::QSS::xQSS2 ) {
		std::cout << "\nQSS Method: xQSS2" << std::endl;
	} else if ( options::qss == options::QSS::xQSS3 ) {
		std::cout << "\nQSS Method: xQSS3" << std::endl;
	} else if ( options::qss == options::QSS::xLIQSS1 ) {
		std::cout << "\nQSS Method: xLIQSS1" << std::endl;
	} else if ( options::qss == options::QSS::xLIQSS2 ) {
		std::cout << "\nQSS Method: xLIQSS2" << std::endl;
	} else if ( options::qss == options::QSS::xLIQSS3 ) {
		std::cout << "\nQSS Method: xLIQSS3" << std::endl;
	} else {
		std::cerr << "\nError: Unsupported QSS method" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Collections
	Variables vars;
	Conditionals cons;

	// Model setup
	if ( options::model == "achilles" ) {
		mdl::achilles( vars );
	} else if ( options::model == "achilles2" ) {
		mdl::achilles2( vars );
	} else if ( options::model == "achillesc" ) {
		mdl::achillesc( vars );
	} else if ( options::model == "achilles_ND" ) {
		mdl::achilles_ND( vars );
	} else if ( options::model == "bball" ) {
		mdl::bball( vars, cons );
	} else if ( options::model == "exponential_decay" ) {
		mdl::exponential_decay( vars );
	} else if ( options::model == "exponential_decay_sine" ) {
		mdl::exponential_decay_sine( vars );
	} else if ( options::model == "exponential_decay_sine_ND" ) {
		mdl::exponential_decay_sine_ND( vars );
	} else if ( options::model == "exponential_decay_step" ) {
		mdl::exponential_decay_step( vars );
	} else if ( options::model == "nonlinear" ) {
		mdl::nonlinear( vars );
	} else if ( options::model == "nonlinear_ND" ) {
		mdl::nonlinear_ND( vars );
	} else if ( options::model == "stiff" ) {
		mdl::stiff( vars );
	} else if ( ( options::model == "StateEvent6" ) || ( options::model == "stateevent6" ) ) {
		mdl::StateEvent6( vars, cons );
	} else if ( options::model == "xy" ) {
		mdl::xy( vars );
	} else if ( options::model == "xyz" ) {
		mdl::xyz( vars );
	} else if ( options::model == "gen" ) {
		mdl::gen( vars, cons );
	} else if ( options::model.substr( 0, 4 ) == "gen:" ) {
		std::istringstream n_stream( options::model.substr( 4 ) );
		std::string nQSS_string;
		std::string nZC_string;
		std::string seed_string;
		std::getline( n_stream, nQSS_string, ',' );
		std::getline( n_stream, nZC_string, ',' );
		std::getline( n_stream, seed_string, ',' );
		size_type nQSS( 9 );
		size_type nZC( 3 );
		size_type seed( 0 );
		bool do_seed( false );
		if ( is_size( nQSS_string ) ) {
			nQSS = size_of( nQSS_string );
		} else if ( ! nQSS_string.empty() ) {
			std::cerr << "\nError: gen model specifier not in gen:nQSS or gen:nQSS,nZC,SEED format: " << options::model << std::endl;
			std::exit( EXIT_FAILURE );
		}
		if ( is_size( nZC_string ) ) {
			nZC = size_of( nZC_string );
		} else if ( ! nZC_string.empty() ) {
			std::cerr << "\nError: gen model specifier not in gen:nQSS or gen:nQSS,nZC,SEED format: " << options::model << std::endl;
			std::exit( EXIT_FAILURE );
		}
		if ( is_size( seed_string ) ) {
			seed = size_of( seed_string );
			do_seed = true;
		} else if ( seed_string == "T" ) {
			do_seed = true;
		} else if ( seed_string == "F" ) {
			do_seed = false;
		} else if ( ! seed_string.empty() ) {
			std::cerr << "\nError: gen model specifier not in gen:nQSS or gen:nQSS,nZC,SEED format: " << options::model << std::endl;
			std::exit( EXIT_FAILURE );
		}
		mdl::gen( vars, cons, nQSS, nZC, seed, do_seed );
	} else {
		std::cerr << "\nError: Unknown model: " << options::model << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Size setup
	size_type const n_vars( vars.size() );

	// Variable-index map setup
	Var_Idx var_idx;
	for ( size_type i = 0; i < n_vars; ++i ) {
		var_idx[ vars[ i ] ] = i;
	}

	// Containers of ZC and non-ZC variables
	Variables vars_ZC;
	Variables vars_NZ;
	int max_QSS_order( 0 );
	for ( auto var : vars ) {
		if ( var->is_ZC() ) { // ZC variable
			vars_ZC.push_back( var );
		} else { // Non-ZC variable
			vars_NZ.push_back( var );
			max_QSS_order = std::max( max_QSS_order, var->order() ); // Max QSS order of non-ZC variables to avoid unnec loop stages
		}
	}
	int const QSS_order_max( max_QSS_order ); // Highest QSS order in use
	assert( QSS_order_max <= 3 );

	// Timing setup
	Time const t0( 0.0 ); // Simulation start time
	Time tE( options::tEnd ); // Simulation end time
	Time t( t0 ); // Simulation current time
	Time tOut( t0 + options::dtOut ); // Sampling time
	size_type iOut( 1u ); // Output step index
	Time const tSim( tE - t0 ); // Simulation time span expected
	int tPer( 0 ); // Percent of simulation time completed

	// Variable initialization
	std::cout << "\nInitialization =====" << std::endl;
	for ( auto var : vars ) {
		var->init_0();
	}
	for ( auto var : vars_NZ ) {
		var->init_1();
	}
	if ( QSS_order_max >= 2 ) {
		for ( auto var : vars_NZ ) {
			var->init_2();
		}
		if ( QSS_order_max >= 3 ) {
			for ( auto var : vars_NZ ) {
				var->init_3();
			}
		}
	}
	for ( auto var : vars_ZC ) {
		var->init();
	}

	// Dependency cycle detection: After init sets up observers
	if ( options::cycles ) cycles< Variable >( vars );

	// Output initialization
	bool const doSOut( options::output::s && ( options::output::x || options::output::q ) );
	bool const doTOut( options::output::t && ( options::output::x || options::output::q ) );
	bool const doROut( options::output::r && ( options::output::x || options::output::q ) );
	if ( ( options::output::t || options::output::r || options::output::s ) && ( options::output::x || options::output::q ) ) { // t0 outputs
		if ( options::output::x ) x_outs.reserve( vars.size() );
		if ( options::output::q ) q_outs.reserve( vars.size() );
		for ( auto var : vars ) {
			if ( options::output::x ) {
				x_outs.push_back( Output( var->name, 'x' ) );
				x_outs.back().append( t, var->x( t ) );
			}
			if ( options::output::q ) {
				q_outs.push_back( Output( var->name, 'q' ) );
				q_outs.back().append( t, var->q( t ) );
			}
		}
	}

	// Simulation loop
	std::cout << "\nSimulation Loop =====" << std::endl;
	size_type const max_pass_count_multiplier( 2 );
	size_type n_discrete_events( 0 );
	size_type n_QSS_events( 0 );
	size_type n_QSS_simultaneous_events( 0 );
	size_type n_ZC_events( 0 );
	double sim_dtMin( options::dtMin );
	bool pass_warned( false );
	Variables observers;
	std::clock_t const sim_time_beg( std::clock() ); // Simulation time
	while ( t <= tE ) {
		t = events.top_time();
		if ( doSOut ) { // Sampled outputs
			Time const tStop( std::min( t, tE ) );
			while ( tOut < tStop ) {
				for ( size_type i = 0; i < n_vars; ++i ) {
					if ( options::output::x ) x_outs[ i ].append( tOut, vars[ i ]->x( tOut ) );
					if ( options::output::q ) q_outs[ i ].append( tOut, vars[ i ]->q( tOut ) );
				}
				assert( iOut < std::numeric_limits< size_type >::max() );
				tOut = t0 + ( ++iOut ) * options::dtOut;
			}
		}
		if ( t <= tE ) { // Perform event(s)
			Event< Target > & event( events.top() );
			SuperdenseTime const s( events.top_superdense_time() );
			if ( s.i >= options::pass ) { // Pass count limit reached
				if ( s.i <= max_pass_count_multiplier * options::pass ) { // Use time step controls
					if ( sim_dtMin > 0.0 ) { // Double dtMin
						if ( sim_dtMin < std::min( 0.5 * infinity, 0.25 * options::dtMax ) ) {
							sim_dtMin = std::min( 2.0 * sim_dtMin, 0.5 * options::dtMax );
						} else {
							std::cerr << "\nError: Pass count limit exceeded at time: " << t << "  Min time step limit reached: Terminating simulation" << std::endl;
							tE = t; // To avoid tE outputs beyond actual simulation
							break;
						}
					} else { // Set dtMin
						sim_dtMin = std::min( std::max( 1.0e-9, tE * 1.0e-12 ), 0.5 * options::dtMax );
					}
					for ( auto var : vars ) {
						var->dt_min = sim_dtMin;
					}
					if ( ! pass_warned ) {
						std::cerr << "\nWarning: Pass count limit reached at time: " << t << "  Min time step control activated" << std::endl;
						pass_warned = true;
					}
				} else { // Time step control doesn't seem to be working: Abort
					std::cerr << "\nError: " << max_pass_count_multiplier << " x pass count limit exceeded at time: " << t << "  Terminating simulation" << std::endl;
					tE = t; // To avoid tE outputs beyond actual simulation
					break;
				}
			}
			events.set_active_time();
			if ( event.is_discrete() ) { // Discrete event
				++n_discrete_events;
				if ( events.single() ) { // Single trigger
					Variable * trigger( event.sub< Variable >() );
					assert( trigger->tD == t );

					trigger->st = s; // Set trigger superdense time

					if ( doTOut ) { // Time event output: before discrete changes
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
							}
						} else { // Time event and observer output
							if ( options::output::t ) { // Time event output
								size_type const i( var_idx[ trigger ] );
								if ( options::output::x ) x_outs[ i ].append( t, trigger->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, trigger->q( t ) );
								for ( Variable const * observer : trigger->observers() ) { // Observer output
									size_type const io( var_idx[ observer ] );
									if ( options::output::x ) x_outs[ io ].append( t, observer->x( t ) );
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										if ( options::output::q ) q_outs[ io ].append( t, observer->q( t ) );
									}
								}
							}
						}
					}

					trigger->advance_discrete();

					if ( doTOut ) { // Time event output: after discrete changes
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
							}
						} else { // Time event and observer output
							if ( options::output::t ) { // Time event output
								size_type const i( var_idx[ trigger ] );
								if ( options::output::x ) x_outs[ i ].append( t, trigger->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, trigger->q( t ) );
								for ( Variable const * observer : trigger->observers() ) { // Observer output
									size_type const io( var_idx[ observer ] );
									if ( options::output::x ) x_outs[ io ].append( t, observer->x( t ) );
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										if ( options::output::q ) q_outs[ io ].append( t, observer->q( t ) );
									}
								}
							}
						}
					}
				} else { // Simultaneous triggers
					Variables triggers( events.top_subs< Variable >() );
					variables_observers( triggers, observers );

					if ( doTOut ) { // Time event output: before discrete changes
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
							}
						} else { // Time event and observer output
							if ( options::output::t ) { // Time event output
								for ( Variable const * trigger : triggers ) { // Triggers
									size_type const i( var_idx[ trigger ] );
									if ( options::output::x ) x_outs[ i ].append( t, trigger->x( t ) );
									if ( options::output::q ) q_outs[ i ].append( t, trigger->q( t ) );
								}
								for ( Variable const * observer : observers ) { // Observer output
									size_type const io( var_idx[ observer ] );
									if ( options::output::x ) x_outs[ io ].append( t, observer->x( t ) );
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										if ( options::output::q ) q_outs[ io ].append( t, observer->q( t ) );
									}
								}
							}
						}
					}

					for ( Variable * trigger : triggers ) {
						assert( trigger->tD == t );
						trigger->st = s; // Set trigger superdense time
						trigger->advance_discrete();
					}

//					// If discrete events can change order 1+ QSS variables this block should be used instead
//					for ( Variable * trigger : triggers ) {
//						assert( trigger->tD == t );
//						trigger->st = s; // Set trigger superdense time
//						trigger->advance_discrete_0();
//					}
//					int const triggers_order_max( triggers.back()->order() );
//					if ( triggers_order_max >= 1 ) { // 1st order pass
//						for ( size_type i = begin_order_index( triggers, 1 ), n = triggers.size(); i < n; ++i ) {
//							triggers[ i ]->advance_discrete_1();
//						}
//						if ( triggers_order_max >= 2 ) { // 2nd order pass
//							for ( size_type i = begin_order_index( triggers, 2 ), n = triggers.size(); i < n; ++i ) {
//								triggers[ i ]->advance_discrete_2();
//							}
//							if ( triggers_order_max >= 3 ) { // 3rd order pass
//								for ( size_type i = begin_order_index( triggers, 3 ), n = triggers.size(); i < n; ++i ) {
//									triggers[ i ]->advance_discrete_3();
//								}
//							}
//						}
//					}

					Variable::advance_observers( observers, t );

					if ( doTOut ) { // Time event output: after discrete changes
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
							}
						} else { // Time event and observer output
							if ( options::output::t ) { // Time event output
								for ( Variable const * trigger : triggers ) { // Triggers
									size_type const i( var_idx[ trigger ] );
									if ( options::output::x ) x_outs[ i ].append( t, trigger->x( t ) );
									if ( options::output::q ) q_outs[ i ].append( t, trigger->q( t ) );
								}
								for ( Variable const * observer : observers ) { // Observer output
									size_type const io( var_idx[ observer ] );
									if ( options::output::x ) x_outs[ io ].append( t, observer->x( t ) );
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										if ( options::output::q ) q_outs[ io ].append( t, observer->q( t ) );
									}
								}
							}
						}
					}
				}
			} else if ( event.is_ZC() ) { // Zero-crossing event
				++n_ZC_events;
				while ( events.top_superdense_time() == s ) {
					Variable * trigger( events.top_sub< Variable >() );
					assert( trigger->tZC() == t );
					trigger->st = s; // Set trigger superdense time
					trigger->advance_ZC();
					if ( doTOut ) { // Time event output
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
							}
						} else { // Time event and observer output
							if ( options::output::t ) { // Time event output
								size_type const i( var_idx[ trigger ] );
								if ( options::output::x ) x_outs[ i ].append( t, trigger->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, trigger->q( t ) );
							}
						}
					}
				}
			} else if ( event.is_conditional() ) { // Conditional event
				while ( events.top_superdense_time() == s ) {
					Conditional * trigger( events.top_sub< Conditional >() );
					trigger->st = s; // Set trigger superdense time
					trigger->advance_conditional();
				}
			} else if ( event.is_handler() ) { // Zero-crossing handler event
				if ( events.single() ) { // Single handler
					Variable * handler( event.sub< Variable >() );

					if ( doROut ) { // Requantization output: before handler changes
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
							}
						} else { // Requantization and observer output
							if ( options::output::r ) { // Requantization output
								size_type const i( var_idx[ handler ] );
								if ( options::output::x ) x_outs[ i ].append( t, handler->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, handler->q( t ) );
								for ( Variable const * observer : handler->observers() ) { // Observer output
									size_type const io( var_idx[ observer ] );
									if ( options::output::x ) x_outs[ io ].append( t, observer->x( t ) );
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										if ( options::output::q ) q_outs[ io ].append( t, observer->q( t ) );
									}
								}
							}
						}
					}

					handler->advance_handler( t, event.val() );

					if ( doROut ) { // Requantization output: after handler changes
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
							}
						} else { // Requantization and observer output
							if ( options::output::r ) { // Requantization output
								size_type const i( var_idx[ handler ] );
								if ( options::output::x ) x_outs[ i ].append( t, handler->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, handler->q( t ) );
								for ( Variable const * observer : handler->observers() ) { // Observer output
									size_type const io( var_idx[ observer ] );
									if ( options::output::x ) x_outs[ io ].append( t, observer->x( t ) );
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										if ( options::output::q ) q_outs[ io ].append( t, observer->q( t ) );
									}
								}
							}
						}
					}
				} else { // Simultaneous handlers
					Variables handlers( events.top_subs< Variable >() );
					variables_observers( handlers, observers );
					int const handlers_order_max( handlers.empty() ? 0 : handlers.back()->order() );

					if ( doROut ) { // Requantization output: before handler changes
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
							}
						} else { // Requantization and observer output
							if ( options::output::r ) { // Requantization output
								for ( Variable const * handler : handlers ) { // Handlers
									size_type const i( var_idx[ handler ] );
									if ( options::output::x ) x_outs[ i ].append( t, handler->x( t ) );
									if ( options::output::q ) q_outs[ i ].append( t, handler->q( t ) );
								}
								for ( Variable const * observer : observers ) { // Observer output
									size_type const io( var_idx[ observer ] );
									if ( options::output::x ) x_outs[ io ].append( t, observer->x( t ) );
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										if ( options::output::q ) q_outs[ io ].append( t, observer->q( t ) );
									}
								}
							}
						}
					}

					for ( auto & e : events.top_events() ) {
						e.sub< Variable >()->advance_handler_0( t, e.val() );
					}
					if ( handlers_order_max >= 1 ) { // 1st order pass
						for ( size_type i = begin_order_index( handlers, 1 ), n = handlers.size(); i < n; ++i ) {
							handlers[ i ]->advance_handler_1();
						}
						if ( handlers_order_max >= 2 ) { // 2nd order pass
							for ( size_type i = begin_order_index( handlers, 2 ), n = handlers.size(); i < n; ++i ) {
								handlers[ i ]->advance_handler_2();
							}
							if ( handlers_order_max >= 3 ) { // 3rd order pass
								for ( size_type i = begin_order_index( handlers, 3 ), n = handlers.size(); i < n; ++i ) {
									handlers[ i ]->advance_handler_3();
								}
							}
						}
					}

					Variable::advance_observers( observers, t );

					if ( doROut ) { // Requantization output: after handler changes
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
							}
						} else { // Requantization and observer output
							if ( options::output::r ) { // Requantization output
								for ( Variable const * handler : handlers ) { // Handlers
									size_type const i( var_idx[ handler ] );
									if ( options::output::x ) x_outs[ i ].append( t, handler->x( t ) );
									if ( options::output::q ) q_outs[ i ].append( t, handler->q( t ) );
								}
								for ( Variable const * observer : observers ) { // Observer output
									size_type const io( var_idx[ observer ] );
									if ( options::output::x ) x_outs[ io ].append( t, observer->x( t ) );
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										if ( options::output::q ) q_outs[ io ].append( t, observer->q( t ) );
									}
								}
							}
						}
					}
				}
			} else if ( event.is_QSS() ) { // QSS requantization event
				++n_QSS_events;
				if ( events.single() ) { // Single trigger
					Variable * trigger( event.sub< Variable >() );
					assert( trigger->tE == t );
					assert( ! trigger->is_ZC() ); // ZC variable requantizations are QSS_ZC events
					trigger->st = s; // Set trigger superdense time

					if ( doROut ) { // Requantization output: Quantized rep before to capture its discrete change
						if ( ( options::output::a ) || ( options::output::r ) ) { // Requantization output
							if ( options::output::q ) {
								size_type const i( var_idx[ trigger ] );
								q_outs[ i ].append( t, trigger->q( t ) );
							}
						}
					}

					trigger->advance_QSS();

					if ( doROut ) { // Requantization output
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
							}
						} else { // Requantization and observer output
							if ( options::output::r ) { // Requantization output
								size_type const i( var_idx[ trigger ] );
								if ( options::output::x ) x_outs[ i ].append( t, trigger->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, trigger->q( t ) );
								for ( Variable const * observer : trigger->observers() ) { // Observer output
									size_type const io( var_idx[ observer ] );
									if ( options::output::x ) x_outs[ io ].append( t, observer->x( t ) );
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										if ( options::output::q ) q_outs[ io ].append( t, observer->q( t ) );
									}
								}
							}
						}
					}
				} else { // Simultaneous triggers
					++n_QSS_simultaneous_events;
					Variables triggers( events.top_subs< Variable >() );
					variables_observers( triggers, observers );

					if ( doROut ) { // Requantization output: Quantized rep before to capture its discrete change
						if ( ( options::output::a ) || ( options::output::r ) ) { // Requantization output
							if ( options::output::q ) {
								for ( Variable const * trigger : triggers ) { // Triggers
									size_type const i( var_idx[ trigger ] );
									q_outs[ i ].append( t, trigger->q( t ) );
								}
							}
						}
					}

					for ( Variable * trigger : triggers ) {
						assert( trigger->tE == t );
						assert( ! trigger->is_ZC() ); // ZC variable requantizations are QSS_ZC events
						trigger->st = s; // Set trigger superdense time
						trigger->advance_QSS_0();
					}
					for ( Variable * trigger : triggers ) {
						trigger->advance_QSS_1();
					}
					int const triggers_order_max( triggers.back()->order() );
					if ( triggers_order_max >= 2 ) { // 2nd order pass
						for ( size_type i = begin_order_index( triggers, 2 ), n = triggers.size(); i < n; ++i ) {
							triggers[ i ]->advance_QSS_2();
						}
						if ( triggers_order_max >= 3 ) { // 3rd order pass
							for ( size_type i = begin_order_index( triggers, 3 ), n = triggers.size(); i < n; ++i ) {
								triggers[ i ]->advance_QSS_3();
							}
						}
					}

					Variable::advance_observers( observers, t );

					if ( doROut ) { // Requantization output
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
								if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
							}
						} else { // Requantization and observer output
							if ( options::output::r ) { // Requantization output
								for ( Variable const * trigger : triggers ) { // Triggers
									size_type const i( var_idx[ trigger ] );
									if ( options::output::x ) x_outs[ i ].append( t, trigger->x( t ) );
									if ( options::output::q ) q_outs[ i ].append( t, trigger->q( t ) );
								}
								for ( Variable const * observer : observers ) { // Observer output
									size_type const io( var_idx[ observer ] );
									if ( options::output::x ) x_outs[ io ].append( t, observer->x( t ) );
									if ( observer->is_ZC() ) { // Zero-crossing variables requantize in observer advance
										if ( options::output::q ) q_outs[ io ].append( t, observer->q( t ) );
									}
								}
							}
						}
					}
				}
			} else if ( event.is_QSS_ZC() ) { // QSS ZC requantization event
				++n_QSS_events;
				Variable * trigger( event.sub< Variable >() );
				assert( trigger->tE == t );
				assert( trigger->is_ZC() ); // ZC variable requantizations are QSS_ZC events
				trigger->st = s; // Set trigger superdense time

				trigger->advance_QSS();

				if ( doROut ) { // Requantization output
					if ( options::output::a ) { // All variables output
						for ( size_type i = 0; i < n_vars; ++i ) {
							if ( options::output::x ) x_outs[ i ].append( t, vars[ i ]->x( t ) );
							if ( options::output::q ) q_outs[ i ].append( t, vars[ i ]->q( t ) );
						}
					} else { // Requantization output
						if ( options::output::r ) { // Requantization output
							size_type const i( var_idx[ trigger ] );
							if ( options::output::x ) x_outs[ i ].append( t, trigger->x( t ) );
							if ( options::output::q ) q_outs[ i ].append( t, trigger->q( t ) );
						}
					}
				}
			} else { // Unsupported event
				assert( false );
			}
		}
		int const tPerNow( static_cast< int >( 100 * ( t - t0 ) / tSim ) );
		if ( tPerNow > tPer ) { // Report % complete
			tPer = tPerNow;
			std::cout << '\r' << std::setw( 3 ) << tPer << "% complete" << std::flush;
		}
	}
	std::clock_t const sim_time_end( std::clock() ); // Simulation time
	std::cout << "\r  " << std::setw( 3 ) << 100 << "% complete" << std::endl;

	// End time outputs
	if ( ( options::output::r || options::output::s ) && ( options::output::x || options::output::q ) ) {
		for ( size_type i = 0; i < n_vars; ++i ) {
			Variable const * var( vars[ i ] );
			if ( var->tQ < tE ) {
				if ( options::output::x ) {
					x_outs[ i ].append( tE, var->x( tE ) );
				}
				if ( options::output::q ) {
					q_outs[ i ].append( tE, var->q( tE ) );
				}
			}
		}
	}

	// Reporting
	std::cout << "\nSimulation Complete =====" << std::endl;
	if ( n_discrete_events > 0 ) std::cout << n_discrete_events << " discrete event passes" << std::endl;
	if ( n_QSS_events > 0 ) std::cout << n_QSS_events << " requantization event passes" << std::endl;
	if ( n_QSS_simultaneous_events > 0 ) std::cout << n_QSS_simultaneous_events << " simultaneous requantization event passes" << std::endl;
	if ( n_ZC_events > 0 ) std::cout << n_ZC_events << " zero-crossing event passes" << std::endl;
	std::cout << "Simulation CPU time: " << double( sim_time_end - sim_time_beg ) / CLOCKS_PER_SEC << " (s)" << std::endl; // Simulation time

	// QSS cleanup
	for ( auto & var : vars ) delete var;
	for ( auto & con : cons ) delete con;
}

} // cod
} // QSS
