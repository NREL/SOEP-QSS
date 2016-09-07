// QSS speed test

// Findings:

// QSS Headers

// C++ Headers
#include <algorithm>
#include <cstddef>
#include <ctime>
#include <iostream>
//#include <random>

int
main()
{
	using namespace std;
	using namespace QSS;

	std::size_t const R( 100000000 ); // Repeat count

	{ //
		double const time_beg = (double)clock()/CLOCKS_PER_SEC;
		std::size_t ns( 0u ), nr( 0u ), nl( 0u );
		for ( std::size_t r = 1; r <= R; ++r ) {
		}
		double const time_end = (double)clock()/CLOCKS_PER_SEC;
		cout << ":   " << time_end - time_beg << " (s) " << ns << ' ' << nr << ' ' << nl << ' ' << R << endl << endl;
	}

}
