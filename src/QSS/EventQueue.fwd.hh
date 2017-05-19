#ifndef QSS_EventQueue_fwd_hh_INCLUDED
#define QSS_EventQueue_fwd_hh_INCLUDED

// QSS Event Queue Foward Declaration
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// This is a simple event queue based on std::multimap that is non-optimal for sequential and concurrent access
// Will need to put mutex locks around modifying operations for concurrent use
// Should explore concurrent-friendly priority queues once we have large scale test cases

namespace QSS {

// QSS Event Queue
template< typename V >
class EventQueue;

} // QSS

#endif
