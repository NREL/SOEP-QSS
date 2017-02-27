// Stiff System Example Setup
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/ex_stiff.hh>
#include <QSS/options.hh>
#include <QSS/Function_LTI.hh>
#include <QSS/Variable_LIQSS1.hh>
#include <QSS/Variable_LIQSS2.hh>
#include <QSS/Variable_QSS1.hh>
#include <QSS/Variable_QSS2.hh>
#include <QSS/Variable_QSS3.hh>

namespace ex {

using Variables = std::vector< Variable * >;

// Stiff System Example Setup
//
// Demonstrates LIQSS benefits for stiff systems
// Recommend --tEnd=600
void
stiff( Variables & vars )
{
	using namespace options;

	// Variables
	using V = Variable_QSS< Function_LTI >;
	V * x1( nullptr );
	V * x2( nullptr );
	vars.clear();
	vars.reserve( 2 );
	if ( qss == QSS::QSS1 ) {
		vars.push_back( x1 = new Variable_QSS1< Function_LTI >( "x1", rTol, aTol, 0.0 ) );
		vars.push_back( x2 = new Variable_QSS1< Function_LTI >( "x2", rTol, aTol, 20.0 ) );
	} else if ( qss == QSS::QSS2 ) {
		vars.push_back( x1 = new Variable_QSS2< Function_LTI >( "x1", rTol, aTol, 0.0 ) );
		vars.push_back( x2 = new Variable_QSS2< Function_LTI >( "x2", rTol, aTol, 20.0 ) );
	} else if ( qss == QSS::QSS3 ) {
		vars.push_back( x1 = new Variable_QSS3< Function_LTI >( "x1", rTol, aTol, 0.0 ) );
		vars.push_back( x2 = new Variable_QSS3< Function_LTI >( "x2", rTol, aTol, 20.0 ) );
	} else if ( qss == QSS::LIQSS1 ) {
		vars.push_back( x1 = new Variable_LIQSS1< Function_LTI >( "x1", rTol, aTol, 0.0 ) );
		vars.push_back( x2 = new Variable_LIQSS1< Function_LTI >( "x2", rTol, aTol, 20.0 ) );
	} else if ( qss == QSS::LIQSS2 ) {
		vars.push_back( x1 = new Variable_LIQSS2< Function_LTI >( "x1", rTol, aTol, 0.0 ) );
		vars.push_back( x2 = new Variable_LIQSS2< Function_LTI >( "x2", rTol, aTol, 20.0 ) );
	} else {
		std::cerr << "Unsupported QSS method" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Derivatives
	x1->d().add( 0.01, x2 );
	x2->d().add( 2020.0 ).add( -100.0, x1 ).add( -100.0, x2 );
}

} // ex
