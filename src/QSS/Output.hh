// QSS Variable Output Signal Class
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
#if ( __cplusplus >= 201703L ) && ( ( _MSC_VER >= 1924 ) || ( ( __GNUC__ >= 11 ) && !defined(__llvm__) ) || ( defined(__llvm__) && ( !defined(__APPLE_CC__) && ( __clang_major__ >= 14 ) ) || ( defined( __APPLE_CC__ ) && ( __clang_major__ >= 15 ) ) ) ) // C++17+
#include <charconv>
#include <cmath>
#endif

namespace QSS {

// QSS Variable Output Signal Class
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
		if ( !dir.empty() ) {
			if ( !path::make_dir( dir ) ) { // Model name must be valid directory name
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
		file_ = var + dec_ + '.' + flag + ".out";
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
		file_ = var + dec_ + '.' + flag + ".out";
		t_.clear();
		v_.clear();
		t_.reserve( capacity_ );
		v_.reserve( capacity_ );
		if ( !dir.empty() ) {
			if ( !path::make_dir( dir ) ) { // Model name must be valid directory name
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
	 Value const & v
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
		s << std::right << std::scientific << std::setprecision( 15 );
		for ( size_type i = 0, e = t_.size(); i < e; ++i ) {
			s << std::setw( 23 ) << t_[ i ] << ' ' << std::setw( 23 ) << v_[ i ] << '\n';
		}
		s.close();
		t_.clear();
		v_.clear();
	}

private: // Static Data

	static constexpr size_type capacity_{ 2048 }; // Buffer size

private: // Data

	std::string dec_; // File name decoration
	std::string file_; // File name
	Times t_; // Time buffer
	Values v_; // Value buffer

}; // Output

#if ( __cplusplus >= 201703L ) && ( ( _MSC_VER >= 1924 ) || ( ( __GNUC__ >= 11 ) && !defined(__llvm__) ) || ( defined(__llvm__) && ( !defined(__APPLE_CC__) && ( __clang_major__ >= 14 ) ) || ( defined( __APPLE_CC__ ) && ( __clang_major__ >= 15 ) ) ) ) // C++17+

	// Flush Buffers to File: double Specialization
	template<>
	inline
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
		char * const te( t0 + 23u );
		char * const v0( t0 + 24u );
		char * const ve( v0 + 23u );
		for ( size_type i = 0, e = t_.size(); i < e; ++i ) {
			tv_string[ 0 ] = tv_string[ 1 ] = tv_string[ 24 ] = tv_string[ 25 ] = ' ';

			Time const t( t_[ i ] );
			std::string::size_type const t_off( ( std::signbit( t ) ? 0u : 1u ) + ( ( t != 0.0 ) && ( ( std::abs( t ) >= 1.0e100 ) || ( std::abs( t ) < 1.0e-99 ) ) ? 0u : 1u ) );
			std::to_chars_result const t_res( std::to_chars( t0 + t_off, te, t, std::chars_format::scientific, 15 ) );
			assert( t_res.ec == std::errc{} );
			char * tp( t_res.ptr );
			while ( tp < te ) *(tp++) = ' ';

			double const v( v_[ i ] );
			std::string::size_type const v_off( ( std::signbit( v ) ? 0u : 1u ) + ( ( v != 0.0 ) && ( ( std::abs( v ) >= 1.0e100 ) || ( std::abs( v ) < 1.0e-99 ) ) ? 0u : 1u ) );
			std::to_chars_result const v_res( std::to_chars( v0 + v_off, ve, v, std::chars_format::scientific, 15 ) );
			assert( v_res.ec == std::errc{} );
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
