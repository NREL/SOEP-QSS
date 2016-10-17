#ifndef QSS_globals_hh_INCLUDED
#define QSS_globals_hh_INCLUDED

// Forward
template< typename > class EventQueue;
class Variable;

// QSS Globals
extern EventQueue< Variable > events;
extern bool diag; // Diagnostic output?

#endif
