// FMU-Based Conditional Blocks
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

#ifndef QSS_fmu_Conditional_hh_INCLUDED
#define QSS_fmu_Conditional_hh_INCLUDED

// QSS Headers
#include <QSS/globals.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <vector>

namespace QSS {
namespace fmu {

// Conditional Abstract Base Class
class Conditional : public Target
{

public: // Types

	using Super = Target;

protected: // Creation

	// Default Constructor
	Conditional() = default;

	// Name Constructor
	explicit
	Conditional( std::string const & name ) :
	 Target( name )
	{}

	// Copy Constructor
	Conditional( Conditional const & ) = default;

	// Move Constructor
	Conditional( Conditional && ) noexcept = default;

protected: // Assignment

	// Copy Assignment
	Conditional &
	operator =( Conditional const & ) = default;

	// Move Assignment
	Conditional &
	operator =( Conditional && ) noexcept = default;

public: // Methods

	// Run Handler of Highest Priority Active Clause
	virtual
	void
	advance_conditional() = 0;

}; // Conditional

// Conditional If Block Template
template< typename V >
class IfV final : public Conditional
{

public: // Nested Types

	// Clause Class
	class Clause
	{

	public: // Types

		using Variable = V;
		using If = IfV< V >;
		using Time = typename Variable::Time;
		using Variables = typename Variable::Variables;
		using size_type = typename Variables::size_type;

	public: // Creation

		// Constructor
		Clause( If * conditional ) :
		 conditional( conditional )
		{}

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

		// Add an Observer Variable
		void
		add_observer( Variable * v )
		{
			observers.push_back( v );
		}

		// Variable Activity Notifier
		void
		activity( Time const t )
		{
			assert( conditional != nullptr );
			conditional->shift_conditional( t );
		}

		// Shift Observers Events to Handlers
		void
		handler( Time const t )
		{
			for ( Variable * observer : observers ) {
				observer->shift_handler( t );
			}
		}

	public: // Data

		If * conditional{ nullptr }; // Containing If
		Variables vars; // Variables in Clause
		Variables observers; // Variables dependent on this one (modified by handler)

	}; // Clause

public: // Types

	using Super = Conditional;
	using Variable = V;
	using Time = typename Variable::Time;
	using Value = typename Variable::Value;

	using Clauses = std::vector< Clause * >;
	using size_type = typename Clauses::size_type;

public: // Creation

	// Default Constructor
	IfV() :
	 Conditional( "If" )
	{
		add_conditional();
	}

	// Name Constructor
	explicit
	IfV( std::string const & name ) :
	 Conditional( name )
	{
		add_conditional();
	}

	// Destructor
	~IfV()
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

public: // Properties

	// Size
	size_type
	size() const
	{
		return clauses.size();
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

public: // Methods

	// Add a Clause
	Clause *
	add_clause()
	{
		Clause * clause( new Clause( this ) );
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
				//break; // Highest priority active Clause found // FMU doesn't provide priority sequence yet so run all possible handlers
			}
		}
		shift_conditional();
	}

private: // Data

	Clauses clauses; // Clauses in decreasing priority sequence

}; // IfV

// Conditional When Block Template
template< typename V >
class WhenV final : public Conditional
{

public: // Nested Types

	// Clause Class
	class Clause
	{

	public: // Types

		using Variable = V;
		using When = WhenV< V >;
		using Time = typename Variable::Time;
		using Variables = typename Variable::Variables;
		using size_type = typename Variables::size_type;

	public: // Creation

		// Constructor
		Clause( When * conditional ) :
		 conditional( conditional )
		{}

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

		// Add an Observer Variable
		void
		add_observer( Variable * v )
		{
			observers.push_back( v );
		}

		// Variable Activity Notifier
		void
		activity( Time const t )
		{
			assert( conditional != nullptr );
			conditional->shift_conditional( t );
		}

		// Shift Observers Events to Handlers
		void
		handler( Time const t )
		{
			for ( Variable * observer : observers ) {
				observer->shift_handler( t );
			}
		}

	public: // Data

		When * conditional{ nullptr }; // Containing When
		Variables vars; // Variables in Clause
		Variables observers; // Variables dependent on this one (modified by handler)

	}; // Clause

public: // Types

	using Super = Conditional;
	using Variable = V;
	using Time = typename Variable::Time;
	using Value = typename Variable::Value;

	using Clauses = std::vector< Clause * >;
	using size_type = typename Clauses::size_type;

public: // Creation

	// Default Constructor
	WhenV() :
	 Conditional( "When" )
	{
		add_conditional();
	}

	// Name Constructor
	explicit
	WhenV( std::string const & name ) :
	 Conditional( name )
	{
		add_conditional();
	}

	// Destructor
	~WhenV()
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

public: // Properties

	// Size
	size_type
	size() const
	{
		return clauses.size();
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

public: // Methods

	// Add a Clause
	Clause *
	add_clause()
	{
		Clause * clause( new Clause( this ) );
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
				//break; // Highest priority active Clause found // FMU doesn't provide priority sequence yet so run all possible handlers
			}
		}
		shift_conditional();
	}

private: // Data

	Clauses clauses; // Clauses in decreasing priority sequence

}; // WhenV

} // fmu
} // QSS

#endif
