#ifndef QSS_dfn_Variable_ZC2_hh_INCLUDED
#define QSS_dfn_Variable_ZC2_hh_INCLUDED

// QSS2 Zero-Crossing Variable
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/dfn/Variable_ZC.hh>

namespace QSS {
namespace dfn {

// QSS2 Zero-Crossing Variable
template< template< typename > class F, template< typename > class H >
class Variable_ZC2 final : public Variable_ZC< F, H >
{

public: // Types

	using Super = Variable_ZC< F, H >;
	using Time = Variable::Time;
	using Value = Variable::Value;
	using Crossing = Variable::Crossing;

	using Super::crossing;
	using Super::name;
	using Super::rTol;
	using Super::aTol;
	using Super::qTol;
	using Super::xIni;
	using Super::tQ;
	using Super::tX;
	using Super::tE;
	using Super::tZ;
	using Super::tZ_prev;
	using Super::dt_min;
	using Super::dt_max;
	using Super::self_observer;

	using Super::event;
	using Super::has;

protected: // Types

	using Super::h_;

	using Super::crossing_type;

private: // Types

	using Super::event_;
	using Super::f_;
	using Super::observers_;

public: // Creation

	// Constructor
	explicit
	Variable_ZC2(
	 std::string const & name,
	 Value const rTol = 1.0e-4,
	 Value const aTol = 1.0e-6
	) :
	 Super( name, rTol, aTol )
	{}

public: // Properties

	// Order of Method
	int
	order() const
	{
		return 2;
	}

	// Continuous Value at Time t
	Value
	x( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		return x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// Continuous First Derivative at Time t
	Value
	x1( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		return x_1_ + ( two * x_2_ * ( t - tX ) );
	}

	// Continuous Second Derivative at Time t
	Value
	x2( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		return two * x_2_;
	}

	// Quantized Value at Time t
	Value
	q( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		return q_0_ + ( q_1_ * ( t - tQ ) );
	}

	// Quantized First Derivative at Time t
	Value
	q1( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		(void)t; // Suppress unused parameter warning
		return q_1_;
	}

public: // Methods

	// Initialization
	void
	init()
	{
		self_observer = f_.finalize( this );
		if ( self_observer ) {
			std::cerr << "Zero-crossing variable is self-observer: " << name << std::endl;
			std::exit( EXIT_FAILURE );
		}
		if ( ! observers_.empty() ) {
			std::cerr << "Zero-crossing variable has observers: " << name << std::endl;
			std::exit( EXIT_FAILURE );
		}
		x_0_ = q_0_ = f_.q( tQ );
		set_qTol();
		x_1_ = q_1_ = f_.q1( tQ );
		x_2_ = one_half * f_.q2( tQ );
		set_tE();
		set_tZ();
		event( tE < tZ ? events.add_QSS( tE, this ) : events.add_ZC( tZ, this ) );
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << "   tZ=" << tZ << '\n';
	}

	// Set Current Tolerance
	void
	set_qTol()
	{
		qTol = std::max( rTol * std::abs( q_0_ ), aTol );
		assert( qTol > 0.0 );
	}

	// QSS Advance
	void
	advance_QSS()
	{
		advance_QSS_core();
		if ( options::output::d ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << "   tZ=" << tZ << '\n';
	}

	// QSS Advance: Simultaneous
	void
	advance_QSS_simultaneous()
	{
		advance_QSS_core();
		if ( options::output::d ) std::cout << "= " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << "   tZ=" << tZ << '\n';
	}

	// Observer Advance
	void
	advance_observer( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		int const sign_old( t == tZ_prev ? 0 : signum( x( t ) ) ); // Treat as if exactly zero if t is previous zero-crossing event time
		x_0_ = q_0_ = f_.q( tX = tQ = t );
		int const sign_new( signum( x_0_ ) );
		set_qTol();
		x_1_ = q_1_ = f_.q1( t );
		x_2_ = one_half * f_.q2( t );
		set_tE();
		if ( sign_old != sign_new ) { // Zero-crossing occurs at t
			Crossing const crossing_check( crossing_type( sign_old, sign_new ) );
			if ( has( crossing_check ) ) { // Crossing type is relevant
				crossing = crossing_check;
				tZ = t;
				event( events.shift_ZC( tZ, event() ) );
			} else {
				set_tZ();
				event( tE < tZ ? events.shift_QSS( tE, event() ) : events.shift_ZC( tZ, event() ) );
			}
		} else {
			set_tZ();
			event( tE < tZ ? events.shift_QSS( tE, event() ) : events.shift_ZC( tZ, event() ) );
		}
		if ( options::output::d ) std::cout << "  " << name << '(' << t << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << "   tZ=" << tZ <<  '\n';
	}

	// Zero-Crossing Trigger Advance
	void
	advance_ZC()
	{
		h_( tZ, crossing ); // Handler
		tZ_prev = tZ;
		if ( options::output::d ) std::cout << "Z " << name << '(' << tZ << ')' << '\n';
		set_tZ( tZ ); // Next zero-crossing: Might be in active segment
		event( tE < tZ ? events.shift_QSS( tE, event() ) : events.shift_ZC( tZ, event() ) );
	}

private: // Methods

	// QSS Advance: Core
	void
	advance_QSS_core()
	{
		x_0_ = q_0_ = f_.q( tX = tQ = tE );
		set_qTol();
		x_1_ = q_1_ = f_.q1( tE );
		x_2_ = one_half * f_.q2( tE );
		set_tE();
		set_tZ();
		event( tE < tZ ? events.shift_QSS( tE, event() ) : events.shift_ZC( tZ, event() ) );
	}

	// New End Time
	Time
	new_tE()
	{
		assert( tX <= tQ );
		assert( dt_min <= dt_max );
		Time tEnd( x_2_ != 0.0 ? tQ + std::sqrt( qTol / std::abs( x_2_ ) ) : infinity );
		if ( dt_max != infinity ) tEnd = std::min( tEnd, tQ + dt_max );
		tEnd = std::max( tEnd, tQ + dt_min );
		if ( ( options::inflection ) && ( x_2_ != 0.0 ) && ( signum( x_1_ ) != signum( x_2_ ) ) ) {
			Time const tI( tX - ( x_1_ / ( two * x_2_ ) ) );
			if ( tQ < tI ) tEnd = std::min( tEnd, tI );
		}
		return tEnd;
	}

	// Set End Time
	void
	set_tE()
	{
		assert( tX <= tQ );
		assert( dt_min <= dt_max );
		tE = ( x_2_ != 0.0 ? tQ + std::sqrt( qTol / std::abs( x_2_ ) ) : infinity );
		if ( dt_max != infinity ) tE = std::min( tE, tQ + dt_max );
		tE = std::max( tE, tQ + dt_min );
		if ( ( options::inflection ) && ( x_2_ != 0.0 ) && ( signum( x_1_ ) != signum( x_2_ ) ) ) {
			Time const tI( tX - ( x_1_ / ( two * x_2_ ) ) );
			if ( tQ < tI ) tE = std::min( tE, tI );
		}
	}

	// Set Zero-Crossing Time and Type on Active Segment
	void
	set_tZ()
	{
		assert( tE == new_tE() ); // tE must be set

		// Simple root search: Only robust for small active segments with continuous rep close to function //Do Make robust version
		int const sign_old( signum( x_0_ ) );
		if ( sign_old == 0 ) { // Check crossing direction at new tZ
			Time const dtX( min_positive_root_quadratic( x_2_, x_1_, x_0_ ) ); // Root of continuous rep
			assert( dtX > 0.0 );
			if ( dtX != infinity ) { // Root found on (tX,tE]
				tZ = tX + dtX;
				int const sign_tZ( -signum( x_2_ ) ); // Sign of function approaching tZ
				Crossing const crossing_check( tZ == tX ? Crossing::Flat : crossing_type( sign_tZ, 0 ) );
				if ( has( crossing_check ) ) { // Crossing type is relevant
					crossing = crossing_check;
				} else { // Crossing type not relevant
					tZ = infinity;
					return;
				}
			} else { // Root not found
				tZ = infinity;
				return;
			}
		} else { // Use sign_old to deduce crossing type
			Crossing const crossing_check( crossing_type( sign_old, 0 ) );
			if ( has( crossing_check ) ) { // Crossing type is relevant
				Time const dtX( min_positive_root_quadratic( x_2_, x_1_, x_0_ ) ); // Root of continuous rep
				assert( dtX > 0.0 );
				if ( dtX != infinity ) { // Root found on (tX,tE]
					tZ = tX + dtX;
					crossing = crossing_check;
				} else { // Root not found
					tZ = infinity;
					return;
				}
			} else { // Crossing type not relevant
				tZ = infinity;
				return;
			}
		}

		// Refine root
		Time t( tZ ), t_p( t );
		Value const vZ( f_.q( tZ ) );
		Value v( vZ ), v_p( vZ );
		Value m( 1.0 ); // Multiplier
		std::size_t i( 0 );
		std::size_t const n( 10u ); // Max iterations
		while ( ( ++i <= n ) && ( std::abs( v ) > aTol ) ) {
			Value const d( f_.q1( t ) );
			if ( d == 0.0 ) break;
			if ( ( signum( d ) != sign_old ) && ( tE < std::min( t_p, t ) ) ) break; // Zero-crossing seems to be >tE so don't refine further
			t -= m * ( v / d );
			v = f_.q( t );
			if ( std::abs( v ) >= std::abs( v_p ) ) m *= 0.5; // Non-converging step: Reduce step size
			t_p = t;
			v_p = v;
		}
		if ( ( t >= tX ) && ( std::abs( v ) < std::abs( vZ ) ) ) tZ = t;
		if ( ( i == n ) && ( options::output::d ) ) std::cout << "  " << name << '(' << t << ')' << " tZ may not have converged" <<  '\n';
	}

	// Set Zero-Crossing Time and Type on (tB,tE]
	void
	set_tZ( Time const /*tB*/ )
	{
		tZ = infinity; //! For now we don't handle multiple roots in active segment
	}

private: // Data

	Value x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }; // Continuous rep coefficients
	Value q_0_{ 0.0 }, q_1_{ 0.0 }; // Quantized rep coefficients

};

} // dfn
} // QSS

#endif
