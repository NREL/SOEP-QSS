// Observers Example
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

// QSS Headers
#include <QSS/cod/mdl/observers.hh>
#include <QSS/cod/mdl/Function_LTI.hh>
#include <QSS/cod/Variable_all.hh>
#include <QSS/options.hh>

namespace QSS {
namespace cod {
namespace mdl {

// Observers Example Setup
void
observers( Variable::Variables & vars )
{
	using namespace options;

	// Timing
	if ( ! options::specified::tEnd ) options::tEnd = 100.0;

	// Variables
	using V = Variable_QSS< Function_LTI >;
	V * x001( nullptr );
	V * x002( nullptr );
	V * x003( nullptr );
	V * x004( nullptr );
	V * x005( nullptr );
	V * x006( nullptr );
	V * x007( nullptr );
	V * x008( nullptr );
	V * x009( nullptr );
	V * x010( nullptr );
	V * x011( nullptr );
	V * x012( nullptr );
	V * x013( nullptr );
	V * x014( nullptr );
	V * x015( nullptr );
	V * x016( nullptr );
	V * x017( nullptr );
	V * x018( nullptr );
	V * x019( nullptr );
	V * x020( nullptr );
	V * x021( nullptr );
	V * x022( nullptr );
	V * x023( nullptr );
	V * x024( nullptr );
	V * x025( nullptr );
	vars.clear();
	vars.reserve( 25 );
	if ( qss == QSS::QSS1 ) {
		vars.push_back( x001 = new Variable_QSS1< Function_LTI >( "x001", rTol, aTol, 001.0 ) );
		vars.push_back( x002 = new Variable_QSS1< Function_LTI >( "x002", rTol, aTol, 002.0 ) );
		vars.push_back( x003 = new Variable_QSS1< Function_LTI >( "x003", rTol, aTol, 003.0 ) );
		vars.push_back( x004 = new Variable_QSS1< Function_LTI >( "x004", rTol, aTol, 004.0 ) );
		vars.push_back( x005 = new Variable_QSS1< Function_LTI >( "x005", rTol, aTol, 005.0 ) );
		vars.push_back( x006 = new Variable_QSS1< Function_LTI >( "x006", rTol, aTol, 006.0 ) );
		vars.push_back( x007 = new Variable_QSS1< Function_LTI >( "x007", rTol, aTol, 007.0 ) );
		vars.push_back( x008 = new Variable_QSS1< Function_LTI >( "x008", rTol, aTol, 008.0 ) );
		vars.push_back( x009 = new Variable_QSS1< Function_LTI >( "x009", rTol, aTol, 009.0 ) );
		vars.push_back( x010 = new Variable_QSS1< Function_LTI >( "x010", rTol, aTol, 010.0 ) );
		vars.push_back( x011 = new Variable_QSS1< Function_LTI >( "x011", rTol, aTol, 011.0 ) );
		vars.push_back( x012 = new Variable_QSS1< Function_LTI >( "x012", rTol, aTol, 012.0 ) );
		vars.push_back( x013 = new Variable_QSS1< Function_LTI >( "x013", rTol, aTol, 013.0 ) );
		vars.push_back( x014 = new Variable_QSS1< Function_LTI >( "x014", rTol, aTol, 014.0 ) );
		vars.push_back( x015 = new Variable_QSS1< Function_LTI >( "x015", rTol, aTol, 015.0 ) );
		vars.push_back( x016 = new Variable_QSS1< Function_LTI >( "x016", rTol, aTol, 016.0 ) );
		vars.push_back( x017 = new Variable_QSS1< Function_LTI >( "x017", rTol, aTol, 017.0 ) );
		vars.push_back( x018 = new Variable_QSS1< Function_LTI >( "x018", rTol, aTol, 018.0 ) );
		vars.push_back( x019 = new Variable_QSS1< Function_LTI >( "x019", rTol, aTol, 019.0 ) );
		vars.push_back( x020 = new Variable_QSS1< Function_LTI >( "x020", rTol, aTol, 020.0 ) );
		vars.push_back( x021 = new Variable_QSS1< Function_LTI >( "x021", rTol, aTol, 021.0 ) );
		vars.push_back( x022 = new Variable_QSS1< Function_LTI >( "x022", rTol, aTol, 022.0 ) );
		vars.push_back( x023 = new Variable_QSS1< Function_LTI >( "x023", rTol, aTol, 023.0 ) );
		vars.push_back( x024 = new Variable_QSS1< Function_LTI >( "x024", rTol, aTol, 024.0 ) );
		vars.push_back( x025 = new Variable_QSS1< Function_LTI >( "x025", rTol, aTol, 025.0 ) );
	} else if ( qss == QSS::QSS2 ) {
		vars.push_back( x001 = new Variable_QSS2< Function_LTI >( "x001", rTol, aTol, 001.0 ) );
		vars.push_back( x002 = new Variable_QSS2< Function_LTI >( "x002", rTol, aTol, 002.0 ) );
		vars.push_back( x003 = new Variable_QSS2< Function_LTI >( "x003", rTol, aTol, 003.0 ) );
		vars.push_back( x004 = new Variable_QSS2< Function_LTI >( "x004", rTol, aTol, 004.0 ) );
		vars.push_back( x005 = new Variable_QSS2< Function_LTI >( "x005", rTol, aTol, 005.0 ) );
		vars.push_back( x006 = new Variable_QSS2< Function_LTI >( "x006", rTol, aTol, 006.0 ) );
		vars.push_back( x007 = new Variable_QSS2< Function_LTI >( "x007", rTol, aTol, 007.0 ) );
		vars.push_back( x008 = new Variable_QSS2< Function_LTI >( "x008", rTol, aTol, 008.0 ) );
		vars.push_back( x009 = new Variable_QSS2< Function_LTI >( "x009", rTol, aTol, 009.0 ) );
		vars.push_back( x010 = new Variable_QSS2< Function_LTI >( "x010", rTol, aTol, 010.0 ) );
		vars.push_back( x011 = new Variable_QSS2< Function_LTI >( "x011", rTol, aTol, 011.0 ) );
		vars.push_back( x012 = new Variable_QSS2< Function_LTI >( "x012", rTol, aTol, 012.0 ) );
		vars.push_back( x013 = new Variable_QSS2< Function_LTI >( "x013", rTol, aTol, 013.0 ) );
		vars.push_back( x014 = new Variable_QSS2< Function_LTI >( "x014", rTol, aTol, 014.0 ) );
		vars.push_back( x015 = new Variable_QSS2< Function_LTI >( "x015", rTol, aTol, 015.0 ) );
		vars.push_back( x016 = new Variable_QSS2< Function_LTI >( "x016", rTol, aTol, 016.0 ) );
		vars.push_back( x017 = new Variable_QSS2< Function_LTI >( "x017", rTol, aTol, 017.0 ) );
		vars.push_back( x018 = new Variable_QSS2< Function_LTI >( "x018", rTol, aTol, 018.0 ) );
		vars.push_back( x019 = new Variable_QSS2< Function_LTI >( "x019", rTol, aTol, 019.0 ) );
		vars.push_back( x020 = new Variable_QSS2< Function_LTI >( "x020", rTol, aTol, 020.0 ) );
		vars.push_back( x021 = new Variable_QSS2< Function_LTI >( "x021", rTol, aTol, 021.0 ) );
		vars.push_back( x022 = new Variable_QSS2< Function_LTI >( "x022", rTol, aTol, 022.0 ) );
		vars.push_back( x023 = new Variable_QSS2< Function_LTI >( "x023", rTol, aTol, 023.0 ) );
		vars.push_back( x024 = new Variable_QSS2< Function_LTI >( "x024", rTol, aTol, 024.0 ) );
		vars.push_back( x025 = new Variable_QSS2< Function_LTI >( "x025", rTol, aTol, 025.0 ) );
	} else if ( qss == QSS::QSS3 ) {
		vars.push_back( x001 = new Variable_QSS3< Function_LTI >( "x001", rTol, aTol, 001.0 ) );
		vars.push_back( x002 = new Variable_QSS3< Function_LTI >( "x002", rTol, aTol, 002.0 ) );
		vars.push_back( x003 = new Variable_QSS3< Function_LTI >( "x003", rTol, aTol, 003.0 ) );
		vars.push_back( x004 = new Variable_QSS3< Function_LTI >( "x004", rTol, aTol, 004.0 ) );
		vars.push_back( x005 = new Variable_QSS3< Function_LTI >( "x005", rTol, aTol, 005.0 ) );
		vars.push_back( x006 = new Variable_QSS3< Function_LTI >( "x006", rTol, aTol, 006.0 ) );
		vars.push_back( x007 = new Variable_QSS3< Function_LTI >( "x007", rTol, aTol, 007.0 ) );
		vars.push_back( x008 = new Variable_QSS3< Function_LTI >( "x008", rTol, aTol, 008.0 ) );
		vars.push_back( x009 = new Variable_QSS3< Function_LTI >( "x009", rTol, aTol, 009.0 ) );
		vars.push_back( x010 = new Variable_QSS3< Function_LTI >( "x010", rTol, aTol, 010.0 ) );
		vars.push_back( x011 = new Variable_QSS3< Function_LTI >( "x011", rTol, aTol, 011.0 ) );
		vars.push_back( x012 = new Variable_QSS3< Function_LTI >( "x012", rTol, aTol, 012.0 ) );
		vars.push_back( x013 = new Variable_QSS3< Function_LTI >( "x013", rTol, aTol, 013.0 ) );
		vars.push_back( x014 = new Variable_QSS3< Function_LTI >( "x014", rTol, aTol, 014.0 ) );
		vars.push_back( x015 = new Variable_QSS3< Function_LTI >( "x015", rTol, aTol, 015.0 ) );
		vars.push_back( x016 = new Variable_QSS3< Function_LTI >( "x016", rTol, aTol, 016.0 ) );
		vars.push_back( x017 = new Variable_QSS3< Function_LTI >( "x017", rTol, aTol, 017.0 ) );
		vars.push_back( x018 = new Variable_QSS3< Function_LTI >( "x018", rTol, aTol, 018.0 ) );
		vars.push_back( x019 = new Variable_QSS3< Function_LTI >( "x019", rTol, aTol, 019.0 ) );
		vars.push_back( x020 = new Variable_QSS3< Function_LTI >( "x020", rTol, aTol, 020.0 ) );
		vars.push_back( x021 = new Variable_QSS3< Function_LTI >( "x021", rTol, aTol, 021.0 ) );
		vars.push_back( x022 = new Variable_QSS3< Function_LTI >( "x022", rTol, aTol, 022.0 ) );
		vars.push_back( x023 = new Variable_QSS3< Function_LTI >( "x023", rTol, aTol, 023.0 ) );
		vars.push_back( x024 = new Variable_QSS3< Function_LTI >( "x024", rTol, aTol, 024.0 ) );
		vars.push_back( x025 = new Variable_QSS3< Function_LTI >( "x025", rTol, aTol, 025.0 ) );
	} else if ( qss == QSS::LIQSS1 ) {
		vars.push_back( x001 = new Variable_LIQSS1< Function_LTI >( "x001", rTol, aTol, 001.0 ) );
		vars.push_back( x002 = new Variable_LIQSS1< Function_LTI >( "x002", rTol, aTol, 002.0 ) );
		vars.push_back( x003 = new Variable_LIQSS1< Function_LTI >( "x003", rTol, aTol, 003.0 ) );
		vars.push_back( x004 = new Variable_LIQSS1< Function_LTI >( "x004", rTol, aTol, 004.0 ) );
		vars.push_back( x005 = new Variable_LIQSS1< Function_LTI >( "x005", rTol, aTol, 005.0 ) );
		vars.push_back( x006 = new Variable_LIQSS1< Function_LTI >( "x006", rTol, aTol, 006.0 ) );
		vars.push_back( x007 = new Variable_LIQSS1< Function_LTI >( "x007", rTol, aTol, 007.0 ) );
		vars.push_back( x008 = new Variable_LIQSS1< Function_LTI >( "x008", rTol, aTol, 008.0 ) );
		vars.push_back( x009 = new Variable_LIQSS1< Function_LTI >( "x009", rTol, aTol, 009.0 ) );
		vars.push_back( x010 = new Variable_LIQSS1< Function_LTI >( "x010", rTol, aTol, 010.0 ) );
		vars.push_back( x011 = new Variable_LIQSS1< Function_LTI >( "x011", rTol, aTol, 011.0 ) );
		vars.push_back( x012 = new Variable_LIQSS1< Function_LTI >( "x012", rTol, aTol, 012.0 ) );
		vars.push_back( x013 = new Variable_LIQSS1< Function_LTI >( "x013", rTol, aTol, 013.0 ) );
		vars.push_back( x014 = new Variable_LIQSS1< Function_LTI >( "x014", rTol, aTol, 014.0 ) );
		vars.push_back( x015 = new Variable_LIQSS1< Function_LTI >( "x015", rTol, aTol, 015.0 ) );
		vars.push_back( x016 = new Variable_LIQSS1< Function_LTI >( "x016", rTol, aTol, 016.0 ) );
		vars.push_back( x017 = new Variable_LIQSS1< Function_LTI >( "x017", rTol, aTol, 017.0 ) );
		vars.push_back( x018 = new Variable_LIQSS1< Function_LTI >( "x018", rTol, aTol, 018.0 ) );
		vars.push_back( x019 = new Variable_LIQSS1< Function_LTI >( "x019", rTol, aTol, 019.0 ) );
		vars.push_back( x020 = new Variable_LIQSS1< Function_LTI >( "x020", rTol, aTol, 020.0 ) );
		vars.push_back( x021 = new Variable_LIQSS1< Function_LTI >( "x021", rTol, aTol, 021.0 ) );
		vars.push_back( x022 = new Variable_LIQSS1< Function_LTI >( "x022", rTol, aTol, 022.0 ) );
		vars.push_back( x023 = new Variable_LIQSS1< Function_LTI >( "x023", rTol, aTol, 023.0 ) );
		vars.push_back( x024 = new Variable_LIQSS1< Function_LTI >( "x024", rTol, aTol, 024.0 ) );
		vars.push_back( x025 = new Variable_LIQSS1< Function_LTI >( "x025", rTol, aTol, 025.0 ) );
	} else if ( qss == QSS::LIQSS2 ) {
		vars.push_back( x001 = new Variable_LIQSS2< Function_LTI >( "x001", rTol, aTol, 001.0 ) );
		vars.push_back( x002 = new Variable_LIQSS2< Function_LTI >( "x002", rTol, aTol, 002.0 ) );
		vars.push_back( x003 = new Variable_LIQSS2< Function_LTI >( "x003", rTol, aTol, 003.0 ) );
		vars.push_back( x004 = new Variable_LIQSS2< Function_LTI >( "x004", rTol, aTol, 004.0 ) );
		vars.push_back( x005 = new Variable_LIQSS2< Function_LTI >( "x005", rTol, aTol, 005.0 ) );
		vars.push_back( x006 = new Variable_LIQSS2< Function_LTI >( "x006", rTol, aTol, 006.0 ) );
		vars.push_back( x007 = new Variable_LIQSS2< Function_LTI >( "x007", rTol, aTol, 007.0 ) );
		vars.push_back( x008 = new Variable_LIQSS2< Function_LTI >( "x008", rTol, aTol, 008.0 ) );
		vars.push_back( x009 = new Variable_LIQSS2< Function_LTI >( "x009", rTol, aTol, 009.0 ) );
		vars.push_back( x010 = new Variable_LIQSS2< Function_LTI >( "x010", rTol, aTol, 010.0 ) );
		vars.push_back( x011 = new Variable_LIQSS2< Function_LTI >( "x011", rTol, aTol, 011.0 ) );
		vars.push_back( x012 = new Variable_LIQSS2< Function_LTI >( "x012", rTol, aTol, 012.0 ) );
		vars.push_back( x013 = new Variable_LIQSS2< Function_LTI >( "x013", rTol, aTol, 013.0 ) );
		vars.push_back( x014 = new Variable_LIQSS2< Function_LTI >( "x014", rTol, aTol, 014.0 ) );
		vars.push_back( x015 = new Variable_LIQSS2< Function_LTI >( "x015", rTol, aTol, 015.0 ) );
		vars.push_back( x016 = new Variable_LIQSS2< Function_LTI >( "x016", rTol, aTol, 016.0 ) );
		vars.push_back( x017 = new Variable_LIQSS2< Function_LTI >( "x017", rTol, aTol, 017.0 ) );
		vars.push_back( x018 = new Variable_LIQSS2< Function_LTI >( "x018", rTol, aTol, 018.0 ) );
		vars.push_back( x019 = new Variable_LIQSS2< Function_LTI >( "x019", rTol, aTol, 019.0 ) );
		vars.push_back( x020 = new Variable_LIQSS2< Function_LTI >( "x020", rTol, aTol, 020.0 ) );
		vars.push_back( x021 = new Variable_LIQSS2< Function_LTI >( "x021", rTol, aTol, 021.0 ) );
		vars.push_back( x022 = new Variable_LIQSS2< Function_LTI >( "x022", rTol, aTol, 022.0 ) );
		vars.push_back( x023 = new Variable_LIQSS2< Function_LTI >( "x023", rTol, aTol, 023.0 ) );
		vars.push_back( x024 = new Variable_LIQSS2< Function_LTI >( "x024", rTol, aTol, 024.0 ) );
		vars.push_back( x025 = new Variable_LIQSS2< Function_LTI >( "x025", rTol, aTol, 025.0 ) );
	} else if ( qss == QSS::LIQSS3 ) {
		vars.push_back( x001 = new Variable_LIQSS3< Function_LTI >( "x001", rTol, aTol, 001.0 ) );
		vars.push_back( x002 = new Variable_LIQSS3< Function_LTI >( "x002", rTol, aTol, 002.0 ) );
		vars.push_back( x003 = new Variable_LIQSS3< Function_LTI >( "x003", rTol, aTol, 003.0 ) );
		vars.push_back( x004 = new Variable_LIQSS3< Function_LTI >( "x004", rTol, aTol, 004.0 ) );
		vars.push_back( x005 = new Variable_LIQSS3< Function_LTI >( "x005", rTol, aTol, 005.0 ) );
		vars.push_back( x006 = new Variable_LIQSS3< Function_LTI >( "x006", rTol, aTol, 006.0 ) );
		vars.push_back( x007 = new Variable_LIQSS3< Function_LTI >( "x007", rTol, aTol, 007.0 ) );
		vars.push_back( x008 = new Variable_LIQSS3< Function_LTI >( "x008", rTol, aTol, 008.0 ) );
		vars.push_back( x009 = new Variable_LIQSS3< Function_LTI >( "x009", rTol, aTol, 009.0 ) );
		vars.push_back( x010 = new Variable_LIQSS3< Function_LTI >( "x010", rTol, aTol, 010.0 ) );
		vars.push_back( x011 = new Variable_LIQSS3< Function_LTI >( "x011", rTol, aTol, 011.0 ) );
		vars.push_back( x012 = new Variable_LIQSS3< Function_LTI >( "x012", rTol, aTol, 012.0 ) );
		vars.push_back( x013 = new Variable_LIQSS3< Function_LTI >( "x013", rTol, aTol, 013.0 ) );
		vars.push_back( x014 = new Variable_LIQSS3< Function_LTI >( "x014", rTol, aTol, 014.0 ) );
		vars.push_back( x015 = new Variable_LIQSS3< Function_LTI >( "x015", rTol, aTol, 015.0 ) );
		vars.push_back( x016 = new Variable_LIQSS3< Function_LTI >( "x016", rTol, aTol, 016.0 ) );
		vars.push_back( x017 = new Variable_LIQSS3< Function_LTI >( "x017", rTol, aTol, 017.0 ) );
		vars.push_back( x018 = new Variable_LIQSS3< Function_LTI >( "x018", rTol, aTol, 018.0 ) );
		vars.push_back( x019 = new Variable_LIQSS3< Function_LTI >( "x019", rTol, aTol, 019.0 ) );
		vars.push_back( x020 = new Variable_LIQSS3< Function_LTI >( "x020", rTol, aTol, 020.0 ) );
		vars.push_back( x021 = new Variable_LIQSS3< Function_LTI >( "x021", rTol, aTol, 021.0 ) );
		vars.push_back( x022 = new Variable_LIQSS3< Function_LTI >( "x022", rTol, aTol, 022.0 ) );
		vars.push_back( x023 = new Variable_LIQSS3< Function_LTI >( "x023", rTol, aTol, 023.0 ) );
		vars.push_back( x024 = new Variable_LIQSS3< Function_LTI >( "x024", rTol, aTol, 024.0 ) );
		vars.push_back( x025 = new Variable_LIQSS3< Function_LTI >( "x025", rTol, aTol, 025.0 ) );
	} else if ( qss == QSS::xQSS1 ) {
		vars.push_back( x001 = new Variable_xQSS1< Function_LTI >( "x001", rTol, aTol, 001.0 ) );
		vars.push_back( x002 = new Variable_xQSS1< Function_LTI >( "x002", rTol, aTol, 002.0 ) );
		vars.push_back( x003 = new Variable_xQSS1< Function_LTI >( "x003", rTol, aTol, 003.0 ) );
		vars.push_back( x004 = new Variable_xQSS1< Function_LTI >( "x004", rTol, aTol, 004.0 ) );
		vars.push_back( x005 = new Variable_xQSS1< Function_LTI >( "x005", rTol, aTol, 005.0 ) );
		vars.push_back( x006 = new Variable_xQSS1< Function_LTI >( "x006", rTol, aTol, 006.0 ) );
		vars.push_back( x007 = new Variable_xQSS1< Function_LTI >( "x007", rTol, aTol, 007.0 ) );
		vars.push_back( x008 = new Variable_xQSS1< Function_LTI >( "x008", rTol, aTol, 008.0 ) );
		vars.push_back( x009 = new Variable_xQSS1< Function_LTI >( "x009", rTol, aTol, 009.0 ) );
		vars.push_back( x010 = new Variable_xQSS1< Function_LTI >( "x010", rTol, aTol, 010.0 ) );
		vars.push_back( x011 = new Variable_xQSS1< Function_LTI >( "x011", rTol, aTol, 011.0 ) );
		vars.push_back( x012 = new Variable_xQSS1< Function_LTI >( "x012", rTol, aTol, 012.0 ) );
		vars.push_back( x013 = new Variable_xQSS1< Function_LTI >( "x013", rTol, aTol, 013.0 ) );
		vars.push_back( x014 = new Variable_xQSS1< Function_LTI >( "x014", rTol, aTol, 014.0 ) );
		vars.push_back( x015 = new Variable_xQSS1< Function_LTI >( "x015", rTol, aTol, 015.0 ) );
		vars.push_back( x016 = new Variable_xQSS1< Function_LTI >( "x016", rTol, aTol, 016.0 ) );
		vars.push_back( x017 = new Variable_xQSS1< Function_LTI >( "x017", rTol, aTol, 017.0 ) );
		vars.push_back( x018 = new Variable_xQSS1< Function_LTI >( "x018", rTol, aTol, 018.0 ) );
		vars.push_back( x019 = new Variable_xQSS1< Function_LTI >( "x019", rTol, aTol, 019.0 ) );
		vars.push_back( x020 = new Variable_xQSS1< Function_LTI >( "x020", rTol, aTol, 020.0 ) );
		vars.push_back( x021 = new Variable_xQSS1< Function_LTI >( "x021", rTol, aTol, 021.0 ) );
		vars.push_back( x022 = new Variable_xQSS1< Function_LTI >( "x022", rTol, aTol, 022.0 ) );
		vars.push_back( x023 = new Variable_xQSS1< Function_LTI >( "x023", rTol, aTol, 023.0 ) );
		vars.push_back( x024 = new Variable_xQSS1< Function_LTI >( "x024", rTol, aTol, 024.0 ) );
		vars.push_back( x025 = new Variable_xQSS1< Function_LTI >( "x025", rTol, aTol, 025.0 ) );
	} else if ( qss == QSS::xQSS2 ) {
		vars.push_back( x001 = new Variable_xQSS2< Function_LTI >( "x001", rTol, aTol, 001.0 ) );
		vars.push_back( x002 = new Variable_xQSS2< Function_LTI >( "x002", rTol, aTol, 002.0 ) );
		vars.push_back( x003 = new Variable_xQSS2< Function_LTI >( "x003", rTol, aTol, 003.0 ) );
		vars.push_back( x004 = new Variable_xQSS2< Function_LTI >( "x004", rTol, aTol, 004.0 ) );
		vars.push_back( x005 = new Variable_xQSS2< Function_LTI >( "x005", rTol, aTol, 005.0 ) );
		vars.push_back( x006 = new Variable_xQSS2< Function_LTI >( "x006", rTol, aTol, 006.0 ) );
		vars.push_back( x007 = new Variable_xQSS2< Function_LTI >( "x007", rTol, aTol, 007.0 ) );
		vars.push_back( x008 = new Variable_xQSS2< Function_LTI >( "x008", rTol, aTol, 008.0 ) );
		vars.push_back( x009 = new Variable_xQSS2< Function_LTI >( "x009", rTol, aTol, 009.0 ) );
		vars.push_back( x010 = new Variable_xQSS2< Function_LTI >( "x010", rTol, aTol, 010.0 ) );
		vars.push_back( x011 = new Variable_xQSS2< Function_LTI >( "x011", rTol, aTol, 011.0 ) );
		vars.push_back( x012 = new Variable_xQSS2< Function_LTI >( "x012", rTol, aTol, 012.0 ) );
		vars.push_back( x013 = new Variable_xQSS2< Function_LTI >( "x013", rTol, aTol, 013.0 ) );
		vars.push_back( x014 = new Variable_xQSS2< Function_LTI >( "x014", rTol, aTol, 014.0 ) );
		vars.push_back( x015 = new Variable_xQSS2< Function_LTI >( "x015", rTol, aTol, 015.0 ) );
		vars.push_back( x016 = new Variable_xQSS2< Function_LTI >( "x016", rTol, aTol, 016.0 ) );
		vars.push_back( x017 = new Variable_xQSS2< Function_LTI >( "x017", rTol, aTol, 017.0 ) );
		vars.push_back( x018 = new Variable_xQSS2< Function_LTI >( "x018", rTol, aTol, 018.0 ) );
		vars.push_back( x019 = new Variable_xQSS2< Function_LTI >( "x019", rTol, aTol, 019.0 ) );
		vars.push_back( x020 = new Variable_xQSS2< Function_LTI >( "x020", rTol, aTol, 020.0 ) );
		vars.push_back( x021 = new Variable_xQSS2< Function_LTI >( "x021", rTol, aTol, 021.0 ) );
		vars.push_back( x022 = new Variable_xQSS2< Function_LTI >( "x022", rTol, aTol, 022.0 ) );
		vars.push_back( x023 = new Variable_xQSS2< Function_LTI >( "x023", rTol, aTol, 023.0 ) );
		vars.push_back( x024 = new Variable_xQSS2< Function_LTI >( "x024", rTol, aTol, 024.0 ) );
		vars.push_back( x025 = new Variable_xQSS2< Function_LTI >( "x025", rTol, aTol, 025.0 ) );
	} else if ( qss == QSS::xQSS3 ) {
		vars.push_back( x001 = new Variable_xQSS3< Function_LTI >( "x001", rTol, aTol, 001.0 ) );
		vars.push_back( x002 = new Variable_xQSS3< Function_LTI >( "x002", rTol, aTol, 002.0 ) );
		vars.push_back( x003 = new Variable_xQSS3< Function_LTI >( "x003", rTol, aTol, 003.0 ) );
		vars.push_back( x004 = new Variable_xQSS3< Function_LTI >( "x004", rTol, aTol, 004.0 ) );
		vars.push_back( x005 = new Variable_xQSS3< Function_LTI >( "x005", rTol, aTol, 005.0 ) );
		vars.push_back( x006 = new Variable_xQSS3< Function_LTI >( "x006", rTol, aTol, 006.0 ) );
		vars.push_back( x007 = new Variable_xQSS3< Function_LTI >( "x007", rTol, aTol, 007.0 ) );
		vars.push_back( x008 = new Variable_xQSS3< Function_LTI >( "x008", rTol, aTol, 008.0 ) );
		vars.push_back( x009 = new Variable_xQSS3< Function_LTI >( "x009", rTol, aTol, 009.0 ) );
		vars.push_back( x010 = new Variable_xQSS3< Function_LTI >( "x010", rTol, aTol, 010.0 ) );
		vars.push_back( x011 = new Variable_xQSS3< Function_LTI >( "x011", rTol, aTol, 011.0 ) );
		vars.push_back( x012 = new Variable_xQSS3< Function_LTI >( "x012", rTol, aTol, 012.0 ) );
		vars.push_back( x013 = new Variable_xQSS3< Function_LTI >( "x013", rTol, aTol, 013.0 ) );
		vars.push_back( x014 = new Variable_xQSS3< Function_LTI >( "x014", rTol, aTol, 014.0 ) );
		vars.push_back( x015 = new Variable_xQSS3< Function_LTI >( "x015", rTol, aTol, 015.0 ) );
		vars.push_back( x016 = new Variable_xQSS3< Function_LTI >( "x016", rTol, aTol, 016.0 ) );
		vars.push_back( x017 = new Variable_xQSS3< Function_LTI >( "x017", rTol, aTol, 017.0 ) );
		vars.push_back( x018 = new Variable_xQSS3< Function_LTI >( "x018", rTol, aTol, 018.0 ) );
		vars.push_back( x019 = new Variable_xQSS3< Function_LTI >( "x019", rTol, aTol, 019.0 ) );
		vars.push_back( x020 = new Variable_xQSS3< Function_LTI >( "x020", rTol, aTol, 020.0 ) );
		vars.push_back( x021 = new Variable_xQSS3< Function_LTI >( "x021", rTol, aTol, 021.0 ) );
		vars.push_back( x022 = new Variable_xQSS3< Function_LTI >( "x022", rTol, aTol, 022.0 ) );
		vars.push_back( x023 = new Variable_xQSS3< Function_LTI >( "x023", rTol, aTol, 023.0 ) );
		vars.push_back( x024 = new Variable_xQSS3< Function_LTI >( "x024", rTol, aTol, 024.0 ) );
		vars.push_back( x025 = new Variable_xQSS3< Function_LTI >( "x025", rTol, aTol, 025.0 ) );
	} else if ( qss == QSS::xLIQSS1 ) {
		vars.push_back( x001 = new Variable_xLIQSS1< Function_LTI >( "x001", rTol, aTol, 001.0 ) );
		vars.push_back( x002 = new Variable_xLIQSS1< Function_LTI >( "x002", rTol, aTol, 002.0 ) );
		vars.push_back( x003 = new Variable_xLIQSS1< Function_LTI >( "x003", rTol, aTol, 003.0 ) );
		vars.push_back( x004 = new Variable_xLIQSS1< Function_LTI >( "x004", rTol, aTol, 004.0 ) );
		vars.push_back( x005 = new Variable_xLIQSS1< Function_LTI >( "x005", rTol, aTol, 005.0 ) );
		vars.push_back( x006 = new Variable_xLIQSS1< Function_LTI >( "x006", rTol, aTol, 006.0 ) );
		vars.push_back( x007 = new Variable_xLIQSS1< Function_LTI >( "x007", rTol, aTol, 007.0 ) );
		vars.push_back( x008 = new Variable_xLIQSS1< Function_LTI >( "x008", rTol, aTol, 008.0 ) );
		vars.push_back( x009 = new Variable_xLIQSS1< Function_LTI >( "x009", rTol, aTol, 009.0 ) );
		vars.push_back( x010 = new Variable_xLIQSS1< Function_LTI >( "x010", rTol, aTol, 010.0 ) );
		vars.push_back( x011 = new Variable_xLIQSS1< Function_LTI >( "x011", rTol, aTol, 011.0 ) );
		vars.push_back( x012 = new Variable_xLIQSS1< Function_LTI >( "x012", rTol, aTol, 012.0 ) );
		vars.push_back( x013 = new Variable_xLIQSS1< Function_LTI >( "x013", rTol, aTol, 013.0 ) );
		vars.push_back( x014 = new Variable_xLIQSS1< Function_LTI >( "x014", rTol, aTol, 014.0 ) );
		vars.push_back( x015 = new Variable_xLIQSS1< Function_LTI >( "x015", rTol, aTol, 015.0 ) );
		vars.push_back( x016 = new Variable_xLIQSS1< Function_LTI >( "x016", rTol, aTol, 016.0 ) );
		vars.push_back( x017 = new Variable_xLIQSS1< Function_LTI >( "x017", rTol, aTol, 017.0 ) );
		vars.push_back( x018 = new Variable_xLIQSS1< Function_LTI >( "x018", rTol, aTol, 018.0 ) );
		vars.push_back( x019 = new Variable_xLIQSS1< Function_LTI >( "x019", rTol, aTol, 019.0 ) );
		vars.push_back( x020 = new Variable_xLIQSS1< Function_LTI >( "x020", rTol, aTol, 020.0 ) );
		vars.push_back( x021 = new Variable_xLIQSS1< Function_LTI >( "x021", rTol, aTol, 021.0 ) );
		vars.push_back( x022 = new Variable_xLIQSS1< Function_LTI >( "x022", rTol, aTol, 022.0 ) );
		vars.push_back( x023 = new Variable_xLIQSS1< Function_LTI >( "x023", rTol, aTol, 023.0 ) );
		vars.push_back( x024 = new Variable_xLIQSS1< Function_LTI >( "x024", rTol, aTol, 024.0 ) );
		vars.push_back( x025 = new Variable_xLIQSS1< Function_LTI >( "x025", rTol, aTol, 025.0 ) );
	} else if ( qss == QSS::xLIQSS2 ) {
		vars.push_back( x001 = new Variable_xLIQSS2< Function_LTI >( "x001", rTol, aTol, 001.0 ) );
		vars.push_back( x002 = new Variable_xLIQSS2< Function_LTI >( "x002", rTol, aTol, 002.0 ) );
		vars.push_back( x003 = new Variable_xLIQSS2< Function_LTI >( "x003", rTol, aTol, 003.0 ) );
		vars.push_back( x004 = new Variable_xLIQSS2< Function_LTI >( "x004", rTol, aTol, 004.0 ) );
		vars.push_back( x005 = new Variable_xLIQSS2< Function_LTI >( "x005", rTol, aTol, 005.0 ) );
		vars.push_back( x006 = new Variable_xLIQSS2< Function_LTI >( "x006", rTol, aTol, 006.0 ) );
		vars.push_back( x007 = new Variable_xLIQSS2< Function_LTI >( "x007", rTol, aTol, 007.0 ) );
		vars.push_back( x008 = new Variable_xLIQSS2< Function_LTI >( "x008", rTol, aTol, 008.0 ) );
		vars.push_back( x009 = new Variable_xLIQSS2< Function_LTI >( "x009", rTol, aTol, 009.0 ) );
		vars.push_back( x010 = new Variable_xLIQSS2< Function_LTI >( "x010", rTol, aTol, 010.0 ) );
		vars.push_back( x011 = new Variable_xLIQSS2< Function_LTI >( "x011", rTol, aTol, 011.0 ) );
		vars.push_back( x012 = new Variable_xLIQSS2< Function_LTI >( "x012", rTol, aTol, 012.0 ) );
		vars.push_back( x013 = new Variable_xLIQSS2< Function_LTI >( "x013", rTol, aTol, 013.0 ) );
		vars.push_back( x014 = new Variable_xLIQSS2< Function_LTI >( "x014", rTol, aTol, 014.0 ) );
		vars.push_back( x015 = new Variable_xLIQSS2< Function_LTI >( "x015", rTol, aTol, 015.0 ) );
		vars.push_back( x016 = new Variable_xLIQSS2< Function_LTI >( "x016", rTol, aTol, 016.0 ) );
		vars.push_back( x017 = new Variable_xLIQSS2< Function_LTI >( "x017", rTol, aTol, 017.0 ) );
		vars.push_back( x018 = new Variable_xLIQSS2< Function_LTI >( "x018", rTol, aTol, 018.0 ) );
		vars.push_back( x019 = new Variable_xLIQSS2< Function_LTI >( "x019", rTol, aTol, 019.0 ) );
		vars.push_back( x020 = new Variable_xLIQSS2< Function_LTI >( "x020", rTol, aTol, 020.0 ) );
		vars.push_back( x021 = new Variable_xLIQSS2< Function_LTI >( "x021", rTol, aTol, 021.0 ) );
		vars.push_back( x022 = new Variable_xLIQSS2< Function_LTI >( "x022", rTol, aTol, 022.0 ) );
		vars.push_back( x023 = new Variable_xLIQSS2< Function_LTI >( "x023", rTol, aTol, 023.0 ) );
		vars.push_back( x024 = new Variable_xLIQSS2< Function_LTI >( "x024", rTol, aTol, 024.0 ) );
		vars.push_back( x025 = new Variable_xLIQSS2< Function_LTI >( "x025", rTol, aTol, 025.0 ) );
	} else if ( qss == QSS::xLIQSS3 ) {
		vars.push_back( x001 = new Variable_xLIQSS3< Function_LTI >( "x001", rTol, aTol, 001.0 ) );
		vars.push_back( x002 = new Variable_xLIQSS3< Function_LTI >( "x002", rTol, aTol, 002.0 ) );
		vars.push_back( x003 = new Variable_xLIQSS3< Function_LTI >( "x003", rTol, aTol, 003.0 ) );
		vars.push_back( x004 = new Variable_xLIQSS3< Function_LTI >( "x004", rTol, aTol, 004.0 ) );
		vars.push_back( x005 = new Variable_xLIQSS3< Function_LTI >( "x005", rTol, aTol, 005.0 ) );
		vars.push_back( x006 = new Variable_xLIQSS3< Function_LTI >( "x006", rTol, aTol, 006.0 ) );
		vars.push_back( x007 = new Variable_xLIQSS3< Function_LTI >( "x007", rTol, aTol, 007.0 ) );
		vars.push_back( x008 = new Variable_xLIQSS3< Function_LTI >( "x008", rTol, aTol, 008.0 ) );
		vars.push_back( x009 = new Variable_xLIQSS3< Function_LTI >( "x009", rTol, aTol, 009.0 ) );
		vars.push_back( x010 = new Variable_xLIQSS3< Function_LTI >( "x010", rTol, aTol, 010.0 ) );
		vars.push_back( x011 = new Variable_xLIQSS3< Function_LTI >( "x011", rTol, aTol, 011.0 ) );
		vars.push_back( x012 = new Variable_xLIQSS3< Function_LTI >( "x012", rTol, aTol, 012.0 ) );
		vars.push_back( x013 = new Variable_xLIQSS3< Function_LTI >( "x013", rTol, aTol, 013.0 ) );
		vars.push_back( x014 = new Variable_xLIQSS3< Function_LTI >( "x014", rTol, aTol, 014.0 ) );
		vars.push_back( x015 = new Variable_xLIQSS3< Function_LTI >( "x015", rTol, aTol, 015.0 ) );
		vars.push_back( x016 = new Variable_xLIQSS3< Function_LTI >( "x016", rTol, aTol, 016.0 ) );
		vars.push_back( x017 = new Variable_xLIQSS3< Function_LTI >( "x017", rTol, aTol, 017.0 ) );
		vars.push_back( x018 = new Variable_xLIQSS3< Function_LTI >( "x018", rTol, aTol, 018.0 ) );
		vars.push_back( x019 = new Variable_xLIQSS3< Function_LTI >( "x019", rTol, aTol, 019.0 ) );
		vars.push_back( x020 = new Variable_xLIQSS3< Function_LTI >( "x020", rTol, aTol, 020.0 ) );
		vars.push_back( x021 = new Variable_xLIQSS3< Function_LTI >( "x021", rTol, aTol, 021.0 ) );
		vars.push_back( x022 = new Variable_xLIQSS3< Function_LTI >( "x022", rTol, aTol, 022.0 ) );
		vars.push_back( x023 = new Variable_xLIQSS3< Function_LTI >( "x023", rTol, aTol, 023.0 ) );
		vars.push_back( x024 = new Variable_xLIQSS3< Function_LTI >( "x024", rTol, aTol, 024.0 ) );
		vars.push_back( x025 = new Variable_xLIQSS3< Function_LTI >( "x025", rTol, aTol, 025.0 ) );
	} else {
		std::cerr << "Error: Unsupported QSS method" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Derivatives
	x001->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x002->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x003->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x004->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x005->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x006->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x007->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x008->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x009->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x010->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x011->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x012->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x013->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x014->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x015->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x016->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x017->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x018->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x019->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x020->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x021->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x022->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x023->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x024->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
	x025->add( 0.001, x001 ).add( 0.001, x002 ).add( 0.001, x003 ).add( 0.001, x004 ).add( 0.001, x005 ).add( 0.001, x006 ).add( 0.001, x007 ).add( 0.001, x008 ).add( 0.001, x009 ).add( 0.001, x010 ).add( 0.001, x011 ).add( 0.001, x012 ).add( 0.001, x013 ).add( 0.001, x014 ).add( 0.001, x015 ).add( 0.001, x016 ).add( 0.001, x017 ).add( 0.001, x018 ).add( 0.001, x019 ).add( 0.001, x020 ).add( 0.001, x021 ).add( 0.001, x022 ).add( 0.001, x023 ).add( 0.001, x024 ).add( 0.001, x025 );
}

} // mdl
} // cod
} // QSS
