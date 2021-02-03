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
#include <string>
#include <vector>

namespace QSS {

// QSS Output Signal Class Template
template< typename Value = double >
class Output_T final
{

public: // Types

	using Time = double;
	using Times = std::vector< Time >;
	using Values = std::vector< Value >;
	using size_type = Times::size_type;

public: // Creation

	// Default Constructor
	Output_T() = default;

	// Name + Type Constructor
	Output_T(
	 std::string const & var,
	 char const type,
	 bool const do_init = true
	) :
	 file_( var + '.' + type + ".out" )
	{
		if ( do_init ) {
			t_.reserve( capacity_ );
			v_.reserve( capacity_ );
			std::ofstream( file_, std::ios_base::binary | std::ios_base::out );
		}
	}

	// Directory + Name + Type Constructor
	Output_T(
	 std::string const & dir,
	 std::string const & var,
	 char const type
	) :
	 file_( var + '.' + type + ".out" )
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
	~Output_T()
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

	// Initialize Without Output Directory
	void
	init(
	 std::string const & var,
	 char const type
	)
	{
		file_ = std::string( var + '.' + type + ".out" );
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
	 char const type
	)
	{
		file_ = std::string( var + '.' + type + ".out" );
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
			s << t_[ i ] << '\t' << v_[ i ] << '\n';
		}
		s.close();
		t_.clear();
		v_.clear();
	}

private: // Static Data

	static size_type const capacity_{ 2048 }; // Buffer size

private: // Data

	std::string file_; // File name
	Times t_; // Time buffer
	Values v_; // Value buffer

}; // Output

	// Static Data Member Template Definitions
	template< typename Value > typename Output_T< Value >::size_type const Output_T< Value >::capacity_;

// Types
using Output = Output_T< double >;

} // QSS

#endif
