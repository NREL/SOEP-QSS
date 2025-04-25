// QSS Solver String Functions
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2025 Objexx Engineering, Inc. All rights reserved.
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

// QSS Headers
#include <QSS/string.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

namespace QSS {

// Is char Pointer Pointing to String Whitespace Tail
bool
is_tail( char const * end )
{
	if ( end == nullptr ) return false;
	while ( std::isspace( *end ) ) ++end;
	return *end == '\0';
}

// char is in a string?
bool
is_any_of( char const c, std::string const & s )
{
#if ( __cplusplus >= 202302L ) // C++23+
	return s.contains( c );
#else
	return std::any_of( s.begin(), s.end(), [ c ]( char const a ){ return a == c; } );
#endif
}

// Has a Character?
bool
has( std::string const & s, char const c )
{
#if ( __cplusplus >= 202302L ) // C++23+
	return s.contains( c );
#else
	return std::any_of( s.begin(), s.end(), [ c ]( char const a ){ return a == c; } );
#endif
}

// Has a Character Case-Insensitively?
bool
HAS( std::string const & s, char const c )
{
	return std::any_of( s.begin(), s.end(), [ b = std::tolower( c ) ]( char const a ){ return std::tolower( a ) == b; } );
}

// Has any Character not in a String?
bool
has_any_not_of( std::string const & s, std::string const & t )
{
#if ( __cplusplus >= 202302L ) // C++23+
	return std::any_of( s.begin(), s.end(), [ &t ]( char const a ){ return !t.contains( a ); } );
#else
	return std::any_of( s.begin(), s.end(), [ &t ]( char const a ){ return t.find( a ) == std::string::npos; } );
#endif
}

// Has a Prefix?
bool
has_prefix( std::string const & s, std::string const & pre )
{
	std::string::size_type const pre_len( pre.length() );
	if ( pre_len == 0 ) {
		return false;
	} else if ( s.length() < pre_len ) {
		return false;
	} else {
		for ( std::string::size_type i = 0u; i < pre_len; ++i ) {
			if ( s[ i ] != pre[ i ] ) return false;
		}
		return true;
	}
}

// Has a Suffix?
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
			for ( std::string::size_type i = 0u; i < suf_len; ++i ) {
				if ( s[ del_len + i ] != suf[ i ] ) return false;
			}
			return true;
		}
	}
}

// Has an Option Without a Value (Case-Insensitive)?
bool
has_option( std::string const & s, std::string const & option )
{
	std::string::size_type const option_len( option.length() );
	if ( s.length() != option_len + 2u ) {
		return false;
	} else if ( ( s[ 0 ] != '-' ) || ( s[ 1 ] != '-' ) ) {
		return false;
	} else {
		for ( std::string::size_type i = 0u; i < option_len; ++i ) {
			if ( std::tolower( s[ i + 2 ] ) != std::tolower( option[ i ] ) ) return false;
		}
		return true;
	}
}

// Has an Option With a Value (Case-Insensitive)?
bool
has_option_value( std::string const & s, std::string const & option )
{
	std::string::size_type const option_len( option.length() );
	if ( s.length() <= option_len + 2u ) {
		return false;
	} else if ( ( s[ 0 ] != '-' ) || ( s[ 1 ] != '-' ) ) {
		return false;
	} else {
		for ( std::string::size_type i = 0u; i < option_len; ++i ) {
			if ( std::tolower( s[ i + 2 ] ) != std::tolower( option[ i ] ) ) return false;
		}
		return is_any_of( s[ option_len + 2u ], "=:+-" );
	}
}

// Lowercased Copy
std::string
lowercased( std::string const & s )
{
	std::string u( s.length(), ' ' );
	for ( std::string::size_type i = 0, e = s.length(); i < e; ++i ) {
		u[ i ] = static_cast< char >( std::tolower( s[ i ] ) );
	}
	return u;
}

// Uppercased Copy
std::string
uppercased( std::string const & s )
{
	std::string u( s.length(), ' ' );
	for ( std::string::size_type i = 0, e = s.length(); i < e; ++i ) {
		u[ i ] = static_cast< char >( std::toupper( s[ i ] ) );
	}
	return u;
}

// Split into Tokens
std::vector< std::string >
split( std::string const & str, char del )
{
	std::vector< std::string > tokens;
	std::istringstream str_stream( str );
	std::string token;
	while ( std::getline( str_stream, token, del ) ) {
		tokens.push_back( token );
	}
	return tokens;
}

// Strip Whitespace from a string's Tails
std::string &
strip( std::string & s )
{
	static std::string const WHITE( " \t\0", 3 );
	if ( !s.empty() ) {
		std::string::size_type const ib( s.find_first_not_of( WHITE ) );
		std::string::size_type const ie( s.find_last_not_of( WHITE ) );
		if ( ( ib == std::string::npos ) || ( ie == std::string::npos ) ) { // All of string is whitespace
			s.clear();
		} else {
			if ( ie < s.length() - 1u ) s.erase( ie + 1u );
			if ( ib > 0u ) s.erase( 0u, ib );
		}
	}
	return s;
}

// Option Separator
char
option_sep( std::string const & s, std::string const & option )
{
	assert( has_option_value( s, option ) );
	std::string::size_type const option_len( option.length() );
	if ( s.length() > option_len + 3u ) { // Has value
		return s[ option.length() + 2u ];
	} else { // No value
		return ' ';
	}
}

// Option Value
std::string
option_value( std::string const & s, std::string const & option )
{
	assert( has_option_value( s, option ) );
	std::string::size_type const option_len( option.length() );
	if ( s.length() > option_len + 3u ) { // Has value
		return s.substr( option.length() + 3u );
	} else { // No value
		return std::string();
	}
}

} // QSS
