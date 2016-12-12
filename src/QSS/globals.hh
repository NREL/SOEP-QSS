#ifndef QSS_globals_hh_INCLUDED
#define QSS_globals_hh_INCLUDED

// Forward
template< typename > class EventQueue;
class Variable;

// QSS Globals
extern bool diag; // Diagnostic output?
extern bool inflection_steps; // Inflection point quantization time steps?
extern EventQueue< Variable > events;

#endif
