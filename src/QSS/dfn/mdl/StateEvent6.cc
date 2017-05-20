// StateEvent6 Example Setup
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/dfn/mdl/StateEvent6.hh>
#include <QSS/dfn/mdl/Function_LTI.hh>
#include <QSS/dfn/Variable_D.hh>
#include <QSS/dfn/Variable_LIQSS1.hh>
#include <QSS/dfn/Variable_LIQSS2.hh>
#include <QSS/dfn/Variable_QSS1.hh>
#include <QSS/dfn/Variable_QSS2.hh>
#include <QSS/dfn/Variable_QSS3.hh>
#include <QSS/dfn/Variable_ZC1.hh>
#include <QSS/dfn/Variable_ZC2.hh>
#include <QSS/math.hh>
#include <QSS/options.hh>

// C++ Headers
#include <cmath>

namespace QSS {
namespace dfn {
namespace mdl {

// Zero-Crossing Handler for StateEvent6 Model
template< typename V > // Template to avoid cyclic inclusion with Variable
class Handler_StateEvent6
{

public: // Types

	using Variable = V;
	using Time = typename Variable::Time;
	using Value = typename Variable::Value;
	using Crossing = typename Variable::Crossing;

public: // Properties

	// Apply at Time t
	void
	operator ()( Time const t, Crossing const crossing )
	{
		if ( crossing >= Crossing::Up ) { // Upward zero-crossing
			y_->shift_handler( t, 1.0 );
		}
	}

public: // Methods

	// Set Variables
	void
	var( Variable_D * y )
	{
		y_ = y;
	}

private: // Data

	Variable_D * y_{ nullptr };

};

// x1 Derivative Function
template< typename V > // Template to avoid cyclic inclusion with Variable
class Function_x1
{

public: // Types

	using Variable = V;
	using Time = typename Variable::Time;
	using Value = typename Variable::Value;
	using Coefficient = double;

public: // Creation

	// Default Constructor
	Function_x1()
	{}

public: // Properties

	// Continuous Value at Time t
	Value
	operator ()( Time const t ) const
	{
		return std::cos( c() * t );
	}

	// Continuous Value at Time t
	Value
	x( Time const t ) const
	{
		return std::cos( c() * t );
	}

	// Continuous First Derivative at Time t
	Value
	x1( Time const t ) const
	{
		return -c() * std::sin( c() * t );
	}

	// Quantized Value at Time t
	Value
	q( Time const t ) const
	{
		return std::cos( c() * t );
	}

	// Quantized First Derivative at Time t
	Value
	q1( Time const t ) const
	{
		return -c() * std::sin( c() * t );
	}

	// Quantized Second Derivative at Time t
	Value
	q2( Time const t ) const
	{
		return -square( c() ) * std::cos( c() * t );
	}

	// Quantized Sequential Value at Time t
	Value
	qs( Time const t ) const
	{
		return q( t );
	}

	// Quantized Forward-Difference Sequential First Derivative at Time t
	Value
	qf1( Time const t ) const
	{
		return q1( t );
	}

	// Quantized Centered-Difference Sequential First Derivative at Time t
	Value
	qc1( Time const t ) const
	{
		return q1( t );
	}

	// Quantized Centered-Difference Sequential Second Derivative at Time t
	Value
	qc2( Time const t ) const
	{
		return q2( t );
	}

public: // Methods

	// Finalize Function Representation
	bool
	finalize( Variable * v )
	{
		assert( v != nullptr );
		return false; // Not self-observer
	}

	// Finalize Function Representation
	bool
	finalize( Variable & v )
	{
		return finalize( &v );
	}

public: // Static Methods

	// Time factor
	static
	Value
	c()
	{
		static Value const c_( 0.8 * pi );
		return c_;
	}

public: // Static Data

	static int const max_order = 3; // Max QSS order supported

};

// StateEvent6 Example Setup
void
StateEvent6( Variables & vars )
{
	using namespace options;

	// Timing
	if ( ! options::tEnd_set ) options::tEnd = 10.0;

	vars.clear();
	vars.reserve( 5 );

	// QSS variables
	using V1 = Variable_QSS< Function_x1 >;
	using V = Variable_QSS< Function_LTI >;
	V1 * x1( nullptr );
	V * x2( nullptr );
	V * x3( nullptr );
	if ( qss == QSS::QSS1 ) {
		vars.push_back( x1 = new Variable_QSS1< Function_x1 >( "x1", rTol, aTol, +1.1 ) );
		vars.push_back( x2 = new Variable_QSS1< Function_LTI >( "x2", rTol, aTol, -2.5 ) );
		vars.push_back( x3 = new Variable_QSS1< Function_LTI >( "x3", rTol, aTol, +4.0 ) );
	} else if ( qss == QSS::QSS2 ) {
		vars.push_back( x1 = new Variable_QSS2< Function_x1 >( "x1", rTol, aTol, +1.1 ) );
		vars.push_back( x2 = new Variable_QSS2< Function_LTI >( "x2", rTol, aTol, -2.5 ) );
		vars.push_back( x3 = new Variable_QSS2< Function_LTI >( "x3", rTol, aTol, +4.0 ) );
	} else if ( qss == QSS::QSS3 ) {
		vars.push_back( x1 = new Variable_QSS3< Function_x1 >( "x1", rTol, aTol, +1.1 ) );
		vars.push_back( x2 = new Variable_QSS3< Function_LTI >( "x2", rTol, aTol, -2.5 ) );
		vars.push_back( x3 = new Variable_QSS3< Function_LTI >( "x3", rTol, aTol, +4.0 ) );
	} else if ( qss == QSS::LIQSS1 ) {
		vars.push_back( x1 = new Variable_QSS1< Function_x1 >( "x1", rTol, aTol, +1.1 ) ); // Add q/xlu1 to enable LIQSS1
		vars.push_back( x2 = new Variable_LIQSS1< Function_LTI >( "x2", rTol, aTol, -2.5 ) );
		vars.push_back( x3 = new Variable_LIQSS1< Function_LTI >( "x3", rTol, aTol, +4.0 ) );
	} else if ( qss == QSS::LIQSS2 ) {
		vars.push_back( x1 = new Variable_QSS2< Function_x1 >( "x1", rTol, aTol, +1.1 ) ); // Add q/xlu12 to enable LIQSS2
		vars.push_back( x2 = new Variable_LIQSS2< Function_LTI >( "x2", rTol, aTol, -2.5 ) );
		vars.push_back( x3 = new Variable_LIQSS2< Function_LTI >( "x3", rTol, aTol, +4.0 ) );
	} else {
		std::cerr << "Unsupported QSS method" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Derivatives
	x2->d().add( +1.0 );
	x3->d().add( -2.0 );

	// Discrete variable
	Variable_D * y( new Variable_D( "y", 0 ) );
	vars.push_back( y );

	// Zero-crossing variable
	using Z = Variable_ZC< Function_LTI, Handler_StateEvent6 >;
	Z * z( nullptr );
	if ( ( qss == QSS::QSS1 ) || ( qss == QSS::LIQSS1 ) ) {
		vars.push_back( z = new Variable_ZC1< Function_LTI, Handler_StateEvent6 >( "z", rTol, aTol ) );
	} else { // Use QSS2
		vars.push_back( z = new Variable_ZC2< Function_LTI, Handler_StateEvent6 >( "z", rTol, aTol ) );
	}
	z->add_crossings_Up();
	z->f().add( x1 ).add( -1.0 );
	z->h().var( y );
}

} // mdl
} // dfn
} // QSS
