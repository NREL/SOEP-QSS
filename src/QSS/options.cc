// QSS Options Support
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2023 Objexx Engineering, Inc. All rights reserved.
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
#include <QSS/version.hh>

// C++ Headers
#include <iostream>
#include <limits>
#include <string_view>

namespace QSS {
namespace options {

QSS qss( QSS::QSS2 ); // QSS method: [n][r][f][LI]QSS(1|2|3)
int order( 2 ); // QSS method order: (1|2|3)
bool d2d( true ); // Directional state second derivatives QSS solver?
bool n2d( false ); // Numerical state second derivatives QSS solver?
bool rQSS( false ); // Relaxation QSS solver?
bool fQSS( false ); // Full-order broadcast QSS solver?
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
double dtND_max( 1.0 ); // Numeric differentiation time step max (s)
bool dtND_optimizer( false ); // Optimize FMU numeric differentiation time step?
double two_dtND( 2.0 * dtND );
double one_over_two_dtND( 1.0 / ( 2.0 * dtND ) );
double one_over_three_dtND( 1.0 / ( 3.0 * dtND ) );
double one_over_four_dtND( 1.0 / ( 4.0 * dtND ) );
double one_over_six_dtND_squared( 1.0 / ( 6.0 * ( dtND * dtND ) ) );
double dtCon( 0.0 ); // FMU connection sync time step (s)
double dtOut( 1.0e-3 ); // Sampled output time step (s)
double tEnd( 1.0 ); // End time (s)  [1|FMU]
std::size_t bin_size( 1u ); // Bin size max
double bin_frac( 0.25 ); // Bin step fraction min
bool bin_auto( false ); // Bin size automaically optimized?
std::size_t pass( 20 ); // Pass count limit
bool cycles( false ); // Report dependency cycles?
bool inflection( false ); // Requantize at inflections?
double inflectionFrac( 0.05 ); // Inflection step fraction min
bool cluster( false ); // Clustering with relaxation solver?
bool refine( false ); // Refine FMU zero-crossing roots?
bool perfect( false ); // Perfect FMU-ME connection sync?
bool active( false ); // Active intermediate variables preferred?
bool passive( true ); // Passive intermediate variables preferred?
bool steps( false ); // Generate requantization step count file?
LogLevel log( LogLevel::warning ); // Logging level
InpFxn fxn; // Map from input variables to function specs
InpOut con; // Map from input variables to output variables
DepSpecs dep; // Additional forward dependencies
bool csv( false ); // CSV results file?
std::pair< double, double > tLoc( 0.0, 0.0 ); // Local output time range (s)
std::string var; // Variable output filter file
Models models; // Name of model(s) or FMU(s)

namespace specified {

bool qss( false ); // QSS method specified?
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
bool s( true ); // Statistics?
bool h( false ); // Headers?
bool R( true ); // Requantizations?
bool O( true ); // Observer updates?
bool Z( true ); // Zero-crossings?
bool D( true ); // Discrete events?
bool S( false ); // Sampled?
bool X( true ); // Continuous trajectories?
bool Q( false ); // Quantized trajectories?
bool T( false ); // Time step?
bool A( false ); // All variables?
bool F( false ); // FMU output variables?
bool L( false ); // FMU local variables?
bool K( false ); // FMU-QSS smooth tokens?

} // output

namespace dot_graph { // Dot graph selections

bool d( false ); // Dependency graph?
bool r( false ); // Computational Observer graph?
bool e( false ); // Computational Observee graph?

} // dot_graph

// Help Display
void
help_display()
{
	std::cout << '\n' << "QSS [options] [model [model ...]]" << "\n\n";
	std::cout << "Options:" << "\n\n";
	std::cout << " --qss=QSS              QSS method: [n][r][f][LI]QSS(1|2|3)  [QSS2|FMU-QSS]" << '\n';
	std::cout << "                                     n: Numerical state second derivatives" << '\n';
	std::cout << "                                        r: Relaxation solver" << '\n';
	std::cout << "                                           f: Full-order broadcast quantized representation" << '\n';
	std::cout << " --rTol=TOL             Relative tolerance  [" << rTol << "|FMU]" << '\n';
	std::cout << " --aTol=TOL             Absolute tolerance  [rTol*aFac*nominal]" << '\n';
	std::cout << " --aFac=FAC             Absolute tolerance factor  [" << aFac << ']' << '\n';
	std::cout << " --zTol=TOL             Zero-crossing/root tolerance  [" << zTol << "|FMU]" << '\n';
	std::cout << " --zMul=MUL             Zero-crossing tolerance bump multiplier  [" << zMul << ']' << '\n';
	std::cout << " --zFac=FAC             Zero-crossing tolerance factor  [" << zFac << ']' << '\n';
	std::cout << " --zrFac=FAC            Zero-crossing relative tolerance factor  [" << zrFac << ']' << '\n';
	std::cout << " --zaFac=FAC            Zero-crossing absolute tolerance factor  [" << zaFac << ']' << '\n';
	std::cout << " --dtMin=STEP           Min time step (s)  [0]" << '\n';
	std::cout << " --dtMax=STEP           Max time step (s)  [infinity]" << '\n';
	std::cout << " --dtInf=STEP           Deactivation control time step (s)  [infinity]" << '\n';
	std::cout << " --dtZMax=STEP          Max time step before zero-crossing (s)  (0 => Off)  [" << dtZMax << ']' << '\n';
	std::cout << " --dtZC=STEP            FMU zero-crossing time step (s)  [" << dtZC << ']' << '\n';
	std::cout << " --dtND=STEP[:Y|U]      Numeric differentiation time step specs" << '\n';
	std::cout << "        STEP            Time step (s)  [1e-6]" << '\n';
	std::cout << "              Y         Use automatic time step" << '\n';
	std::cout << "              U         Upper time step for automatic scan (s)  [" << dtND_max << ']' << '\n';
	std::cout << " --dtCon=STEP           FMU connection sync time step (s)  [0]" << '\n';
	std::cout << " --dtOut=STEP           Sampled output time step (s)  [computed]" << '\n';
	std::cout << " --tEnd=TIME            End time (s)  [1|FMU]" << '\n';
	std::cout << " --pass=COUNT           Pass count limit  [" << pass << ']' << '\n';
	std::cout << " --cycles               Report dependency cycles" << '\n';
	std::cout << " --inflection           Requantize at inflections" << '\n';
	std::cout << " --inflectionFrac=FRAC  Inflection step fraction min  [" << inflectionFrac << ']' << '\n';
	std::cout << " --cluster              Clustering with relaxation solver  [Off]" << '\n';
	std::cout << " --refine               Refine FMU zero-crossing roots" << '\n';
	std::cout << " --perfect              Perfect FMU-ME connection sync" << '\n';
	std::cout << " --active               Active intermediate variables preferred  [Off]" << '\n';
	std::cout << " --passive              Passive intermediate variables preferred  [On]" << '\n';
	std::cout << " --steps                Generate step count file for FMU" << '\n';
	std::cout << " --log=LEVEL            Logging level  [warning]" << '\n';
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
	std::cout << " --bin=SIZE:FRAC:AUTO  FMU requantization binning controls  [1:0.25:N]" << '\n';
	std::cout << "       SIZE  Bin size  (Size or U for Unlimited)  [U]" << '\n';
	std::cout << "            FRAC  Min time step fraction  (0-1]  [0.25]" << '\n';
	std::cout << "                 AUTO  Automatic bin size optimization?  (Y|N)  [N]" << '\n';
	std::cout << " --out=OUTPUTS  Outputs  [sROZDX]" << '\n';
	std::cout << "       d  Diagnostics" << '\n';
	std::cout << "       s  Statistics" << '\n';
	std::cout << "       h  Headers" << '\n';
	std::cout << "     QSS Variables:" << '\n';
	std::cout << "       R  Requantizations" << '\n';
	std::cout << "       O  Observer updates" << '\n';
	std::cout << "       Z  Zero crossings" << '\n';
	std::cout << "       D  Discrete events" << '\n';
	std::cout << "       S  Sampled (@ dtOut)" << '\n';
	std::cout << "       X  Continuous trajectories" << '\n';
	std::cout << "       Q  Quantized trajectories" << '\n';
	std::cout << "       T  Time steps" << '\n';
	std::cout << "       A  All variables at every event" << '\n';
	std::cout << "     FMU Variables (sampled @ dtOut):" << '\n';
	std::cout << "       F  Ouput variables" << '\n';
	std::cout << "       L  Local variables" << '\n';
	std::cout << "       K  FMU-QSS smooth tokens" << '\n';
	std::cout << " --csv  Output CSV results file" << '\n';
	std::cout << " --dot=GRAPHS  Outputs  [dre]" << '\n';
	std::cout << "       d  Dependency graph" << '\n';
	std::cout << "       r  Computational Observer graph" << '\n';
	std::cout << "       e  Computational Observee graph" << '\n';
	std::cout << " --tLoc=TIME1:TIME2  FMU local variable full output time range (s)" << '\n';
	std::cout << " --var=FILE  Variable output spec file" << '\n';
	std::cout << '\n';
}

// Process command line arguments
void
process_args( Args const & args )
{
	Args::size_type const argc( args.size() );
	bool help( false );
	if ( argc <= 1u ) { // No arguments: Show help
		help_display();
		help = true;
	}
	bool version_arg( false );
	bool fatal( false ); // Fatal error occurred?
	for ( Args::size_type i = 1; i < argc; ++i ) {
		std::string const arg( args[ i ] );
		if ( ( arg == "--help" ) || ( arg == "-h" ) ) { // Show help
			help_display();
			help = true;
		} else if ( ( arg == "--version" ) || ( arg == "-v" ) ) { // Show version
			std::cout << "\nSOEP-QSS " << version() << '\n';
			version_arg = true;
		} else if ( has_option_value( arg, "qss" ) ) {
			specified::qss = true;
			std::string const qss_name( option_value( arg, "qss" ) );
			if ( qss_name == "QSS1" ) {
				qss = QSS::QSS1;
				order = 1;
				d2d = true;
				rQSS = false;
				fQSS = false;
			} else if ( qss_name == "QSS2" ) {
				qss = QSS::QSS2;
				order = 2;
				d2d = true;
				rQSS = false;
				fQSS = false;
			} else if ( qss_name == "QSS3" ) {
				qss = QSS::QSS3;
				order = 3;
				d2d = true;
				rQSS = false;
				fQSS = false;
			} else if ( qss_name == "LIQSS1" ) {
				qss = QSS::LIQSS1;
				order = 1;
				d2d = true;
				rQSS = false;
				fQSS = false;
			} else if ( qss_name == "LIQSS2" ) {
				qss = QSS::LIQSS2;
				order = 2;
				d2d = true;
				rQSS = false;
				fQSS = false;
			} else if ( qss_name == "LIQSS3" ) {
				qss = QSS::LIQSS3;
				order = 3;
				d2d = true;
				rQSS = false;
				fQSS = false;
			} else if ( qss_name == "fQSS1" ) {
				qss = QSS::fQSS1;
				order = 1;
				d2d = true;
				rQSS = false;
				fQSS = true;
			} else if ( qss_name == "fQSS2" ) {
				qss = QSS::fQSS2;
				order = 2;
				d2d = true;
				rQSS = false;
				fQSS = true;
			} else if ( qss_name == "fQSS3" ) {
				qss = QSS::fQSS3;
				order = 3;
				d2d = true;
				rQSS = false;
				fQSS = true;
			} else if ( qss_name == "rQSS2" ) {
				qss = QSS::rQSS2;
				order = 2;
				d2d = true;
				rQSS = true;
				fQSS = false;
			} else if ( qss_name == "rLIQSS2" ) {
				qss = QSS::rLIQSS2;
				order = 2;
				d2d = true;
				rQSS = true;
				fQSS = false;
			} else if ( qss_name == "rfQSS2" ) {
				qss = QSS::rfQSS2;
				order = 2;
				d2d = true;
				rQSS = true;
				fQSS = true;
			} else if ( qss_name == "nQSS2" ) {
				qss = QSS::nQSS2;
				order = 2;
				d2d = false;
				rQSS = false;
				fQSS = false;
			} else if ( qss_name == "nQSS3" ) {
				qss = QSS::nQSS3;
				order = 3;
				d2d = false;
				rQSS = false;
				fQSS = false;
			} else if ( qss_name == "nLIQSS2" ) {
				qss = QSS::nLIQSS2;
				order = 2;
				d2d = false;
				rQSS = false;
				fQSS = false;
			} else if ( qss_name == "nLIQSS3" ) {
				qss = QSS::nLIQSS3;
				order = 3;
				d2d = false;
				rQSS = false;
				fQSS = false;
			} else if ( qss_name == "nfQSS2" ) {
				qss = QSS::nfQSS2;
				order = 2;
				d2d = false;
				rQSS = false;
				fQSS = true;
			} else if ( qss_name == "nfQSS3" ) {
				qss = QSS::nfQSS3;
				order = 3;
				d2d = false;
				rQSS = false;
				fQSS = true;
			} else if ( qss_name == "nrQSS2" ) {
				qss = QSS::nrQSS2;
				order = 2;
				d2d = false;
				rQSS = true;
				fQSS = false;
			} else if ( qss_name == "nrLIQSS2" ) {
				qss = QSS::nrLIQSS2;
				order = 2;
				d2d = false;
				rQSS = true;
				fQSS = false;
			} else if ( qss_name == "nrfQSS2" ) {
				qss = QSS::nrfQSS2;
				order = 2;
				d2d = false;
				rQSS = true;
				fQSS = true;
			} else {
				std::cerr << "\nError: Unsupported QSS method: " << qss_name << std::endl;
				fatal = true;
			}
			n2d = !d2d;
		} else if ( has_option( arg, "cycles" ) ) {
			cycles = true;
		} else if ( has_option( arg, "inflection" ) ) {
			inflection = true;
		} else if ( has_option_value( arg, "inflectionFrac" ) ) {
			std::string const inflectionFrac_str( option_value( arg, "inflectionFrac" ) );
			if ( is_double( inflectionFrac_str ) ) {
				inflectionFrac = double_of( inflectionFrac_str );
				if ( inflectionFrac < 0.0 ) {
					std::cerr << "\nError: Negative inflectionFrac: " << inflectionFrac_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "\nError: Nonnumeric inflectionFrac: " << inflectionFrac_str << std::endl;
				fatal = true;
			}
			if ( inflectionFrac > 1.0 ) {
				std::cerr << "\nWarning: inflectionFrac " << inflectionFrac << " > 1: Clipped to 1" << std::endl;
				inflectionFrac = 1.0;
			}
		} else if ( has_option( arg, "cluster" ) ) {
			cluster = true;
		} else if ( has_option( arg, "refine" ) ) {
			refine = true;
		} else if ( has_option( arg, "perfect" ) ) {
			perfect = true;
		} else if ( has_option( arg, "active" ) ) {
			active = true;
			passive = false;
		} else if ( has_option( arg, "passive" ) ) {
			active = false;
			passive = true;
		} else if ( has_option( arg, "steps" ) ) {
			steps = true;
		} else if ( has_option_value( arg, "log" ) ) { // Accept PyFMI numeric logging levels for scripting convenience
			std::string const log_str( lowercased( option_value( arg, "log" ) ) );
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
		} else if ( has_option_value( arg, "rTol" ) ) {
			specified::rTol = true;
			std::string const rTol_str( option_value( arg, "rTol" ) );
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
		} else if ( has_option_value( arg, "aTol" ) ) {
			specified::aTol = true;
			std::string const aTol_str( option_value( arg, "aTol" ) );
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
		} else if ( has_option_value( arg, "aFac" ) ) {
			std::string const aFac_str( option_value( arg, "aFac" ) );
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
		} else if ( has_option_value( arg, "zTol" ) ) {
			specified::zTol = true;
			std::string const zTol_str( option_value( arg, "zTol" ) );
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
		} else if ( has_option_value( arg, "zMul" ) ) {
			std::string const zMul_str( option_value( arg, "zMul" ) );
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
		} else if ( has_option_value( arg, "zFac" ) ) {
			std::string const zFac_str( option_value( arg, "zFac" ) );
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
		} else if ( has_option_value( arg, "zrFac" ) ) {
			std::string const zrFac_str( option_value( arg, "zrFac" ) );
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
		} else if ( has_option_value( arg, "zaFac" ) ) {
			std::string const zaFac_str( option_value( arg, "zaFac" ) );
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
		} else if ( has_option_value( arg, "dtMin" ) ) {
			std::string const dtMin_str( option_value( arg, "dtMin" ) );
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
		} else if ( has_option_value( arg, "dtMax" ) ) {
			std::string const dtMax_str( option_value( arg, "dtMax" ) );
			if ( is_double( dtMax_str ) ) {
				dtMax = double_of( dtMax_str );
				if ( dtMax <= 0.0 ) {
					std::cerr << "\nError: Nonpositive dtMax: " << dtMax_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "\nError: Nonnumeric dtMax: " << dtMax_str << std::endl;
				fatal = true;
			}
		} else if ( has_option_value( arg, "dtInf" ) ) {
			std::string const dtInf_str( option_value( arg, "dtInf" ) );
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
		} else if ( has_option_value( arg, "dtZMax" ) ) {
			std::string const dtZMax_str( option_value( arg, "dtZMax" ) );
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
		} else if ( has_option_value( arg, "dtZC" ) ) {
			specified::dtZC = true;
			std::string const dtZC_str( option_value( arg, "dtZC" ) );
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
		} else if ( has_option_value( arg, "dtND" ) ) {
			std::vector< std::string > const dtND_args( split( option_value( arg, "dtND" ), ':' ) );
			std::string const dtND_str( dtND_args[ 0 ] );
			if ( is_double( dtND_str ) ) {
				dtND_set( double_of( dtND_str ) );
				if ( dtND <= 0.0 ) {
					std::cerr << "\nError: Nonpositive dtND: " << dtND << std::endl;
					fatal = true;
				}
			} else if ( ( dtND_args.size() == 1u ) && ( is_any_of( dtND_args[ 0 ][ 0 ], "YyTtOo" ) ) ) { // dtND optimizer on
				dtND_optimizer = true;
			} else {
				std::cerr << "\nError: Nonnumeric dtND: " << dtND_str << std::endl;
				fatal = true;
			}
			if ( dtND_args.size() > 1u ) {
				bool const dtND_arg_1_char( dtND_args[ 1 ].length() == 1u );
				char const dtND_arg_1( dtND_args[ 1 ][ 0 ] );
				if ( dtND_arg_1_char && is_any_of( dtND_arg_1, "YyTtOo" ) ) { // dtND optimizer on
					dtND_optimizer = true;
				} else if ( dtND_arg_1_char && is_any_of( dtND_arg_1, "NnFf" ) ) { // dtND optimizer off
					dtND_optimizer = false;
				} else if ( is_double( dtND_args[ 1 ] ) ) {
					dtND_optimizer = true;
					std::string const dtND_max_str( dtND_args[ 1 ] );
					dtND_max = double_of( dtND_max_str );
					if ( dtND_max <= 0.0 ) {
						std::cerr << "\nError: Nonpositive upper dtND: " << dtND_max << std::endl;
						fatal = true;
					} else if ( dtND_max < 4.0 * dtND ) {
						dtND_max = 4.0 * dtND;
						std::cerr << "\nInfo: Upper dtND increased to 4 * dtND: " << dtND_max << std::endl;
					}
				} else {
					std::cerr << "\nError: Invalid dtND optimizer flag: " << dtND_args[ 1 ] << std::endl;
					fatal = true;
				}
			}
			if ( dtND_optimizer ) dtND_max = std::max( 4.0 * dtND, dtND_max );
		} else if ( has_option( arg, "dtND" ) ) {
			dtND_optimizer = true;
			dtND_max = std::max( 4.0 * dtND, dtND_max );
		} else if ( has_option_value( arg, "dtCon" ) ) {
			std::string const dtCon_str( option_value( arg, "dtCon" ) );
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
		} else if ( has_option_value( arg, "dtOut" ) ) {
			specified::dtOut = true;
			std::string const dtOut_str( option_value( arg, "dtOut" ) );
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
		} else if ( has_option_value( arg, "tEnd" ) ) {
			specified::tEnd = true;
			std::string const tEnd_str( option_value( arg, "tEnd" ) );
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
		} else if ( has_option( arg, "bin" ) ) {
			specified::bin = true;
			bin_size = std::numeric_limits< std::size_t >::max();
			bin_frac = 0.25;
			bin_auto = false;
		} else if ( has_option_value( arg, "bin" ) ) {
			specified::bin = true;
			std::string const bin_str( option_value( arg, "bin" ) );
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

			} else if ( !bin_str.empty() ) { // Treat single parameter as bin_size
				if ( is_size( bin_str ) ) {
					bin_size = size_of( bin_str );
				} else if ( bin_str == "U" ) { // Unlimited max bin size
					bin_size = std::numeric_limits< std::size_t >::max();
				} else {
					std::cerr << "\nError: bin size is not valid: " << bin_str << std::endl;
					fatal = true;
				}
			}
		} else if ( has_option_value( arg, "pass" ) ) {
			std::string const pass_str( option_value( arg, "pass" ) );
			if ( is_size( pass_str ) ) {
				pass = size_of( pass_str );
				if ( pass < 1 ) {
					std::cerr << "\nError: Nonpositive pass option: " << pass_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "\nError: Nonintegral pass option: " << pass_str << std::endl;
				fatal = true;
			}
		} else if ( has_option_value( arg, "fxn" ) ) {
			std::string const var_fxn( option_value( arg, "fxn" ) );
			if ( var_fxn[ 0 ] == '"' ) { // Quoted variable name
				std::string::size_type const qe( var_fxn.find( '"', 1u ) );
				if ( qe != std::string::npos ) {
					std::string::size_type const isep( var_fxn.find( ':', qe ) );
					if ( isep != std::string::npos ) {
						std::string const var_name( var_fxn.substr( 1u, qe - 1u ) );
						std::string const fxn_spec( var_fxn.substr( isep + 1u ) );
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
					std::string const var_name( var_fxn.substr( 0u, isep ) );
					std::string const fxn_spec( var_fxn.substr( isep + 1u ) );
					fxn[ var_name ] = fxn_spec;
				} else {
					std::cerr << "\nError: Input variable function spec not in variable:function format: " << var_fxn << std::endl;
					fatal = true;
				}
			}
		} else if ( has_option_value( arg, "con" ) ) {
			std::string const inp_out( option_value( arg, "con" ) );
			std::string inp_name;
			std::string out_name;
			if ( inp_out[ 0 ] == '"' ) { // Quoted input variable name
				std::string::size_type const qe( inp_out.find( '"', 1u ) );
				if ( qe != std::string::npos ) {
					inp_name = std::string_view( inp_out ).substr( 1u, qe - 1u );
					std::string::size_type const isep( inp_out.find( ':', qe ) );
					if ( isep != std::string::npos ) {
						out_name = std::string_view( inp_out ).substr( isep + 1u );
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
					inp_name = std::string_view( inp_out ).substr( 0u, isep );
					out_name = std::string_view( inp_out ).substr( isep + 1u );
				} else {
					std::cerr << "\nError: Input-output connection spec not in input:output format: " << inp_out << std::endl;
					fatal = true;
				}
			}
			if ( out_name[ 0 ] == '"' ) { // Quoted output variable name
				std::string::size_type const qe( inp_out.find( '"', 1u ) );
				if ( qe != std::string::npos ) {
					out_name = std::string_view( out_name ).substr( 1u, qe - 1u );
				} else {
					std::cerr << "\nError: Input-output connection spec quoted output variable name missing end quote: " << inp_out << std::endl;
					fatal = true;
				}
			}
			con[ inp_name ] = out_name;
		} else if ( has_option( arg, "dep" ) || has_option_value( arg, "dep" ) ) {
			std::string var_spec;
			std::string deps_spec;
			if ( has_option( arg, "dep" ) ) {
				var_spec = deps_spec = '*';
			} else {
				std::string const var_deps( option_value( arg, "dep" ) );
				if ( var_deps.empty() ) { // Implied all
					var_spec = deps_spec = '*';
				} else if ( var_deps[ 0 ] == '"' ) { // Quoted input variable name
					std::string::size_type const qe( var_deps.find( '"', 1u ) );
					if ( qe != std::string::npos ) {
						var_spec = std::string_view( var_deps ).substr( 1u, qe - 1u );
						std::string::size_type const isep( var_deps.find( ':', qe ) );
						if ( isep != std::string::npos ) {
							deps_spec = std::string_view( var_deps ).substr( isep + 1u );
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
						var_spec = std::string_view( var_deps ).substr( 0u, isep );
						deps_spec = std::string_view( var_deps ).substr( isep + 1u );
					} else {
						var_spec = var_deps;
						deps_spec = '*'; // Implied all
					}
				}
				if ( var_spec.empty() ) var_spec = '*'; // Implied all
			}
			std::vector< std::string > dep_specs;
			{ // Scope
			std::string dep_spec;
			bool in_quote( false );
			for ( char const c : deps_spec ) {
				if ( c == '"' ) {
					if ( in_quote ) { // End quoted string
						if ( !strip( dep_spec ).empty() ) {
							dep_specs.push_back( dep_spec );
							dep_spec.clear();
						}
					}
					in_quote = !in_quote;
				} else if ( ( !in_quote ) && ( c == ',' ) ) {
					if ( !strip( dep_spec ).empty() ) {
						dep_specs.push_back( dep_spec );
						dep_spec.clear();
					}
				} else {
					dep_spec.push_back( c );
				}
			}
			if ( !strip( dep_spec ).empty() ) dep_specs.push_back( dep_spec );
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
				var_regex = DepSpecs::regex( var_spec );
			} catch (...) {
				std::cerr << "\nError: Dependency variable spec cannot be converted into a regex " << var_spec << std::endl;
				fatal = true;
			}
			std::vector< std::regex > deps_regex;
			for ( std::string const & dep_spec : dep_specs ) {
				try {
					deps_regex.push_back( DepSpecs::regex( dep_spec ) );
				} catch (...) {
					std::cerr << "\nError: Dependency spec cannot be converted into a regex " << dep_spec << std::endl;
					fatal = true;
				}
			}
			dep.add( var_regex, deps_regex );
		} else if ( has_option_value( arg, "out" ) ) {
			static std::string const out_flags( "dshROZDSXQTAFLK" );
			char const sep( option_sep( arg, "out" ) );
			std::string const out( option_value( arg, "out" ) );
			if ( has_any_not_of( out, out_flags ) ) {
				std::cerr << "\nError: Output option has flag not in " << out_flags << ": " << out << std::endl;
				fatal = true;
			}
			if ( is_any_of( sep, "=:" ) ) {
				output::d = has( out, 'd' );
				output::s = has( out, 's' );
				output::h = has( out, 'h' );
				output::R = has( out, 'R' );
				output::O = has( out, 'O' );
				output::Z = has( out, 'Z' );
				output::D = has( out, 'D' );
				output::S = has( out, 'S' );
				output::X = has( out, 'X' );
				output::Q = has( out, 'Q' );
				output::T = has( out, 'T' );
				output::A = has( out, 'A' );
				output::F = has( out, 'F' );
				output::L = has( out, 'L' );
				output::K = has( out, 'K' );
			} else if ( sep == '+' ) { // Add specified outputs
				for ( char c : out ) {
					switch ( c ) {
					case 'd':
						output::d = true;
						break;
					case 's':
						output::s = true;
						break;
					case 'h':
						output::h = true;
						break;
					case 'R':
						output::R = true;
						break;
					case 'O':
						output::O = true;
						break;
					case 'Z':
						output::Z = true;
						break;
					case 'D':
						output::D = true;
						break;
					case 'S':
						output::S = true;
						break;
					case 'X':
						output::X = true;
						break;
					case 'Q':
						output::Q = true;
						break;
					case 'T':
						output::T = true;
						break;
					case 'A':
						output::A = true;
						break;
					case 'F':
						output::F = true;
						break;
					case 'L':
						output::L = true;
						break;
					case 'K':
						output::K = true;
						break;
					}
				}
			} else if ( sep == '-' ) { // Remove specified outputs
				for ( char c : out ) {
					switch ( c ) {
					case 'd':
						output::d = false;
						break;
					case 's':
						output::s = false;
						break;
					case 'h':
						output::h = false;
						break;
					case 'R':
						output::R = false;
						break;
					case 'O':
						output::O = false;
						break;
					case 'Z':
						output::Z = false;
						break;
					case 'D':
						output::D = false;
						break;
					case 'S':
						output::S = false;
						break;
					case 'X':
						output::X = false;
						break;
					case 'Q':
						output::Q = false;
						break;
					case 'T':
						output::T = false;
						break;
					case 'A':
						output::A = false;
						break;
					case 'F':
						output::F = false;
						break;
					case 'L':
						output::L = false;
						break;
					case 'K':
						output::K = false;
						break;
					}
				}
			} else if ( sep == ' ' ) { // Remove all outputs
				output::d = false;
				output::s = false;
				output::h = false;
				output::R = false;
				output::O = false;
				output::Z = false;
				output::D = false;
				output::S = false;
				output::X = false;
				output::Q = false;
				output::T = false;
				output::A = false;
				output::F = false;
				output::L = false;
				output::K = false;
			}
		} else if ( has_option( arg, "out" ) ) {
			output::d = false;
			output::s = false;
			output::h = false;
			output::R = false;
			output::O = false;
			output::Z = false;
			output::D = false;
			output::S = false;
			output::X = false;
			output::Q = false;
			output::T = false;
			output::A = false;
			output::F = false;
			output::L = false;
			output::K = false;
		} else if ( has_option( arg, "csv" ) ) {
			csv = true;
		} else if ( has_option_value( arg, "dot" ) ) {
			static std::string const dot_flags( "dre" );
			std::string const dot( option_value( arg, "dot" ) );
			if ( dot.empty() ) { // Default to all dot graphs if --dot= specified with no argument
				dot_graph::d = true;
				dot_graph::r = true;
				dot_graph::e = true;
			} else if ( has_any_not_of( dot, dot_flags ) ) {
				std::cerr << "\nError: Dot graph option has flag not in " << dot_flags << ": " << dot << std::endl;
				fatal = true;
			} else {
				dot_graph::d = has( dot, 'd' );
				dot_graph::r = has( dot, 'r' );
				dot_graph::e = has( dot, 'e' );
			}
		} else if ( has_option( arg, "dot" ) ) {
			dot_graph::d = true;
			dot_graph::r = true;
			dot_graph::e = true;
		} else if ( has_option_value( arg, "tLoc" ) ) {
			specified::tLoc = true;
			std::string const tLoc_str( option_value( arg, "tLoc" ) );
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
		} else if ( has_option_value( arg, "var" ) ) {
			var = option_value( arg, "var" );
			if ( !path::is_file( var ) ) {
				std::cerr << "\nError: File specified in --var option not found: " << var << ": Output filtering disabled" << std::endl;
			}
		} else if ( arg[ 0 ] == '-' ) {
			std::cerr << "\nError: Unsupported option: " << arg << std::endl;
			fatal = true;
		} else { // Treat non-option argument as model
			models.push_back( arg );
		}
		if ( !specified::aTol ) aTol = rTol * aFac; // Make unspecified aTol consistent with rTol * aFac
	}

	// Inter-option checks
	if ( specified::rTol && ( rTol * zFac * zrFac >= 1.0 ) ) {
		std::cerr << "\nWarning: Zero-crossing relative tolerance: rTol * zFac * zrFac = " << rTol * zFac * zrFac << " >= 1" << std::endl;
	}

	if ( help ) std::exit( EXIT_SUCCESS );
	if ( version_arg ) std::exit( EXIT_SUCCESS );
	if ( fatal ) std::exit( EXIT_FAILURE );

	std::cout << "\nQSS Command Line Arguments:\n";
	for ( Args::size_type i = 1; i < argc; ++i ) {
		std::cout << ' ' <<  args[ i ] << '\n';
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
	return ( !con.empty() );
}

// Set dtOut to Default for a Given Time Span
void
dtOut_set( double const t )
{
	if ( !specified::dtOut ) dtOut = std::pow( 10.0, std::round( std::log10( t * 0.0002 ) ) );
}

// Set dtND
void
dtND_set( double const dt )
{
	dtND = dt;
	two_dtND = 2.0 * dtND;
	one_over_two_dtND = 1.0 / ( 2.0 * dtND );
	one_over_three_dtND = 1.0 / ( 3.0 * dtND );
	one_over_four_dtND = 1.0 / ( 4.0 * dtND );
	one_over_six_dtND_squared = 1.0 / ( 6.0 * ( dtND * dtND ) );
}

} // options
} // QSS
