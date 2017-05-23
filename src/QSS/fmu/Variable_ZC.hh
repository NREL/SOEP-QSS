#ifndef QSS_fmu_Variable_ZC_hh_INCLUDED
#define QSS_fmu_Variable_ZC_hh_INCLUDED

// FMU-Based QSS Zero-Crossing Variable Abstract Base Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/fmu/Variable.hh>
#include <QSS/EnumHash.hh> // GCC 5.x needs this

// C++ Headers
#include <unordered_set>

namespace QSS {
namespace fmu {

// FMU-Based QSS Zero-Crossing Variable Abstract Base Class
class Variable_ZC : public Variable
{

public: // Types

	using Super = Variable;
	using Crossings = std::unordered_set< Crossing, EnumHash >;

protected: // Creation

	// Constructor
	explicit
	Variable_ZC(
	 std::string const & name,
	 Value const rTol = 1.0e-4,
	 Value const aTol = 1.0e-6,
	 FMU_Variable const var = FMU_Variable(),
	 FMU_Variable const der = FMU_Variable()
	) :
	 Super( name, rTol, aTol, 0.0, var, der )
	{
		add_crossings_all(); // FMI API doesn't currently expose crossing information
	}

	// Copy Constructor
	Variable_ZC( Variable_ZC const & ) = default;

	// Move Constructor
	Variable_ZC( Variable_ZC && ) noexcept = default;

public: // Creation

	// Destructor
	virtual
	~Variable_ZC()
	{}

protected: // Assignment

	// Copy Assignment
	Variable_ZC &
	operator =( Variable_ZC const & ) = default;

	// Move Assignment
	Variable_ZC &
	operator =( Variable_ZC && ) noexcept = default;

public: // Predicate

	// Zero-Crossing Variable?
	bool
	is_ZC() const
	{
		return true;
	}

	// Has Crossing Type?
	bool
	has( Crossing const crossing )
	{
		return ( crossings.find( crossing ) != crossings.end() );
	}

public: // Properties

	// Category
	Cat
	cat() const
	{
		return Cat::ZC;
	}

	// Zero-Crossing Time
	Time
	tZC() const
	{
		return tZ;
	}

public: // Methods

	// Add Crossing Type
	Variable_ZC &
	add( Crossing const crossing )
	{
		crossings.insert( crossing );
		return *this;
	}

	// Add All Crossing Types
	Variable_ZC &
	add_crossings_all()
	{
		add_crossings_Dn();
		crossings.insert( Crossing::Flat );
		add_crossings_Up();
		return *this;
	}

	// Add All non-Flat Crossing Types
	Variable_ZC &
	add_crossings_non_Flat()
	{
		add_crossings_Dn();
		add_crossings_Up();
		return *this;
	}

	// Add All Downward Crossing Types
	Variable_ZC &
	add_crossings_Dn()
	{
		crossings.insert( Crossing::DnPN );
		crossings.insert( Crossing::DnPZ );
		crossings.insert( Crossing::DnZN );
		crossings.insert( Crossing::Dn );
		return *this;
	}

	// Add All Downward and Flat Crossing Types
	Variable_ZC &
	add_crossings_Dn_Flat()
	{
		add_crossings_Dn();
		crossings.insert( Crossing::Flat );
		return *this;
	}

	// Add All Upward Crossing Types
	Variable_ZC &
	add_crossings_Up()
	{
		crossings.insert( Crossing::Up );
		crossings.insert( Crossing::UpZP );
		crossings.insert( Crossing::UpNZ );
		crossings.insert( Crossing::UpNP );
		return *this;
	}

	// Add All Upward and Flat Crossing Types
	Variable_ZC &
	add_crossings_Up_Flat()
	{
		crossings.insert( Crossing::Flat );
		add_crossings_Up();
		return *this;
	}

	// Shift Observers Events to Handlers
	void
	shift_handlers()
	{
		assert( tZ != infinity );
		fmu_set_observees_q( tZ + ( tZ - tX ) * 1.01 ); // Use slightly later time to let FMU detect the zero crossing: This may not be robust until output variable derivatives are analytical, not numeric
		for ( auto observer : observers_ ) {
			observer->shift_handler( tZ );
		}
	}

protected: // Methods

	// Crossing Type
	static
	Crossing
	crossing_type( int const sign_old, int const sign_new )
	{
		assert( ( -1 <= sign_old ) && ( sign_old <= +1 ) );
		assert( ( -1 <= sign_new ) && ( sign_new <= +1 ) );
		if ( sign_old == 0 ) {
			return ( sign_new == +1 ? Crossing::UpZP : ( sign_new == 0 ? Crossing::Flat : Crossing::DnZN ) );
		} else if ( sign_new == 0 ) {
			return ( sign_old == +1 ? Crossing::DnPZ : Crossing::UpNZ );
		} else {
			return ( sign_old == +1 ? Crossing::DnPN : Crossing::UpNP );
		}
	}

	// Crossed Value for Signaling FMU Event Processing
	static
	Value
	crossed( Crossing const crossing )
	{
		static Value const zc_eps( 1.0e-6 ); //Do Set this based on how FMU behaves wrt chattering prevention and tolerances
		if ( ( crossing == Crossing::Dn ) || ( crossing == Crossing::DnPN ) || ( crossing == Crossing::DnZN ) ) {
			return -zc_eps;
		} else if ( ( crossing == Crossing::DnPZ ) || ( crossing == Crossing::Flat ) || ( crossing == Crossing::UpNZ ) ) {
			return 0.0;
		} else if ( ( crossing == Crossing::Up ) || ( crossing == Crossing::UpNP ) || ( crossing == Crossing::UpZP ) ) {
			return +zc_eps;
		} else {
			assert( false ); return 0.0;
		}
	}

public: // Data

	Time tZ{ infinity }; // Zero-crossing time: tQ <= tZ and tX <= tZ
	Time tZ_prev{ infinity }; // Previous zero-crossing time
	Crossing crossing{ Crossing::Flat }; // Zero-crossing type
	Crossings crossings; // Zero-crossing types handled

protected: // Data

	int sign_old_{ 0 }; // Sign of zero-crossing function before advance

};

} // fmu
} // QSS

#endif
