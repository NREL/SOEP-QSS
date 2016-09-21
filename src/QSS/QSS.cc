// QSS Solver Prototype Runner

// C++ Headers
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>

// QSS Headers
#include <QSS/globals.hh>
#include <QSS/VariableQSS1.hh>
#include <QSS/VariableQSS2.hh>
#include <QSS/VariableQSS3.hh>

int
main()
{
	std::cout << std::setprecision( 16 );
	std::cerr << std::setprecision( 16 );

	// Achilles and the Tortoise
	double const tEnd( 10.0 );
	VariableQSS3 x1( "x1", 1.0e-4, 1.0e-4 );
	VariableQSS3 x2( "x2", 1.0e-4, 1.0e-4 );
//	VariableQSS3 x1( "x1", 1.0, 0.0 ); // For coarse testing with absolute tolerance only
//	VariableQSS3 x2( "x2", 1.0, 0.0 );
	x1.init_val( 0.0 );
	x2.init_val( 2.0 );
	x1.add_der( -0.5, x1 ).add_der( 1.5, x2 );
	x2.add_der( -1.0, x1 );
	x1.init_der();
	x2.init_der();
	x1.init_der2();
	x2.init_der2();
	x1.init_der3();
	x2.init_der3();
	x1.init_event();
	x2.init_event();

	std::ofstream x1_stream( "x1.out" );
	std::ofstream x2_stream( "x2.out" );
	std::ofstream q1_stream( "q1.out" );
	std::ofstream q2_stream( "q2.out" );

	double t( 0.0 ), dto( 1.0e-3 ), to( dto );
	x1_stream << std::setprecision( 16 ) << t << '\t' << x1.x( t ) << '\n';
	x2_stream << std::setprecision( 16 ) << t << '\t' << x2.x( t ) << '\n';
	q1_stream << std::setprecision( 16 ) << t << '\t' << x1.q( t ) << '\n';
	q2_stream << std::setprecision( 16 ) << t << '\t' << x2.q( t ) << '\n';
	while ( ( t <= tEnd ) || ( to <= tEnd ) ) {
		Variable * const x( events.top() );
		t = x->tEnd;
		while ( to < std::min( t, tEnd ) ) { // Sampled outputs
//			x1_stream << to << '\t' << x1.x( to ) << '\n'; // Comment me out for no sampled output
//			q1_stream << to << '\t' << x1.q( to ) << '\n'; // Comment me out for no sampled output
//			x2_stream << to << '\t' << x2.x( to ) << '\n'; // Comment me out for no sampled output
//			q2_stream << to << '\t' << x2.q( to ) << '\n'; // Comment me out for no sampled output
			to += dto;
		}
		x->advance();
		if ( t <= tEnd ) { // Requantization outputs
			if ( x == &x1 ) { // Variable needs access to its stream to avoid this if block
				x1_stream << t << '\t' << x1.x( t ) << '\n';
				q1_stream << t << '\t' << x1.q( t ) << '\n';
			} else {
				x2_stream << t << '\t' << x2.x( t ) << '\n';
				q2_stream << t << '\t' << x2.q( t ) << '\n';
			}
		}
	}
}
