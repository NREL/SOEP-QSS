#ifndef QSS_FunctionLIQSSLTI_hh_INCLUDED
#define QSS_FunctionLIQSSLTI_hh_INCLUDED

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
class FunctionLIQSSLTI
{

public: // Types

	using Value = double;

	using Coefficient = double;
	using Coefficients = std::vector< Coefficient >;

	using Variable = V;
	using Variables = typename V::Variables;

	using Time = typename Variable::Time;
	using size_type = Coefficients::size_type;

public: // Creation

	// Default Constructor
	FunctionLIQSSLTI()
	{}

	// Constructor
	FunctionLIQSSLTI(
	 Coefficients const & c,
	 Variables const & x
	) :
	 c_( c ),
	 x_( x )
	{
		assert( c_.size() == x_.size() );
	}

public: // Properties

	// Quantized Value at Initialization Time
	Value
	q() const
	{
		assert( c_.size() == x_.size() );
		Value v( c0_ );
		for ( size_type i = 0, n = c_.size(); i < n; ++i ) {
			v += c_[ i ] * x_[ i ]->q();
		}
		return v;
	}

	// Quantized First Derivative at Initialization Time
	Value
	q1() const
	{
		assert( c_.size() == x_.size() );
		Value s( 0.0 );
		for ( size_type i = iBeg[ 2 ], n = c_.size(); i < n; ++i ) {
			s += c_[ i ] * x_[ i ]->q1();
		}
		return s;
	}

	// Quantized Second Derivative at Initialization Time
	Value
	q2() const
	{
		assert( c_.size() == x_.size() );
		Value c( 0.0 );
		for ( size_type i = iBeg[ 3 ], n = c_.size(); i < n; ++i ) {
			c += c_[ i ] * x_[ i ]->q2();
		}
		return c;
	}

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

	// Continuous Value at Time t of Contribution from Non-Integral Variables
	Value
	xo( Time const t ) const
	{
		assert( co_.size() == xo_.size() );
		Value v( c0_ );
		for ( size_type i = 0, n = co_.size(); i < n; ++i ) {
			v += co_[ i ] * xo_[ i ]->x( t );
		}
		return v;
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

	// Quantized Value at Time t of Contribution from Non-Integral Variables
	Value
	qo( Time const t ) const
	{
		assert( co_.size() == xo_.size() );
		Value v( c0_ );
		for ( size_type i = 0, n = co_.size(); i < n; ++i ) {
			v += co_[ i ] * xo_[ i ]->q( t );
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

public: // Methods

	// Add Constant
	FunctionLIQSSLTI &
	add( Coefficient const c0 )
	{
		c0_ = c0;
		return *this;
	}

	// Add a Coefficient + Variable
	FunctionLIQSSLTI &
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

	// Add a Variable + Coefficient
	FunctionLIQSSLTI &
	add(
	 Variable & x_i,
	 Coefficient const c_i
	)
	{
		assert( c_.size() == x_.size() );
		c_.push_back( c_i );
		x_.push_back( &x_i );
		assert( c_.size() == x_.size() );
		return *this;
	}

	// Add a Coefficient + Variable
	FunctionLIQSSLTI &
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

	// Add a Variable + Coefficient
	FunctionLIQSSLTI &
	add(
	 Variable * x_i,
	 Coefficient const c_i
	)
	{
		assert( c_.size() == x_.size() );
		c_.push_back( c_i );
		x_.push_back( x_i );
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
			for ( size_type i = 0; i < n; ++i ) {
				if ( x_[ i ]->order() == order ) {
					c.push_back( c_[ i ] );
					x.push_back( x_[ i ] );
					if ( x_[ i ] == v ) {
						cv_ = c_[ i ];
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

	// LIQSS1 Self-Observer Requantization
	void
	liqss1(
	 Time const t,
	 Value const qTol,
	 Value & q0,
	 Value & x1
	)
	{
		assert( qTol > 0.0 );
		assert( xv_ != nullptr );
		assert( xv_->self_observer );
		Value const db( qo( t ) ); // Quantized rep used for single trigger
		Value const dc( db + ( cv_ * q0 ) );
		Value const cv_qTol( cv_ * qTol );
		Value const dl( dc - cv_qTol );
		Value const du( dc + cv_qTol );
		int const dls( signum( dl ) );
		int const dus( signum( du ) );
		if ( ( dls == +1 ) && ( dus == +1 ) ) { // Upward trajectory
			q0 += qTol;
			x1 = du;
		} else if ( ( dls == -1 ) && ( dus == -1 ) ) { // Downward trajectory
			q0 -= qTol;
			x1 = dl;
		} else { // Flat trajectory
			q0 = std::min( std::max( -db / cv_, q0 - qTol ), q0 + qTol ); // cv_ != 0 since dls != dus // Clipped in case of roundoff
			x1 = 0.0;
		}
	}

	// LIQSS1 Self-Observer Requantization: Continuous Representation
	void
	liqss1_x(
	 Time const t,
	 Value const qTol,
	 Value & q0,
	 Value & x1
	)
	{
		assert( qTol > 0.0 );
		assert( xv_ != nullptr );
		assert( xv_->self_observer );
		Value const db( xo( t ) ); // Continuous rep used to avoid cyclic dependency
		Value const dc( db + ( cv_ * q0 ) );
		Value const cv_qTol( cv_ * qTol );
		Value const dl( dc - cv_qTol );
		Value const du( dc + cv_qTol );
		int const dls( signum( dl ) );
		int const dus( signum( du ) );
		if ( ( dls == +1 ) && ( dus == +1 ) ) { // Upward trajectory
			q0 += qTol;
			x1 = du;
		} else if ( ( dls == -1 ) && ( dus == -1 ) ) { // Downward trajectory
			q0 -= qTol;
			x1 = dl;
		} else { // Flat trajectory
			q0 = std::min( std::max( -db / cv_, q0 - qTol ), q0 + qTol ); // cv_ != 0 since dls != dus // Clipped in case of roundoff
			x1 = 0.0;
		}
	}

public: // Static Data

	static int const max_order = 3; // Max QSS order supported

private: // Data

	size_type iBeg[ max_order + 1 ]; // Index of first Variable of each QSS order
	Coefficient c0_{ 0.0 }; // Constant term
	Coefficients c_; // Coefficients
	Variables x_; // Variables
	Coefficient cv_{ 0.0 }; // Coefficient of integral Variable
	Variable * xv_{ nullptr }; // Integral Variable
	Coefficients co_; // Coefficients for Variables other than integral Variable
	Variables xo_; // Variables other than integral Variable

};

#endif
