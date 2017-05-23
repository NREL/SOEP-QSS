#ifndef QSS_dfn_Variable_ZC_hh_INCLUDED
#define QSS_dfn_Variable_ZC_hh_INCLUDED

// QSS Zero-Crossing Variable Abstract Base Class
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// QSS Headers
#include <QSS/dfn/Variable.hh>
#include <QSS/EnumHash.hh> // GCC 5.x needs this

// C++ Headers
#include <unordered_set>

namespace QSS {
namespace dfn {

// QSS Zero-Crossing Variable Abstract Base Class
template< template< typename > class F, template< typename > class H >
class Variable_ZC : public Variable
{

public: // Types

	using Super = Variable;
	using Time = Variable::Time;
	using Value = Variable::Value;
	using Function = F< Variable >;
	using Handler = H< Variable >;
	using Crossing = Variable::Crossing;
	using Crossings = std::unordered_set< Crossing, EnumHash >;

protected: // Creation

	// Constructor
	explicit
	Variable_ZC(
	 std::string const & name,
	 Value const rTol = 1.0e-4,
	 Value const aTol = 1.0e-6
	) :
	 Super( name, rTol, aTol )
	{}

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

	// Function
	Function const &
	f() const
	{
		return f_;
	}

	// Function
	Function &
	f()
	{
		return f_;
	}

	// Handler
	Handler const &
	h() const
	{
		return h_;
	}

	// Handler
	Handler &
	h()
	{
		return h_;
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

protected: // Methods

	// Crossing Type
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

public: // Data

	Time tZ{ infinity }; // Zero-crossing time: tQ <= tZ and tX <= tZ
	Time tZ_prev{ infinity }; // Previous zero-crossing time
	Crossing crossing{ Crossing::Flat }; // Zero-crossing type
	Crossings crossings; // Zero-crossing types handled

protected: // Data

	Function f_; // Zero-crossing function
	Handler h_; // Handler function

};

} // dfn
} // QSS

#endif
