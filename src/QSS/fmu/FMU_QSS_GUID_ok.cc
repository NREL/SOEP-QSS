
// FMU-QSS Headers
#include <QSS/fmu/FMU_QSS_GUID_ok.hh>
#include <QSS/fmu/FMU_QSS_GUID.hh>

namespace QSS {
namespace fmu {

// GUID is correct?
bool
FMU_QSS_GUID_ok( std::string const & GUID )
{
	static std::string const no_check_GUID( "FMU-QSS_GUID" );
	return ( FMU_QSS_GUID == no_check_GUID ? true : GUID == FMU_QSS_GUID );
}

} // fmu
} // QSS
