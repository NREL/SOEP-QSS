// CPU Time Function
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2020 Objexx Engineering, Inc. All rights reserved.
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

// C++ Headers
#ifdef _WIN32
#include <windows.h>
#else // Posix
#include <ctime>
#endif

namespace QSS {

double
get_cpu_time()
{
#ifdef _WIN32 // std::clock on Windows VC is non-compliant because it returns wall time not CPU time
	FILETIME a, b, c, d;
	if ( GetProcessTimes( GetCurrentProcess(), &a, &b, &c, &d ) != 0 ) { // OK
		return (double)( d.dwLowDateTime | ( (unsigned long long)d.dwHighDateTime << 32 ) ) * 0.0000001;
	} else { // Error
		return 0.0;
	}
#else // Posix
	return double( std::clock() ) / CLOCKS_PER_SEC; // This may wrap on some implementations

// Maybe more accurate
//	struct std::timespec ts;
//	std::clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts );
//	return ts.tv_sec + ( 1e-9 * ts.tv_nsec );

#endif
}

} // QSS
