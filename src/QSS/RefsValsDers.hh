// FMU References + Values + Derivatives
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

#ifndef QSS_RefsValsDers_hh_INCLUDED
#define QSS_RefsValsDers_hh_INCLUDED

// C++ Headers
#include <cassert>

namespace QSS {

// FMU References + Values + Derivatives
template< typename V >
struct RefsValsDers final
{

public: // Types

	using Variable = V;
	using Variables = typename Variable::Variables;
	using Ref = typename Variable::VariableRef;
	using Refs = typename Variable::VariableRefs;
	using Val = typename Variable::Real;
	using Vals = typename Variable::Reals;
	using Der = typename Variable::Real;
	using Ders = typename Variable::Reals;
	using size_type = typename Variables::size_type;

public: // Property

	// Size
	size_type
	size() const
	{
		assert( refs.size() == vals.size() );
		assert( ders.empty() || ( refs.size() == ders.size() ) );
		assert( ders_p.empty() || ( refs.size() == ders_p.size() ) );
		return refs.size();
	}

public: // Methods

	// Clear
	void
	clear()
	{
		refs.clear();
		vals.clear();
		ders.clear();
		ders_p.clear();
	}

	// Clear and Reserve
	void
	clear_and_reserve( size_type const n )
	{
		refs.clear(); refs.reserve( n );
		vals.clear(); vals.reserve( n );
		ders.clear(); ders.reserve( n );
		ders_p.clear(); ders_p.reserve( n );
	}

	// Push Back
	void
	push_back( Ref const & ref )
	{
		refs.push_back( ref );
		vals.push_back( 0.0 );
		ders.push_back( 0.0 );
		ders_p.push_back( 0.0 );
	}

public: // Data

	Refs refs; // FMU value reference array
	Vals vals; // FMU value array
	Ders ders; // FMU derivative array
	Ders ders_p; // FMU derivative array at positive ND step

}; // RefsValsDers

} // QSS

#endif
