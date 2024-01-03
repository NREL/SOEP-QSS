// QSS CSV Results Class
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

#ifndef QSS_Results_CSV_hh_INCLUDED
#define QSS_Results_CSV_hh_INCLUDED

// QSS Headers
#include <QSS/path.hh>

// C++ Headers
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>
#if ( __cplusplus >= 201703L ) && ( ( _MSC_VER >= 1922 ) || ( ( __GNUC__ >= 11 ) && !defined(__llvm__) && !defined(__INTEL_COMPILER) && !defined(__INTEL_LLVM_COMPILER) ) ) // C++17+ // VS 16.2+ and GCC 11+ have floating point to_chars support
#include <charconv>
#endif

namespace QSS {

// QSS CSV Results Class
template< typename Value = double >
class Results_CSV final
{

public: // Types

	using Label = std::string;
	using Labels = std::vector< Label >;
	using Values = std::vector< Value >;
	using size_type = typename Values::size_type;

public: // Creation

	// Default Constructor
	Results_CSV() = default;

	// Name Constructor
	explicit
	Results_CSV( std::string const & nam ) :
	 csv_file_( nam + ".csv" ),
	 csv_stream_( csv_file_, std::ios_base::binary | std::ios_base::out )
	{}

	// Directory + Name Constructor
	Results_CSV(
	 std::string const & dir,
	 std::string const & nam
	) :
	 csv_file_( dir + path::sep + nam + ".csv" ),
	 csv_stream_( csv_file_, std::ios_base::binary | std::ios_base::out )
	{}

	// Destructor
	~Results_CSV()
	{
		if ( csv_stream_.is_open() ) csv_stream_.close();
	}

public: // Property

	// File
	std::string const &
	file() const
	{
		return csv_file_;
	}

public: // Methods

	// Name Initializer
	void
	init( std::string const & nam )
	{
		csv_file_ = nam + ".csv";
		csv_stream_ = std::ofstream( csv_file_, std::ios_base::binary | std::ios_base::out );
	}

	// Directory + Name Initializer
	void
	init(
	 std::string const & dir,
	 std::string const & nam
	)
	{
		csv_file_ = dir + path::sep + nam + ".csv";
		csv_stream_ = std::ofstream( csv_file_, std::ios_base::binary | std::ios_base::out );
	}

	// Write Labels Line
	void
	labels( Labels const & labels )
	{
		if ( labels.empty() ) return;
		csv_stream_ << '"' << labels[ 0 ] << '"';
		for ( size_type i = 1, n = labels.size(); i < n; ++i ) csv_stream_ << ",\"" << labels[ i ] << '"';
		csv_stream_ << '\n';
	}

	// Write Values Line
	void
	values( Values const & values )
	{
		if ( values.empty() ) return;
		csv_stream_ << std::setprecision( 16 );
		csv_stream_ << values[ 0 ];
		for ( size_type i = 1, n = values.size(); i < n; ++i ) csv_stream_ << ',' << values[ i ];
		csv_stream_ << '\n';
	}

private: // Static Functions

	// Precision and Width Scientific-Formatted String of a double
	static
	std::string
	sci( double const num )
	{
#if ( __cplusplus >= 201703L ) && ( ( _MSC_VER >= 1922 ) || ( ( __GNUC__ >= 11 ) && !defined(__llvm__) && !defined(__INTEL_COMPILER) && !defined(__INTEL_LLVM_COMPILER) ) ) // C++17+ // VS 16.2+ and GCC 11+ have floating point to_chars support
		std::string num_string( 23, ' ' );
		char * xb( num_string.data() + 1u );
		char * xe( xb + 22u );
		std::to_chars_result const x_res( std::to_chars( ( std::signbit( num ) ? xb : xb + 1u ), xe, num, std::chars_format::scientific, 15 ) );
		if ( x_res.ec == std::errc{} ) return num_string; // Fall through on error
#endif
		std::ostringstream num_stream;
		num_stream << std::right << std::uppercase << std::scientific << std::setprecision( 16 ) << std::setw( 23 ) << num;
		return num_stream.str();
	}

private: // Data

	std::string csv_file_; // CSV file name
	std::ofstream csv_stream_; // CSV file stream

}; // Results_CSV

#if ( __cplusplus >= 201703L ) && ( ( _MSC_VER >= 1922 ) || ( ( __GNUC__ >= 11 ) && !defined(__llvm__) && !defined(__INTEL_COMPILER) && !defined(__INTEL_LLVM_COMPILER) ) ) // C++17+ // VS 16.2+ and GCC 11+ have floating point to_chars support

	// Write Values Line: double Specialization
	template<>
	inline
	void
	Results_CSV< double >::
	values( Values const & values )
	{
		if ( values.empty() ) return;
		std::string v_string( 23u, ' ' );
		char * const v0( v_string.data() );
		char * vb( v0 + 1u );
		char * vc( vb + 1u );
		char * ve( vb + 22u );
		int const precision( 15 );
		for ( size_type i = 0, n = values.size(); i < n; ++i ) {
			double const v( values[ i ] );
			char * vB;
			if ( std::signbit( v ) ) {
				vB = vb;
			} else {
				vB = vc;
				*vb = ' ';
			}
			std::to_chars_result const v_res( std::to_chars( vB, ve, v, std::chars_format::scientific, precision ) );
			if ( v_res.ec != std::errc{} ) v_string.replace( 0u, 23u, sci( v ) ); // Error: fallback code
			char * vp( v_res.ptr );
			while ( vp < ve ) *(vp++) = ' ';
			if ( i > 0u ) csv_stream_ << ',';
			csv_stream_ << v_string;
		}
		csv_stream_ << '\n';
	}

#endif

} // QSS

#endif
