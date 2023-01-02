// LIQSS/QSS Step Size Ratio
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2023 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_LIQSS_QSS_Step_Ratio_hh_INCLUDED
#define QSS_LIQSS_QSS_Step_Ratio_hh_INCLUDED

// QSS Headers
#include <QSS/math.hh>

// C++ Headers
#include <cstddef>

namespace QSS {

// LIQSS/QSS Step Size Ratio
class LIQSS_QSS_Step_Ratio final
{

public: // Types

	using Real = double;
	using size_type = std::size_t;

public: // Predicate

	// Check Ratio This Step?
	bool
	check_ratio()
	{
		return ( step_count_++ % step_check_every_ ) == 0u;
	}

public: // Properties

	// Ratio
	Real
	ratio() const
	{
		size_type const ratio_den_( ratio_count_ - ratio_inf_count_ );
		return ( ratio_den_ > 0u ? ratio_sum_ / ratio_den_ : 1.0 );
	}

	// Ratio Infinity percent
	Real
	ratio_inf_percent() const
	{
		return ( ratio_count_ > 0u ? ( 100.0 * ratio_inf_count_ ) / ratio_count_ : 0.0 );
	}

public: // Methods

	// Add a Ratio
	void
	add( Real const ratio )
	{
		if ( ratio == infinity ) {
			++ratio_inf_count_;
		} else {
			ratio_sum_ += ratio;
		}
		++ratio_count_;
	}

private: // Data

	Real ratio_sum_{ 0.0 };
	size_type ratio_inf_count_{ 0u };
	size_type ratio_count_{ 0u };
	size_type step_count_{ 0u };
	static size_type const step_check_every_{ 100u };

}; // LIQSS_QSS_Step_Ratio

} // QSS

#endif
