#ifndef QSS_VectorPair_hh_INCLUDED
#define QSS_VectorPair_hh_INCLUDED

// Vector Pair Wrapper Class Template
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2018 Objexx Engineerinc, Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
// (3) Neither the name of the copyright holder nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, THE UNITED STATES
// GOVERNMENT, OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Note: This allows iterating two vectors as one at the cost of slower iteration

// C++ Headers
#include <cassert>
#include <iterator>
#include <vector>

namespace QSS {

// Vector Pair Wrapper Class Template
template< typename T >
class VectorPair
{

public: // Types

	using Value = T;
	using Vector = std::vector< T >;
	using size_type = typename Vector::size_type;

	class Iterator final : public std::iterator< std::bidirectional_iterator_tag, long, long, long const *, long >
	{

	public: // Types

		using const_iterator = typename Vector::const_iterator;
		using iterator = typename Vector::iterator;

	public: // Creation

		// Default Constructor
		Iterator() :
		 v1_( nullptr ),
		 v2_( nullptr ),
		 va_( nullptr )
		{}

		// Constructor
		Iterator(
		 Vector * v1,
		 Vector * v2,
		 Vector * va,
		 iterator p
		) :
		 v1_( v1 ),
		 v2_( v2 ),
		 va_( va ),
		 p_( p )
		{
			assert( ( va_ == v1_ ) || ( va_ == v2_ ) );
			assert( ( p_ >= va_->begin() ) && ( p_ <= va_->end() ) );
			assert( ( p_ != v1_->end() ) || ( p_ == v2_->begin() ) ); // Invariant
		}

		// Begin Named Constructor
		static
		Iterator
		begin(
		 Vector * v1,
		 Vector * v2
		)
		{
			return v1->empty() ? Iterator( v1, v2, v2, v2->begin() ) : Iterator( v1, v2, v1, v1->begin() );
		}

		// End Named Constructor
		static
		Iterator
		end(
		 Vector * v1,
		 Vector * v2
		)
		{
			return Iterator( v1, v2, v2, v2->end() );
		}

	public: // Methods

		// Pre-Increment
		Iterator &
		operator ++()
		{
			assert( ( p_ != v1_->end() ) || ( p_ == v2_->begin() ) ); // Invariant
			assert( p_ != v2_->end() );
			++p_;
			if ( p_ == v1_->end() ) { // Switch to v2
				va_ = v2_;
				p_ = v2_->begin();
			}
			assert( ( p_ != v1_->end() ) || ( p_ == v2_->begin() ) ); // Invariant
			return *this;
		}

		// Pre-Decrement
		Iterator &
		operator --()
		{
			assert( ( p_ != v1_->end() ) || ( p_ == v2_->begin() ) ); // Invariant
			assert( p_ != v1_->begin() );
			if ( p_ == v2_->begin() ) { // Switch to v1
				assert( ! v1_->empty() );
				va_ = v1_;
				p_ = v1_->end();
			}
			--p_;
			assert( ( p_ != v1_->end() ) || ( p_ == v2_->begin() ) ); // Invariant
			return *this;
		}

		// Post-Increment
		Iterator
		operator ++( int )
		{
			Iterator old( *this );
			operator ++();
			return old;
		}

		// Post-Decrement
		Iterator
		operator --( int )
		{
			Iterator old( *this );
			operator --();
			return old;
		}

		// Dereference
		T const &
		operator *() const
		{
			return *p_;
		}

		// Dereference
		T &
		operator *()
		{
			return *p_;
		}

		// Dereference
		T *
		operator ->()
		{
			return &*p_;
		}

		// Dereference
		T const *
		operator ->() const
		{
			return &*p_;
		}

	public: // Comparison

		friend
		bool
		operator ==(
		 Iterator const i1,
		 Iterator const i2
		)
		{
			return ( ( i1.va_ == i2.va_ ) && ( i1.p_ == i2.p_ ) );
		}

		friend
		bool
		operator !=(
		 Iterator const i1,
		 Iterator const i2
		)
		{
			return ( ( i1.va_ != i2.va_ ) || ( i1.p_ != i2.p_ ) );
		}

	private: // Data

		Vector * v1_; // Vector 1
		Vector * v2_; // Vector 2
		Vector * va_{ nullptr }; // Vector active
		typename Vector::iterator p_; // Position

	};

	using const_iterator = Iterator const;
	using iterator = Iterator;

public: // Creation

	// Pointer Constructor
	VectorPair(
	 Vector * v1,
	 Vector * v2
	) :
	 v1_( v1 ),
	 v2_( v2 )
	{}

	// Reference Constructor
	VectorPair(
	 Vector & v1,
	 Vector & v2
	) :
	 v1_( &v1 ),
	 v2_( &v2 )
	{}

public: // Predicates

	// Empty?
	bool
	empty() const
	{
		return v1_->empty() && v2_->empty();
	}

public: // Properties

	// Size
	size_type
	size() const
	{
		return v1_->size() + v2_->size();
	}

public: // Iterators

	// Begin Iterator
	const_iterator
	begin() const
	{
		return Iterator::begin( v1_, v2_ );
	}

	// Begin Iterator
	iterator
	begin()
	{
		return Iterator::begin( v1_, v2_ );
	}

	// End Iterator
	const_iterator
	end() const
	{
		return Iterator::end( v1_, v2_ );
	}

	// End Iterator
	iterator
	end()
	{
		return Iterator::end( v1_, v2_ );
	}

private: // Data

	Vector * v1_; // Vector 1
	Vector * v2_; // Vector 2

};

} // QSS

#endif
