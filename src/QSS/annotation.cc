// FMU-ME XML Annotation Support
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
#include <QSS/annotation.hh>
#include <QSS/Dependencies.hh>
#include <QSS/EventIndicators.hh>
#include <QSS/string.hh>

// C++ Headers
#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include <utility> // std::move

namespace QSS {

// Globals
static std::size_t ei_index( 0u );

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
	assert( parentName != nullptr );
	assert( elm != nullptr );
	assert( attr != nullptr );
	if ( std::strcmp( parentName, "OCT_StateEvents" ) == 0 ) {
		FMU_EventIndicators & fmu_eventindicators( all_eventindicators.back() ); // This is not thread safe!
		FMU_Dependencies & fmu_dependencies( all_dependencies.back() ); // This is not thread safe!
		if ( std::strcmp( elm, "EventIndicators" ) == 0 ) { // EventIndicators section start
			if ( fmu_eventindicators.inEventIndicators || !fmu_eventindicators.empty() ) {
				std::cerr << "\nError: XML OCT annotations EventIndicators block repeats or is ill-formed" << std::endl;
				std::exit( EXIT_FAILURE );
			}
			fmu_eventindicators.inEventIndicators = true;
			std::cout << "\nEventIndicators" << std::endl;
		} else if ( fmu_eventindicators.inEventIndicators && ( std::strcmp( elm, "Element" ) == 0 ) ) {
			std::size_t i( 0u );
			EventIndicator ei( ei_index++ );
			bool has_index( false );
			// bool has_reverseDependencies( false );
			assert( attr[ i ] != nullptr );
			while ( attr[ i ] ) {
				if ( std::strcmp( attr[ i ], "index" ) == 0 ) {
					if ( has_index ) std::cerr << "\nWarning: XML EventIndicators Element has multiple index attributes: Last one is used" << std::endl;
					std::string const index_string( attr[ i + 1u ] );
					if ( is_int( index_string ) ) {
						ei.index = int_of( index_string );
					} else {
						std::cerr << "\nError: XML EventIndicators Element has non-integer variable index: " << index_string << std::endl;
						std::exit( EXIT_FAILURE );
					}
					has_index = true;
				} else if ( std::strcmp( attr[ i ], "reverseDependencies" ) == 0 ) {
					// if ( has_reverseDependencies ) std::cerr << "\nWarning: XML EventIndicators Element has multiple reverseDependencies attributes: Last one is used" << std::endl;
					// std::istringstream reverseDependencies_stream( attr[ i + 1u ] );
					// std::string reverseDependency_string;
					// while ( reverseDependencies_stream >> reverseDependency_string ) {
					// 	if ( is_int( reverseDependency_string ) ) {
					// 		ei.add( int_of( reverseDependency_string ) );
					// 		has_reverseDependencies = true;
					// 	} else {
					// 		std::cerr << "\nError: XML EventIndicators Element has non-integer reverseDependencies entry: " << reverseDependency_string << std::endl;
					// 		std::exit( EXIT_FAILURE );
					// 	}
					// }
					std::cerr << "\nError: XML EventIndicators Element has an obsolete reverseDependencies entry: Regenerate the FMU with a later Optimica release" << std::endl;
					std::exit( EXIT_FAILURE );
				} else if ( std::strcmp( attr[ i ], "inAssert" ) == 0 ) {
					//Do Exploit inAssert to ignore event indicators where safe
				} else {
					std::cerr << "\nWarning: XML EventIndicators Element has unsupported attribute: " << attr[ i ] << std::endl;
				}
				i += 2;
			}
			if ( !has_index ) {
				std::cerr << "\nError: XML EventIndicators Element has no index attribute" << std::endl;
				std::exit( EXIT_FAILURE );
			}
			std::cout << "\n EventIndicator Element\n";
			std::cout << "  index: " << ei.index << '\n';
			// if ( has_reverseDependencies ) {
			// 	std::cout << "  reverseDependencies:";
			// 	for ( EventIndicator::size_type const d : ei.reverseDependencies ) std::cout << ' ' << d;
			// 	std::cout << std::endl;
			// }
			fmu_eventindicators.add( std::move( ei ) );
		} else if ( std::strcmp( elm, "Dependencies" ) == 0 ) { // Dependencies section start
			if ( fmu_dependencies.inDependencies || ( !fmu_dependencies.empty() ) ) {
				std::cerr << "\nError: XML OCT annotations Dependencies block repeats or is ill-formed" << std::endl;
				std::exit( EXIT_FAILURE );
			}
			fmu_dependencies.inDependencies = true;
			std::cout << "\nDependencies" << std::endl;
		} else if ( fmu_dependencies.inDependencies && ( std::strcmp( elm, "Element" ) == 0 ) ) {
			std::size_t i( 0u );
			FMU_Dependencies::Variable var;
			bool has_index( false );
			bool has_dependencies( false );
			assert( attr[ i ] != nullptr );
			while ( attr[ i ] ) {
				if ( std::strcmp( attr[ i ], "index" ) == 0 ) {
					if ( has_index ) std::cerr << "\nWarning: XML Dependencies Element has multiple index attributes: Last one is used" << std::endl;
					std::string const index_string( attr[ i + 1u ] );
					if ( is_int( index_string ) ) {
						var.index( int_of( index_string ) );
					} else {
						std::cerr << "\nError: XML Dependencies Element has non-integer variable index: " << index_string << std::endl;
						std::exit( EXIT_FAILURE );
					}
					has_index = true;
				} else if ( std::strcmp( attr[ i ], "dependencies" ) == 0 ) {
					if ( has_dependencies ) std::cerr << "\nWarning: XML Dependencies Element has multiple dependencies attributes: Last one is used" << std::endl;
					std::istringstream dependencies_stream( attr[ i + 1u ] );
					std::string dependency_string;
					while ( dependencies_stream >> dependency_string ) {
						if ( is_int( dependency_string ) ) {
							var.add_observee( int_of( dependency_string ) );
						} else {
							std::cerr << "\nError: XML Dependencies Element has non-integer dependencies entry: " << dependency_string << std::endl;
							std::exit( EXIT_FAILURE );
						}
					}
					has_dependencies = true;
				} else {
					std::cerr << "\nWarning: XML Dependencies Element has unsupported attribute: " << attr[ i ] << std::endl;
				}
				i += 2;
			}
			if ( !has_index ) {
				std::cerr << "\nError: XML Dependencies Element has no index attribute" << std::endl;
				std::exit( EXIT_FAILURE );
			}
			std::cout << "\n Dependencies Element\n";
			std::cout << "  index: " << var.index() << '\n';
			if ( has_dependencies ) {
				std::cout << "  dependencies:";
				for ( FMU_Dependencies::Index const idx : var.observees ) std::cout << ' ' << idx;
				std::cout << std::endl;
			}
			fmu_dependencies.add( std::move( var ) );
		}
	}
	return 0;
}

int
annotation_end_handle(
 void * /* context */,
 char const * elm
)
{
	assert( elm != nullptr );
	FMU_EventIndicators & fmu_eventindicators( all_eventindicators.back() ); // This is not thread safe!
	FMU_Dependencies & fmu_dependencies( all_dependencies.back() ); // This is not thread safe!
	if ( std::strcmp( elm, "EventIndicators" ) == 0 ) { // EventIndicators section end
		if ( fmu_eventindicators.inEventIndicators ) {
			fmu_eventindicators.inEventIndicators = false;
			// fmu_eventindicators.sort(); // Don't sort yet: The order in XML is the order of FMU's event_indicators and we need to use that later
		} else {
			std::cerr << "\nError: XML OCT annotations EventIndicators block is ill-formed" << std::endl;
			std::exit( EXIT_FAILURE );
		}
	} else if ( std::strcmp( elm, "Dependencies" ) == 0 ) { // Dependencies section end
		if ( fmu_dependencies.inDependencies ) {
			fmu_dependencies.inDependencies = false;
			// fmu_dependencies.finalize(); // Defer this until after processing in FMU_ME::pre_simulate()
		} else {
			std::cerr << "\nError: XML OCT annotations Dependencies block is ill-formed" << std::endl;
			std::exit( EXIT_FAILURE );
		}
	}
	return 0;
}

} // extern "C"

} // QSS
