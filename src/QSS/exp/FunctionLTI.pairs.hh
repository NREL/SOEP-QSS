#ifndef QSS_FunctionLTI_hh_INCLUDED
#define QSS_FunctionLTI_hh_INCLUDED

// Linear Time-Invariant Function

// Notes:
// . This variant uses an array of variable+coefficient pairs that is probably slower wrt vectorization but should be tried for larger cases just to see

// C++ Headers
#include <cassert>
//#include <numeric> // std::iota
#include <utility>
#include <vector>

// Linear Time-Invariant Function
template< typename V > // Template to avoid cyclic inclusion with Variable
class FunctionLTI
{

public: // Types

	using Coefficient = double;

	using Variable = V;

	using Element = std::pair< Variable *, Coefficient >;
	using Elements = std::vector< Element >;

	using Value = typename Variable::Value;
	using Time = typename Variable::Time;
	using size_type = typename Elements::size_type;

public: // Creation

	// Default Constructor
	FunctionLTI()
	{}

	// Constructor
	explicit
	FunctionLTI( Elements const & e ) :
	 e_( e )
	{}

public: // Properties

	// Continuous Value at Time t
	Value
	operator ()( Time const t ) const
	{
		Value v( c0_ );
		for ( size_type i = 0, n = e_.size(); i < n; ++i ) {
			v += e_[ i ].second * e_[ i ].first->x( t );
		}
		return v;
	}

	// Continuous Value at Time t
	Value
	x( Time const t ) const
	{
		Value v( c0_ );
		for ( size_type i = 0, n = e_.size(); i < n; ++i ) {
			v += e_[ i ].second * e_[ i ].first->x( t );
		}
		return v;
	}

	// Quantized Value at Time t
	Value
	q( Time const t ) const
	{
		Value v( c0_ );
		for ( size_type i = 0, n = e_.size(); i < n; ++i ) {
			v += e_[ i ].second * e_[ i ].first->q( t );
		}
		return v;
	}

	// Quantized First Derivative at Time t
	Value
	q1( Time const t ) const
	{
		Value s( 0.0 );
		for ( size_type i = iBeg[ 2 ], n = e_.size(); i < n; ++i ) {
			s += e_[ i ].second * e_[ i ].first->q1( t );
		}
		return s;
	}

	// Quantized Second Derivative at Time t
	Value
	q2( Time const t ) const
	{
		Value c( 0.0 );
		for ( size_type i = iBeg[ 3 ], n = e_.size(); i < n; ++i ) {
			c += e_[ i ].second * e_[ i ].first->q2( t );
		}
		return c;
	}

	// Quantized Forward-Difference Sequential Value at Time t
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
	 Coefficient const c,
	 Variable & x
	)
	{
		e_.push_back( Element( &x, c ) );
		return *this;
	}

	// Add a Variable + Coefficient
	FunctionLTI &
	add(
	 Variable & x,
	 Coefficient const c
	)
	{
		e_.push_back( Element( &x, c ) );
		return *this;
	}

	// Add a Coefficient + Variable
	FunctionLTI &
	add(
	 Coefficient const c,
	 Variable * x
	)
	{
		e_.push_back( Element( x, c ) );
		return *this;
	}

	// Add a Variable + Coefficient
	FunctionLTI &
	add(
	 Variable * x,
	 Coefficient const c
	)
	{
		e_.push_back( Element( x, c ) );
		return *this;
	}

	// Finalize Function Representation
	bool
	finalize( Variable * v )
	{
		assert( v != nullptr );
		size_type n( e_.size() );

		// Sort elements by QSS method order (not max efficiency!)
		Elements e;
		e.reserve( n );
		for ( int order = 1; order <= max_order; ++order ) {
			iBeg[ order ] = e.size();
			for ( size_type i = 0; i < n; ++i ) {
				if ( e_[ i ].first->order() == order ) {
					e.push_back( e_[ i ] );
				}
			}
		}
		e_.swap( e );
// Consider doing an in-place permutation if this is a bottleneck
//		std::vector< size_type > p( n ); // Permutation
//		std::iota( p.begin(), p.end(), 0u );
//		std::stable_sort( p.begin(), p.end(), [&]( size_type i, size_type j ){ return e_[ i ].first->order() < e_[ j ].first->order() } );
//		...

		// Add variables as observer of owning variable
		bool self_observer( false );
		for ( Element & e : e_ ) {
			e.first->add_observer( v );
			if ( e.first == v ) self_observer = true;
		}
		return self_observer;
	}

	// Finalize Function Representation
	bool
	finalize( Variable & v )
	{
		return finalize( &v );
	}

public: // Static Data

	static int const max_order = 3; // Max QSS order supported

private: // Data

	size_type iBeg[ max_order + 1 ]; // Index of first Variable of each QSS order
	Coefficient c0_{ 0.0 }; // Constant term
	Elements e_; // Elements

};

#endif
