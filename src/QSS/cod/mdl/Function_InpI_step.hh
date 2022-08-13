// Integer Step Input Function
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2022 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_cod_mdl_Function_InpI_step_hh_INCLUDED
#define QSS_cod_mdl_Function_InpI_step_hh_INCLUDED

// QSS Headers
#include <QSS/math.hh>

// C++ Headers
#include <cassert>
#include <cstdint>

namespace QSS {
namespace cod {
namespace mdl {

// Step Input Function
class Function_InpI_step final
{

public: // Types

	using Time = double;
	using Integer = std::int64_t;
	using Coefficient = Integer;

public: // Creation

	// Constructor
	explicit
	Function_InpI_step(
	 Coefficient const h_0 = 0,
	 Coefficient const h = 1,
	 Time const d = 1.0
	) :
	 h_0_( h_0 ),
	 h_( h ),
	 d_( d )
	{
		assert( d_ > Time( 0.0 ) );
	}

public: // Property

	// Initial Height
	Coefficient
	h_0() const
	{
		return h_0_;
	}

	// Step Height
	Coefficient
	h() const
	{
		return h_;
	}

	// Step Time Delta
	Time
	d() const
	{
		return d_;
	}

	// Value at Time t
	Integer
	operator ()( Time const t ) const
	{
		return h_0_ + ( h_ * step_number( t ) );
	}

	// Value at Time t
	Integer
	v( Time const t ) const
	{
		return h_0_ + ( h_ * step_number( t ) );
	}

	// Sequential Value at Time t
	Integer
	vs( Time const t ) const
	{
		return h_0_ + ( h_ * step_number( t ) );
	}

	// Discrete Event after Time t
	Time
	tD( Time const t ) const
	{
		Integer const n_next( Integer( std::floor( t / d_ ) ) + 1 );
		Time const t_next( d_ * n_next );
		return ( t_next > t ? t_next : d_ * ( n_next + 1 ) );
	}

public: // Methods

	// Set Initial Height
	Function_InpI_step &
	h_0( Coefficient const h_0 )
	{
		h_0_ = h_0;
		return *this;
	}

	// Set Height
	Function_InpI_step &
	h( Coefficient const h )
	{
		h_ = h;
		return *this;
	}

	// Set Step Time Delta
	Function_InpI_step &
	d( Time const d )
	{
		d_ = d;
		assert( d_ > Time( 0.0 ) );
		return *this;
	}

private: // Methods

	// Step Number at Time t
	Integer
	step_number( Time const t ) const
	{
		assert( d_ > Time( 0.0 ) );
		Integer const ftd( Integer( std::floor( t / d_ ) ) );
		return ( d_ * ( ftd + 1 ) > t ? ftd : ftd + 1 );
	}

private: // Data

	Coefficient h_0_{ 0 }; // Initial height
	Coefficient h_{ 1 }; // Step height
	Time d_{ 1.0 }; // Step time delta

};

} // mdl
} // cod
} // QSS

#endif
