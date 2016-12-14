// QSS Headers
#include <QSS/globals.hh>
#include <QSS/EventQueue.hh>

// QSS Globals
bool diag( false ); // Diagnostic output?
bool inflection_steps( false ); // Inflection point quantization time steps?
double dtnd( 1.0e-6 ); // Numeric differentiation default time step
EventQueue< Variable > events;
