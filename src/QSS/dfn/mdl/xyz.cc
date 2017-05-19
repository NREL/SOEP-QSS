// Simple xyz Example Setup
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/dfn/mdl/xyz.hh>
#include <QSS/dfn/mdl/Function_LTI.hh>
#include <QSS/dfn/Variable_LIQSS1.hh>
#include <QSS/dfn/Variable_LIQSS2.hh>
#include <QSS/dfn/Variable_QSS1.hh>
#include <QSS/dfn/Variable_QSS2.hh>
#include <QSS/dfn/Variable_QSS3.hh>
#include <QSS/options.hh>

namespace QSS {
namespace dfn {
namespace mdl {

using Variables = std::vector< Variable * >;

// Simple xyz Example Setup
void
xyz( Variables & vars )
{
	using namespace options;

	// Timing
	if ( ! options::tEnd_set ) options::tEnd = 2.0;

	// Variables
	using V = Variable_QSS< Function_LTI >;
	V * x( nullptr );
	V * y( nullptr );
	V * z( nullptr );
	vars.clear();
	vars.reserve( 3 );
	if ( qss == QSS::QSS1 ) {
		vars.push_back( x = new Variable_QSS1< Function_LTI >( "x", rTol, aTol, 0.0 ) );
		vars.push_back( y = new Variable_QSS1< Function_LTI >( "y", rTol, aTol, 0.0 ) );
		vars.push_back( z = new Variable_QSS1< Function_LTI >( "z", rTol, aTol, 0.0 ) );
	} else if ( qss == QSS::QSS2 ) {
		vars.push_back( x = new Variable_QSS2< Function_LTI >( "x", rTol, aTol, 0.0 ) );
		vars.push_back( y = new Variable_QSS2< Function_LTI >( "y", rTol, aTol, 0.0 ) );
		vars.push_back( z = new Variable_QSS2< Function_LTI >( "z", rTol, aTol, 0.0 ) );
	} else if ( qss == QSS::QSS3 ) {
		vars.push_back( x = new Variable_QSS3< Function_LTI >( "x", rTol, aTol, 0.0 ) );
		vars.push_back( y = new Variable_QSS3< Function_LTI >( "y", rTol, aTol, 0.0 ) );
		vars.push_back( z = new Variable_QSS3< Function_LTI >( "z", rTol, aTol, 0.0 ) );
	} else if ( qss == QSS::LIQSS1 ) {
		vars.push_back( x = new Variable_LIQSS1< Function_LTI >( "x", rTol, aTol, 0.0 ) );
		vars.push_back( y = new Variable_LIQSS1< Function_LTI >( "y", rTol, aTol, 0.0 ) );
		vars.push_back( z = new Variable_LIQSS1< Function_LTI >( "z", rTol, aTol, 0.0 ) );
	} else if ( qss == QSS::LIQSS2 ) {
		vars.push_back( x = new Variable_LIQSS2< Function_LTI >( "x", rTol, aTol, 0.0 ) );
		vars.push_back( y = new Variable_LIQSS2< Function_LTI >( "y", rTol, aTol, 0.0 ) );
		vars.push_back( z = new Variable_LIQSS2< Function_LTI >( "z", rTol, aTol, 0.0 ) );
	} else {
		std::cerr << "Unsupported QSS method" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Derivatives
	x->d().add( 1.0 );
	y->d().add( 1.0, x );
	z->d().add( 1.0, y );
}

} // mdl
} // dfn
} // QSS