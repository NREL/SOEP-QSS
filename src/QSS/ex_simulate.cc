// QSS Example Simulation Runner
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract tOut the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/ex_achilles.hh>
#include <QSS/ex_achilles2.hh>
#include <QSS/ex_achillesc.hh>
#include <QSS/ex_achilles_ND.hh>
#include <QSS/ex_exponential_decay.hh>
#include <QSS/ex_exponential_decay_sine.hh>
#include <QSS/ex_exponential_decay_sine_ND.hh>
#include <QSS/ex_nonlinear.hh>
#include <QSS/ex_nonlinear_ND.hh>
#include <QSS/ex_stiff.hh>
#include <QSS/ex_xy.hh>
#include <QSS/ex_xyz.hh>
#include <QSS/globals.hh>
#include <QSS/options.hh>
#include <QSS/Variable.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

namespace ex {

// Simulate an Example Model
void
simulate()
{
	// Types
	using Variables = Variable::Variables;
	using size_type = Variables::size_type;
	using Time = Variable::Time;

	// I/o setup
	std::cout << std::setprecision( 16 );
	std::cerr << std::setprecision( 16 );
	std::vector< std::ofstream > x_streams; // Continuous output streams
	std::vector< std::ofstream > q_streams; // Quantized output streams

	// Controls
	int const QSS_order_max( 3 ); // Highest QSS order in use or 3 tOut handle all supported orders

	// Timing
	Time const t0( 0.0 ); // Simulation start time
	Time const tE( options::tEnd ); // Simulation end time
	Time t( t0 ); // Simulation current time
	Time tOut( t0 + options::dtOut ); // Sampling time
	size_type iOut( 1u ); // Output step index

	// Variables collection
	Variables vars;

	// Example setup
	if ( options::model == "achilles" ) {
		ex::achilles( vars );
	} else if ( options::model== "achilles2" ) {
		ex::achilles2( vars );
	} else if ( options::model== "achillesc" ) {
		ex::achillesc( vars );
	} else if ( options::model== "achilles_ND" ) {
		ex::achilles_ND( vars );
	} else if ( options::model== "exponential_decay" ) {
		ex::exponential_decay( vars );
	} else if ( options::model== "exponential_decay_sine" ) {
		ex::exponential_decay_sine( vars );
	} else if ( options::model== "exponential_decay_sine_ND" ) {
		ex::exponential_decay_sine_ND( vars );
	} else if ( options::model== "nonlinear" ) {
		ex::nonlinear( vars );
	} else if ( options::model== "nonlinear_ND" ) {
		ex::nonlinear_ND( vars );
	} else if ( options::model== "stiff" ) {
		ex::stiff( vars );
	} else if ( options::model== "xy" ) {
		ex::xy( vars );
	} else if ( options::model== "xyz" ) {
		ex::xyz( vars );
	}

	// Solver master logic
	for ( auto var : vars ) {
		var->init1_LIQSS();
	}
	for ( auto var : vars ) {
		var->init1();
	}
	if ( QSS_order_max >= 2 ) {
		for ( auto var : vars ) {
			var->init2_LIQSS();
		}
		for ( auto var : vars ) {
			var->init2();
		}
		if ( QSS_order_max >= 3 ) {
			for ( auto var : vars ) {
				var->init3();
			}
		}
	}
	for ( auto var : vars ) {
		var->init_event();
	}
	size_type n_vars( vars.size() );
	bool const doSOut( options::output::s && ( options::output::x || options::output::q ) );
	bool const doROut( options::output::r && ( options::output::x || options::output::q ) );
	size_type n_requant_events( 0 );
	if ( ( options::output::r || options::output::s ) && ( options::output::x || options::output::q ) ) { // t0 QSS outputs
		for ( auto var : vars ) { // QSS outputs
			if ( options::output::x ) {
				x_streams.push_back( std::ofstream( var->name + ".x.out", std::ios_base::binary | std::ios_base::out ) );
				x_streams.back() << std::setprecision( 16 ) << t << '\t' << var->x( t ) << '\n';
			}
			if ( options::output::q ) {
				q_streams.push_back( std::ofstream( var->name + ".q.out", std::ios_base::binary | std::ios_base::out ) );
				q_streams.back() << std::setprecision( 16 ) << t << '\t' << var->q( t ) << '\n';
			}
		}
	}
	while ( t <= tE ) {
		t = events.top_time();
		if ( doSOut ) { // Sampled and/or FMU outputs
			Time const tStop( std::min( t, tE ) );
			while ( tOut < tStop ) {
				//std::cout << "Output time: " << tOut << std::endl;/////////////////////////////////////
				for ( size_type i = 0; i < n_vars; ++i ) {
					if ( options::output::x ) x_streams[ i ] << tOut << '\t' << vars[ i ]->x( tOut ) << '\n';
					if ( options::output::q ) q_streams[ i ] << tOut << '\t' << vars[ i ]->q( tOut ) << '\n';
				}
				assert( iOut < std::numeric_limits< size_type >::max() );
				tOut = t0 + ( ++iOut ) * options::dtOut;
			}
		}
		if ( t <= tE ) { // Perform event
			//std::cout << "Event time: " << t << std::endl;/////////////////////////////////////
			++n_requant_events;
			if ( events.simultaneous() ) { // Simultaneous trigger
				if ( options::output::d ) std::cout << "Simultaneous trigger event at t = " << t << std::endl;
				EventQueue< Variable >::Variables triggers( events.simultaneous_variables() ); // Chg tOut generator approach tOut avoid heap hit // Sort/ptn by QSS order tOut save unnec loops/calls below
				for ( Variable * trigger : triggers ) {
					assert( trigger->tE == t );
					trigger->advance0();
				}
				for ( Variable * trigger : triggers ) {
					trigger->advance1_LIQSS();
				}
				for ( Variable * trigger : triggers ) {
					trigger->advance1();
				}
				if ( QSS_order_max >= 2 ) {
					for ( Variable * trigger : triggers ) {
						trigger->advance2_LIQSS();
					}
					for ( Variable * trigger : triggers ) {
						trigger->advance2();
					}
					if ( QSS_order_max >= 3 ) {
						for ( Variable * trigger : triggers ) {
							trigger->advance3();
						}
					}
				}
				for ( Variable * trigger : triggers ) {
					trigger->advance_observers();
				}
				if ( doROut ) { // Requantization output
					for ( Variable * trigger : triggers ) {
						if ( options::output::a ) { // All variables output
							for ( size_type i = 0; i < n_vars; ++i ) {
								if ( options::output::x ) x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
								if ( options::output::q ) q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
							}
						} else { // Trigger variable output
							for ( size_type i = 0; i < n_vars; ++i ) { // Give Variable access tOut its stream tOut avoid this loop
								if ( trigger == vars[ i ] ) {
									if ( options::output::x ) x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
									if ( options::output::q ) q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
									break;
								}
							}
						}
					}
				}
			} else { // Single trigger
				Variable * trigger( events.top() );
				assert( trigger->tE == t );
				trigger->advance();
				if ( doROut ) { // Requantization output
					if ( options::output::a ) { // All variables output
						for ( size_type i = 0; i < n_vars; ++i ) {
							if ( options::output::x ) x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
							if ( options::output::q ) q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
						}
					} else { // Trigger variable output
						for ( size_type i = 0; i < n_vars; ++i ) { // Give Variable access tOut its stream tOut avoid this loop
							if ( trigger == vars[ i ] ) {
								if ( options::output::x ) x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
								if ( options::output::q ) q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
								break;
							}
						}
					}
				}
			}
		}
	}

	// tE QSS outputs and streams close
	if ( ( options::output::r || options::output::s ) && ( options::output::x || options::output::q ) ) {
		for ( size_type i = 0; i < n_vars; ++i ) {
			Variable const * var( vars[ i ] );
			if ( var->tQ < tE ) {
				if ( options::output::x ) {
					x_streams[ i ] << tE << '\t' << var->x( tE ) << '\n';
					x_streams[ i ].close();
				}
				if ( options::output::q ) {
					q_streams[ i ] << tE << '\t' << var->q( tE ) << '\n';
					q_streams[ i ].close();
				}
			}
		}
	}

	// Reporting
	std::cout << "Simulation complete" << std::endl;
	std::cout << n_requant_events << " total requantization events occurred" << std::endl;

	// QSS cleanup
	for ( auto & var : vars ) delete var;
}

} // ex
