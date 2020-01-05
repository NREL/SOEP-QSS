// QSS Solver String Functions
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

#ifndef QSS_string_hh_INCLUDED
#define QSS_string_hh_INCLUDED

// C++ Headers
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace QSS {

// Uppercased string
inline
std::string
uppercased( std::string const & s )
{
	std::string u( s.length(), ' ' );
	for ( std::string::size_type i = 0, e = s.length(); i < e; ++i ) {
		u[ i ] = static_cast< char >( std::toupper( s[ i ] ) );
	}
	return u;
}

// Is char Pointer Pointing to String Whitespace Tail
inline
bool
is_tail( char * end )
{
	if ( end == nullptr ) return false;
	while ( std::isspace( *end ) ) ++end;
	return ( *end == '\0' );
}

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

// int of a string
inline
int
int_of( std::string const & s )
{
	return std::stoi( s ); // Check is_int first
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

// size of a string
inline
std::size_t
size_of( std::string const & s )
{
	return std::stoull( s ); // Check is_size first
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

// double of a string
inline
double
double_of( std::string const & s )
{
	return std::stod( s ); // Check is_double first
}

// Has a Character Case-Insensitively?
inline
bool
has( std::string const & s, char const c )
{
	char const b( static_cast< char >( std::tolower( c ) ) );
	for ( char const a : s ) {
		if ( std::tolower( a ) == b ) return true;
	}
	return false;
}

// Has any Character not in a String Case-Insensitively?
inline
bool
has_any_not_of( std::string const & s, std::string const & t ) // Pass lowercase t
{
	for ( char const a : s ) {
		if ( t.find( static_cast< char >( std::tolower( a ) ) ) == std::string::npos ) return true;
	}
	return false;
}

// Has a Prefix Case-Optionally?
inline
bool
has_prefix( std::string const & s, std::string const & pre )
{
	std::string::size_type const pre_len( pre.length() );
	if ( pre_len == 0 ) {
		return false;
	} else if ( s.length() < pre_len ) {
		return false;
	} else {
		for ( std::string::size_type i = 0; i < pre_len; ++i ) {
			if ( s[ i ] != pre[ i ] ) return false;
		}
		return true;
	}
}

// Has a Prefix?
inline
bool
has_prefix( std::string const & s, char const * const pre )
{
	std::string::size_type const pre_len( std::strlen( pre ) );
	if ( pre_len == 0 ) {
		return false;
	} else {
		std::string::size_type const s_len( s.length() );
		if ( s_len < pre_len ) {
			return false;
		} else {
			for ( std::string::size_type i = 0; i < pre_len; ++i ) {
				if ( s[ i ] != pre[ i ] ) return false;
			}
			return true;
		}
	}
}

// Has a Suffix Case-Optionally?
inline
bool
has_suffix( std::string const & s, std::string const & suf )
{
	std::string::size_type const suf_len( suf.length() );
	if ( suf_len == 0 ) {
		return false;
	} else {
		std::string::size_type const s_len( s.length() );
		if ( s_len < suf_len ) {
			return false;
		} else {
			std::string::size_type const del_len( s_len - suf_len );
			for ( std::string::size_type i = 0; i < suf_len; ++i ) {
				if ( s[ del_len + i ] != suf[ i ] ) return false;
			}
			return true;
		}
	}
}

// Has a Suffix?
inline
bool
has_suffix( std::string const & s, char const * const suf )
{
	std::string::size_type const suf_len( std::strlen( suf ) );
	if ( suf_len == 0 ) {
		return false;
	} else {
		std::string::size_type const s_len( s.length() );
		if ( s_len < suf_len ) {
			return false;
		} else {
			std::string::size_type const del_len( s_len - suf_len );
			for ( std::string::size_type i = 0; i < suf_len; ++i ) {
				if ( s[ del_len + i ] != suf[ i ] ) return false;
			}
			return true;
		}
	}
}

// Has an Option (Case-Insensitive)?
inline
bool
has_option( std::string const & s, char const * const option )
{
	std::string const opt( "--" + std::string( option ) );
	std::string::size_type const opt_len( opt.length() );
	if ( s.length() != opt_len ) {
		return false;
	} else {
		for ( std::string::size_type i = 0; i < opt_len; ++i ) {
			if ( std::tolower( s[ i ] ) != std::tolower( opt[ i ] ) ) return false;
		}
		return true;
	}
}

// Has a Value Option (Case-Insensitive)?
inline
bool
has_value_option( std::string const & s, char const * const option )
{
	std::string const opt( "--" + std::string( option ) );
	std::string::size_type const opt_len( opt.length() );
	if ( s.length() <= opt_len ) {
		return false;
	} else {
		for ( std::string::size_type i = 0; i < opt_len; ++i ) {
			if ( std::tolower( s[ i ] ) != std::tolower( opt[ i ] ) ) return false;
		}
		return ( s[ opt_len ] == '=' ) || ( s[ opt_len ] == ':' );
	}
}

// Argument Value
inline
std::string
arg_value( std::string const & arg )
{
	std::string::size_type const i( arg.find_first_of( "=:" ) );
	if ( i != std::string::npos ) {
		return arg.substr( i + 1 );
	} else {
		return std::string();
	}
}

// Split into Tokens
inline
std::vector< std::string >
split( std::string const & str, char del = ' ' )
{
	std::vector< std::string > toks;
	std::istringstream ss( str );
	std::string tok;
	while ( std::getline( ss, tok, del ) ) {
		toks.push_back( tok );
	}
	return toks;
}

} // QSS

#endif
