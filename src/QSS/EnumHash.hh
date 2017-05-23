#ifndef QSS_EnumHash_hh_INCLUDED
#define QSS_EnumHash_hh_INCLUDED

// Hashing for Enumerators
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

namespace QSS {

// Hashing for Enumerators
struct EnumHash
{
	template< typename T >
	std::size_t
	operator ()( T const t ) const
	{
		return static_cast< std::size_t >( t );
	}
};

} // QSS

#endif
