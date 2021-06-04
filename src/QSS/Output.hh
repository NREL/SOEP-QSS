// QSS Output Signal Class
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

#ifndef QSS_Output_hh_INCLUDED
#define QSS_Output_hh_INCLUDED

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
#if ( __cplusplus >= 201703L ) && ( ( _MSC_VER >= 1922 ) || ( ( __GNUC__ >= 11 ) && !defined(__llvm__) && !defined(__INTEL_COMPILER) ) ) // C++17+ // VS 16.2+ and GCC 11+ have floating point to_chars support
#include <charconv>
#endif

namespace QSS {

// QSS Output Signal Class
template< typename Value = double >
class Output final
{

public: // Types

	using Time = double;
	using Times = std::vector< Time >;
	using Values = std::vector< Value >;
	using size_type = Times::size_type;

public: // Creation

	// Default Constructor
	Output() = default;

	// Name + Flag Constructor
	Output(
	 std::string const & var,
	 char const flag,
	 bool const do_init = true
	) :
	 file_( var + '.' + flag + ".out" )
	{
		if ( do_init ) {
			t_.reserve( capacity_ );
			v_.reserve( capacity_ );
			std::ofstream( file_, std::ios_base::binary | std::ios_base::out );
		}
	}

	// Name + Flag + Decoration Constructor
	Output(
	 std::string const & var,
	 char const flag,
	 std::string const & dec,
	 bool const do_init = true
	) :
	 dec_( dec ),
	 file_( var + dec + '.' + flag + ".out" )
	{
		if ( do_init ) {
			t_.reserve( capacity_ );
			v_.reserve( capacity_ );
			std::ofstream( file_, std::ios_base::binary | std::ios_base::out );
		}
	}

	// Directory + Name + Flag Constructor
	Output(
	 std::string const & dir,
	 std::string const & var,
	 char const flag,
	 std::string const & dec = std::string()
	) :
	 dec_( dec ),
	 file_( var + dec + '.' + flag + ".out" )
	{
		t_.reserve( capacity_ );
		v_.reserve( capacity_ );
		if ( ! dir.empty() ) {
			if ( ! path::make_dir( dir ) ) { // Model name must be valid directory name
				std::cerr << "\nError: Output directory creation failed: " << dir << std::endl;
				std::exit( EXIT_FAILURE );
			}
			file_ = dir + path::sep + file_;
		}
		std::ofstream( file_, std::ios_base::binary | std::ios_base::out );
	}

	// Destructor
	~Output()
	{
		assert( t_.size() == v_.size() );
		assert( t_.size() < capacity_ );
		if ( t_.size() > 0u ) flush();
	}

public: // Property

	// File
	std::string const &
	file() const
	{
		return file_;
	}

public: // Methods

	// Decoration Set
	void
	decorate( std::string const & dec = std::string() )
	{
		dec_ = dec;
	}

	// Initialize Without Output Directory
	void
	init(
	 std::string const & var,
	 char const flag,
	 std::string const & dec = std::string() // Decoration
	)
	{
		if ( !dec.empty() ) dec_ = dec;
		file_ = std::string( var + dec_ + '.' + flag + ".out" );
		t_.clear();
		v_.clear();
		t_.reserve( capacity_ );
		v_.reserve( capacity_ );
		std::ofstream( file_, std::ios_base::binary | std::ios_base::out );
	}

	// Initialize With Output Directory
	void
	init(
	 std::string const & dir,
	 std::string const & var,
	 char const flag,
	 std::string const & dec = std::string() // Decoration
	)
	{
		if ( !dec.empty() ) dec_ = dec;
		file_ = std::string( var + dec_ + '.' + flag + ".out" );
		t_.clear();
		v_.clear();
		t_.reserve( capacity_ );
		v_.reserve( capacity_ );
		if ( ! dir.empty() ) {
			if ( ! path::make_dir( dir ) ) { // Model name must be valid directory name
				std::cerr << "\nError: Output directory creation failed: " << dir << std::endl;
				std::exit( EXIT_FAILURE );
			}
			file_ = dir + path::sep + file_;
		}
		std::ofstream( file_, std::ios_base::binary | std::ios_base::out );
	}

	// Write Header Lines
	void
	header(
	 std::string const & v_type = std::string(),
	 std::string const & v_unit = std::string()
	)
	{
		std::ofstream s( file_, std::ios_base::binary | std::ios_base::out | std::ios_base::app );
		s << "Time " << v_type << '\n' << "s " << v_unit << '\n';
		s.close();
	}

	// Append Time and Value Pair
	void
	append(
	 Time const t,
	 Value const v
	)
	{
		assert( t_.size() == v_.size() );
		assert( t_.size() < capacity_ );
		t_.push_back( t );
		v_.push_back( v );
		if ( t_.size() == capacity_ ) flush();
	}

	// Append Time and Value Pair
	template< typename V >
	void
	append(
	 Time const t,
	 V const v
	)
	{
		assert( t_.size() == v_.size() );
		assert( t_.size() < capacity_ );
		t_.push_back( t );
		v_.push_back( Value( v ) );
		if ( t_.size() == capacity_ ) flush();
	}

	// Flush Buffers to File
	void
	flush()
	{
		assert( t_.size() == v_.size() );
		assert( t_.size() <= capacity_ );
		if ( t_.size() == 0u ) return;
		std::ofstream s( file_, std::ios_base::binary | std::ios_base::out | std::ios_base::app );
		s << std::setprecision( 15 );
		for ( size_type i = 0, e = t_.size(); i < e; ++i ) {
			s << t_[ i ] << ' ' << v_[ i ] << '\n';
		}
		s.close();
		t_.clear();
		v_.clear();
	}

private: // Static Functions

	// Precision and Width Scientific-Formatted String of a double
	static
	std::string
	sci( double const num )
	{
#if ( __cplusplus >= 201703L ) && ( ( _MSC_VER >= 1922 ) || ( ( __GNUC__ >= 11 ) && !defined(__llvm__) && !defined(__INTEL_COMPILER) ) ) // C++17+ // VS 16.2+ and GCC 11+ have floating point to_chars support
		std::string num_string( 23, ' ' );
		char * xb( num_string.data() + 1u );
		char * xe( xb + 22u );
		std::to_chars_result const x_res( std::to_chars( ( std::signbit( num ) ? xb : xb + 1u ), xe, num, std::chars_format::scientific, 15 ) );
		if ( x_res.ec == std::errc{} ) return num_string; // Fall through on error
#endif
		std::ostringstream num_stream;
		num_stream << std::right << std::uppercase << std::scientific << std::setprecision( 15 ) << std::setw( 23 ) << num;
		return num_stream.str();
	}

private: // Static Data

	static size_type constexpr capacity_{ 2048 }; // Buffer size

private: // Data

	std::string dec_; // File name decoration
	std::string file_; // File name
	Times t_; // Time buffer
	Values v_; // Value buffer

}; // Output

#if ( __cplusplus >= 201703L ) && ( ( _MSC_VER >= 1922 ) || ( ( __GNUC__ >= 11 ) && !defined(__llvm__) && !defined(__INTEL_COMPILER) ) ) // C++17+ // VS 16.2+ and GCC 11+ have floating point to_chars support

	// Flush Buffers to File: double Specialization
	template<>
	void
	Output< double >::
	flush()
	{
		assert( t_.size() == v_.size() );
		assert( t_.size() <= capacity_ );
		if ( t_.size() == 0u ) return;
		std::ofstream s( file_, std::ios_base::binary | std::ios_base::out | std::ios_base::app );
		std::string tv_string( 48u, ' ' );
		tv_string[ 47 ] = '\n';
		char * const t0( tv_string.data() );
		char * tb( t0 + 1u );
		char * tc( tb + 1u );
		char * te( tb + 22u );
		char * vb( t0 + 25u );
		char * vc( vb + 1u );
		char * ve( vb + 22u );
		int const precision( 15 );
		for ( size_type i = 0, e = t_.size(); i < e; ++i ) {
			Time const t( t_[ i ] );
			char * tB;
			if ( std::signbit( t ) ) {
				tB = tb;
			} else {
				tB = tc;
				*tb = ' ';
			}
			std::to_chars_result const t_res( std::to_chars( tB, te, t, std::chars_format::scientific, precision ) );
			if ( t_res.ec != std::errc{} ) tv_string.replace( 0u, 23u, sci( t ) ); // Error: fallback code
			char * tp( t_res.ptr );
			while ( tp < te ) *(tp++) = ' ';

			double const v( v_[ i ] );
			char * vB;
			if ( std::signbit( v ) ) {
				vB = vb;
			} else {
				vB = vc;
				*vb = ' ';
			}
			std::to_chars_result const v_res( std::to_chars( vB, ve, v, std::chars_format::scientific, precision ) );
			if ( v_res.ec != std::errc{} ) tv_string.replace( 24u, 23u, sci( v ) ); // Error: fallback code
			char * vp( v_res.ptr );
			while ( vp < ve ) *(vp++) = ' ';

			s << tv_string;
		}
		s.close();
		t_.clear();
		v_.clear();
	}

#endif

} // QSS

#endif
