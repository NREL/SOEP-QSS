// Achilles and the Tortoise Custom Function Example Setup
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/dfn/mdl/achillesc.hh>
#include <QSS/dfn/mdl/Function_achilles1.hh>
#include <QSS/dfn/mdl/Function_achilles2.hh>
//#include <QSS/dfn/Variable_LIQSS1.hh>
//#include <QSS/dfn/Variable_LIQSS2.hh>
#include <QSS/dfn/Variable_QSS1.hh>
#include <QSS/dfn/Variable_QSS2.hh>
#include <QSS/dfn/Variable_QSS3.hh>
#include <QSS/options.hh>

namespace QSS {
namespace dfn {
namespace mdl {

using Variables = std::vector< Variable * >;

// Achilles and the Tortoise Custom Function Example Setup
void
achillesc( Variables & vars )
{
	using namespace options;

	// Timing
	if ( ! options::tEnd_set ) options::tEnd = 10.0;

	// Variables
	using V1 = Variable_QSS< Function_achilles1 >;
	using V2 = Variable_QSS< Function_achilles2 >;
	V1 * x1( nullptr );
	V2 * x2( nullptr );
	vars.clear();
	vars.reserve( 2 );
	if ( qss == QSS::QSS1 ) {
		vars.push_back( x1 = new Variable_QSS1< Function_achilles1 >( "x1", rTol, aTol, 0.0 ) );
		vars.push_back( x2 = new Variable_QSS1< Function_achilles2 >( "x2", rTol, aTol, 2.0 ) );
	} else if ( qss == QSS::QSS2 ) {
		vars.push_back( x1 = new Variable_QSS2< Function_achilles1 >( "x1", rTol, aTol, 0.0 ) );
		vars.push_back( x2 = new Variable_QSS2< Function_achilles2 >( "x2", rTol, aTol, 2.0 ) );
	} else if ( qss == QSS::QSS3 ) {
		vars.push_back( x1 = new Variable_QSS3< Function_achilles1 >( "x1", rTol, aTol, 0.0 ) );
		vars.push_back( x2 = new Variable_QSS3< Function_achilles2 >( "x2", rTol, aTol, 2.0 ) );
//	} else if ( qss == QSS::LIQSS1 ) {
//		vars.push_back( x1 = new Variable_LIQSS1< Function_achilles1 >( "x1", rTol, aTol, 0.0 ) );
//		vars.push_back( x2 = new Variable_LIQSS1< Function_achilles2 >( "x2", rTol, aTol, 2.0 ) );
//	} else if ( qss == QSS::LIQSS2 ) {
//		vars.push_back( x1 = new Variable_LIQSS2< Function_achilles1 >( "x1", rTol, aTol, 0.0 ) );
//		vars.push_back( x2 = new Variable_LIQSS2< Function_achilles2 >( "x2", rTol, aTol, 2.0 ) );
	} else {
		std::cerr << "Unsupported QSS method" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Derivatives
	x1->d().var( x1, x2 );
	x2->d().var( x1 );
}

} // mdl
} // dfn
} // QSS
