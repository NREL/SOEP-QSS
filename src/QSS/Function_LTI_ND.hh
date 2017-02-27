#ifndef QSS_Function_LTI_ND_hh_INCLUDED
#define QSS_Function_LTI_ND_hh_INCLUDED

// Linear Time-Invariant Function Using Numeric Differentiation
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// C++ Headers
//#include <algorithm> // std::stable_sort
#include <cassert>
//#include <numeric> // std::iota
#include <vector>

// Linear Time-Invariant Function Using Numeric Differentiation
//
// Note: Not set up for use with LIQSS methods
template< typename V > // Template to avoid cyclic inclusion with Variable
class Function_LTI_ND
{

public: // Types

	using Coefficient = double;
	using Coefficients = std::vector< Coefficient >;

	using Variable = V;
	using Variables = typename V::Variables;

	using Time = typename Variable::Time;
	using Value = typename Variable::Value;
	using size_type = Coefficients::size_type;

public: // Creation

	// Default Constructor
	Function_LTI_ND()
	{}

	// Constructor
	Function_LTI_ND(
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

	// Quantized Numeric Differentiation Value at Time t
	Value
	qn( Time const t ) const
	{
		assert( c_.size() == x_.size() );
		Value v( c0_ );
		for ( size_type i = 0, n = c_.size(); i < n; ++i ) {
			v += c_[ i ] * x_[ i ]->qn( t );
		}
		return v;
	}

	// Quantized First Derivative at Time t
	Value
	q1( Time const t ) const
	{
		return dtn_inv_2_ * ( qn( t + dtn_ ) - qn( t - dtn_ ) );
	}

	// Quantized Second Derivative at Time t
	Value
	q2( Time const t ) const
	{
		return dtn_inv_sq_ * ( qn( t + dtn_ ) - ( 2.0 * qn( t ) ) + qn( t - dtn_ ) );
	}

	// Quantized Sequential Value at Time t
	Value
	qs( Time const t ) const
	{
		return q_t_ = qn( t );
	}

	// Quantized Forward-Difference Sequential First Derivative at Time t
	Value
	qf1( Time const t ) const
	{
		return dtn_inv_ * ( qn( t + dtn_ ) - q_t_ );
	}

	// Quantized Centered-Difference Sequential First Derivative at Time t
	Value
	qc1( Time const t ) const
	{
		return dtn_inv_2_ * ( ( q_p_ = qn( t + dtn_ ) ) - ( q_m_ = qn( t - dtn_ ) ) );
	}

	// Quantized Centered-Difference Sequential Second Derivative at Time t
	Value
	qc2( Time const t ) const
	{
		return dtn_inv_sq_ * ( q_p_ - ( 2.0 * q_t_ ) + q_m_ );
	}

	// Differentiation Time Step
	Time
	dtn() const
	{
		return dtn_;
	}

public: // Methods

	// Add Constant
	Function_LTI_ND &
	add( Coefficient const c0 )
	{
		c0_ = c0;
		return *this;
	}

	// Add a Variable
	Function_LTI_ND &
	add( Variable * x )
	{
		assert( c_.size() == x_.size() );
		c_.push_back( 1.0 );
		x_.push_back( x );
		assert( c_.size() == x_.size() );
		return *this;
	}

	// Add a Coefficient + Variable
	Function_LTI_ND &
	add( Variable & x )
	{
		assert( c_.size() == x_.size() );
		c_.push_back( 1.0 );
		x_.push_back( &x );
		assert( c_.size() == x_.size() );
		return *this;
	}

	// Add a Coefficient + Variable
	Function_LTI_ND &
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
	Function_LTI_ND &
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
	Function_LTI_ND &
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
	Function_LTI_ND &
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
			for ( size_type i = 0; i < n; ++i ) {
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

		// Add variables as observees of self variable
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

	// Set Differentiation Time Step
	void
	dtn( Time const dtn )
	{
		assert( dtn > 0.0 );
		dtn_ = dtn;
		dtn_inv_ = 1.0 / dtn_;
		dtn_inv_2_ = 0.5 / dtn_;
		dtn_inv_sq_ = dtn_inv_ * dtn_inv_;
	}

public: // Static Data

	static int const max_order = 3; // Max QSS order supported

private: // Data

	size_type iBeg[ max_order + 1 ]; // Index of first Variable of each QSS order
	Coefficient c0_{ 0.0 }; // Constant term
	Coefficients c_; // Coefficients
	Variables x_; // Variables
	mutable Value q_t_; // Last q(t) computed
	mutable Value q_p_; // Last q(t+dtn) computed
	mutable Value q_m_; // Last q(t-dtn) computed
	Time dtn_{ options::dtND }; // Differentiation time step
	Time dtn_inv_{ 1.0 / options::dtND }; // Differentiation time step inverse
	Time dtn_inv_2_{ 0.5 / options::dtND }; // Differentiation time step half inverse
	Time dtn_inv_sq_{ 1.0 / ( options::dtND * options::dtND ) }; // Differentiation time step inverse squared

};

#endif
