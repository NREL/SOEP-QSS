#ifndef QSS_FunctionLTI_hh_INCLUDED
#define QSS_FunctionLTI_hh_INCLUDED

// Linear Time-Invariant Function

// C++ Headers
#include <cassert>
//#include <numeric> // std::iota
#include <vector>

// Linear Time-Invariant Function
template< typename V > // Template to avoid cyclic inclusion with Variable
class FunctionLTI
{

public: // Types

	using Coefficient = double;
	using Coefficients = std::vector< Coefficient >;

	using Variable = V;
	using Variables = typename V::Variables;

	using size_type = Coefficients::size_type;

public: // Creation

	// Default Constructor
	FunctionLTI()
	{}

	// Constructor
	FunctionLTI(
	 Coefficients const & c,
	 Variables const & x
	) :
	 c_( c ),
	 x_( x )
	{
		assert( c_.size() == x_.size() );
	}

public: // Methods

	// Add Constant
	FunctionLTI &
	add( Coefficient const c0 )
	{
		c0_ = c0;
		return *this;
	}

	// Add a Coefficient + Variable
	FunctionLTI &
	add(
	 Coefficient const c_i,
	 Variable & x_i
	)
	{
		assert( c_.size() == x_.size() );
		c_.push_back( c_i );
		x_.push_back( &x_i );
		assert( c_.size() == x_.size() );
		return *this;
	}

	// Add a Coefficient + Variable
	FunctionLTI &
	add(
	 Coefficient const c_i,
	 Variable * x_i
	)
	{
		assert( c_.size() == x_.size() );
		c_.push_back( c_i );
		x_.push_back( x_i );
		assert( c_.size() == x_.size() );
		return *this;
	}

	// Finalize Function Representation
	void
	finalize( Variable * v )
	{
		assert( v != nullptr );
		assert( c_.size() == x_.size() );
		size_type n( c_.size() );

		// Sort elements by QSS method order (not max efficiency!)
		Coefficients c;
		c.reserve( n );
		Variables x;
		x.reserve( n );
		for ( int order = 1; order <= max_order; ++order ) {
			iBeg[ order ] = c.size();
			for ( size_type i = 0, n = c_.size(); i < n; ++i ) {
				if ( x_[ i ]->order() == order ) {
					c.push_back( c_[ i ] );
					x.push_back( x_[ i ] );
				}
			}
		}
		c_.swap( c );
		x_.swap( x );
// Consider doing an in-place permutation if this is a bottleneck
//		std::vector< size_type > p( n ); // Permutation
//		std::iota( p.begin(), p.end(), 0u );
//		std::stable_sort( p.begin(), p.end(), [&]( size_type i, size_type j ){ return x_[ i ]->order() < x_[ j ]->order() } );
//		...

		// Add variables as observer of owning variable
		for ( Variable * x : x_ ) x->add_observer( v );
	}

	// Finalize Function Representation
	void
	finalize( Variable & v )
	{
		finalize( &v );
	}

	// Quantized Value at Initialization Time
	double
	q() const
	{
		assert( c_.size() == x_.size() );
		double v( c0_ ); // Value
		for ( size_type i = 0, n = c_.size(); i < n; ++i ) {
			v += c_[ i ] * x_[ i ]->q();
		}
		return v;
	}

	// Quantized First Derivative at Initialization Time
	double
	q1() const
	{
		assert( c_.size() == x_.size() );
		double s( 0.0 ); // Slope
		for ( size_type i = iBeg[ 2 ], n = c_.size(); i < n; ++i ) {
			s += c_[ i ] * x_[ i ]->q1();
		}
		return s;
	}

	// Quantized Second Derivative at Initialization Time
	double
	q2() const
	{
		assert( c_.size() == x_.size() );
		double c( 0.0 ); // Curvature
		for ( size_type i = iBeg[ 3 ], n = c_.size(); i < n; ++i ) {
			c += c_[ i ] * x_[ i ]->q2();
		}
		return c;
	}

	// Continuous Value at Time t
	double
	operator ()( double const t ) const
	{
		assert( c_.size() == x_.size() );
		double v( c0_ ); // Value
		for ( size_type i = 0, n = c_.size(); i < n; ++i ) {
			v += c_[ i ] * x_[ i ]->x( t );
		}
		return v;
	}

	// Continuous Value at Time t
	double
	x( double const t ) const
	{
		assert( c_.size() == x_.size() );
		double v( c0_ ); // Value
		for ( size_type i = 0, n = c_.size(); i < n; ++i ) {
			v += c_[ i ] * x_[ i ]->x( t );
		}
		return v;
	}

	// Quantized Value at Time t
	double
	q( double const t ) const
	{
		assert( c_.size() == x_.size() );
		double v( c0_ ); // Value
		for ( size_type i = 0, n = c_.size(); i < n; ++i ) {
			v += c_[ i ] * x_[ i ]->q( t );
		}
		return v;
	}

	// Quantized First Derivative at Time t
	double
	q1( double const t ) const
	{
		assert( c_.size() == x_.size() );
		double s( 0.0 ); // Slope
		for ( size_type i = iBeg[ 2 ], n = c_.size(); i < n; ++i ) {
			s += c_[ i ] * x_[ i ]->q1( t );
		}
		return s;
	}

	// Quantized Second Derivative at Time t
	double
	q2( double const t ) const
	{
		assert( c_.size() == x_.size() );
		double c( 0.0 ); // Curvature
		for ( size_type i = iBeg[ 3 ], n = c_.size(); i < n; ++i ) {
			c += c_[ i ] * x_[ i ]->q2( t );
		}
		return c;
	}

public: // Static Data

	static int const max_order = 3; // Max QSS order supported

private: // Data

	Coefficient c0_{ 0.0 };
	Coefficients c_;
	Variables x_;
	size_type iBeg[ max_order + 1 ];

};

#endif
