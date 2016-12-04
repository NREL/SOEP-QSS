// QSS::EventQueue Performance Tests

// QSS Headers
#include <QSS/EventQueue.hh>

// C++ Headers
#include <cstddef>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

// Variable Mock
class V {};

// Types
using EventQ = EventQueue< V >;
using Variables = std::vector< V >;
using Time = double;

namespace { // Internal shared global
std::default_random_engine random_generator;
}

int
main()
{
	using namespace std;

	random_generator.seed( 42 );
	size_t const N( 10000 ); // Variable count
	size_t const R( 100000000 ); // Repeat count

	Variables vars;
	vars.reserve( N ); // Prevent reallocation
	EventQ events;
	Time t( 0.0 );
	Time const tE( 10.0 );
	std::uniform_real_distribution< Time > distribution( t, tE );
	for ( Variables::size_type i = 0; i < N; ++i ) {
		vars.emplace_back( V() );
		events.add( distribution( random_generator ), &vars[ i ] );
	}

	double const time_beg = (double)clock()/CLOCKS_PER_SEC;
	size_t ns( 0u ), nr( 0u ), nl( 0u );
	for ( size_t r = 1; r <= R; ++r ) {
		auto i( events.top_iterator() );
		events.shift( i->first + ( 0.5 * ( 10.0 - i->first ) ), i ); // Move halfway to tE
	}
	double const time_end = (double)clock()/CLOCKS_PER_SEC;
	cout << std::setprecision( 15 ) << time_end - time_beg << " (s) " << events.top_time() << ' ' << N << ' ' << R << endl << endl;
}
