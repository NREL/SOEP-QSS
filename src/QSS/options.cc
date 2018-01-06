// Options Support
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017 Objexx Engineerinc, Inc. All rights reserved.
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
#include <QSS/options.hh>
#include <QSS/math.hh>
#include <QSS/string.hh>

// C++ Headers
#include <iostream>
#include <limits>

namespace QSS {
namespace options {

QSS qss( QSS::QSS2 ); // QSS method: (LI)QSS1|2|3
int qss_order( 2 ); // QSS method order  [computed]
bool cycles( false ); // Report dependency cycles?
bool inflection( false ); // Requantize at inflections?
bool refine( false ); // Refine FMU zero-crossing roots?
double rTol( 1.0e-4 ); // Relative tolerance  [1e-4|FMU]
bool rTol_set( false ); // Relative tolerance set?
double aTol( 1.0e-6 ); // Absolute tolerance
double zTol( 0.0 ); // Zero-crossing anti-chatter tolerance
double dtMin( 0.0 ); // Min time step (s)
double dtMax( std::numeric_limits< double >::has_infinity ? std::numeric_limits< double >::infinity() : std::numeric_limits< double >::max() ); // Max time step (s)
double dtInf( std::numeric_limits< double >::has_infinity ? std::numeric_limits< double >::infinity() : std::numeric_limits< double >::max() ); // Inf time step (s)
double dtZC( 1.0e-9 ); // FMU zero-crossing time step (s)
double dtNum( 1.0e-6 ); // Numeric differentiation time step (s)
double one_over_dtNum( 1.0e6 ); // 1 / dtNum  [computed]
double one_half_over_dtNum( 5.0e5 ); // 0.5 / dtNum  [computed]
double dtOut( 1.0e-3 ); // Sampled & FMU output time step (s)
double tEnd( 1.0 ); // End time (s)  [1|FMU]
bool tEnd_set( false ); // End time set?
std::size_t pass( 20 ); // Pass count limit
std::string out; // Outputs
std::string model; // Model|FMU name

namespace output { // Output selections

bool t( true ); // Time events?  [T]
bool r( true ); // Requantizations?  [T]
bool o( false ); // Observers?  [F]
bool a( false ); // All variables?  [F]
bool s( false ); // Sampled output?  [F]
bool f( true ); // FMU outputs?  [T]
bool x( true ); // Continuous trajectories?  [T]
bool q( false ); // Quantized trajectories?  [F]
bool d( false ); // Diagnostic output?  [F]

} // output

// Help Display
void
help_display()
{
	std::cout << '\n' << "QSS [options] [model|fmu]" << "\n\n";
	std::cout << "Options:" << "\n\n";
	std::cout << " --qss=METHOD  QSS method: (LI)QSS1|2|3  [QSS2]" << '\n';
	std::cout << " --cycles      Report dependency cycles?  [F]" << '\n';
	std::cout << " --inflection  Requantize at inflections?  [F]" << '\n';
	std::cout << " --refine      Refine FMU zero-crossing roots?  [F]" << '\n';
	std::cout << " --rTol=TOL    Relative tolerance  [1e-4|FMU]" << '\n';
	std::cout << " --aTol=TOL    Absolute tolerance  [1e-6]" << '\n';
	std::cout << " --zTol=TOL    Zero-crossing anti-chatter tolerance  [0]" << '\n';
	std::cout << " --dtMin=STEP  Min time step (s)  [0]" << '\n';
	std::cout << " --dtMax=STEP  Max time step (s)  [infinity]" << '\n';
	std::cout << " --dtInf=STEP  Inf alt time step (s)  [infinity]" << '\n';
	std::cout << " --dtZC=STEP   FMU zero-crossing step (s)  [1e-9]" << '\n';
	std::cout << " --dtNum=STEP  Numeric differentiation step (s)  [1e-6]" << '\n';
	std::cout << " --dtOut=STEP  Sampled & FMU output step (s)  [1e-3]" << '\n';
	std::cout << " --tEnd=TIME   End time (s)  [1|FMU]" << '\n';
	std::cout << " --pass=COUNT  Pass count limit  [20]" << '\n';
	std::cout << " --out=OUTPUTS Outputs  [trfx]" << '\n';
	std::cout << "       t       Time events" << '\n';
	std::cout << "       r       Requantizations" << '\n';
	std::cout << "       o       Observers" << '\n';
	std::cout << "       a       All variables" << '\n';
	std::cout << "       s       Sampled time steps" << '\n';
	std::cout << "       f       FMU outputs" << '\n';
	std::cout << "       x       Continuous trajectories" << '\n';
	std::cout << "       q       Quantized trajectories" << '\n';
	std::cout << "       d       Diagnostic output" << '\n';
	std::cout << '\n';
	std::cout << "Models:" << "\n\n";
	std::cout << "  achilles : Achilles and the Tortoise" << '\n';
	std::cout << "  achilles2 : Adds symmetry for simultaneous triggering" << '\n';
	std::cout << "  achillesc : Custom functions demo " << '\n';
	std::cout << "  achilles_ND : Numeric differentiation" << '\n';
	std::cout << "  bball : Bouncing ball (discrete events)" << '\n';
	std::cout << "  exponential_decay : Exponential decay" << '\n';
	std::cout << "  exponential_decay_sine : Adds sine input function" << '\n';
	std::cout << "  exponential_decay_sine_ND : Numeric differentiation" << '\n';
	std::cout << "  exponential_decay_step : Adds step input function" << '\n';
	std::cout << "  nonlinear : Nonlinear derivative demo" << '\n';
	std::cout << "  nonlinear_ND : Numeric differentiation" << '\n';
	std::cout << "  StateEvent6 : Zero-crossing model (use --dtInf)" << '\n';
	std::cout << "  stiff : Stiff system from literature" << '\n';
	std::cout << "  xy : Simple 2 variable model" << '\n';
	std::cout << "  xyz : Simple 3 variable model" << '\n';
	std::cout << "  gen:nQSS,nZC,SEED : Generated model with optional specs:" << '\n';
	std::cout << "      nQSS            Number of QSS variables  [9]" << '\n';
	std::cout << "           nZC        Number of zero-crossing variables  [3]" << '\n';
	std::cout << "               SEED   Random generator seed (T|F|#)  [F]" << '\n';
	std::cout << "                       T: Use current time as seed" << '\n';
	std::cout << "                       F: Use default (deterministic) seed" << '\n';
	std::cout << "                       #: Integer to use as seed" << '\n';
	std::cout << '\n';
}

// Process command line arguments
void
process_args( int argc, char * argv[] )
{
	bool help( false );
	if ( argc == 1 ) { // No arguments: Show help
		help_display();
		help = true;
	}
	bool fatal( false ); // Fatal error occurred?
	for ( int i = 1; i < argc; ++i ) {
		std::string const arg( argv[ i ] );
		if ( ( arg == "--help" ) || ( arg == "-h" ) ) { // Show help
			help_display();
			help = true;
		} else if ( has_value_option( arg, "qss" ) || has_value_option( arg, "QSS" ) ) {
			std::string const qss_name( uppercased( arg_value( arg ) ) );
			if ( qss_name == "QSS1" ) {
				qss = QSS::QSS1;
				qss_order = 1;
			} else if ( qss_name == "QSS2" ) {
				qss = QSS::QSS2;
				qss_order = 2;
			} else if ( qss_name == "QSS3" ) {
				qss = QSS::QSS3;
				qss_order = 3;
			} else if ( qss_name == "LIQSS1" ) {
				qss = QSS::LIQSS1;
				qss_order = 1;
			} else if ( qss_name == "LIQSS2" ) {
				qss = QSS::LIQSS2;
				qss_order = 2;
			} else if ( qss_name == "LIQSS3" ) {
				qss = QSS::LIQSS3;
				qss_order = 3;
				std::cerr << "Error: The LIQSS3 method is not yet implemented" << std::endl;
				fatal = true;
			} else {
				std::cerr << "Error: Unsupported QSS method: " << qss_name << std::endl;
				fatal = true;
			}
		} else if ( has_option( arg, "cycles" ) ) {
			cycles = true;
		} else if ( has_option( arg, "inflection" ) ) {
			inflection = true;
		} else if ( has_option( arg, "refine" ) ) {
			refine = true;
		} else if ( has_value_option( arg, "rTol" ) ) {
			std::string const rTol_str( arg_value( arg ) );
			if ( is_double( rTol_str ) ) {
				rTol = double_of( rTol_str );
				if ( rTol < 0.0 ) {
					std::cerr << "Error: Negative rTol: " << rTol_str << std::endl;
					fatal = true;
				}
				rTol_set = true;
			} else {
				std::cerr << "Error: Nonnumeric rTol: " << rTol_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "aTol" ) ) {
			std::string const aTol_str( arg_value( arg ) );
			if ( is_double( aTol_str ) ) {
				aTol = double_of( aTol_str );
				if ( aTol == 0.0 ) {
					aTol = std::numeric_limits< double >::min();
					std::cerr << "Warning: aTol set to: " << aTol << std::endl;
				} else if ( aTol < 0.0 ) {
					std::cerr << "Error: Negative aTol: " << aTol_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "Error: Nonnumeric aTol: " << aTol_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "zTol" ) ) {
			std::string const zTol_str( arg_value( arg ) );
			if ( is_double( zTol_str ) ) {
				zTol = double_of( zTol_str );
				if ( zTol < 0.0 ) {
					std::cerr << "Error: Negative zTol: " << zTol_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "Error: Nonnumeric zTol: " << zTol_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "dtMin" ) ) {
			std::string const dtMin_str( arg_value( arg ) );
			if ( is_double( dtMin_str ) ) {
				dtMin = double_of( dtMin_str );
				if ( dtMin < 0.0 ) {
					std::cerr << "Error: Negative dtMin: " << dtMin_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "Error: Nonnumeric dtMin: " << dtMin_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "dtMax" ) ) {
			std::string const dtMax_str( arg_value( arg ) );
			if ( is_double( dtMax_str ) ) {
				dtMax = double_of( dtMax_str );
				if ( dtMax < 0.0 ) {
					std::cerr << "Error: Negative dtMax: " << dtMax_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "Error: Nonnumeric dtMax: " << dtMax_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "dtInf" ) ) {
			std::string const dtInf_str( arg_value( arg ) );
			if ( is_double( dtInf_str ) ) {
				dtInf = double_of( dtInf_str );
				if ( dtInf < 0.0 ) {
					std::cerr << "Error: Negative dtInf: " << dtInf_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "Error: Nonnumeric dtInf: " << dtInf_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "dtZC" ) ) {
			std::string const dtZC_str( arg_value( arg ) );
			if ( is_double( dtZC_str ) ) {
				dtZC = double_of( dtZC_str );
				if ( dtZC < 0.0 ) {
					std::cerr << "Error: Negative dtZC: " << dtZC_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "Error: Nonnumeric dtZC: " << dtZC_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "dtNum" ) ) {
			std::string const dtNum_str( arg_value( arg ) );
			if ( is_double( dtNum_str ) ) {
				dtNum = double_of( dtNum_str );
				if ( dtNum < 0.0 ) {
					std::cerr << "Error: Negative dtNum: " << dtNum_str << std::endl;
					fatal = true;
				}
				one_over_dtNum = 1.0 / dtNum;
				one_half_over_dtNum = 0.5 / dtNum;
			} else {
				std::cerr << "Error: Nonnumeric dtNum: " << dtNum_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "dtOut" ) ) {
			std::string const dtOut_str( arg_value( arg ) );
			if ( is_double( dtOut_str ) ) {
				dtOut = double_of( dtOut_str );
				if ( dtOut < 0.0 ) {
					std::cerr << "Error: Negative dtOut: " << dtOut_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "Error: Nonnumeric dtOut: " << dtOut_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "tEnd" ) ) {
			std::string const tEnd_str( arg_value( arg ) );
			if ( is_double( tEnd_str ) ) {
				tEnd = double_of( tEnd_str );
				if ( tEnd < 0.0 ) {
					std::cerr << "Error: Negative tEnd: " << tEnd_str << std::endl;
					fatal = true;
				}
				tEnd_set = true;
			} else {
				std::cerr << "Error: Nonnumeric tEnd: " << tEnd_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "pass" ) ) {
			std::string const pass_str( arg_value( arg ) );
			if ( is_size( pass_str ) ) {
				std::size_t const pass( size_of( pass_str ) );
				if ( pass < 1 ) {
					std::cerr << "Error: Nonpositive pass: " << pass_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "Error: Nonintegral pass: " << pass_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "out" ) ) {
			out = arg_value( arg );
			if ( has_any_not_of( out, "troasfxqd" ) ) {
				std::cerr << "Error: Output flag not in troasfxqd: " << out << std::endl;
				fatal = true;
			}
			output::t = has( out, 't' );
			output::r = has( out, 'r' );
			output::o = has( out, 'o' );
			output::a = has( out, 'a' );
			output::s = has( out, 's' );
			output::f = has( out, 'f' );
			output::x = has( out, 'x' );
			output::q = has( out, 'q' );
			output::d = has( out, 'd' );
			if ( output::a ) output::o = true; // a => o
		} else if ( arg[ 0 ] == '-' ) {
			std::cerr << "Error: Unsupported option: " << arg << std::endl;
			fatal = true;
		} else { // Treat non-option argument as model
			model = arg;
		}
		if ( ( dtMax != infinity ) && ( dtInf != infinity ) ) {
			std::cerr << "Warning: dtInf has no effect when dtMax is specified" << std::endl;
		}
	}

	if ( help ) std::exit( EXIT_SUCCESS );
	if ( fatal ) std::exit( EXIT_FAILURE );
}

} // options
} // QSS
