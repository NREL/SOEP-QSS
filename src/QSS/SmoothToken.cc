// SmoothToken Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2019 Objexx Engineering, Inc. All rights reserved.
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
#include <QSS/SmoothToken.hh>

// C++ Headers
#include <iomanip>
#include <ostream>
#include <sstream>

namespace QSS {

	// String Representation
	std::string
	SmoothToken::
	rep() const
	{
		std::ostringstream stream;
		stream << std::setprecision( 15 ) << x0;
		if ( order >= 1 ) {
			stream << ' ' << x1;
			if ( order >= 2 ) {
				stream << ' ' << x2;
				if ( order >= 3 ) stream << ' ' << x3;
			}
		}
		if ( tD < infinity ) stream << " ->| " << tD << " s";
		return stream.str();
	}

	// Stream << SmoothToken (For Plotting so tD Omitted)
	std::ostream &
	operator <<( std::ostream & stream, SmoothToken const & s )
	{
		stream << std::setprecision( 15 ) << s.x0;
		if ( s.order >= 1 ) {
			stream << '\t' << s.x1;
			if ( s.order >= 2 ) {
				stream << '\t' << s.x2;
				if ( s.order >= 3 ) stream << '\t' << s.x3;
			}
		}
		return stream;
	}

} // QSS
