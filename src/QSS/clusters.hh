// QSS State Dependency Cycle Clusters
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2023 Objexx Engineering, Inc. All rights reserved.
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

#ifndef QSS_clusters_hh_INCLUDED
#define QSS_clusters_hh_INCLUDED

// QSS Headers
#include <QSS/options.hh>

// C++ Headers
#include <cassert>
#include <deque>
#include <iostream>
#include <vector>

namespace QSS {

// QSS State Dependency Cycle Clusters
template< typename Variable, typename Variable_QSS >
void
clusters( typename Variable_QSS::Variables_QSS & vars )
{

	// Node
	struct Node
	{

	public: // Types

		using Observees = std::vector< Node * >;
		enum class State { None, Stack };

	public: // Creation

		// Constructor
		explicit
		Node( Variable_QSS * var ) :
		 var( var )
		{}

	public: // Property

		// Active Child Node
		Node *
		child()
		{
			return ( i != observees.end() ? *i : nullptr );
		}

	public: // Methods

		// Enter Node in DFS Traversal
		void
		enter()
		{
			state = Node::State::Stack; // Mark node as on active DFS branch
			i = observees.begin(); // Initialize child iterator
			entered = true;
		}

		// Advance Child Iterator in DFS Traversal
		bool
		advance_child()
		{
			assert( !observees.empty() );
			return ++i != observees.end();
		}

		// Leave Node in DFS Traversal
		void
		leave()
		{
			state = Node::State::None; // Mark node as not on active DFS branch
		}

	public: // Data

		bool entered{ false };
		Variable_QSS * var{ nullptr };
		State state{ State::None };
		Observees observees;
		typename Observees::iterator i;

	}; // Node

	// Types
	using Nodes = std::vector< Node >; //Do Use a set for sufficiently large models

	// Create graph
	Nodes nodes;
	for ( Variable_QSS * var : vars ) { // Add nodes
		if ( var->is_State() ) nodes.emplace_back( var );
	}
	for ( Node & node : nodes ) { // Add directed edges
		for ( Variable * obs : node.var->observees() ) {
			assert( ( obs->is_State() ) || ( obs->is_Input() ) ); // This should be run after variable observees are changed to computational observees
			if ( obs->is_State() ) node.observees.push_back( &*std::find_if( nodes.begin(), nodes.end(), [obs]( Node const & n ){ return n.var == obs; } ) );
		}
	}

	// Traverse graph via non-recursive DFS and detect cycles
	enum class Step { Push, Pop };
	Step step( Step::Push );
	using Branch = std::deque< Node * >;
	Branch branch;
	for ( Node & root : nodes ) {
		assert( branch.empty() );
		if ( root.entered ) continue; // Don't need to search from root that has already been entered
		branch.push_front( &root );
		step = Step::Push;
		Node * node( &root );
		while ( !branch.empty() ) {
			if ( node->state == Node::State::None ) { // Node not on active branch
				assert( step == Step::Push );
				node->enter();
				Node * child( node->child() );
				if ( child == nullptr ) { // No more children: Move up
					node->leave();
					branch.pop_front();
					step = Step::Pop;
					node = ( !branch.empty() ? branch.front() : nullptr );
				} else { // Move down branch to first child
					node = child;
					branch.push_front( node );
				}
			} else if ( node->state == Node::State::Stack ) { // Node on active branch
				if ( step == Step::Push ) { // Cycle detected
					if ( options::output::d ) std::cout << "\nContinuous state variable dependency cycle (cluster):" << std::endl;
					bool active( false );
					typename Branch::reverse_iterator i( branch.rbegin() );
					while ( !active ) {
						if ( *i == node ) {
							active = true;
						} else {
							++i;
						}
					}
					typename Branch::reverse_iterator i_active( i );
					for ( ; i != branch.rend(); ++i ) {
						if ( options::output::d ) std::cout << ' ' << (*i)->var->name() << std::endl;
						for ( typename Branch::reverse_iterator j = i_active; j != branch.rend(); ++j ) {
							(*i)->var->add_to_cluster( (*j)->var );
						}
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
						node->leave();
						branch.pop_front();
						step = Step::Pop;
						node = ( !branch.empty() ? branch.front() : nullptr );
					}
				}
			}
		}
	}

}

} // QSS

#endif
