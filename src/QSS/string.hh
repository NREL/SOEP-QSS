// QSS Solver String Functions
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

#ifndef QSS_string_hh_INCLUDED
#define QSS_string_hh_INCLUDED

// C++ Headers
#include <cstdlib>
#include <cstddef>
#include <limits>
#include <string>
#include <vector>

namespace QSS {

// Globals
extern std::string const delta;
extern std::string const x_delta;
extern std::string const x_delta_2;
extern std::string const x_delta_3;
extern std::string const x_delta_4;

// Is char Pointer Pointing to String Whitespace Tail
bool
is_tail( char const * end );

// string is Readable as a int?
inline
bool
is_int( std::string const & s )
{
	char const * str( s.c_str() );
	char * end;
	long int const i( std::strtol( str, &end, 10 ) );
	return ( ( end != str ) && is_tail( end ) && ( std::numeric_limits< int >::min() <= i ) && ( i <= std::numeric_limits< int >::max() ) );
}

// string is Readable as a size?
inline
bool
is_size( std::string const & s )
{
	char const * str( s.c_str() );
	char * end;
	static_cast< void >( std::strtoull( str, &end, 10 ) );
	return ( ( end != str ) && is_tail( end ) );
}

// string is Readable as a double?
inline
bool
is_double( std::string const & s )
{
	char const * str( s.c_str() );
	char * end;
	static_cast< void >( std::strtod( str, &end ) );
	return ( ( end != str ) && is_tail( end ) );
}

// char is in a string?
bool
is_any_of( char const c, std::string const & s );

// char is not in a string?
inline
bool
not_any_of( char const c, std::string const & s )
{
	return !is_any_of( c, s );
}

// Has a Character?
bool
has( std::string const & s, char const c );

// Has a Character Case-Insensitively?
bool
HAS( std::string const & s, char const c );

// Has any Character not in a String?
bool
has_any_not_of( std::string const & s, std::string const & t );

// Has a Prefix?
bool
has_prefix( std::string const & s, std::string const & pre );

// Has a Suffix Case-Optionally?
bool
has_suffix( std::string const & s, std::string const & suf );

// Has a Suffix?
bool
has_suffix( std::string const & s, std::string const & suf );

// Has an Option Without a Value (Case-Insensitive)?
bool
has_option( std::string const & s, std::string const & option );

// Has an Option With a Value (Case-Insensitive)?
bool
has_option_value( std::string const & s, std::string const & option );

// int of a string
inline
int
int_of( std::string const & s )
{
	return std::stoi( s ); // Check is_int first
}

// size of a string
inline
std::size_t
size_of( std::string const & s )
{
	return std::stoull( s ); // Check is_size first
}

// double of a string
inline
double
double_of( std::string const & s )
{
	return std::stod( s ); // Check is_double first
}

// Lowercased Copy
std::string
lowercased( std::string const & s );

// Uppercased Copy
std::string
uppercased( std::string const & s );

// Split into Tokens
std::vector< std::string >
split( std::string const & str, char del = ' ' );

// Strip Whitespace from a string's Tails
std::string &
strip( std::string & s );

// Option Separator
char
option_sep( std::string const & s, std::string const & option );

// Option Value
std::string
option_value( std::string const & arg, std::string const & option );

} // QSS

#endif
