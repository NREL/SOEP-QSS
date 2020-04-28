// QSS Output Filter Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2020 Objexx Engineering, Inc. All rights reserved.
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

// C++ Headers
#include <fstream>
#include <regex>
#include <string>
#include <vector>

namespace QSS {

// QSS Output Filter Class
class OutputFilter
{

public: // Types

	using Filter = std::regex;
	using Filters = std::vector< Filter >;
	using size_type = Filters::size_type;

public: // Creation

	// File Name Constructor
	OutputFilter( std::string const & var_file )
	{
		std::ifstream var_stream( var_file, std::ios_base::binary | std::ios_base::in );
		if ( var_stream.is_open() ) {
			std::string line;
			while ( std::getline( var_stream, line ) ) {

				// Convert line from glob to regex
				std::string reg_line;
				for ( char const c : line ) {
					if ( c == '?' ) {
						reg_line.push_back( '.' );
					} else if ( c == '*' ) {
						reg_line.append( ".*" );
					} else {
						reg_line.push_back( c );
					}
				}

				// Add to filter
				filters_.push_back( std::regex( reg_line ) );
			}
			var_stream.close();
		}
	}

public: // Predicate

	// Variable Name OK?
	bool
	operator ()( std::string const & var_name ) const
	{
		if ( filters_.empty() ) return true;

		// Variable name with regex special characters protected
		std::string reg_name;
		for ( char const c : var_name ) {
			if ( ( c == '[' ) || ( c == ']' ) ) { // Protect special characters
				reg_name.push_back( '[' );
				reg_name.push_back( c );
				reg_name.push_back( ']' );
			} else {
				reg_name.push_back( c );
			}
		}

		// Check if name matches a filter
		for ( auto const & filter : filters_ ) {
			if ( std::regex_match( reg_name, filter ) ) return true;
		}
		return false;
	}

private: // Data

	Filters filters_; // Variable name filters

}; // OutputFilter

} // QSS

#endif
