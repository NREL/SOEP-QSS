// QSS Output Filter Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2023 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_OutputFilter_hh_INCLUDED
#define QSS_OutputFilter_hh_INCLUDED

// QSS Headers
#include <QSS/string.hh>

// C++ Headers
#include <fstream>
#include <regex>
#include <string>
#include <vector>

namespace QSS {

// QSS Output Filter Class
class OutputFilter final
{

public: // Types

	using Filter = std::regex;
	using Filters = std::vector< Filter >;
	using size_type = Filters::size_type;

public: // Creation

	// Default Constructor
	OutputFilter() = default;

	// Strings Constructor
	OutputFilter( std::vector< std::string > const & var_specs )
	{
		for ( std::string var_spec : var_specs ) {
			if ( !strip( var_spec ).empty() ) { // Add to filter
				try {
					filters_.push_back( std::regex( regex_string( var_spec ) ) );
				} catch (...) {
					std::cerr << "\nError: Skipping output filter spec that yields invalid regex: " << var_spec << std::endl;
				}

			}
		}
	}

	// File Name Constructor
	OutputFilter( std::string const & var_file )
	{
		std::ifstream var_stream( var_file, std::ios_base::binary | std::ios_base::in );
		if ( var_stream.is_open() ) {
			std::string line;
			while ( std::getline( var_stream, line ) ) {
				if ( ( !strip( line ).empty() ) && ( line[ 0 ] != '#' ) ) { // Add to filter
					try {
						filters_.push_back( std::regex( regex_string( line ) ) );
					} catch (...) {
						std::cerr << "\nError: Skipping --var filter line that yields invalid regex: " << line << std::endl;
					}

				}
			}
			var_stream.close();
		}
	}

public: // Predicate

	// Generate QSS Outputs for a Variable with Given Name?
	bool
	operator ()( std::string const & var_name ) const
	{
		if ( filters_.empty() ) { // Default filtering
			if ( var_name == "time" ) return false; // Omit time variable
			if ( has_prefix( var_name, "der(" ) && has_suffix( var_name, ")" ) ) return false; // Omit derivatives
			if ( ( var_name[ 0 ] == '_' ) && !has_prefix( var_name, "_eventIndicator_" ) ) return false; // Omit internals except for event indicators
			if ( has_prefix( var_name, "temp_" ) && is_int( var_name.substr( 5 ) ) ) return false; // Omit temporary variables
			return true;
		}
		for ( auto const & filter : filters_ ) { // Check if name matches filter
			if ( std::regex_match( var_name, filter ) ) return true;
		}
		return false;
	}

	// Generate QSS Outputs for a Variable with Given Name?
	bool
	qss( std::string const & var_name ) const
	{
		if ( filters_.empty() ) { // Default filtering
			if ( var_name == "time" ) return false; // Omit time variable
			if ( has_prefix( var_name, "der(" ) && has_suffix( var_name, ")" ) ) return false; // Omit derivatives
			if ( ( var_name[ 0 ] == '_' ) && !has_prefix( var_name, "_eventIndicator_" ) ) return false; // Omit internals except for event indicators
			if ( has_prefix( var_name, "temp_" ) && is_int( var_name.substr( 5 ) ) ) return false; // Omit temporary variables
			return true;
		}
		for ( auto const & filter : filters_ ) { // Check if name matches filter
			if ( std::regex_match( var_name, filter ) ) return true;
		}
		return false;
	}

	// Generate FMU Outputs for a Variable with Given Name?
	bool
	fmu( std::string const & var_name ) const
	{
		if ( filters_.empty() ) { // Default filtering
			if ( var_name == "time" ) return false; // Omit time variable
			if ( has_prefix( var_name, "der(" ) && has_suffix( var_name, ")" ) ) return false; // Omit derivatives
			if ( ( var_name[ 0 ] == '_' ) && !has_prefix( var_name, "_eventIndicator_" ) ) return false; // Omit internals except for event indicators
			if ( has_prefix( var_name, "temp_" ) && is_int( var_name.substr( 5 ) ) ) return false; // Omit temporary variables
			return true;
		}
		for ( auto const & filter : filters_ ) { // Check if name matches filter
			if ( std::regex_match( var_name, filter ) ) return true;
		}
		return false;
	}

	// Generate Results Outputs for a Variable with Given Name?
	bool
	res( std::string const & var_name ) const
	{
		if ( filters_.empty() ) return true; // Default to all signals
		if ( var_name == "time" ) return true; // Always include time in results outputs
		for ( auto const & filter : filters_ ) { // Check if name matches filter
			if ( std::regex_match( var_name, filter ) ) return true;
		}
		return false;
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

	Filters filters_; // Variable name filters

}; // OutputFilter

} // QSS

#endif
