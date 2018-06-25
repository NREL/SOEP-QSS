// QSS Solver Path Functions
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

// QSS  Headers
#include <QSS/path.hh>

// C++ Headers
#include <cctype>
#ifdef _WIN32
#include <direct.h>
#include <errno.h>
#else
#include <sys/stat.h>
#include <sys/errno.h>
#endif

namespace QSS {
namespace path {

// Base Name
std::string
base( std::string const & path )
{
#ifdef _WIN32
	char const path_sep( '\\' );
#else
	char const path_sep( '/' );
#endif
	std::size_t ibeg( path.rfind( path_sep ) );
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
		return name.substr( 0, idot );
	}
}

// Directory Name
std::string
dir( std::string const & path )
{
#ifdef _WIN32
	char const path_sep( '\\' );
#else
	char const path_sep( '/' );
#endif
	std::size_t l( path.length() ); // Length to search up to
	while ( ( l > 0u ) && ( path[ l - 1 ] == path_sep ) ) --l; // Skip trailing path separators
	while ( ( l > 0u ) && ( path[ l - 1 ] != path_sep ) ) --l; // Skip dir/file name
	if ( ( l > 0u ) && ( path[ l - 1 ] == path_sep ) ) --l; // Skip trailing path separator
	if ( l > 0u ) {
		return path.substr( 0, l );
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
	return ( ( mkdir( dir.c_str() ) == 0 ) || ( errno == EEXIST ) );
#else
	return ( ( _mkdir( dir.c_str() ) == 0 ) || ( errno == EEXIST ) );
#endif
#else
	return ( ( mkdir( dir.c_str(), S_IRWXU ) == 0 ) || ( errno == EEXIST ) );
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
				return make_dir( path.substr( 0, i ) );
			} else if ( ! make_dir( path.substr( 0, i ) ) ) { // Failed
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
				return make_dir( path.substr( 0, i ) );
			} else if ( ! make_dir( path.substr( 0, i ) ) ) { // Failed
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

} // path
} // QSS
