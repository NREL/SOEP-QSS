// QSS Solver Main
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/dfn/simulate_dfn.hh>
#include <QSS/fmu/simulate_fmu.hh>
#include <QSS/options.hh>

// C++ Headers
#include <cstdlib>
#include <iostream>

// QSS Solver Main
int
main( int argc, char * argv[] )
{
	using namespace QSS;

	// Process command line arguments
	options::process_args( argc, argv );

	// Run FMU or example model simulation
	if ( options::model.empty() ) {
		std::cerr << "No model name or FMU file specified" << std::endl;
		std::exit( EXIT_FAILURE );
	} else if ( ( options::model.length() >= 5 ) && ( options::model.rfind( ".fmu" ) == options::model.length() - 4u ) ) { // FMU
		fmu::simulate_fmu();
	} else { // Example
		dfn::simulate_dfn();
	}
}