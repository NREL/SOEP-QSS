// Exponential Decay with Sine Input and Numeric Differentiation Example Setup
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/dfn/mdl/exponential_decay_sine.hh>
#include <QSS/dfn/mdl/Function_LTI.hh>
#include <QSS/dfn/mdl/Function_sin_ND.hh>
#include <QSS/dfn/Variable_Inp1.hh>
#include <QSS/dfn/Variable_Inp2.hh>
#include <QSS/dfn/Variable_Inp3.hh>
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

// Exponential Decay with Sine Input and Numeric Differentiation Example Setup
void
exponential_decay_sine_ND( Variables & vars )
{
	using namespace options;

	// Timing
	if ( ! options::tEnd_set ) options::tEnd = 50.0;

	// Variables
	using V = Variable_QSS< Function_LTI >;
	using I = Variable_Inp< Function_sin_ND >;
	V * x( nullptr );
	I * u( nullptr );
	vars.clear();
	vars.reserve( 2 );
	if ( qss == QSS::QSS1 ) {
		vars.push_back( x = new Variable_QSS1< Function_LTI >( "x", rTol, aTol, 1.0 ) );
		vars.push_back( u = new Variable_Inp1< Function_sin_ND >( "u", rTol, aTol ) );
	} else if ( qss == QSS::QSS2 ) {
		vars.push_back( x = new Variable_QSS2< Function_LTI >( "x", rTol, aTol, 1.0 ) );
		vars.push_back( u = new Variable_Inp2< Function_sin_ND >( "u", rTol, aTol ) );
	} else if ( qss == QSS::QSS3 ) {
		vars.push_back( x = new Variable_QSS3< Function_LTI >( "x", rTol, aTol, 1.0 ) );
		vars.push_back( u = new Variable_Inp3< Function_sin_ND >( "u", rTol, aTol ) );
	} else if ( qss == QSS::LIQSS1 ) {
		vars.push_back( x = new Variable_LIQSS1< Function_LTI >( "x", rTol, aTol, 1.0 ) );
		vars.push_back( u = new Variable_Inp1< Function_sin_ND >( "u", rTol, aTol ) );
	} else if ( qss == QSS::LIQSS2 ) {
		vars.push_back( x = new Variable_LIQSS2< Function_LTI >( "x", rTol, aTol, 1.0 ) );
		vars.push_back( u = new Variable_Inp2< Function_sin_ND >( "u", rTol, aTol ) );
	} else {
		std::cerr << "Unsupported QSS method" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Input
	u->set_dt_max( 0.1 );
	u->f().c( 0.05 ).s( 0.5 );

	// Derivatives
	x->d().add( -1.0, x ).add( u );
}

} // mdl
} // dfn
} // QSS
