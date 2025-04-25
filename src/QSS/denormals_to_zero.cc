// Programmatically Set Denormals to Flush to Zero
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

// QSS Headers
#include <QSS/denormals_to_zero.hh>

// C++ Headers
#ifdef _WIN32
#include <intrin.h>
#else
#include <xmmintrin.h>
#include <pmmintrin.h>
#endif

namespace QSS {

// Notes
//  Intel C++
//   Windows
//    /Od or /fp:strict retain denormals if not doing the denormals_to_zero calls
//    /O1, /O2, or /O3 or /fp:fast flushes denormals to zero
//    /fp:precise /Qftz flush denormals to zero
//   Linux
//    -O0 or -fp-model=strict retain denormals if not doing the denormals_to_zero calls
//    -O1, -O2, or =O3 or -fp-model=fast flushes denormals to zero
//    -fp-model=precise -ftz flushes denormals to zero
//  Visual C++
//   /O2 retains denormals
//   No compiler option to flush denormals
//  GCC
//   -mdaz-ftz flushes denormals to zero

// Set FTZ and DAZ bits to flush denormals to zero
void
denormals_to_zero()
{
#ifdef _WIN32
	_mm_setcsr( _mm_getcsr() | 0x8040 );
#else
	_MM_SET_FLUSH_ZERO_MODE( _MM_FLUSH_ZERO_ON );
	_MM_SET_DENORMALS_ZERO_MODE( _MM_DENORMALS_ZERO_ON );
#endif
}

} // QSS
