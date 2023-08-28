// QSS Observers Collection Performance Tests

// C++ Headers
#include <cstddef>
#include <ctime>
#include <iostream>
#include <vector>

int
main()
{
	using Variables = std::vector< int >;

	std::size_t const R( 10000000 ); // Repeat count

	Variables v, w;
	v.resize( 1000u, 0 );
	w.resize( 1000u, 0 );

	{ // push_back
		int s( 0 );
		double const time_beg = (double)clock()/CLOCKS_PER_SEC;
		for ( std::size_t r = 0; r < R; ++r ) {
			v.clear(); v.reserve( 100 );
			w.clear(); w.reserve( 100 );
			for ( int i = 0; i < 200; ++i ) {
				v.push_back( i%5 );
				w.push_back( 0 );
			}
			w[ 5 ] = int( r );
		}
		double const time_end = (double)clock()/CLOCKS_PER_SEC;
		for ( auto t : v ) s += t;
		for ( auto t : w ) s += t;
		std::cout << "push_back: " << time_end - time_beg << " (s)  " << s << std::endl;
	}

	{ // resize: This is almost 2X faster
		int s( 0 );
		double const time_beg = (double)clock()/CLOCKS_PER_SEC;
		for ( std::size_t r = 0; r < R; ++r ) {
			v.clear(); v.reserve( 100 );
			w.clear(); w.resize( 100 ); // Faster with no initial value if you don't need it
			for ( int i = 0; i < 200; ++i ) {
				v.push_back( i%5 );
			}
			w[ 5 ] = int( r );
		}
		double const time_end = (double)clock()/CLOCKS_PER_SEC;
		for ( auto t : v ) s += t;
		for ( auto t : w ) s += t;
		std::cout << "resize: " << time_end - time_beg << " (s)  " << s << std::endl;
	}
}
