// QSS Solver Path Functions
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2024 Objexx Engineering, Inc. All rights reserved.
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

// QSS  Headers
#include <QSS/path.hh>
#include <QSS/string.hh>

// C++ Headers
#include <cctype>
#include <regex>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
#include <direct.h>
#include <errno.h>
#else
#include <sys/errno.h>
#endif
#ifdef _WIN32
#ifndef S_ISREG
#define S_ISREG(mode) (((mode)&S_IFMT)==S_IFREG)
#endif
#ifdef _WIN64
#define stat _stat64
#else
#define stat _stat
#endif
#endif

namespace QSS {
namespace path {

// Globals
#ifdef _WIN32

std::string
get_tmp_dir()
{
	char * buf;
	std::size_t sz;
	errno_t const err( _dupenv_s( &buf, &sz, "TEMP" ) );
	std::string const tmp_dir( ( err == 0 ) && ( buf != nullptr ) ? buf : "." );
	free( buf );
	return tmp_dir;
}

char const sep( '\\' );
std::string const tmp( get_tmp_dir() );
#else
char const sep( '/' );
std::string const tmp( "/tmp" );
#endif

// Is Name a File?
bool
is_file( std::string const & name )
{
	if ( name.empty() ) return false;
	struct stat stat_buf;
#ifdef _WIN32
	return ( stat( name.c_str(), &stat_buf ) == 0 ) && S_ISREG( stat_buf.st_mode ) && ( stat_buf.st_mode & _S_IREAD );
#else
	return ( stat( name.c_str(), &stat_buf ) == 0 ) && S_ISREG( stat_buf.st_mode ) && ( stat_buf.st_mode & S_IRUSR );
#endif
}

// Base Name
std::string
base( std::string const & path )
{
	std::size_t ibeg( path.rfind( sep ) );
	if ( ibeg == std::string::npos ) {
		ibeg = 0;
	} else {
		++ibeg;
	}
	std::string const name( path, ibeg, path.length() - ibeg );
	std::size_t const idot( name.rfind( '.' ) );
	if ( idot == std::string::npos ) {
		return name;
	} else {
		return name.substr( 0u, idot );
	}
}

// Directory Name
std::string
dir( std::string const & path )
{
	std::size_t l( path.length() ); // Length to search up to
	while ( ( l > 0u ) && ( path[ l - 1 ] == sep ) ) --l; // Skip trailing path separators
	while ( ( l > 0u ) && ( path[ l - 1 ] != sep ) ) --l; // Skip dir/file name
	if ( ( l > 0u ) && ( path[ l - 1 ] == sep ) ) --l; // Skip trailing path separator
	if ( l > 0u ) {
		return path.substr( 0u, l );
	} else {
		return std::string( "." );
	}
}

// Make a Directory
bool
make_dir( std::string const & dir )
{
#ifdef _WIN32
#ifdef __MINGW32__
	return ( mkdir( dir.c_str() ) == 0 ) || ( errno == EEXIST );
#else
	return ( _mkdir( dir.c_str() ) == 0 ) || ( errno == EEXIST );
#endif
#else
	return ( mkdir( dir.c_str(), S_IRWXU ) == 0 ) || ( errno == EEXIST );
#endif
}

// Make a Path
bool
make_path( std::string const & path )
{
#ifdef _WIN32
	// Find the starting position
	if ( path.empty() ) return false;
	std::string::size_type i( 0 );
	std::string::size_type const path_len( path.length() );
	if ( path_len >= 2 ) {
		if ( ( isalpha( path[ 0 ] ) ) && ( path[ 1 ] == ':' ) ) { // X:
			i = 2;
		}
	}
	if ( i < path_len ) i = path.find_first_not_of( ".\\/", i );

	// Create the directories of the path
	if ( ( i < path_len ) && ( i != std::string::npos ) ) {
		while ( ( i = path.find_first_of( "\\/", i ) ) != std::string::npos ) {
			if ( i + 1 == path.length() ) { // Last directory
				return make_dir( path.substr( 0u, i ) );
			} else if ( !make_dir( path.substr( 0u, i ) ) ) { // Failed
				return false;
			}
			++i;
		}
		return make_dir( path ); // One more directory
	} else { // Nothing to do
		return true;
	}
#else
	// Find the starting position
	if ( path.empty() ) return false;
	std::string::size_type i( path.find_first_not_of( "./" ) );
	std::string::size_type const path_len( path.length() );

	// Create the directories of the path
	if ( ( i < path_len ) && ( i != std::string::npos ) ) {
		while ( ( i = path.find_first_of( "/", i ) ) != std::string::npos ) {
			if ( i + 1 == path.length() ) { // Last directory
				return make_dir( path.substr( 0u, i ) );
			} else if ( !make_dir( path.substr( 0u, i ) ) ) { // Failed
				return false;
			}
			++i;
		}
		return make_dir( path ); // One more directory
	} else { // Nothing to do
		return true;
	}
#endif
}

// URI of a Path
std::string
uri( std::string const & path )
{
	std::string uri;
	for ( char c : path ) {
		switch ( c ) {
		case '%':
			uri += "%25";
			break;
		case ' ':
			uri += "%20";
			break;
		case '!':
			uri += "%21";
			break;
		case '#':
			uri += "%23";
			break;
		case '$':
			uri += "%24";
			break;
		case '&':
			uri += "%26";
			break;
		case '\'':
			uri += "%27";
			break;
		case '(':
			uri += "%28";
			break;
		case ')':
			uri += "%29";
			break;
		case '*':
			uri += "%2A";
			break;
		case '+':
			uri += "%2B";
			break;
		case ',':
			uri += "%2C";
			break;
		case ':':
			uri += "%3A";
			break;
		case ';':
			uri += "%3B";
			break;
		case '=':
			uri += "%3D";
			break;
		case '?':
			uri += "%3F";
			break;
		case '@':
			uri += "%40";
			break;
		case '[':
			uri += "%5B";
			break;
		case ']':
			uri += "%5D";
			break;
		case '^':
			uri += "%5E";
			break;
		case '`':
			uri += "%60";
			break;
		case '{':
			uri += "%7B";
			break;
		case '}':
			uri += "%7D";
			break;
#ifdef _WIN32
		case '\\':
			uri += '/';
			break;
#endif
		default:
			uri += c;
			break;
		}
	}
	return uri;
}

// Path of a URI
std::string
un_uri( std::string const & uri )
{
	std::string path( uri );
	if ( has_prefix( path, "file://" ) ) path.erase( 0, 7 );
	path = std::regex_replace( path, std::regex( "%20" ), " " );
	path = std::regex_replace( path, std::regex( "%21" ), "!" );
	path = std::regex_replace( path, std::regex( "%23" ), "#" );
	path = std::regex_replace( path, std::regex( "%24" ), "$" );
	path = std::regex_replace( path, std::regex( "%26" ), "&" );
	path = std::regex_replace( path, std::regex( "%27" ), "'" );
	path = std::regex_replace( path, std::regex( "%28" ), "(" );
	path = std::regex_replace( path, std::regex( "%29" ), ")" );
	path = std::regex_replace( path, std::regex( "%2A" ), "*" );
	path = std::regex_replace( path, std::regex( "%2B" ), "+" );
	path = std::regex_replace( path, std::regex( "%2C" ), "," );
	path = std::regex_replace( path, std::regex( "%3A" ), ":" );
	path = std::regex_replace( path, std::regex( "%3B" ), ";" );
	path = std::regex_replace( path, std::regex( "%3D" ), "=" );
	path = std::regex_replace( path, std::regex( "%3F" ), "?" );
	path = std::regex_replace( path, std::regex( "%40" ), "@" );
	path = std::regex_replace( path, std::regex( "%5B" ), "[" );
	path = std::regex_replace( path, std::regex( "%5D" ), "]" );
	path = std::regex_replace( path, std::regex( "%5E" ), "^" );
	path = std::regex_replace( path, std::regex( "%60" ), "`" );
	path = std::regex_replace( path, std::regex( "%7B" ), "{" );
	path = std::regex_replace( path, std::regex( "%7D" ), "}" );
#ifdef _WIN32
	path = std::regex_replace( path, std::regex( "/" ), "\\" );
#endif
	path = std::regex_replace( path, std::regex( "%25" ), "%" );
	return path;
}

} // path
} // QSS
