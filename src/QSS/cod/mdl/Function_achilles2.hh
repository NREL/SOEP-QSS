// Function for Achilles and the Tortoise Derivative Variable 2
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

#ifndef QSS_cod_mdl_Function_achilles2_hh_INCLUDED
#define QSS_cod_mdl_Function_achilles2_hh_INCLUDED

// C++ Headers
#include <cassert>

namespace QSS {
namespace cod {
namespace mdl {

// Linear Time-Invariant Function
template< typename V > // Template to avoid cyclic inclusion with Variable
class Function_achilles2 final
{

public: // Types

	using Coefficient = double;

	using Variable = V;

	using Time = typename Variable::Time;
	using Real = typename Variable::Real;

public: // Property

	// Continuous Value at Time t
	Real
	operator ()( Time const t ) const
	{
		return c1_ * x1_->x( t );
	}

	// Continuous Value at Time t
	Real
	x( Time const t ) const
	{
		return c1_ * x1_->x( t );
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const t ) const
	{
		return c1_ * x1_->x1( t );
	}

	// Quantized Value at Time t
	Real
	q( Time const t ) const
	{
		return c1_ * x1_->q( t );
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const t ) const
	{
		return c1_ * x1_->q1( t );
	}

	// Quantized Second Derivative at Time t
	Real
	q2( Time const t ) const
	{
		return c1_ * x1_->q2( t );
	}

	// Quantized Sequential Value at Time t
	Real
	qs( Time const t ) const
	{
		return q( t );
	}

	// Quantized Forward-Difference Sequential First Derivative at Time t
	Real
	qf1( Time const t ) const
	{
		return q1( t );
	}

	// Quantized Centered-Difference Sequential First Derivative at Time t
	Real
	qc1( Time const t ) const
	{
		return q1( t );
	}

	// Quantized Centered-Difference Sequential Second Derivative at Time t
	Real
	qc2( Time const t ) const
	{
		return q2( t );
	}

public: // Methods

	// Add Variable
	void
	add( Variable * v, Variable * = nullptr )
	{
		x1_ = v;
	}

private: // Data

	Coefficient const c0_{ 0.0 }, c1_{ -1.0 };
	Variable * x1_{ nullptr };

};

} // mdl
} // cod
} // QSS

#endif
