#pragma once
#include "MathParameterEstimateList.h"
namespace MathModels 
{

//#pragma warning( disable : 4231 ) //disable warnings on extern before template instantiation

	template< class T >
	class param_vector_ex : public param_vector<T> {
	public:
		void Construct( const CMathParamBaseEstimates& Estimates ) {
			resize( Estimates.size() );
			typename std::vector<T>::iterator it = std::vector<T>::begin();
			for (typename CMathParamBaseEstimates::const_iterator eit = Estimates.begin(); eit != Estimates.end(); ++eit, ++it )
				it->Construct( **eit );
		}
	};

	bool operator<(const CMathParameterInfo&, const CMathParameterInfo&);
	bool operator==(const CMathParameterInfo&, const CMathParameterInfo&);
#ifdef USE_PRAGMA
#pragma warning( disable : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA
	EC_EXPIMP_TEMPLATE template class ECALC_API std::allocator< CMathParameterInfo >;
	EC_EXPIMP_TEMPLATE template class ECALC_API std::vector< CMathParameterInfo >;
	EC_EXPIMP_TEMPLATE template class ECALC_API param_vector< CMathParameterInfo >;
	EC_EXPIMP_TEMPLATE template class ECALC_API param_vector_ex< CMathParameterInfo >;
#ifdef USE_PRAGMA
#pragma warning( default : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA

	bool operator<(const CMathParameterValue&, const CMathParameterValue&);
	bool operator==(const CMathParameterValue&, const CMathParameterValue&);
#ifdef USE_PRAGMA
#pragma warning( disable : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA
	EC_EXPIMP_TEMPLATE template class ECALC_API std::allocator< CMathParameterValue >;
	EC_EXPIMP_TEMPLATE template class ECALC_API std::vector< CMathParameterValue >;
	EC_EXPIMP_TEMPLATE template class ECALC_API param_vector< CMathParameterValue >;
	EC_EXPIMP_TEMPLATE template class ECALC_API param_vector_ex< CMathParameterValue >;
#ifdef USE_PRAGMA
#pragma warning( default : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA


//#pragma warning( default : 4231 )

} // MathModels
