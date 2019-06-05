// Conditional Blocks
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2019 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_cod_Conditional_hh_INCLUDED
#define QSS_cod_Conditional_hh_INCLUDED

// QSS Headers
#include <QSS/Conditional.hh>
#include <QSS/cod/events.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <vector>

namespace QSS {
namespace cod {

// Conditional If Block Template
template< typename V >
class Conditional_If final : public Conditional
{

public: // Nested Types

	// Clause Abstract Base Class
	class Clause
	{

	public: // Types

		using Variable = V;
		using If = Conditional_If< V >;
		using Time = typename Variable::Time;
		using Variables = typename Variable::Variables;
		using size_type = typename Variables::size_type;

	protected: // Creation

		// Default Constructor
		Clause() = default;

		// Copy Constructor
		Clause( Clause const & ) = delete;

		// Move Constructor
		Clause( Clause && ) noexcept = default;

		// Constructor
		explicit
		Clause( If * conditional ) :
		 conditional( conditional )
		{}

	public: // Creation

		// Destructor
		virtual
		~Clause()
		{}

	protected: // Assignment

		// Copy Assignment
		Clause &
		operator =( Clause const & ) = delete;

		// Move Assignment
		Clause &
		operator =( Clause && ) noexcept = default;

	public: // Predicates

		// Empty?
		bool
		empty() const
		{
			return vars.empty();
		}

	public: // Properties

		// Size
		size_type
		size() const
		{
			return vars.size();
		}

		// Handler-Modified Variables
		virtual
		Variables const &
		observers() const = 0;

		// Handler-Modified Variables
		virtual
		Variables &
		observers() = 0;

		// Boolean Value at SuperdenseTime s
		bool
		b( SuperdenseTime const & s ) const
		{
			if ( vars.empty() ) { // Else clause: Vacuously true
				return true;
			} else {
				for ( Variable const * var : vars ) {
					if ( var->b( s.t ) ) return true; // Only support OR logic for now
				}
				return false;
			}
		}

		// Boolean Value at Time t
		bool
		b( Time const t ) const
		{
			if ( vars.empty() ) { // Else clause: Vacuously true
				return true;
			} else {
				for ( Variable const * var : vars ) {
					if ( var->b( t ) ) return true; // Only support OR logic for now
				}
				return false;
			}
		}

	public: // Methods

		// Add a Variable
		Clause &
		add( Variable * var )
		{
			assert( std::find( vars.begin(), vars.end(), var ) == vars.end() );
			vars.push_back( var );
			var->if_clauses.push_back( this );
			return *this;
		}

		// Variable Activity Notifier
		void
		activity( Time const t )
		{
			assert( conditional != nullptr );
			conditional->shift_conditional( t );
		}

		// Handler Call at Time t
		virtual
		void
		handler( Time const t ) = 0;

	public: // Data

		If * conditional{ nullptr }; // Containing If
		Variables vars; // Variables in Clause

	}; // Clause

	// Clause + Handler Template
	template< template< typename > class H >
	class ClauseH final : public Clause
	{

	public: // Types

		using Super = Clause;
		using Handler = H< V >;

		using typename Super::Variable;
		using typename Super::If;
		using typename Super::Time;
		using typename Super::Variables;
		using typename Super::size_type;

	public: // Creation

		// Constructor
		explicit
		ClauseH( If * conditional ) :
		 Clause( conditional )
		{}

	public: // Properties

		// Handler-Modified Variables
		Variables const &
		observers() const
		{
			return h.observers();
		}

		// Handler-Modified Variables
		Variables &
		observers()
		{
			return h.observers();
		}

	public: // Methods

		// Handler
		void
		handler( Time const t )
		{
			h( t );
		}

	public: // Data

		Handler h; // Handler function

	}; // ClauseH

public: // Types

	using Super = Conditional;
	using Variable = V;
	using Time = typename Variable::Time;
	using Real = typename Variable::Real;

	using Clauses = std::vector< Clause * >;
	using size_type = typename Clauses::size_type;

public: // Creation

	// Default Constructor
	Conditional_If() :
	 Conditional( "If" )
	{
		add_conditional();
	}

	// Name Constructor
	explicit
	Conditional_If( std::string const & name ) :
	 Conditional( name )
	{
		add_conditional();
	}

	// Destructor
	~Conditional_If()
	{
		for ( Clause * clause : clauses ) delete clause;
	}

public: // Predicates

	// Empty?
	bool
	empty() const
	{
		return clauses.empty();
	}

	// Valid?
	bool
	valid() const
	{
		if ( clauses.empty() ) {
			return false; // Treat empty conditional as invalid
		} else {
			bool no_vars( false );
			for ( Clause const * clause : clauses ) {
				if ( no_vars ) { // Previous clause had no booleans
					return false; // Else clause not at end
				} else if ( clause->empty() ) { // Else clause: Must be last
					no_vars = true;
				}
			}
			return true;
		}
	}

public: // Properties

	// Size
	size_type
	size() const
	{
		return clauses.size();
	}

public: // Methods

	// Add a Clause
	template< template< typename > class H >
	ClauseH< H > *
	add_clause()
	{
		ClauseH< H > * clause( new ClauseH< H >( this ) );
		clauses.push_back( clause );
		return clause;
	}

	// If Shift Event to Time Infinity
	void
	add_conditional()
	{
		event_ = events.add_conditional( this );
	}

	// If Shift Event to Time Infinity
	void
	shift_conditional()
	{
		event_ = events.shift_conditional( event_ );
	}

	// If Shift Event to Time t
	void
	shift_conditional( Time const t )
	{
		event_ = events.shift_conditional( t, event_ );
	}

	// Run Handler of Highest Priority Active Clause
	void
	advance_conditional()
	{
		assert( valid() );
		for ( Clause * clause : clauses ) {
			if ( clause->b( st ) ) {
				clause->handler( st.t );
				break; // Highest priority active Clause found
			}
		}
		shift_conditional();
	}

private: // Data

	Clauses clauses; // Clauses in decreasing priority sequence

}; // Conditional_If

// Conditional When Block Template
template< typename V >
class Conditional_When final : public Conditional
{

public: // Nested Types

	// Clause Abstract Base Class
	class Clause
	{

	public: // Types

		using Variable = V;
		using When = Conditional_When< V >;
		using Time = typename Variable::Time;
		using Variables = typename Variable::Variables;
		using size_type = typename Variables::size_type;

	protected: // Creation

		// Default Constructor
		Clause() = default;

		// Copy Constructor
		Clause( Clause const & ) = delete;

		// Move Constructor
		Clause( Clause && ) noexcept = default;

		// Constructor
		explicit
		Clause( When * conditional ) :
		 conditional( conditional )
		{}

	public: // Creation

		// Destructor
		virtual
		~Clause()
		{}

	protected: // Assignment

		// Copy Assignment
		Clause &
		operator =( Clause const & ) = delete;

		// Move Assignment
		Clause &
		operator =( Clause && ) noexcept = default;

	public: // Predicates

		// Empty?
		bool
		empty() const
		{
			return vars.empty();
		}

	public: // Properties

		// Size
		size_type
		size() const
		{
			return vars.size();
		}

		// Handler-Modified Variables
		virtual
		Variables const &
		observers() const = 0;

		// Handler-Modified Variables
		virtual
		Variables &
		observers() = 0;

		// Boolean Value at SuperdenseTime s
		bool
		b( SuperdenseTime const & s ) const
		{
			bool any( false ); // Any Variables becoming true at s pass
			for ( Variable const * var : vars ) {
				if ( same_pass( var->st, s ) ) {
					assert( var->b( s.t ) );
					any = true;
				} else if ( var->b( s.t ) ) { // Only support OR logic for now
					return false; // When OR clause is only true when all true Variables are becoming true in the same pass
				}
			}
			return any;
		}

	public: // Methods

		// Add a Variable
		Clause &
		add( Variable * var )
		{
			assert( std::find( vars.begin(), vars.end(), var ) == vars.end() );
			vars.push_back( var );
			var->when_clauses.push_back( this );
			return *this;
		}

		// Variable Activity Notifier
		void
		activity( Time const t )
		{
			assert( conditional != nullptr );
			conditional->shift_conditional( t );
		}

		// Handler
		virtual
		void
		handler( Time const t ) = 0;

	public: // Data

		When * conditional{ nullptr }; // Containing When
		Variables vars; // Variables in Clause

	}; // Clause

	// Clause + Handler Template
	template< template< typename > class H >
	class ClauseH final : public Clause
	{

	public: // Types

		using Super = Clause;
		using Handler = H< V >;

		using typename Super::Variable;
		using typename Super::When;
		using typename Super::Time;
		using typename Super::Variables;
		using typename Super::size_type;

	public: // Creation

		// Constructor
		explicit
		ClauseH( When * conditional ) :
		 Clause( conditional )
		{}

	public: // Properties

		// Handler-Modified Variables
		Variables const &
		observers() const
		{
			return h.observers();
		}

		// Handler-Modified Variables
		Variables &
		observers()
		{
			return h.observers();
		}

	public: // Methods

		// Handler
		void
		handler( Time const t )
		{
			h( t );
		}

	public: // Data

		Handler h; // Handler function

	}; // ClauseH

public: // Types

	using Super = Conditional;
	using Variable = V;
	using Time = typename Variable::Time;
	using Real = typename Variable::Real;

	using Clauses = std::vector< Clause * >;
	using size_type = typename Clauses::size_type;

public: // Creation

	// Default Constructor
	Conditional_When() :
	 Conditional( "When" )
	{
		add_conditional();
	}

	// Name Constructor
	explicit
	Conditional_When( std::string const & name ) :
	 Conditional( name )
	{
		add_conditional();
	}

	// Destructor
	~Conditional_When()
	{
		for ( Clause * clause : clauses ) delete clause;
	}

public: // Predicates

	// Empty?
	bool
	empty() const
	{
		return clauses.empty();
	}

	// Valid?
	bool
	valid() const
	{
		if ( clauses.empty() ) {
			return false; // Treat empty conditional as invalid
		} else {
			for ( Clause const * clause : clauses ) {
				if ( clause->empty() ) { // Else clause: Not supported in When conditional
					return false;
				}
			}
			return true;
		}
	}

public: // Properties

	// Size
	size_type
	size() const
	{
		return clauses.size();
	}

public: // Methods

	// Add a Clause
	template< template< typename > class H >
	ClauseH< H > *
	add_clause()
	{
		ClauseH< H > * clause( new ClauseH< H >( this ) );
		clauses.push_back( clause );
		return clause;
	}

	// If Shift Event to Time Infinity
	void
	add_conditional()
	{
		event_ = events.add_conditional( this );
	}

	// When Shift Event to Time Infinity
	void
	shift_conditional()
	{
		event_ = events.shift_conditional( event_ );
	}

	// When Shift Event to Time t
	void
	shift_conditional( Time const t )
	{
		event_ = events.shift_conditional( t, event_ );
	}

	// Run Handler of Highest Priority Active Clause
	void
	advance_conditional()
	{
		assert( valid() );
		for ( Clause * clause : clauses ) {
			if ( clause->b( st ) ) {
				clause->handler( st.t );
				break; // Highest priority active Clause found
			}
		}
		shift_conditional();
	}

private: // Data

	Clauses clauses; // Clauses in decreasing priority sequence

}; // Conditional_When

} // cod
} // QSS

#endif
