// QSS Options Support
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

#ifndef QSS_options_hh_INCLUDED
#define QSS_options_hh_INCLUDED

// QSS Headers
#include <QSS/Depends.hh>

// C++ Headers
#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
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

// Logging Level Enumerator
enum class LogLevel {
 fatal,
 error,
 warning,
 info,
 verbose,
 debug,
 all
};

extern QSS qss; // QSS method: (x)(LI)QSS(1|2|3)
extern bool eidd; // Use event indicator directional derivatives?
extern double rTol; // Relative tolerance
extern double aTol; // Absolute tolerance
extern double aFac; // Absolute tolerance factor
extern double zTol; // Zero-crossing tolerance
extern double zFac; // Zero-crossing tolerance factor
extern double dtMin; // Min time step (s)
extern double dtMax; // Max time step (s)
extern double dtInf; // Inf time step (s)
extern double dtZC; // FMU zero-crossing time step (s)
extern double dtND; // Numeric differentiation time step (s)
extern double two_dtND; // 2 * dtND
extern double one_over_dtND; // 1 / dtND
extern double one_over_two_dtND; // 1 / ( 2 * dtND )
extern double one_over_two_dtND_squared; // 1 / ( 2 * dtND^2 )
extern double one_over_four_dtND; // 1 / ( 4 * dtND )
extern double one_over_six_dtND; // 1 / ( 6 * dtND )
extern double one_over_six_dtND_squared; // 1 / ( 6 * dtND^2 )
extern double one_over_six_dtND_cubed; // 1 / ( 6 * dtND^3 )
extern double dtCon; // FMU connection sync time step (s)
extern double dtOut; // Sampled output time step (s)
extern double tEnd; // End time (s)
extern std::size_t bin_size; // Bin size max
extern double bin_frac; // Bin step fraction min
extern bool bin_auto; // Bin size automaically optimized?
extern std::size_t pass; // Pass count limit
extern bool cycles; // Report dependency cycles?
extern bool inflection; // Requantize at inflections?
extern bool refine; // Refine FMU zero-crossing roots?
extern bool prune; // Prune variables with no observers?
extern bool perfect; // Perfect FMU-ME connection sync?
extern bool steps; // Generate requantization step count file?
extern LogLevel log; // Logging level
extern InpFxn fxn; // Map from input variables to function specs
extern InpOut con; // Map from input variables to output variables
extern Depends dep; // Dependencies
extern Depends rdep; // Reverse dependencies
extern std::string out; // Outputs: r, a, s, x, q, f
extern std::pair< double, double > tLoc; // Local output time range (s)
extern std::string var; // Variable output filter file
extern Models models; // Name of model(s) or FMU(s)

namespace specified {

extern bool qss; // QSS method specified?
extern bool rTol; // Relative tolerance specified?
extern bool aTol; // Absolute tolerance specified?
extern bool zTol; // Zero-crossing tolerance specified?
extern bool dtZC; // FMU zero-crossing time step specified?
extern bool dtOut; // Sampled output time step specified?
extern bool tEnd; // End time specified?
extern bool tLoc; // Local output time range specified?
extern bool bin; // Bin controls specified?

} // specified

namespace output { // Output selections

extern bool d; // Diagnostics?
extern bool s; // Statistics?
extern bool R; // Requantizations?
extern bool O; // Observer updates?
extern bool Z; // Zero-crossings?
extern bool D; // Discrete events?
extern bool S; // Sampled?
extern bool X; // Continuous trajectories?
extern bool Q; // Quantized trajectories?
extern bool A; // All variables?
extern bool F; // FMU output variables?
extern bool L; // FMU local variables?
extern bool K; // FMU-QSS smooth tokens?

} // output

// Process command line arguments
void
process_args( int argc, char * argv[] );

// Multiple models?
bool
have_multiple_models();

// Input-output connections?
bool
connected();

// Set dtOut to Default for a Given Time Span
void
dtOut_set( double const t );

} // options
} // QSS

#endif
