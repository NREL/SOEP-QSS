// Options Support
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

#ifndef QSS_options_hh_INCLUDED
#define QSS_options_hh_INCLUDED

// C++ Headers
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace QSS {
namespace options {

using Models = std::vector< std::string >;
using InpFxn = std::unordered_map< std::string, std::string >;
using InpOut = std::unordered_map< std::string, std::string >;

// QSS Method Enumerator
enum class QSS {
 QSS1,
 QSS2,
 QSS3,
 LIQSS1,
 LIQSS2,
 LIQSS3,
 xQSS1,
 xQSS2,
 xQSS3,
 xLIQSS1,
 xLIQSS2,
 xLIQSS3
};

extern QSS qss; // QSS method: (x)(LI)QSS(1|2|3)
extern double rTol; // Relative tolerance
extern double aTol; // Absolute tolerance
extern double zTol; // Zero-crossing tolerance
extern double dtMin; // Min time step (s)
extern double dtMax; // Max time step (s)
extern double dtInf; // Inf time step (s)
extern double dtZC; // FMU zero-crossing time step (s)
extern double dtNum; // Numeric differentiation time step (s)
extern double dtOut; // Sampled & FMU output time step (s)
extern double one_over_dtNum; // 1 / dtNum
extern double one_half_over_dtNum; // 1 / ( 2 * dtNum )
extern double one_sixth_over_dtNum_squared; // 1 / ( 6 * dtNum^2 )  [computed]
extern double tEnd; // End time (s)
extern std::size_t pass; // Pass count limit
extern bool cycles; // Report dependency cycles?
extern bool inflection; // Requantize at inflections?
extern bool refine; // Refine FMU zero-crossing roots?
extern InpFxn fxn; // Map from input variables to function specs
extern InpOut con; // Map from input variables to output variables
extern std::string out; // Outputs: r, a, s, x, q, f
extern Models models; // Name of model(s) or FMU(s)

namespace specified {

extern bool qss; // QSS method specified?
extern bool rTol; // Relative tolerance specified?
extern bool aTol; // Absolute tolerance specified?
extern bool tEnd; // End time specified?

} // specified

namespace output { // Output selections

extern bool t; // Time events?
extern bool r; // Requantizations?
extern bool a; // All variables?
extern bool s; // Sampled output?
extern bool f; // FMU outputs?
extern bool k; // FMU smooth tokens?
extern bool x; // Continuous trajectories?
extern bool q; // Quantized trajectories?
extern bool d; // Diagnostic output?

} // output

// Process command line arguments
void
process_args( int argc, char * argv[] );

// Multiple models?
bool
have_multiple_models();

// Input-output connections?
bool
have_connections();

} // options
} // QSS

#endif
