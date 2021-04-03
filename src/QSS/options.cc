// QSS Options Support
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

// QSS Headers
#include <QSS/options.hh>
#include <QSS/math.hh>
#include <QSS/path.hh>
#include <QSS/string.hh>

// C++ Headers
#include <iostream>
#include <limits>
#include <regex>

namespace QSS {
namespace options {

QSS qss( QSS::QSS2 ); // QSS method: (x)(LI)QSS(1|2|3)
bool eidd( true ); // Use event indicator directional derivatives?
double rTol( 1.0e-4 ); // Relative tolerance
double aTol( 1.0e-6 ); // Absolute tolerance
double aFac( 0.01 ); // Absolute tolerance factor
double zTol( 1.0e-6 ); // Zero-crossing/root tolerance
double zMul( 10.0 ); // Zero-crossing tolerance bump multiplier
double zFac( 1.0 ); // Zero-crossing tolerance factor
double zrFac( 10.0 ); // Zero-crossing relative tolerance factor
double zaFac( 0.1 ); // Zero-crossing absolute tolerance factor
double dtMin( 0.0 ); // Min time step (s)
double dtMax( std::numeric_limits< double >::has_infinity ? std::numeric_limits< double >::infinity() : std::numeric_limits< double >::max() ); // Max time step (s)
double dtInf( std::numeric_limits< double >::has_infinity ? std::numeric_limits< double >::infinity() : std::numeric_limits< double >::max() ); // Inf time step (s)
double dtZMax( 0.01 ); // Max time step before zero-crossing (s)
double dtZC( 1.0e-9 ); // FMU zero-crossing time step (s)
double dtND( 1.0e-6 ); // Numeric differentiation time step (s)
double two_dtND( 2.0e-6 ); // 2 * dtND
double three_dtND( 3.0e-6 ); // 3 * dtND
double one_over_dtND( 1.0e6 ); // 1 / dtND
double one_over_two_dtND( 5.0e5 ); // 1 / ( 2 * dtND )
double one_over_two_dtND_squared( 5.0e11 ); // 1 / ( 2 * dtND^2 )
double one_over_four_dtND( 2.5e5 ); // 1 / ( 4 * dtND )
double one_over_six_dtND( 1.0e6 / 6.0 ); // 1 / ( 6 * dtND )
double one_over_six_dtND_squared( 1.0e12 / 6.0 ); // 1 / ( 6 * dtND^2 )
double one_over_six_dtND_cubed( 1.0e18 / 6.0 ); // 1 / ( 6 * dtND^3 )
double dtCon( 0.0 ); // FMU connection sync time step (s)
double dtOut( 1.0e-3 ); // Sampled output time step (s)
double tEnd( 1.0 ); // End time (s)  [1|FMU]
std::size_t bin_size( 1u ); // Bin size max
double bin_frac( 0.25 ); // Bin step fraction min
bool bin_auto( false ); // Bin size automaically optimized?
std::size_t pass( 20 ); // Pass count limit
bool cycles( false ); // Report dependency cycles?
bool inflection( false ); // Requantize at inflections?
bool refine( false ); // Refine FMU zero-crossing roots?
bool prune( false ); // Prune variables with no observers?
bool perfect( false ); // Perfect FMU-ME connection sync?
bool steps( false ); // Generate requantization step count file?
LogLevel log( LogLevel::warning ); // Logging level
InpFxn fxn; // Map from input variables to function specs
InpOut con; // Map from input variables to output variables
Depends dep; // Dependencies
Depends rdep; // Reverse dependencies
std::string out; // Outputs
std::pair< double, double > tLoc( 0.0, 0.0 ); // Local output time range (s)
std::string var; // Variable output filter file
Models models; // Name of model(s) or FMU(s)

namespace specified {

bool qss( false ); // QSS method specified?
bool eidd( false ); // Event indicators directional derivative specified?
bool rTol( false ); // Relative tolerance specified?
bool aTol( false ); // Absolute tolerance specified?
bool zTol( false ); // Zero-crossing/root tolerance specified?
bool dtZC( false ); // FMU zero-crossing time step specified?
bool dtOut( false ); // Sampled output time step specified?
bool tEnd( false ); // End time specified?
bool tLoc( false ); // Local output time range specified?
bool bin( false ); // Bin controls specified?

} // specified

namespace output { // Output selections

bool d( false ); // Diagnostics?
bool s( false ); // Statistics?
bool R( true ); // Requantizations?
bool O( true ); // Observer updates?
bool Z( true ); // Zero-crossings?
bool D( true ); // Discrete events?
bool S( false ); // Sampled?
bool X( true ); // Continuous trajectories?
bool Q( false ); // Quantized trajectories?
bool A( false ); // All variables?
bool F( false ); // FMU output variables?
bool L( false ); // FMU local variables?
bool K( false ); // FMU-QSS smooth tokens?

} // output

// Help Display
void
help_display()
{
	std::cout << '\n' << "QSS [options] [model [model ...]]" << "\n\n";
	std::cout << "Options:" << "\n\n";
	std::cout << " --qss=QSS        QSS method: (x)(LI)QSS(1|2|3)  [QSS2|FMU-QSS]" << '\n';
	std::cout << " --eidd           Use FMU event indicator directional derivatives" << '\n';
	std::cout << " --no-eidd        Don't use FMU event indicator directional derivatives" << '\n';
	std::cout << " --rTol=TOL       Relative tolerance  [" << rTol << "|FMU]" << '\n';
	std::cout << " --aTol=TOL       Absolute tolerance  [rTol*aFac*nominal]" << '\n';
	std::cout << " --aFac=FAC       Absolute tolerance factor  [" << aTol << "]" << '\n';
	std::cout << " --zTol=TOL       Zero-crossing/root tolerance  [" << zTol << "|FMU]" << '\n';
	std::cout << " --zMul=MUL       Zero-crossing tolerance bump multiplier  [" << zMul << "]" << '\n';
	std::cout << " --zFac=FAC       Zero-crossing tolerance factor  [" << zFac << "]" << '\n';
	std::cout << " --zrFac=FAC      Zero-crossing relative tolerance factor  [" << zrFac << "]" << '\n';
	std::cout << " --zaFac=FAC      Zero-crossing absolute tolerance factor  [" << zaFac << "]" << '\n';
	std::cout << " --dtMin=STEP     Min time step (s)  [0]" << '\n';
	std::cout << " --dtMax=STEP     Max time step (s)  [infinity]" << '\n';
	std::cout << " --dtInf=STEP     Deactivation control time step (s)  [infinity]" << '\n';
	std::cout << " --dtZMax=STEP    Max time step before zero-crossing (s)  (0 => Off)  [" << dtZMax << "]" << '\n';
	std::cout << " --dtZC=STEP      FMU zero-crossing time step (s)  [" << dtZC << "]" << '\n';
	std::cout << " --dtND=STEP      Numeric differentiation time step (s)  [" << dtND << "]" << '\n';
	std::cout << " --dtCon=STEP     FMU connection sync time step (s)  [0]" << '\n';
	std::cout << " --dtOut=STEP     Sampled output time step (s)  [computed]" << '\n';
	std::cout << " --tEnd=TIME      End time (s)  [1|FMU]" << '\n';
	std::cout << " --pass=COUNT     Pass count limit  [" << pass << "]" << '\n';
	std::cout << " --cycles         Report dependency cycles" << '\n';
	std::cout << " --inflection     Requantize at inflections" << '\n';
	std::cout << " --refine         Refine FMU zero-crossing roots" << '\n';
	std::cout << " --prune          Prune variables with no observers" << '\n';
	std::cout << " --perfect        Perfect FMU-ME connection sync" << '\n';
	std::cout << " --steps          Generate step count file for FMU" << '\n';
	std::cout << " --log=LEVEL      Logging level  [warning]" << '\n';
	std::cout << "       fatal" << '\n';
	std::cout << "       error" << '\n';
	std::cout << "       warning" << '\n';
	std::cout << "       info" << '\n';
	std::cout << "       verbose" << '\n';
	std::cout << "       debug" << '\n';
	std::cout << "       all" << '\n';
	std::cout << " --fxn=INP:FXN  FMU input variable function  [step[0|start,1,1]]" << '\n';
	std::cout << "       INP can be <model>.<var> with 2+ models" << '\n';
	std::cout << "           FXN is function spec:" << '\n';
	std::cout << "           constant[c] => c" << '\n';
	std::cout << "           sin[a,b,c] => a * sin( b * t ) + c" << '\n';
	std::cout << "           step[h0,h,d] => h0 + h * floor( t / d )" << '\n';
	std::cout << "           toggle[h0,h,d] => h0 + h * ( floor( t / d ) % 2 )" << '\n';
	std::cout << " --con=INP:OUT  Connect FMU input and output variables" << '\n';
	std::cout << "       INP and OUT syntax is <model>.<var>" << '\n';
	std::cout << " --dep=VAR[:DEP[,DEP,...]]  FMU dependencies to add" << '\n';
	std::cout << "       VAR  Variable (name or glob/regex)" << '\n';
	std::cout << "            No VAR => All variables" << '\n';
	std::cout << "            DEP  Dependency variable (name or glob/regex)" << '\n';
	std::cout << "                 No DEP => All variables" << '\n';
	std::cout << " --rdep=VAR[:DEP[,DEP,...]]  FMU reverse dependencies to add" << '\n';
	std::cout << "        VAR  Variable (name or glob/regex)" << '\n';
	std::cout << "             No VAR => All variables" << '\n';
	std::cout << "             DEP  Dependency variable (name or glob/regex)" << '\n';
	std::cout << "                  No DEP => All variables" << '\n';
	std::cout << " --bin=SIZE:FRAC:AUTO  FMU requantization binning controls  [1:0.5:N]" << '\n';
	std::cout << "       SIZE  Bin size  (Size or U for Unlimited)  [U]" << '\n';
	std::cout << "            FRAC  Min time step fraction  (0-1]  [0.25]" << '\n';
	std::cout << "                 AUTO  Automatic bin size optimization?  (Y|N)  [N]" << '\n';
	std::cout << " --out=OUTPUTS  Outputs  [ROZDX]" << '\n';
	std::cout << "       d  Diagnostics" << '\n';
	std::cout << "       s  Statistics" << '\n';
	std::cout << "     QSS Variables:" << '\n';
	std::cout << "       R  Requantizations" << '\n';
	std::cout << "       O  Observer updates" << '\n';
	std::cout << "       Z  Zero crossings" << '\n';
	std::cout << "       D  Discrete events" << '\n';
	std::cout << "       S  Sampled (@ dtOut)" << '\n';
	std::cout << "       X  Continuous trajectories" << '\n';
	std::cout << "       Q  Quantized trajectories" << '\n';
	std::cout << "       A  All variables at every event" << '\n';
	std::cout << "     FMU Variables (sampled @ dtOut):" << '\n';
	std::cout << "       F  Ouput variables" << '\n';
	std::cout << "       L  Local variables" << '\n';
	std::cout << "       K  FMU-QSS smooth tokens" << '\n';
	std::cout << " --tLoc=TIME1:TIME2  FMU local variable full output time range (s)" << '\n';
	std::cout << " --var=FILE       Variable output spec file" << '\n';
	std::cout << '\n';
	std::cout << "Code-Defined Models:" << "\n\n";
	std::cout << "  <model>.fmu : FMU-ME (FMU for Model Exchange)" << '\n';
	std::cout << "  <model>_QSS.fmu : FMU-QSS (FMU wrapping an FMU-ME and QSS)" << '\n';
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
	std::cout << "  observers : Observers" << '\n';
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
		} else if ( has_value_option( arg, "qss" ) ) {
			specified::qss = true;
			std::string const qss_name( uppercased( arg_value( arg ) ) );
			if ( qss_name == "QSS1" ) {
				qss = QSS::QSS1;
			} else if ( qss_name == "QSS2" ) {
				qss = QSS::QSS2;
			} else if ( qss_name == "QSS3" ) {
				qss = QSS::QSS3;
			} else if ( qss_name == "LIQSS1" ) {
				qss = QSS::LIQSS1;
			} else if ( qss_name == "LIQSS2" ) {
				qss = QSS::LIQSS2;
			} else if ( qss_name == "LIQSS3" ) {
				qss = QSS::LIQSS3;
			} else if ( qss_name == "XQSS1" ) {
				qss = QSS::xQSS1;
			} else if ( qss_name == "XQSS2" ) {
				qss = QSS::xQSS2;
			} else if ( qss_name == "XQSS3" ) {
				qss = QSS::xQSS3;
			} else if ( qss_name == "XLIQSS1" ) {
				qss = QSS::xLIQSS1;
			} else if ( qss_name == "XLIQSS2" ) {
				qss = QSS::xLIQSS2;
			} else if ( qss_name == "XLIQSS3" ) {
				qss = QSS::xLIQSS3;
			} else {
				std::cerr << "\nError: Unsupported QSS method: " << qss_name << std::endl;
				fatal = true;
			}
		} else if ( has_option( arg, "eidd" ) ) {
			specified::eidd = true;
			eidd = true;
		} else if ( ( has_option( arg, "no-eidd" ) ) || ( has_option( arg, "noeidd" ) ) ) {
			specified::eidd = true;
			eidd = false;
		} else if ( has_option( arg, "cycles" ) ) {
			cycles = true;
		} else if ( has_option( arg, "inflection" ) ) {
			inflection = true;
		} else if ( has_option( arg, "refine" ) ) {
			refine = true;
		} else if ( has_option( arg, "prune" ) ) {
			prune = true;
		} else if ( has_option( arg, "perfect" ) ) {
			perfect = true;
		} else if ( has_option( arg, "steps" ) ) {
			steps = true;
		} else if ( has_value_option( arg, "log" ) ) { // Accept PyFMI numeric logging levels for scripting convenience
			std::string const log_str( lowercased( arg_value( arg ) ) );
			if ( ( log_str == "fatal" ) || ( log_str == "f" ) || ( log_str == "0" ) ) {
				log = LogLevel::fatal;
			} else if ( ( log_str == "error" ) || ( log_str == "e" ) || ( log_str == "1" ) ) {
				log = LogLevel::error;
			} else if ( ( log_str == "warning" ) || ( log_str == "w" ) || ( log_str == "2" ) ) {
				log = LogLevel::warning;
			} else if ( ( log_str == "info" ) || ( log_str == "i" ) || ( log_str == "3" ) ) {
				log = LogLevel::info;
			} else if ( ( log_str == "verbose" ) || ( log_str == "v" ) || ( log_str == "4" ) ) {
				log = LogLevel::verbose;
			} else if ( ( log_str == "debug" ) || ( log_str == "d" ) || ( log_str == "5" ) ) {
				log = LogLevel::debug;
			} else if ( ( log_str == "all" ) || ( log_str == "a" ) || ( log_str == "6" ) || ( log_str == "7" ) ) {
				log = LogLevel::all;
			} else {
				std::cerr << "\nError: Unrecognized log level: " << log_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "rTol" ) ) {
			specified::rTol = true;
			std::string const rTol_str( arg_value( arg ) );
			if ( is_double( rTol_str ) ) {
				rTol = double_of( rTol_str );
				if ( rTol < 0.0 ) {
					std::cerr << "\nError: Negative rTol: " << rTol_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "\nError: Nonnumeric rTol: " << rTol_str << std::endl;
				fatal = true;
			}
			if ( rTol >= 1.0 ) {
				std::cerr << "\nWarning: rTol >= 1: " << rTol << std::endl;
			}
		} else if ( has_value_option( arg, "aTol" ) ) {
			specified::aTol = true;
			std::string const aTol_str( arg_value( arg ) );
			if ( is_double( aTol_str ) ) {
				aTol = double_of( aTol_str );
				if ( aTol == 0.0 ) {
					aTol = std::numeric_limits< double >::min();
					std::cerr << "\nWarning: aTol set to: " << aTol << std::endl;
				} else if ( aTol < 0.0 ) {
					std::cerr << "\nError: Negative aTol: " << aTol_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "\nError: Nonnumeric aTol: " << aTol_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "aFac" ) ) {
			std::string const aFac_str( arg_value( arg ) );
			if ( is_double( aFac_str ) ) {
				aFac = double_of( aFac_str );
				if ( aFac <= 0.0 ) {
					std::cerr << "\nError: Nonpositive aFac: " << aFac_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "\nError: Nonnumeric aFac: " << aFac_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "zTol" ) ) {
			specified::zTol = true;
			std::string const zTol_str( arg_value( arg ) );
			if ( is_double( zTol_str ) ) {
				zTol = double_of( zTol_str );
				if ( zTol < 0.0 ) {
					std::cerr << "\nError: Negative zTol: " << zTol_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "\nError: Nonnumeric zTol: " << zTol_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "zMul" ) ) {
			std::string const zMul_str( arg_value( arg ) );
			if ( is_double( zMul_str ) ) {
				zMul = double_of( zMul_str );
				if ( zMul <= 0.0 ) {
					std::cerr << "\nError: zMul <= 0.0: " << zMul_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "\nError: Nonnumeric zMul: " << zMul_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "zFac" ) ) {
			std::string const zFac_str( arg_value( arg ) );
			if ( is_double( zFac_str ) ) {
				zFac = double_of( zFac_str );
				if ( zFac <= 0.0 ) {
					std::cerr << "\nError: zFac <= 0.0: " << zFac_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "\nError: Nonnumeric zFac: " << zFac_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "zrFac" ) ) {
			std::string const zrFac_str( arg_value( arg ) );
			if ( is_double( zrFac_str ) ) {
				zrFac = double_of( zrFac_str );
				if ( zrFac <= 0.0 ) {
					std::cerr << "\nError: zrFac <= 0.0: " << zrFac_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "\nError: Nonnumeric zrFac: " << zrFac_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "zaFac" ) ) {
			std::string const zaFac_str( arg_value( arg ) );
			if ( is_double( zaFac_str ) ) {
				zaFac = double_of( zaFac_str );
				if ( zaFac <= 0.0 ) {
					std::cerr << "\nError: zaFac <= 0.0: " << zaFac_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "\nError: Nonnumeric zaFac: " << zaFac_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "dtMin" ) ) {
			std::string const dtMin_str( arg_value( arg ) );
			if ( is_double( dtMin_str ) ) {
				dtMin = double_of( dtMin_str );
				if ( dtMin < 0.0 ) {
					std::cerr << "\nError: Negative dtMin: " << dtMin_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "\nError: Nonnumeric dtMin: " << dtMin_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "dtMax" ) ) {
			std::string const dtMax_str( arg_value( arg ) );
			if ( is_double( dtMax_str ) ) {
				dtMax = double_of( dtMax_str );
				if ( dtMax < 0.0 ) {
					std::cerr << "\nError: Negative dtMax: " << dtMax_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "\nError: Nonnumeric dtMax: " << dtMax_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "dtInf" ) ) {
			std::string const dtInf_str( arg_value( arg ) );
			if ( is_double( dtInf_str ) ) {
				dtInf = double_of( dtInf_str );
				if ( dtInf < 0.0 ) {
					std::cerr << "\nError: Negative dtInf: " << dtInf_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "\nError: Nonnumeric dtInf: " << dtInf_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "dtZMax" ) ) {
			std::string const dtZMax_str( arg_value( arg ) );
			if ( is_double( dtZMax_str ) ) {
				dtZMax = double_of( dtZMax_str );
				if ( dtZMax < 0.0 ) {
					std::cerr << "\nError: Negative dtZMax: " << dtZMax_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "\nError: Nonnumeric dtZMax: " << dtZMax_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "dtZC" ) ) {
			specified::dtZC = true;
			std::string const dtZC_str( arg_value( arg ) );
			if ( is_double( dtZC_str ) ) {
				dtZC = double_of( dtZC_str );
				if ( dtZC < 0.0 ) {
					std::cerr << "\nError: Negative dtZC: " << dtZC_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "\nError: Nonnumeric dtZC: " << dtZC_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "dtND" ) || has_value_option( arg, "dtNum" ) ) { // dtNum was prior name for dtND
			std::string const dtND_str( arg_value( arg ) );
			if ( is_double( dtND_str ) ) {
				dtND = double_of( dtND_str );
				if ( dtND <= 0.0 ) {
					std::cerr << "\nError: Nonpositive dtND: " << dtND_str << std::endl;
					fatal = true;
				}
				two_dtND = 2.0 * dtND;
				three_dtND = 3.0 * dtND;
				one_over_dtND = 1.0 / dtND;
				one_over_two_dtND = 1.0 / ( 2.0 * dtND );
				one_over_two_dtND_squared = 1.0 / ( 2.0 * ( dtND * dtND ) );
				one_over_four_dtND = 1.0 / ( 4.0 * dtND );
				one_over_six_dtND = 1.0 / ( 6.0 * dtND );
				one_over_six_dtND_squared = 1.0 / ( 6.0 * ( dtND * dtND ) );
				one_over_six_dtND_cubed = 1.0 / ( 6.0 * ( dtND * dtND * dtND ) );
			} else {
				std::cerr << "\nError: Nonnumeric dtND: " << dtND_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "dtCon" ) ) {
			std::string const dtCon_str( arg_value( arg ) );
			if ( is_double( dtCon_str ) ) {
				dtCon = double_of( dtCon_str );
				if ( dtCon < 0.0 ) {
					std::cerr << "\nError: Negative dtCon: " << dtCon_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "\nError: Nonnumeric dtCon: " << dtCon_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "dtOut" ) ) {
			specified::dtOut = true;
			std::string const dtOut_str( arg_value( arg ) );
			if ( is_double( dtOut_str ) ) {
				dtOut = double_of( dtOut_str );
				if ( dtOut < 0.0 ) {
					std::cerr << "\nError: Negative dtOut: " << dtOut_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "\nError: Nonnumeric dtOut: " << dtOut_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "tEnd" ) ) {
			specified::tEnd = true;
			std::string const tEnd_str( arg_value( arg ) );
			if ( is_double( tEnd_str ) ) {
				tEnd = double_of( tEnd_str );
				if ( tEnd < 0.0 ) {
					std::cerr << "\nError: Negative tEnd: " << tEnd_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "\nError: Nonnumeric tEnd: " << tEnd_str << std::endl;
				fatal = true;
			}
		} else if ( has_option( arg, "bin" ) || has_value_option( arg, "bin" ) ) {
			specified::bin = true;
			std::string const bin_str( arg_value( arg ) );
			std::vector< std::string > const bin_args( split( bin_str, ':' ) );
			if ( bin_args.size() > 1u ) { // : separated entries present

				// Bin size max
				std::string const bin_size_str( bin_args[ 0 ] );
				if ( bin_size_str.empty() ) {
					bin_size = std::numeric_limits< std::size_t >::max();
				} else if ( bin_size_str == "U" ) { // Unlimited bin size max
					bin_size = std::numeric_limits< std::size_t >::max();
				} else if ( is_size( bin_size_str ) ) { // Specified bin size max
					bin_size = size_of( bin_size_str );
				} else {
					std::cerr << "\nError: bin size is not valid: " << bin_size_str << std::endl;
					fatal = true;
				}

				// Bin fraction min
				std::string const bin_frac_str( bin_args[ 1 ] );
				if ( bin_frac_str.empty() ) {
					bin_frac = 0.25;
				} else if ( is_double( bin_frac_str ) ) {
					bin_frac = double_of( bin_frac_str );
					if ( ( bin_frac < 0.0 ) || ( bin_frac > 1.0 ) ) {
						std::cerr << "\nError: bin frac is outside of [0,1] range: " << bin_frac << std::endl;
						fatal = true;
					}
				} else {
					std::cerr << "\nError: Nonumeric bin frac: " << bin_frac_str << std::endl;
					fatal = true;
				}

				// Bin auto-optimize
				if ( bin_args.size() > 2u ) {
					std::string const bin_auto_str( bin_args[ 2 ] );
					if ( bin_auto_str.empty() ) {
						bin_auto = false;
					} else if ( is_any_of( bin_auto_str[ 0 ], "YyTt1" ) ) {
						bin_auto = true;
					} else if ( is_any_of( bin_auto_str[ 0 ], "NnFf0" ) ) {
						bin_auto = false;
					} else {
						std::cerr << "\nError: Invalid bin auto: " << bin_auto_str << std::endl;
						fatal = true;
					}
				}

			} else if ( ! bin_str.empty() ) { // Treat single parameter as bin_size
				if ( is_size( bin_str ) ) {
					bin_size = size_of( bin_str );
				} else if ( bin_str == "U" ) { // Unlimited max bin size
					bin_size = std::numeric_limits< std::size_t >::max();
				} else {
					std::cerr << "\nError: bin size is not valid: " << bin_str << std::endl;
					fatal = true;
				}
			} else { // Use bin defaults
				bin_size = std::numeric_limits< std::size_t >::max();
				bin_frac = 0.25;
				bin_auto = false;
			}
		} else if ( has_value_option( arg, "pass" ) ) {
			std::string const pass_str( arg_value( arg ) );
			if ( is_size( pass_str ) ) {
				pass = size_of( pass_str );
				if ( pass < 1 ) {
					std::cerr << "\nError: Nonpositive pass: " << pass_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "\nError: Nonintegral pass: " << pass_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "fxn" ) ) {
			std::string const var_fxn( arg_value( arg ) );
			std::string var_name;
			std::string fxn_spec;
			if ( var_fxn[ 0 ] == '"' ) { // Quoted variable name
				std::string::size_type const qe( var_fxn.find( '"', 1u ) );
				if ( qe != std::string::npos ) {
					var_name = var_fxn.substr( 1u, qe - 1u );
					std::string::size_type const isep( var_fxn.find( ':', qe ) );
					if ( isep != std::string::npos ) {
						fxn_spec = var_fxn.substr( isep + 1u );
						fxn[ var_name ] = fxn_spec;
					} else {
						std::cerr << "\nError: Input function spec not in variable:function format: " << var_fxn << std::endl;
						fatal = true;
					}
				} else {
					std::cerr << "\nError: Input function quoted variable name missing end quote: " << var_fxn << std::endl;
					fatal = true;
				}
			} else {
				std::string::size_type const isep( var_fxn.find( ':' ) );
				if ( isep != std::string::npos ) {
					var_name = var_fxn.substr( 0u, isep );
					fxn_spec = var_fxn.substr( isep + 1u );
					fxn[ var_name ] = fxn_spec;
				} else {
					std::cerr << "\nError: Input variable function spec not in variable:function format: " << var_fxn << std::endl;
					fatal = true;
				}
			}
		} else if ( has_value_option( arg, "con" ) ) {
			std::string const inp_out( arg_value( arg ) );
			std::string inp_name;
			std::string out_name;
			if ( inp_out[ 0 ] == '"' ) { // Quoted input variable name
				std::string::size_type const qe( inp_out.find( '"', 1u ) );
				if ( qe != std::string::npos ) {
					inp_name = inp_out.substr( 1u, qe - 1u );
					std::string::size_type const isep( inp_out.find( ':', qe ) );
					if ( isep != std::string::npos ) {
						out_name = inp_out.substr( isep + 1u );
					} else {
						std::cerr << "\nError: Input-output connection spec not in input:output format: " << inp_out << std::endl;
						fatal = true;
					}
				} else {
					std::cerr << "\nError: Input-output connection spec quoted input variable name missing end quote: " << inp_out << std::endl;
					fatal = true;
				}
			} else {
				std::string::size_type const isep( inp_out.find( ':' ) );
				if ( isep != std::string::npos ) {
					inp_name = inp_out.substr( 0u, isep );
					out_name = inp_out.substr( isep + 1u );
				} else {
					std::cerr << "\nError: Input-output connection spec not in input:output format: " << inp_out << std::endl;
					fatal = true;
				}
			}
			if ( out_name[ 0 ] == '"' ) { // Quoted output variable name
				std::string::size_type const qe( inp_out.find( '"', 1u ) );
				if ( qe != std::string::npos ) {
					out_name = out_name.substr( 1u, qe - 1u );
				} else {
					std::cerr << "\nError: Input-output connection spec quoted output variable name missing end quote: " << inp_out << std::endl;
					fatal = true;
				}
			}
			con[ inp_name ] = out_name;
		} else if ( has_option( arg, "dep" ) || has_value_option( arg, "dep" ) ) {
			std::string const var_deps( arg_value( arg ) );
			std::string var_spec;
			std::string deps_spec;
			if ( var_deps.empty() ) { // Implied all
				var_spec = deps_spec = '*';
			} else if ( var_deps[ 0 ] == '"' ) { // Quoted input variable name
				std::string::size_type const qe( var_deps.find( '"', 1u ) );
				if ( qe != std::string::npos ) {
					var_spec = var_deps.substr( 1u, qe - 1u );
					std::string::size_type const isep( var_deps.find( ':', qe ) );
					if ( isep != std::string::npos ) {
						deps_spec = var_deps.substr( isep + 1u );
					} else {
						deps_spec = '*'; // Implied all
					}
				} else {
					std::cerr << "\nError: Dependencies spec quoted input variable spec missing end quote: " << var_deps << std::endl;
					fatal = true;
				}
			} else {
				std::string::size_type const isep( var_deps.find( ':' ) );
				if ( isep != std::string::npos ) {
					var_spec = var_deps.substr( 0u, isep );
					deps_spec = var_deps.substr( isep + 1u );
				} else {
					var_spec = var_deps;
					deps_spec = '*'; // Implied all
				}
			}
			if ( var_spec.empty() ) var_spec = '*'; // Implied all
			std::vector< std::string > dep_specs;
			{ // Scope
			std::string dep_spec;
			bool in_quote( false );
			for ( char const c : deps_spec ) {
				if ( c == '"' ) {
					if ( in_quote ) { // End quoted string
						if ( ! strip( dep_spec ).empty() ) {
							dep_specs.push_back( dep_spec );
							dep_spec.clear();
						}
					}
					in_quote = ! in_quote;
				} else if ( ( ! in_quote ) && ( c == ',' ) ) {
					if ( ! strip( dep_spec ).empty() ) {
						dep_specs.push_back( dep_spec );
						dep_spec.clear();
					}
				} else {
					dep_spec.push_back( c );
				}
			}
			if ( ! strip( dep_spec ).empty() ) dep_specs.push_back( dep_spec );
			} // Scope
			if ( var_spec == "*" ) { // Check for all-depends-on-all
				if ( dep_specs.empty() ) {
					dep.all() = true;
				} else {
					for ( std::string const & dep_spec : dep_specs ) {
						if ( dep_spec == "*" ) {
							dep.all() = true;
							break;
						}
					}
				}
			}
			std::regex var_regex;
			try {
				var_regex = Depends::regex( var_spec );
			} catch (...) {
				std::cerr << "\nError: Dependency variable spec cannot be converted into a regex " << var_spec << std::endl;
				fatal = true;
			}
			std::vector< std::regex > deps_regex;
			for ( std::string const & dep_spec : dep_specs ) {
				try {
					deps_regex.push_back( Depends::regex( dep_spec ) );
				} catch (...) {
					std::cerr << "\nError: Dependency spec cannot be converted into a regex " << dep_spec << std::endl;
					fatal = true;
				}
			}
			dep.add( var_regex, deps_regex );
		} else if ( has_option( arg, "rdep" ) || has_value_option( arg, "rdep" ) ) {
			std::string const var_rdeps( arg_value( arg ) );
			std::string var_spec;
			std::string rdeps_spec;
			if ( var_rdeps.empty() ) { // Implied all
				var_spec = rdeps_spec = '*';
			} else if ( var_rdeps[ 0 ] == '"' ) { // Quoted input variable name
				std::string::size_type const qe( var_rdeps.find( '"', 1u ) );
				if ( qe != std::string::npos ) {
					var_spec = var_rdeps.substr( 1u, qe - 1u );
					std::string::size_type const isep( var_rdeps.find( ':', qe ) );
					if ( isep != std::string::npos ) {
						rdeps_spec = var_rdeps.substr( isep + 1u );
					} else {
						rdeps_spec = '*'; // Implied all
					}
				} else {
					std::cerr << "\nError: Reverse dependencies spec quoted input variable spec missing end quote: " << var_rdeps << std::endl;
					fatal = true;
				}
			} else {
				std::string::size_type const isep( var_rdeps.find( ':' ) );
				if ( isep != std::string::npos ) {
					var_spec = var_rdeps.substr( 0u, isep );
					rdeps_spec = var_rdeps.substr( isep + 1u );
				} else {
					var_spec = var_rdeps;
					rdeps_spec = '*'; // Implied all
				}
			}
			if ( var_spec.empty() ) var_spec = '*'; // Implied all
			std::vector< std::string > rdep_specs;
			{ // Scope
			std::string rdep_spec;
			bool in_quote( false );
			for ( char const c : rdeps_spec ) {
				if ( c == '"' ) {
					if ( in_quote ) { // End quoted string
						if ( ! strip( rdep_spec ).empty() ) {
							rdep_specs.push_back( rdep_spec );
							rdep_spec.clear();
						}
					}
					in_quote = ! in_quote;
				} else if ( ( ! in_quote ) && ( c == ',' ) ) {
					if ( ! strip( rdep_spec ).empty() ) {
						rdep_specs.push_back( rdep_spec );
						rdep_spec.clear();
					}
				} else {
					rdep_spec.push_back( c );
				}
			}
			if ( ! strip( rdep_spec ).empty() ) rdep_specs.push_back( rdep_spec );
			} // Scope
			if ( var_spec == "*" ) { // Check for all-rdepends-on-all
				if ( rdep_specs.empty() ) {
					rdep.all() = true;
				} else {
					for ( std::string const & rdep_spec : rdep_specs ) {
						if ( rdep_spec == "*" ) {
							rdep.all() = true;
							break;
						}
					}
				}
			}
			std::regex var_regex;
			try {
				var_regex = Depends::regex( var_spec );
			} catch (...) {
				std::cerr << "\nError: Reverse dependency variable spec cannot be converted into a regex " << var_spec << std::endl;
				fatal = true;
			}
			std::vector< std::regex > rdeps_regex;
			for ( std::string const & rdep_spec : rdep_specs ) {
				try {
					rdeps_regex.push_back( Depends::regex( rdep_spec ) );
				} catch (...) {
					std::cerr << "\nError: Reverse dependency spec cannot be converted into a regex " << rdep_spec << std::endl;
					fatal = true;
				}
			}
			rdep.add( var_regex, rdeps_regex );
		} else if ( has_option( arg, "out" ) || has_value_option( arg, "out" ) ) {
			static std::string const out_flags( "dsROZDSXQAFLK" );
			out = arg_value( arg );
			if ( has_any_not_of( out, out_flags ) ) {
				std::cerr << "\nError: Output flag not in " << out_flags << ": " << out << std::endl;
				fatal = true;
			}
			output::d = has( out, 'd' );
			output::s = has( out, 's' );
			output::R = has( out, 'R' );
			output::O = has( out, 'O' );
			output::Z = has( out, 'Z' );
			output::D = has( out, 'D' );
			output::S = has( out, 'S' );
			output::X = has( out, 'X' );
			output::Q = has( out, 'Q' );
			output::A = has( out, 'A' );
			output::F = has( out, 'F' );
			output::L = has( out, 'L' );
			output::K = has( out, 'K' );
		} else if ( has_value_option( arg, "tLoc" ) ) {
			specified::tLoc = true;
			std::string const tLoc_str( arg_value( arg ) );
			std::vector< std::string > const tLoc_tokens( split( tLoc_str, ':' ) );
			if ( tLoc_tokens.size() == 2 ) { // Process/check time range

				// Begin time
				std::string tLoc_beg_str( tLoc_tokens[ 0 ] );
				if ( is_double( tLoc_beg_str ) ) {
					tLoc.first = double_of( tLoc_beg_str );
					if ( tLoc.first < 0.0 ) {
						std::cerr << "\nError: Negative tLoc start time: " << tLoc_beg_str << std::endl;
						fatal = true;
					}
				} else {
					std::cerr << "\nError: Nonnumeric tLoc start time: " << tLoc_beg_str << std::endl;
					fatal = true;
				}

				// End time
				std::string tLoc_end_str( tLoc_tokens[ 1 ] );
				if ( is_double( tLoc_end_str ) ) {
					tLoc.second = double_of( tLoc_end_str );
					if ( tLoc.second < 0.0 ) {
						std::cerr << "\nError: Negative tLoc end time: " << tLoc_end_str << std::endl;
						fatal = true;
					}
				} else {
					std::cerr << "\nError: Nonnumeric tLoc end time: " << tLoc_end_str << std::endl;
					fatal = true;
				}

				// Check valid range
				if ( tLoc.first > tLoc.second ) {
					std::cerr << "\nError: Invalid tLoc time range: " << tLoc.first << " to " << tLoc.second << std::endl;
					fatal = true;
				}

			} else {
				std::cerr << "\nError: tLoc not in TIME1:TIME2 format: " << tLoc_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "var" ) ) {
			var = arg_value( arg );
			if ( ! path::is_file( var ) ) {
				std::cerr << "\nError: File specified in --var option not found: " << var << ": Output filtering disabled" << std::endl;
			}
		} else if ( arg[ 0 ] == '-' ) {
			std::cerr << "\nError: Unsupported option: " << arg << std::endl;
			fatal = true;
		} else { // Treat non-option argument as model
			models.push_back( arg );
		}
		if ( ! specified::aTol ) aTol = rTol * aFac; // Make unspecified aTol consistent with rTol * aFac for use in cod
	}

	// Inter-option checks
	if ( specified::rTol && ( rTol * zFac * zrFac >= 1.0 ) ) {
		std::cerr << "\nWarning: Zero-crossing relative tolerance: rTol * zFac * zrFac >= 1: " << rTol * zFac * zrFac << std::endl;
	}

	if ( help ) std::exit( EXIT_SUCCESS );
	if ( fatal ) std::exit( EXIT_FAILURE );

	std::cout << "\nQSS Command Line Arguments:\n";
	for ( int i = 1; i < argc; ++i ) {
		std::cout << ' ' <<  argv[ i ] << '\n';
	}
	std::cout << std::endl;
}

// Multiple models?
bool
have_multiple_models()
{
	return ( models.size() > 1u );
}

// Input-output connections?
bool
connected()
{
	return ( ! con.empty() );
}

// Set dtOut to Default for a Given Time Span
void
dtOut_set( double const t )
{
	if ( ! specified::dtOut ) dtOut = std::pow( 10.0, std::round( std::log10( t * 0.0002 ) ) );
}

} // options
} // QSS
