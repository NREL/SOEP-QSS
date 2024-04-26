// QSS Variable Clusters Class
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

#ifndef QSS_Clusters_hh_INCLUDED
#define QSS_Clusters_hh_INCLUDED

// QSS Headers
#include <QSS/Cluster.hh>
#include <QSS/string.hh>

// C++ Headers
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace QSS {

// QSS Variable Clusters Class
class Clusters final
{

public: // Types

	using ClusterSpecs = std::vector< Cluster >;
	using const_iterator = ClusterSpecs::const_iterator;
	using iterator = ClusterSpecs::iterator;
	using size_type = ClusterSpecs::size_type;
	using index_type = size_type;

public: // Creation

	// File Name Constructor
	explicit
	Clusters( std::string const & var_file )
	{
		std::ifstream var_stream( var_file, std::ios_base::binary | std::ios_base::in );
		if ( var_stream.is_open() ) {
			std::string line;
			std::vector< std::string > specs; // Specs for the current cluster
			while ( std::getline( var_stream, line ) ) {
				strip( line );
				if ( line.empty() ) { // Set up to create new cluster
					if ( !specs.empty() ) { // Save previous cluster if any
						clusters_.push_back( Cluster( specs ) );
						specs.clear();
					}
				} else if ( line[ 0 ] != '#' ) { // Add to current cluster
					specs.push_back( line );
				}
			}
			var_stream.close();
			if ( !specs.empty() ) {
				clusters_.push_back( Cluster( specs ) );
			}
		}
	}

public: // Predicate

	// Empty?
	bool
	empty() const
	{
		return clusters_.empty();
	}

public: // Subscript

	// Get a Cluster
	Cluster const &
	operator []( index_type const idx ) const
	{
		assert( idx < clusters_.size() );
		return clusters_[ idx ];
	}

	// Get a Cluster
	Cluster &
	operator []( index_type const idx )
	{
		assert( idx < clusters_.size() );
		return clusters_[ idx ];
	}

public: // Iterator

	// Begin Iterator
	const_iterator
	begin() const
	{
		return clusters_.begin();
	}

	// Begin Iterator
	iterator
	begin()
	{
		return clusters_.begin();
	}

	// End Iterator
	const_iterator
	end() const
	{
		return clusters_.end();
	}

	// End Iterator
	iterator
	end()
	{
		return clusters_.end();
	}

private: // Data

	ClusterSpecs clusters_; // Variable_QSS clusters

}; // Clusters

} // QSS

#endif
