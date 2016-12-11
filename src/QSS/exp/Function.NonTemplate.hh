#ifndef QSS_Function_hh_INCLUDED
#define QSS_Function_hh_INCLUDED

// Linear Function Class

// QSS Headers
#include <QSS/Variable.hh>

// C++ Headers
#include <cassert>
#include <vector>

// Linear Function
class Function // : public FunctionABC
{

public: // Types

	using Coefficient = double;
	using Coefficients = std::vector< Coefficient >;

	using Variables = Variable::Variables;

	using Value = Variable::Value;
	using Time = typename Variable::Time;

public: // Creation

	// Default Constructor
	Function()
	{}

//	// Copy Constructor
//	Function( Function const & ) = default;
//
//	// Move Constructor
//	Function( Function && ) noexcept = default;

	// Constructor
	Function(
	 Coefficients const & c,
	 Variables const & x
	) :
	 c_( c ),
	 x_( x )
	{
		assert( c_.size() == x_.size() );
	}

//public: // Assignment
//
//	// Copy Assignment
//	Function &
//	operator =( Function const & ) = default;
//
//	// Move Assignment
//	Function &
//	operator =( Function && ) noexcept = default;

public: // Methods

	// Add Constant
	Function &
	add( Coefficient const c0 )
	{
		c0_ = c0;
		return *this;
	}

	// Add a Coefficient + Variable
	Function &
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
	Function &
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

	// Shrink Coefficient and Variable Collections
	void
	shrink() // May be worth calling after all entries added to improve memory and cache use
	{
		assert( c_.size() == x_.size() );
		c_.shrink_to_fit();
		x_.shrink_to_fit();
		assert( c_.size() == x_.size() );
	}

	// Continuous Value at Time t
	Value
	operator ()( Time const t ) const
	{
		assert( c_.size() == x_.size() );
		Value v( c0_ ); // Value
		for ( Coefficients::size_type i = 0, n = c_.size(); i < n; ++i ) {
			v += c_[ i ] * x_[ i ]->x( t );
		}
		return v;
	}

	// Continuous Value at Time t
	Value
	x( Time const t ) const
	{
		assert( c_.size() == x_.size() );
		Value v( c0_ ); // Value
		for ( Coefficients::size_type i = 0, n = c_.size(); i < n; ++i ) {
			v += c_[ i ] * x_[ i ]->x( t );
		}
		return v;
	}

	// Quantized Value at Time t
	Value
	q( Time const t ) const
	{
		assert( c_.size() == x_.size() );
		Value v( c0_ ); // Value
		for ( Coefficients::size_type i = 0, n = c_.size(); i < n; ++i ) {
			v += c_[ i ] * x_[ i ]->q( t );
		}
		return v;
	}

private: // Data

	Coefficient c0_{ 0.0 };
	Coefficients c_;
	Variables x_;

};

#endif
