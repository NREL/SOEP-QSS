// Options Support
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/options.hh>

// C++ Headers
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace options {

QSS qss( QSS::QSS2 ); // QSS method: (LI)QSS1|2|3  [QSS2]
int qss_order( 2 ); // QSS method order  [computed]
bool inflection( false ); // Requantize at inflections?  [F]
double rTol( 1.0e-4 ); // Relative tolerance  [1e-4|FMU]
double aTol( 1.0e-6 ); // Absolute tolerance  [1e-6]
bool rTol_set( false ); // Relative tolerance set?
double dtOut( 1.0e-3 ); // Sampled & FMU output time step (s)  [1e-3]
double dtND( 1.0e-6 ); // Numeric differentiation time step (s)  [1e-6]
double one_half_over_dtND( 5.0e5 ); // 0.5 / dtND  [computed]
double tEnd( 1.0 ); // End time (s)  [1|FMU]
bool tEnd_set( false ); // End time set?
std::string out; // Outputs: r, a, s, x, q, f  [rx]
std::string model; // Name of model or FMU

namespace output { // Output selections

bool r( true ); // Requantization events?  [T]
bool a( false ); // All variables at requantization events?  [F]
bool s( false ); // Sampled output?  [F]
bool f( true ); // FMU outputs?  [T]
bool d( false ); // Diagnostic output?  [F]
bool x( true ); // Continuous trajectories?  [T]
bool q( false ); // Quantized trajectories?  [F]

} // out

// Uppercased string
std::string
uppercased( std::string const & s )
{
	std::string u( s.length(), ' ' );
	for ( std::string::size_type i = 0, e = s.length(); i < e; ++i ) {
		u[ i ] = std::toupper( s[ i ] );
	}
	return u;
}

// Is char Pointer Pointing to String Whitespace Tail
inline
bool
is_tail( char * end )
{
	if ( end == nullptr ) return false;
	while ( std::isspace( *end ) ) ++end;
	return ( *end == '\0' );
}

// string is Readable as a double?
inline
bool
is_double( std::string const & s )
{
	char const * str( s.c_str() );
	char * end;
	static_cast< void >( std::strtod( str, &end ) );
	return ( ( end != str ) && is_tail( end ) );
}

// double of a string
inline
double
double_of( std::string const & s )
{
	return std::stod( s ); // Check is_double first
}

// Has an Option (Case-Insensitive)?
bool
has_option( std::string const & s, char const * const option )
{
	std::string const opt( "--" + std::string( option ) );
	std::string::size_type const opt_len( opt.length() );
	if ( s.length() != opt_len ) {
		return false;
	} else {
		for ( std::string::size_type i = 0; i < opt_len; ++i ) {
			if ( std::tolower( s[ i ] ) != std::tolower( opt[ i ] ) ) return false;
		}
		return true;
	}
}

// Has a Value Option (Case-Insensitive)?
bool
has_value_option( std::string const & s, char const * const option )
{
	std::string const opt( "--" + std::string( option ) );
	std::string::size_type const opt_len( opt.length() );
	if ( s.length() <= opt_len ) {
		return false;
	} else {
		for ( std::string::size_type i = 0; i < opt_len; ++i ) {
			if ( std::tolower( s[ i ] ) != std::tolower( opt[ i ] ) ) return false;
		}
		return ( s[ opt_len ] == '=' ) || ( s[ opt_len ] == ':' );
	}
}

// Has a Character Case-Insensitively?
bool
has( std::string const & s, char const c )
{
	char const b( std::tolower( c ) );
	for ( char const a : s ) {
		if ( std::tolower( a ) == b ) return true;
	}
	return false;
}

// Has any Character not in a String Case-Insensitively?
bool
has_any_not_of( std::string const & s, std::string const & t ) // Pass lowercase t
{
	for ( char const a : s ) {
		if ( t.find( std::tolower( a ) ) == std::string::npos ) return true;
	}
	return false;
}

// Argument Value
std::string
arg_value( std::string const & arg )
{
	std::string::size_type const i( arg.find_first_of( "=:" ) );
	if ( i != std::string::npos ) {
		return arg.substr( i + 1 );
	} else {
		return std::string();
	}
}

// Help Display
void
help_display()
{
	std::cout << '\n' << "QSS [options] [model|fmu]" << "\n\n";
	std::cout << "Options:" << "\n\n";
	std::cout << " --qss=METHOD  QSS method: (LI)QSS1|2|3  [QSS2]" << '\n';
	std::cout << " --inflection  Requantize at inflections?  [F]" << '\n';
	std::cout << " --rTol=TOL    Relative tolerance  [1e-4|FMU]" << '\n';
	std::cout << " --aTol=TOL    Absolute tolerance  [1e-6]" << '\n';
	std::cout << " --dtOut=STEP  Sampled & FMU output step (s)  [1e-3]" << '\n';
	std::cout << " --dtND=STEP   Numeric differentiation step (s)  [1e-6]" << '\n';
	std::cout << " --tEnd=TIME   End time (s)  [1|FMU]" << '\n';
	std::cout << " --out=OUTPUTS Outputs: r, a, s, d, x, q, f  [rfx]" << '\n';
	std::cout << "       r       Requantization events" << '\n';
	std::cout << "       a       All variables at requantizations (=> r)" << '\n';
	std::cout << "       s       Sampled time steps" << '\n';
	std::cout << "       f       FMU outputs" << '\n';
	std::cout << "       d       Diagnostic output" << '\n';
	std::cout << "       x       Continuous trajectories" << '\n';
	std::cout << "       q       Quantized trajectories" << '\n';
	std::cout << '\n';
	std::cout << "Models:" << "\n\n";
	std::cout << "  achilles : Achilles and the Tortoise" << '\n';
	std::cout << "  achilles2 : Adds symmetry for simultaneous triggering" << '\n';
	std::cout << "  achillesc : Custom functions demo " << '\n';
	std::cout << "  achilles_ND : Numeric differentiation" << '\n';
	std::cout << "  exponential_decay : Exponential decay" << '\n';
	std::cout << "  exponential_decay_sine : Adds sine input function" << '\n';
	std::cout << "  exponential_decay_sine_ND : Numeric differentiation" << '\n';
	std::cout << "  nonlinear : Nonlinear derivative demo" << '\n';
	std::cout << "  nonlinear_ND : Numeric differentiation" << '\n';
	std::cout << "  stiff : Stiff system from literature" << '\n';
	std::cout << "  xy : Simple 2 variable model" << '\n';
	std::cout << "  xyz : Simple 3 variable model" << '\n';
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
				std::cerr << "The LIQSS3 method is not yet implemented" << std::endl;
				fatal = true;
			} else {
				std::cerr << "Unsupported QSS method: " << qss_name << std::endl;
				fatal = true;
			}
		} else if ( has_option( arg, "inflection" ) ) {
			inflection = true;
		} else if ( has_value_option( arg, "rTol" ) ) {
			std::string const rTol_str( arg_value( arg ) );
			if ( is_double( rTol_str ) ) {
				rTol = double_of( rTol_str );
				if ( rTol < 0.0 ) {
					std::cerr << "Negative rTol: " << rTol_str << std::endl;
					fatal = true;
				}
				rTol_set = true;
			} else {
				std::cerr << "Nonnumeric rTol: " << rTol_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "aTol" ) ) {
			std::string const aTol_str( arg_value( arg ) );
			if ( is_double( aTol_str ) ) {
				aTol = double_of( aTol_str );
				if ( aTol <= 0.0 ) {
					std::cerr << "Nonpositive aTol: " << aTol_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "Nonnumeric aTol: " << aTol_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "dtOut" ) ) {
			std::string const dtOut_str( arg_value( arg ) );
			if ( is_double( dtOut_str ) ) {
				dtOut = double_of( dtOut_str );
				if ( dtOut < 0.0 ) {
					std::cerr << "Negative dtOut: " << dtOut_str << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "Nonnumeric dtOut: " << dtOut_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "dtND" ) ) {
			std::string const dtND_str( arg_value( arg ) );
			if ( is_double( dtND_str ) ) {
				dtND = double_of( dtND_str );
				if ( dtND < 0.0 ) {
					std::cerr << "Negative dtND: " << dtND_str << std::endl;
					fatal = true;
				}
				one_half_over_dtND = 0.5 / dtND;
			} else {
				std::cerr << "Nonnumeric dtND: " << dtND_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "tEnd" ) ) {
			std::string const tEnd_str( arg_value( arg ) );
			if ( is_double( tEnd_str ) ) {
				tEnd = double_of( tEnd_str );
				if ( tEnd < 0.0 ) {
					std::cerr << "Negative tEnd: " << tEnd_str << std::endl;
					fatal = true;
				}
				tEnd_set = true;
			} else {
				std::cerr << "Nonnumeric tEnd: " << tEnd_str << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "out" ) ) {
			out = arg_value( arg );
			if ( has_any_not_of( out, "rasfdxq" ) ) {
				std::cerr << "Output flag not in rasfdxq: " << out << std::endl;
				fatal = true;
			}
			output::r = has( out, 'r' );
			output::a = has( out, 'a' );
			output::s = has( out, 's' );
			output::f = has( out, 'f' );
			output::d = has( out, 'd' );
			output::x = has( out, 'x' );
			output::q = has( out, 'q' );
			if ( output::a ) output::r = true; // a => r
		} else { // Treat non-option argument as model
			model = arg;
		}
	}

	if ( help ) std::exit( EXIT_SUCCESS );
	if ( fatal ) std::exit( EXIT_FAILURE );
}

} // options
