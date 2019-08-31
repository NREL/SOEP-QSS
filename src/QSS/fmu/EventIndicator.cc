// FMU-ME Event Indicator Support
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
#include <QSS/fmu/EventIndicator.hh>
#include <QSS/string.hh>

// C++ Headers
#include <cstring>
#include <iostream>

namespace QSS {
namespace fmu {

// EventIndicator Global Lookup by FMU-ME Context
AllEventIndicators allEventIndicators;

// XML Callbacks Global
fmi2_xml_callbacks_t xml_callbacks = {
 annotation_start_handle,
 annotation_data_handle,
 annotation_end_handle,
 NULL
};

extern "C" {

int
annotation_start_handle(
 void * /* context */,
 char const * parentName,
 void * /* parent */,
 char const * elm,
 char const ** attr
)
{
	if ( std::strcmp( parentName, "OCT_StateEvents" ) == 0 ) {
		FMUEventIndicators & fmuEventIndicators( allEventIndicators.back() ); // This is not thread safe!
		if ( std::strcmp( elm, "EventIndicators" ) == 0 ) {
			if ( fmuEventIndicators.inEventIndicators || ( ! fmuEventIndicators.eventIndicators.empty() ) ) {
				std::cerr << "\nError: XML EventIndicators block is ill-formed" << std::endl;
				std::exit( EXIT_FAILURE );
			}
			fmuEventIndicators.inEventIndicators = true;
			std::cout << "\nEventIndicators" << std::endl;
		} else if ( fmuEventIndicators.inEventIndicators && ( std::strcmp( elm, "Element" ) == 0 ) ) {
			int i( 0 );
			EventIndicator ei;
			bool has_index( false );
			bool has_reverseDependencies( false );
			while ( attr[ i ] ) {
				if ( std::strcmp( attr[ i ], "index" ) == 0 ) {
					if ( has_index ) std::cerr << "\nWarning: XML EventIndicators Element has multiple index attributes: Last one is used" << std::endl;
					std::string const index_string( attr[ i + 1 ] );
					if ( is_int( index_string ) ) {
						ei.index = int_of( index_string );
					} else {
						std::cerr << "\nError: XML EventIndicators Element has non-integer index: " << index_string << std::endl;
						std::exit( EXIT_FAILURE );
					}
					has_index = true;
				} else if ( std::strcmp( attr[ i ], "reverseDependencies" ) == 0 ) {
					if ( has_reverseDependencies ) std::cerr << "\nWarning: XML EventIndicators Element has multiple reverseDependencies attributes: Last one is used" << std::endl;
					std::istringstream reverseDependencies_stream( attr[ i + 1 ] );
					std::string reverseDependency_string;
					while ( reverseDependencies_stream >> reverseDependency_string ) {
						if ( is_int( reverseDependency_string ) ) {
							ei.reverseDependencies.push_back( int_of( reverseDependency_string ) );
							has_reverseDependencies = true;
						} else {
							std::cerr << "\nError: XML EventIndicators Element has non-integer reverseDependencies entry: " << reverseDependency_string << std::endl;
							std::exit( EXIT_FAILURE );
						}
					}
				} else {
					std::cerr << "\nWarning: XML EventIndicators Element has unsupported attribute: " << attr[ i + 1 ] << std::endl;
				}
				i += 2;
			}
			if ( ! has_index ) {
				std::cerr << "\nError: XML EventIndicators Element has no index attribute" << std::endl;
				std::exit( EXIT_FAILURE );
			}
			if ( ! has_reverseDependencies ) {
				std::cerr << "\nError: XML EventIndicators Element has no reverseDependencies" << std::endl;
//				std::exit( EXIT_FAILURE ); //OCT Let run proceed while waiting for OCT fixes so we can profile
			}
			fmuEventIndicators.eventIndicators.push_back( ei );
			std::cout << "\n EventIndicator Element\n";
			std::cout << "  index: " << ei.index << '\n';
			std::cout << "  reverseDependencies:";
			for ( EventIndicator::size_type const d : ei.reverseDependencies ) std::cout << ' ' << d;
			std::cout << std::endl;
		} else {
			fmuEventIndicators.inEventIndicators = false;
		}
	}
	return 0;
}

} // extern "C"

} // fmu
} // QSS
