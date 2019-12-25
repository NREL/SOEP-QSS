// FMU-Based Variable Abstract Base Class
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
#include <QSS/fmu/Variable_Con.hh>

namespace QSS {
namespace fmu {

	// Advance Connections
	void
	Variable::
	advance_connections()
	{
		for ( Variable_Con * connection : connections_ ) {
			connection->advance_connection( tQ );
		}
	}

	// Advance Connections for Observer Update
	void
	Variable::
	advance_connections_observer()
	{
		for ( Variable_Con * connection : connections_ ) {
			connection->advance_connection_observer();
		}
	}

	// Connections Output at Time t
	void
	Variable::
	connections_out( Time const t )
	{
		for ( Variable_Con * connection : connections_ ) {
			connection->out( t );
		}
	}

	// Connections Output at Time t
	void
	Variable::
	connections_out_q( Time const t )
	{
		for ( Variable_Con * connection : connections_ ) {
			connection->out_q( t );
		}
	}

	// Connections Pre-Event Observer Output at Time t
	void
	Variable::
	connections_observer_out_pre( Time const t )
	{
		for ( Variable_Con * connection : connections_ ) {
			connection->observer_out_pre( t );
		}
	}

	// Connections Post-Event Observer Output at Time t
	void
	Variable::
	connections_observer_out_post( Time const t )
	{
		for ( Variable_Con * connection : connections_ ) {
			connection->observer_out_post( t );
		}
	}

} // fmu
} // QSS