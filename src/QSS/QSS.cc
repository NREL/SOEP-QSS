// C++ Headers
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>

// QSS Headers
#include <QSS/VariableQSS1.hh>

int
main()
{
	// Achilles and the Tortoise
	double const tEnd( 10.0 );
	VariableQSS1 x1( "x1", 1.0e-4, 1.0e-4 );
	VariableQSS1 x2( "x2", 1.0e-4, 1.0e-4 );
	x1.init_val( 0.0 );
	x2.init_val( 2.0 );
	x1.add_der( -0.5, x1 ).add_der( 1.5, x2 );
	x2.add_der( -1.0, x1 );
	x1.init_der();
	x2.init_der();

	std::ofstream x1_stream( "x1.out" );
	std::ofstream x2_stream( "x2.out" );
	x1_stream << std::setprecision( 16 ) << 0.0 << '\t' << x1.x( 0.0 ) << '\n';
	x2_stream << std::setprecision( 16 ) << 0.0 << '\t' << x2.x( 0.0 ) << '\n';
//	std::cout << 0.0 << ' ' << x1.name << ' ' << x1.x( 0.0 ) << ' ' << x1.q( 0.0 ) << ' ' << x1.tEnd << '\n'
//	std::cout << 0.0 << ' ' << x2.name << ' ' << x2.x( 0.0 ) << ' ' << x2.q( 0.0 ) << ' ' << x2.tEnd << '\n'

	double t( std::min( x1.tEnd, x2.tEnd ) );
	while ( t < tEnd ) {
		if ( x1.tEnd == t ) {
			x1.advance();
			x1_stream << t << '\t' << x1.x( t ) << '\n';
//			std::cout << t << ' ' << x1.name << ' ' << x1.x( t ) << ' ' << x1.q( t ) << ' ' << x1.tEnd << '\n';
		} else { // x2.tEnd == t
			x2.advance();
			x2_stream << t << '\t' << x2.x( t ) << '\n';
//			std::cout << t << ' ' << x2.name << ' ' << x2.x( t ) << ' ' << x2.q( t ) << ' ' << x2.tEnd << '\n';
		}
		t = std::min( x1.tEnd, x2.tEnd );
	}
}
