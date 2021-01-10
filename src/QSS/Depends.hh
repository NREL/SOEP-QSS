// QSS Dependency Specs Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2021 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_Depends_hh_INCLUDED
#define QSS_Depends_hh_INCLUDED

// QSS Headers
#include <QSS/string.hh>

// C++ Headers
#include <cassert>
#include <regex>
#include <string>
#include <vector>

namespace QSS {

// QSS Dependency Specs Class
class Depends final
{

public: // Types

	using Spec = std::regex;
	using Deps = std::vector< Spec >;
	using size_type = Deps::size_type;

	struct Dependency final
	{

		// Variable Spec Constructor
		Dependency( std::regex const & var_regex ) :
		 spec( var_regex )
		{}

		// Variable and Dependency Spec Constructor
		Dependency( std::regex const & var_regex, std::regex const & dep_regex ) :
		 spec( var_regex ),
		 deps( 1u, std::regex( dep_regex ) )
		{}

		// Variable and Dependency Specs Constructor
		Dependency( std::regex const & var_regex, std::vector< std::regex > const & dep_regexs ) :
		 spec( var_regex ),
		 deps( dep_regexs )
		{}

		// Empty?
		bool
		empty() const
		{
			return deps.empty();
		}

		// Any?
		bool
		any() const
		{
			return ( ! deps.empty() );
		}

		// Size
		size_type
		size() const
		{
			return deps.size();
		}

		Spec spec; // Variable
		Deps deps; // Dependencies

	}; // Dependency

	using Dependencies = std::vector< Dependency >;
	using size_type = Dependencies::size_type;

public: // Predicate

	// Empty?
	bool
	empty() const
	{
		return dependencies_.empty();
	}

	// Any?
	bool
	any() const
	{
		return ( ! dependencies_.empty() );
	}

	// All Depend on All?
	bool
	all() const
	{
		return all_;
	}

	// All Depend on All?
	bool &
	all()
	{
		return all_;
	}

	// Dependencies Has a Variable?
	bool
	has( std::string const & var_name ) const
	{
		if ( all_ ) return true;
		for ( Dependency const & dependency : dependencies_ ) {
			if ( std::regex_match( regex_string( var_name ), dependency.spec ) ) return true;
		}
		return false;
	}

	// Dependencies Has a Variable and Dependency?
	bool
	has( std::string const & var_name, std::string const & dep_name ) const
	{
		if ( all_ ) return true;
		for ( Dependency const & dependency : dependencies_ ) {
			if ( std::regex_match( regex_string( var_name ), dependency.spec ) ) {
				for ( Spec const & spec : dependency.deps ) {
					if ( std::regex_match( regex_string( dep_name ), spec ) ) return true;
				}
			}
		}
		return false;
	}

public: // Property

	// Size
	size_type
	size() const
	{
		return dependencies_.size();
	}

	// Dependencies
	Dependencies const &
	dependencies() const
	{
		return dependencies_;
	}

	// Add a Variable and Dependencies
	void
	add( std::regex const & var_regex, std::vector< std::regex > const & dep_regexs )
	{
		dependencies_.emplace_back( var_regex, dep_regexs );
	}

public: // Static Methods

	// Regex String of a Variable Spec
	static
	std::string
	regex_string( std::string spec )
	{
		// Convert glob usage to regex (imperfect)
		std::string re_spec;
		for ( char const c : spec ) {
			if ( c == '?' ) {
				re_spec.push_back( '.' );
			} else if ( c == '*' ) {
				re_spec.append( ".*" );
			} else if ( c == '.' ) {
				re_spec.append( "\\." );
			} else if ( c == '[' ) {
				re_spec.append( "\\[" );
			} else if ( c == ']' ) {
				re_spec.append( "\\]" );
			} else {
				re_spec.push_back( c );
			}
		}
		return re_spec;
	}

	// Regex of a Variable Spec
	static
	std::regex
	regex( std::string spec )
	{
		return std::regex( regex_string( spec ) ); // Can throw exception if resulting string is not a valid regex
	}

private: // Data

	bool all_{ false }; // All variables depend on all others?
	Dependencies dependencies_; // Dependency specs

}; // Depends

} // QSS

#endif
