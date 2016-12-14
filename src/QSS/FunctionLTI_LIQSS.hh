#ifndef QSS_FunctionLTI_LIQSS_hh_INCLUDED
#define QSS_FunctionLTI_LIQSS_hh_INCLUDED

// Linear Time-Invariant Function

// QSS Headers
#include <QSS/math.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
//#include <numeric> // std::iota
#include <vector>

// Linear Time-Invariant Function
template< typename V > // Template to avoid cyclic inclusion with Variable
class FunctionLTI_LIQSS
{

public: // Types

	using Coefficient = double;
	using Coefficients = std::vector< Coefficient >;

	using Variable = V;
	using Variables = typename V::Variables;

	using Value = typename Variable::Value;
	using Time = typename Variable::Time;
	using AdvanceSpecsLIQSS1 = typename Variable::AdvanceSpecsLIQSS1;
	using AdvanceSpecsLIQSS2 = typename Variable::AdvanceSpecsLIQSS2;
	using size_type = Coefficients::size_type;

public: // Creation

	// Default Constructor
	FunctionLTI_LIQSS()
	{}

	// Constructor
	FunctionLTI_LIQSS(
	 Coefficients const & c,
	 Variables const & x
	) :
	 c_( c ),
	 x_( x )
	{
		assert( c_.size() == x_.size() );
	}

public: // Properties

	// Continuous Value at Time t
	Value
	operator ()( Time const t ) const
	{
		assert( c_.size() == x_.size() );
		Value v( c0_ );
		for ( size_type i = 0, n = c_.size(); i < n; ++i ) {
			v += c_[ i ] * x_[ i ]->x( t );
		}
		return v;
	}

	// Continuous Value at Time t
	Value
	x( Time const t ) const
	{
		assert( c_.size() == x_.size() );
		Value v( c0_ );
		for ( size_type i = 0, n = c_.size(); i < n; ++i ) {
			v += c_[ i ] * x_[ i ]->x( t );
		}
		return v;
	}

	// Continuous First Derivative at Time t
	Value
	x1( Time const t ) const
	{
		assert( c_.size() == x_.size() );
		Value s( 0.0 );
		for ( size_type i = 0, n = c_.size(); i < n; ++i ) {
			s += c_[ i ] * x_[ i ]->x1( t );
		}
		return s;
	}

	// Quantized Value at Time t
	Value
	q( Time const t ) const
	{
		assert( c_.size() == x_.size() );
		Value v( c0_ );
		for ( size_type i = 0, n = c_.size(); i < n; ++i ) {
			v += c_[ i ] * x_[ i ]->q( t );
		}
		return v;
	}

	// Quantized First Derivative at Time t
	Value
	q1( Time const t ) const
	{
		assert( c_.size() == x_.size() );
		Value s( 0.0 );
		for ( size_type i = iBeg[ 2 ], n = c_.size(); i < n; ++i ) {
			s += c_[ i ] * x_[ i ]->q1( t );
		}
		return s;
	}

	// Quantized Second Derivative at Time t
	Value
	q2( Time const t ) const
	{
		assert( c_.size() == x_.size() );
		Value c( 0.0 );
		for ( size_type i = iBeg[ 3 ], n = c_.size(); i < n; ++i ) {
			c += c_[ i ] * x_[ i ]->q2( t );
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

	// Quantized Values at Time t and at Variable +/- Delta
	AdvanceSpecsLIQSS1
	qlu( Time const t, Value const del ) const
	{
		assert( co_.size() == xo_.size() );

		// Value at +/- del
		Value v( c0_ );
		for ( size_type i = 0, n = co_.size(); i < n; ++i ) {
			v += co_[ i ] * xo_[ i ]->q( t );
		}
		Value const vc( v + ( cv_ * xv_->q( t ) ) );
		Value const cv_del( cv_ * del );
		Value const vl( vc - cv_del );
		Value const vu( vc + cv_del );

		// Zero point
		Value const z( signum( vl ) != signum( vu ) ? -( v * cv_inv_ ) : 0.0 );

		return AdvanceSpecsLIQSS1{ vl, vu, z };
	}

	// Continuous Values at Time t and at Variable +/- Delta
	AdvanceSpecsLIQSS1
	xlu( Time const t, Value const del ) const
	{
		assert( co_.size() == xo_.size() );

		// Value at +/- del
		Value v( c0_ );
		for ( size_type i = 0, n = co_.size(); i < n; ++i ) {
			v += co_[ i ] * xo_[ i ]->x( t );
		}
		Value const vc( v + ( cv_ * xv_->x( t ) ) );
		Value const cv_del( cv_ * del );
		Value const vl( vc - cv_del );
		Value const vu( vc + cv_del );

		// Zero point
		Value const z( signum( vl ) != signum( vu ) ? -( v * cv_inv_ ) : 0.0 );

		return AdvanceSpecsLIQSS1{ vl, vu, z };
	}

	// Quantized Values and Derivatives at Time t and at Variable +/- Delta
	AdvanceSpecsLIQSS2
	qlu2( Time const t, Value const del ) const
	{
		assert( co_.size() == xo_.size() );

		// Value at +/- del
		Value v( c0_ );
		for ( size_type i = 0, n = co_.size(); i < n; ++i ) {
			v += co_[ i ] * xo_[ i ]->q( t );
		}
		Value const vc( v + ( cv_ * xv_->q( t ) ) );
		Value const cv_del( cv_ * del );
		Value const vl( vc - cv_del );
		Value const vu( vc + cv_del );

		// Derivative at +/- del
		Value s( 0.0 );
		for ( size_type i = ioBeg[ 2 ], n = co_.size(); i < n; ++i ) {
			s += co_[ i ] * xo_[ i ]->q1( t );
		}
		Value const sl( s + ( cv_ * vl ) );
		Value const su( s + ( cv_ * vu ) );

		// Zero point
		Value const z1( -( s * cv_inv_ ) );
		Value const z2( signum( sl ) != signum( su ) ? ( z1 - v ) * cv_inv_ : 0.0 );

		return AdvanceSpecsLIQSS2{ vl, vu, z1, sl, su, z2 };
	}

	// Continuous Values and Derivatives at Time t and at Variable +/- Delta
	AdvanceSpecsLIQSS2
	xlu2( Time const t, Value const del ) const
	{
		assert( co_.size() == xo_.size() );

		// Value at +/- del
		Value v( c0_ );
		for ( size_type i = 0, n = co_.size(); i < n; ++i ) {
			v += co_[ i ] * xo_[ i ]->x( t );
		}
		Value const vc( v + ( cv_ * xv_->x( t ) ) );
		Value const cv_del( cv_ * del );
		Value const vl( vc - cv_del );
		Value const vu( vc + cv_del );

		// Derivative at +/- del
		Value s( 0.0 );
		for ( size_type i = ioBeg[ 2 ], n = co_.size(); i < n; ++i ) {
			s += co_[ i ] * xo_[ i ]->x1( t );
		}
		Value const sl( s + ( cv_ * vl ) );
		Value const su( s + ( cv_ * vu ) );

		// Zero point
		Value const z1( -( s * cv_inv_ ) );
		Value const z2( signum( sl ) != signum( su ) ? ( z1 - v ) * cv_inv_ : 0.0 );

		return AdvanceSpecsLIQSS2{ vl, vu, z1, sl, su, z2 };
	}

public: // Methods

	// Add Constant
	FunctionLTI_LIQSS &
	add( Coefficient const c0 )
	{
		c0_ = c0;
		return *this;
	}

	// Add a Coefficient + Variable
	FunctionLTI_LIQSS &
	add(
	 Coefficient const c,
	 Variable & x
	)
	{
		assert( c_.size() == x_.size() );
		c_.push_back( c );
		x_.push_back( &x );
		assert( c_.size() == x_.size() );
		return *this;
	}

	// Add a Variable + Coefficient
	FunctionLTI_LIQSS &
	add(
	 Variable & x,
	 Coefficient const c
	)
	{
		assert( c_.size() == x_.size() );
		c_.push_back( c );
		x_.push_back( &x );
		assert( c_.size() == x_.size() );
		return *this;
	}

	// Add a Coefficient + Variable
	FunctionLTI_LIQSS &
	add(
	 Coefficient const c,
	 Variable * x
	)
	{
		assert( c_.size() == x_.size() );
		c_.push_back( c );
		x_.push_back( x );
		assert( c_.size() == x_.size() );
		return *this;
	}

	// Add a Variable + Coefficient
	FunctionLTI_LIQSS &
	add(
	 Variable * x,
	 Coefficient const c
	)
	{
		assert( c_.size() == x_.size() );
		c_.push_back( c );
		x_.push_back( x );
		assert( c_.size() == x_.size() );
		return *this;
	}

	// Finalize Function Representation
	bool
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
			ioBeg[ order ] = co_.size();
			for ( size_type i = 0; i < n; ++i ) {
				if ( x_[ i ]->order() == order ) {
					c.push_back( c_[ i ] );
					x.push_back( x_[ i ] );
					if ( x_[ i ] == v ) {
						cv_ = c_[ i ];
						cv_inv_ = ( cv_ != 0.0 ? 1.0 / cv_ : infinity );
					} else {
						co_.push_back( c_[ i ] );
						xo_.push_back( x_[ i ] );
					}
				}
			}
		}
		xv_ = v;
		c_.swap( c );
		x_.swap( x );
// Consider doing an in-place permutation if this is a bottleneck
//		std::vector< size_type > p( n ); // Permutation
//		std::iota( p.begin(), p.end(), 0u );
//		std::stable_sort( p.begin(), p.end(), [&]( size_type i, size_type j ){ return x_[ i ]->order() < x_[ j ]->order() } );
//		...

		// Add variables as observer of owning variable
		bool self_observer( false );
		for ( Variable * x : x_ ) {
			if ( x == v ) {
				self_observer = true;
			} else {
				x->add_observer( v );
			}
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
	size_type ioBeg[ max_order + 1 ]; // Index of first non-self Variable of each QSS order
	Coefficient c0_{ 0.0 }; // Constant term
	Coefficients c_; // Coefficients
	Variables x_; // Variables
	Coefficient cv_{ 0.0 }; // Coefficient of self Variable
	Coefficient cv_inv_{ 0.0 }; // Inverse of coefficient of self Variable
	Variable * xv_{ nullptr }; // Self Variable
	Coefficients co_; // Coefficients for Variables other than self Variable
	Variables xo_; // Variables other than self Variable

};

#endif
