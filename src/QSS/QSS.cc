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

//	// Simple x, y, z
//	double const tEnd( 5.0 );
//	VariableQSS2 x( "x", 1.0, 0.0 ); // For coarse testing with absolute tolerance only
//	VariableQSS2 y( "y", 1.0, 0.0 );
//	VariableQSS2 z( "z", 1.0, 0.0 );
//	x.init_val( 0.0 );
//	y.init_val( 0.0 );
//	z.init_val( 0.0 );
//	x.add_der( 1.0 ).finalize_der();
//	y.add_der( 1.0, x ).finalize_der();
//	z.add_der( 1.0, y ).finalize_der();
//	x.init_der();
//	y.init_der();
//	z.init_der();
//	x.init_der2();
//	y.init_der2();
//	z.init_der2();
////	x.init_der3();
////	y.init_der3();
////	z.init_der3();
//	x.init_event();
//	y.init_event();
//	z.init_event();
//
//	std::ofstream x_stream( "x.out" );
//	std::ofstream y_stream( "y.out" );
//	std::ofstream z_stream( "z.out" );
//	std::ofstream qx_stream( "qx.out" );
//	std::ofstream qy_stream( "qy.out" );
//	std::ofstream qz_stream( "qz.out" );
//
//	double t( 0.0 ), dto( 1.0e-3 ), to( dto );
//	x_stream << std::setprecision( 16 ) << t << '\t' << x.x( t ) << '\n';
//	y_stream << std::setprecision( 16 ) << t << '\t' << y.x( t ) << '\n';
//	z_stream << std::setprecision( 16 ) << t << '\t' << z.x( t ) << '\n';
//	qx_stream << std::setprecision( 16 ) << t << '\t' << x.q( t ) << '\n';
//	qy_stream << std::setprecision( 16 ) << t << '\t' << y.q( t ) << '\n';
//	qz_stream << std::setprecision( 16 ) << t << '\t' << z.q( t ) << '\n';
//	while ( ( t <= tEnd ) || ( to <= tEnd ) ) {
//		Variable * const xTop( events.top() );
//		t = xTop->tEnd;
//		while ( to < std::min( t, tEnd ) ) { // Sampled outputs
//			x_stream << to << '\t' << x.x( to ) << '\n'; // Comment me out for no sampled output
//			y_stream << to << '\t' << y.x( to ) << '\n'; // Comment me out for no sampled output
//			z_stream << to << '\t' << z.x( to ) << '\n'; // Comment me out for no sampled output
//			qx_stream << to << '\t' << x.q( to ) << '\n'; // Comment me out for no sampled output
//			qy_stream << to << '\t' << y.q( to ) << '\n'; // Comment me out for no sampled output
//			qz_stream << to << '\t' << z.q( to ) << '\n'; // Comment me out for no sampled output
//			to += dto;
//		}
//		xTop->advance();
//		if ( t <= tEnd ) { // Requantization outputs
//			if ( xTop == &x ) { // Variable needs access to its stream to avoid this if block
//				x_stream << t << '\t' << x.x( t ) << '\n';
//				qx_stream << t << '\t' << x.q( t ) << '\n';
//			} else if ( xTop == &y ) {
//				y_stream << t << '\t' << y.x( t ) << '\n';
//				qy_stream << t << '\t' << y.q( t ) << '\n';
//			} else {
//				z_stream << t << '\t' << z.x( t ) << '\n';
//				qz_stream << t << '\t' << z.q( t ) << '\n';
//			}
//		}
//	}

	// Achilles and the Tortoise
	double const tEnd( 10.0 );
	VariableQSS2 x1( "x1", 1.0e-4, 1.0e-4 );
	VariableQSS2 x2( "x2", 1.0e-4, 1.0e-4 );
//	VariableQSS2 x1( "x1", 1.0, 0.0 ); // For coarse testing with absolute tolerance only
//	VariableQSS2 x2( "x2", 1.0, 0.0 );
	x1.init_val( 0.0 );
	x2.init_val( 2.0 );
	x1.add_der( -0.5, x1 ).add_der( 1.5, x2 ).finalize_der();
	x2.add_der( -1.0, x1 ).finalize_der();
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
