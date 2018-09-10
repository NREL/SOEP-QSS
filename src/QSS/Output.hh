// QSS Output Signal Class
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

#ifndef QSS_Output_hh_INCLUDED
#define QSS_Output_hh_INCLUDED

// C++ Headers
#include <cassert>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

namespace QSS {

// QSS Output Signal Class Template
template< typename Real = double >
class Output_T
{

public: // Types

	using Time = double;
	using Times = std::vector< Time >;
	using Reals = std::vector< Real >;
	using size_type = Times::size_type;

public: // Creation

	// Name + Type Constructor
	Output_T(
	 std::string const & name,
	 char const type
	) :
	 file_( name + '.' + type + ".out" )
	{
		t_.reserve( capacity_ );
		v_.reserve( capacity_ );
		std::ofstream( file_, std::ios_base::binary | std::ios_base::out );
	}

	// Destructor
	~Output_T()
	{
		assert( t_.size() == v_.size() );
		assert( t_.size() < capacity_ );
		if ( t_.size() > 0u ) flush();
	}

public: // Methods

	// Append Time and Value Pair
	void
	append(
	 Time const t,
	 Real const v
	)
	{
		assert( t_.size() == v_.size() );
		assert( t_.size() < capacity_ );
		t_.push_back( t );
		v_.push_back( v );
		if ( t_.size() == capacity_ ) flush();
	}

private: // Methods

	// Flush Buffers to File
	void
	flush()
	{
		assert( t_.size() == v_.size() );
		assert( t_.size() <= capacity_ );
		std::ofstream s( file_, std::ios_base::binary | std::ios_base::out | std::ios_base::app );
		s << std::setprecision( 15 );
		for ( size_type i = 0, e = t_.size(); i < e; ++i ) {
			s << t_[ i ] << '\t' << v_[ i ] << '\n';
		}
		t_.clear();
		v_.clear();
	}

private: // Data

	std::string file_; // File name
	Times t_; // Time buffer
	Reals v_; // Value buffer
	size_type const capacity_{ 2048 }; // Buffer capacities

};

// Types
using Output = Output_T< double >;

} // QSS

#endif
