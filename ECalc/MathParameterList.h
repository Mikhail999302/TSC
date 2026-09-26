#pragma once
#include <vector>
#include "MathParameter.h"
#include "ExportTricks2.h"

namespace MathModels 
{
class ECALC_API CMathParameterInfos : public param_vector_ex< CMathParameterInfo > 
{
public:
	CMathParameterInfos() {}
	explicit CMathParameterInfos( const CMathParamBaseEstimates& Estimates ) { Construct( Estimates ); }
};

class ECALC_API CMathParameterValues : public param_vector_ex< CMathParameterValue > 
{
public:
	void SetContextValues() const;
};

bool operator<(const CMathParameterValues&, const CMathParameterValues&);
bool operator==(const CMathParameterValues&, const CMathParameterValues&);

#ifdef USE_PRAGMA
#pragma warning( disable : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA
	EC_EXPIMP_TEMPLATE template class ECALC_API std::allocator<CMathParameterValues>;
	EC_EXPIMP_TEMPLATE template class ECALC_API std::vector<CMathParameterValues>;
#ifdef USE_PRAGMA
#pragma warning( default : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA
}//namespace MathModels
