// QSS Dependency Cycle Detection
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

#ifndef QSS_fmu_cycles_hh_INCLUDED
#define QSS_fmu_cycles_hh_INCLUDED

// C++ Headers
#include <cassert>
#include <deque>
#include <iostream>
#include <vector>

namespace QSS {
namespace fmu {

// QSS Dependency Cycle Detection
template< typename Variable >
inline
void
cycles( typename Variable::Variables const & vars )
{

	// Node Type
	struct Node
	{

	public: // Types

		using Observers = std::vector< Node * >;
		enum class State { None, Stack, Done };

	public: // Creation

		// Constructor
		Node( Variable const * var ) :
		 var( var )
		{}

	public: // Property

		// Active Child Node
		Node *
		child()
		{
			return ( i != observers.end() ? *i : nullptr );
		}

	public: // Methods

		// Enter Node in DFS Traversal
		void
		enter()
		{
			state = Node::State::Stack; // Mark node as on active DFS branch
			i = observers.begin(); // Initialize child iterator
		}

		// Advance Child Iterator in DFS Traversal
		bool
		advance_child()
		{
			assert( !observers.empty() );
			return ++i != observers.end();
		}

	public: // Data

		Variable const * var{ nullptr };
		State state{ State::None };
		Observers observers;
		typename Observers::const_iterator i;

	}; // Node

	// Types
	using Nodes = std::vector< Node >;

	// Create graph
	Nodes nodes;
	for ( Variable const * var : vars ) { // Add nodes
		nodes.emplace_back( var );
	}
	for ( Node & node : nodes ) { // Add directed edges
		for ( Variable const * obs : node.var->observers() ) {
			node.observers.push_back( &*std::find_if( nodes.begin(), nodes.end(), [obs]( Node const & n ){ return n.var == obs; } ) );
		}
		if ( node.var->in_conditional() ) { // Short-circuit conditional dependencies
			for ( Variable const * mod : node.var->conditional->observers() ) {
				node.observers.push_back( &*std::find_if( nodes.begin(), nodes.end(), [mod]( Node const & n ){ return n.var == mod; } ) );
			}
		}
	}

	// Traverse graph via non-recursive DFS and detect cycles
	enum class Step { Push, Pop };
	Step step( Step::Push );
	using Branch = std::deque< Node * >;
	Branch branch;
	for ( Node & root : nodes ) {
		if ( root.state == Node::State::None ) { // DFS traversal from this root
			assert( branch.empty() );
			branch.push_front( &root );
			step = Step::Push;
			Node * node( &root );
			while ( !branch.empty() ) {
				if ( node->state == Node::State::None ) { // First visit to node
					assert( step == Step::Push );
					node->enter();
					Node * child( node->child() );
					if ( child != nullptr ) { // Move down branch to first child
						node = child;
						branch.push_front( node );
					} else { // No more children: Move up
						node->state = Node::State::Done;
						branch.pop_front();
						step = Step::Pop;
						node = ( !branch.empty() ? branch.front() : nullptr );
					}
				} else if ( node->state == Node::State::Stack ) { // Revisiting node on branch
					if ( step == Step::Push ) { // Cycle detected
						std::cerr << "\nVariable dependency cycle present:" << std::endl;
						bool active( false );
						for ( typename Branch::const_iterator i = branch.begin(); i != branch.end(); ++i ) {
							if ( ( !active ) && ( *i == node ) ) active = true;
							if ( active ) std::cerr << ' ' << (*i)->var->name() << std::endl;
						}
						branch.pop_front();
						step = Step::Pop;
						node = ( !branch.empty() ? branch.front() : nullptr );
					} else { // Moved up from child
						assert( step == Step::Pop );
						if ( node->advance_child() ) { // Move down branch to next child
							node = node->child();
							branch.push_front( node );
							step = Step::Push;
						} else { // No more children: Move up
							node->state = Node::State::Done;
							branch.pop_front();
							step = Step::Pop;
							node = ( !branch.empty() ? branch.front() : nullptr );
						}
					}
				} else { // Move up branch
					assert( node->state == Node::State::Done );
					branch.pop_front();
					step = Step::Pop;
					node = ( !branch.empty() ? branch.front() : nullptr );
				}
			}
		}
	}
}

} // cod
} // QSS

#endif
