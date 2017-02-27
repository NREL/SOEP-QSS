// Nonlinear Derivative Example Setup
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/ex_nonlinear.hh>
#include <QSS/options.hh>
#include <QSS/Function_nonlinear.hh>
#include <QSS/Variable_LIQSS1.hh>
#include <QSS/Variable_LIQSS2.hh>
#include <QSS/Variable_QSS1.hh>
#include <QSS/Variable_QSS2.hh>
#include <QSS/Variable_QSS3.hh>

// C++ Headers
#include <cstddef>
#include <fstream>

namespace ex {

using Variables = std::vector< Variable * >;

// Nonlinear Derivative Example Setup
//
// Recommend --tEnd=5
void
nonlinear( Variables & vars )
{
	using namespace options;

	// Variables
	using V = Variable_QSS< Function_nonlinear >;
	V * y( nullptr );
	vars.clear();
	vars.reserve( 1 );
	if ( qss == QSS::QSS1 ) {
		vars.push_back( y = new Variable_QSS1< Function_nonlinear >( "y", rTol, aTol, 2.0 ) );
	} else if ( qss == QSS::QSS2 ) {
		vars.push_back( y = new Variable_QSS2< Function_nonlinear >( "y", rTol, aTol, 2.0 ) );
	} else if ( qss == QSS::QSS3 ) {
		vars.push_back( y = new Variable_QSS3< Function_nonlinear >( "y", rTol, aTol, 2.0 ) );
	} else if ( qss == QSS::LIQSS1 ) {
		vars.push_back( y = new Variable_LIQSS1< Function_nonlinear >( "y", rTol, aTol, 2.0 ) );
	} else if ( qss == QSS::LIQSS2 ) {
		vars.push_back( y = new Variable_LIQSS2< Function_nonlinear >( "y", rTol, aTol, 2.0 ) );
	} else {
		std::cerr << "Unsupported QSS method" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Derivatives
	y->d().var( y );

	// Analytical solution output
	std::ofstream e_stream( "y.e.out" );
	std::size_t iOut( 0 );
	double tOut( 0.0 );
	while ( tOut <= options::tEnd * ( 1.0 + 1.0e-14 ) ) {
		e_stream << tOut << '\t' << y->d().e( tOut ) << '\n';
		tOut = ( ++iOut ) * options::dtOut;
	}
	e_stream.close();
}

} // ex
