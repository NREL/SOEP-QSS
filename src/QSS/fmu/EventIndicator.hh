// FMU-ME Event Indicator Support
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

#ifndef QSS_fmu_EventIndicator_hh_INCLUDED
#define QSS_fmu_EventIndicator_hh_INCLUDED

// FMI Library Headers
#include <fmilib.h>

// C++ Headers
#include <cstdlib>
#include <unordered_map>
#include <vector>

namespace QSS {
namespace fmu {

// Event Indicator XML Entry Specs
struct EventIndicator final
{
	// Types
	using size_type = std::size_t;

	// Data
	size_type index{ 0u };
	std::vector< size_type > reverseDependencies;
};

// FMU-ME EventIndicators Collection
struct FMUEventIndicators final
{
	// Types
	using EventIndicators = std::vector< EventIndicator >;

	// Constructor
	FMUEventIndicators( void * context ) :
	 context( context )
	{}

	// Data
	EventIndicators eventIndicators;
	bool inEventIndicators{ false };
	void * context{ nullptr }; // Context pointer to its FMU-ME
};

// EventIndicator Global Lookup by FMU-ME Context
namespace { // Pollution control
using AllEventIndicators = std::vector< FMUEventIndicators >;
}
extern AllEventIndicators allEventIndicators;

// XML Callbacks Global
extern fmi2_xml_callbacks_t xml_callbacks;

extern "C" {

int
annotation_start_handle(
 void * /* context */,
 char const * parentName,
 void * /* parent */,
 char const * elm,
 char const ** attr
);

inline
int
annotation_data_handle(
 void * /* context */,
 char const * /* s */,
 int const /* len */
)
{
	return 0;
}

inline
int
annotation_end_handle(
 void * /* context */,
 char const * /* elm */
)
{
	return 0;
}

} // extern "C"

} // fmu
} // QSS

#endif