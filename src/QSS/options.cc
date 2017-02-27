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
bool inflection( false ); // Requantize at inflections?  [F]
double rTol( 1.0e-4 ); // Relative tolerance  [1e-4|FMU]
double aTol( 1.0e-6 ); // Absolute tolerance  [1e-6]
bool rTol_set( false ); // Relative tolerance set?
double dtOut( 1.0e-3 ); // Sampled & FMU output time step (s)  [1e-3]
double dtND( 1.0e-6 ); // Numeric differentiation time step (s)  [1e-6]
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

// Process command line arguments
void
process_args( int argc, char * argv[] )
{
	bool fatal( false ); // Fatal error occurred?
	for ( int i = 1; i < argc; ++i ) {
		std::string const arg( argv[ i ] );
		if ( ( arg == "--help" ) || ( arg == "-h" ) ) {
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
			std::exit( EXIT_SUCCESS );
		} else if ( has_value_option( arg, "qss" ) ) {
			std::string const qss_name( uppercased( arg_value( arg ) ) );
			if ( qss_name == "QSS1" ) {
				options::qss = QSS::QSS1;
			} else if ( qss_name == "QSS2" ) {
				options::qss = QSS::QSS2;
			} else if ( qss_name == "QSS3" ) {
				options::qss = QSS::QSS3;
			} else if ( qss_name == "LIQSS1" ) {
				options::qss = QSS::LIQSS1;
			} else if ( qss_name == "LIQSS2" ) {
				options::qss = QSS::LIQSS2;
			} else if ( qss_name == "LIQSS3" ) {
				options::qss = QSS::LIQSS3;
				std::cerr << "The LIQSS3 method is not yet implemented" << std::endl;
				fatal = true;
			} else {
				std::cerr << "Unsupported QSS method: " << qss_name << std::endl;
				fatal = true;
			}
		} else if ( has_option( arg, "inflection" ) ) {
			options::inflection = true;
		} else if ( has_value_option( arg, "rTol" ) ) {
			std::string const rTol( arg_value( arg ) );
			if ( is_double( rTol ) ) {
				options::rTol = double_of( rTol );
				if ( options::rTol < 0.0 ) {
					std::cerr << "Negative rTol: " << rTol << std::endl;
					fatal = true;
				}
				rTol_set = true;
			} else {
				std::cerr << "Nonnumeric rTol: " << rTol << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "aTol" ) ) {
			std::string const aTol( arg_value( arg ) );
			if ( is_double( aTol ) ) {
				options::aTol = double_of( aTol );
				if ( options::aTol <= 0.0 ) {
					std::cerr << "Nonpositive aTol: " << aTol << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "Nonnumeric aTol: " << aTol << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "dtOut" ) ) {
			std::string const dtOut( arg_value( arg ) );
			if ( is_double( dtOut ) ) {
				options::dtOut = double_of( dtOut );
				if ( options::dtOut < 0.0 ) {
					std::cerr << "Negative dtOut: " << dtOut << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "Nonnumeric dtOut: " << dtOut << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "dtND" ) ) {
			std::string const dtND( arg_value( arg ) );
			if ( is_double( dtND ) ) {
				options::dtND = double_of( dtND );
				if ( options::dtND < 0.0 ) {
					std::cerr << "Negative dtND: " << dtND << std::endl;
					fatal = true;
				}
			} else {
				std::cerr << "Nonnumeric dtND: " << dtND << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "tEnd" ) ) {
			std::string const tEnd( arg_value( arg ) );
			if ( is_double( tEnd ) ) {
				options::tEnd = double_of( tEnd );
				if ( options::tEnd < 0.0 ) {
					std::cerr << "Negative tEnd: " << tEnd << std::endl;
					fatal = true;
				}
				options::tEnd_set = true;
			} else {
				std::cerr << "Nonnumeric tEnd: " << tEnd << std::endl;
				fatal = true;
			}
		} else if ( has_value_option( arg, "out" ) ) {
			options::out = arg_value( arg );
			if ( has_any_not_of( options::out, "rasfdxq" ) ) {
				std::cerr << "Output flag not in rasfdxq: " << options::out << std::endl;
				fatal = true;
			}
			options::output::r = has( options::out, 'r' );
			options::output::a = has( options::out, 'a' );
			options::output::s = has( options::out, 's' );
			options::output::f = has( options::out, 'f' );
			options::output::d = has( options::out, 'd' );
			options::output::x = has( options::out, 'x' );
			options::output::q = has( options::out, 'q' );
			if ( options::output::a ) options::output::r = true; // a => r
		} else { // Treat non-option argument as model
			options::model = arg;
		}
	}

	if ( fatal ) std::exit( EXIT_FAILURE );
}

} // options
