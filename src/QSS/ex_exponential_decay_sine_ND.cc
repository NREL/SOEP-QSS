// Exponential Decay with Sine Input and Numeric Differentiation Example Setup
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/ex_exponential_decay_sine.hh>
#include <QSS/options.hh>
#include <QSS/Function_LTI.hh>
#include <QSS/Function_sin_ND.hh>
#include <QSS/Variable_Inp1.hh>
#include <QSS/Variable_Inp2.hh>
#include <QSS/Variable_Inp3.hh>
#include <QSS/Variable_LIQSS1.hh>
#include <QSS/Variable_LIQSS2.hh>
#include <QSS/Variable_QSS1.hh>
#include <QSS/Variable_QSS2.hh>
#include <QSS/Variable_QSS3.hh>

namespace ex {

using Variables = std::vector< Variable * >;

// Exponential Decay with Sine Input and Numeric Differentiation Example Setup
//
// Recommend --tEnd=50
void
exponential_decay_sine_ND( Variables & vars )
{
	using namespace options;

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

} // ex
