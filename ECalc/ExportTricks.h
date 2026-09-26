#pragma once
#include <boost/shared_ptr.hpp>
#include "ECalcMisc.h"
#include "ECalcTemplates.h"
#include "MathParameterEstimate.h"

namespace MathCalc 
{
	bool operator<(const CExpression&, const CExpression&);
	bool operator==(const CExpression&, const CExpression&);
} // MathCalc

namespace MathModels 
{
	using MathCalc::CExpression;
	using namespace std;
#ifdef USE_PRAGMA
#pragma warning( disable : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA
	EC_EXPIMP_TEMPLATE template class ECALC_API std::allocator<CExpression>;
	EC_EXPIMP_TEMPLATE template class ECALC_API std::vector< CExpression >;
#ifdef USE_PRAGMA
#pragma warning( default : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA

	typedef std::vector< CExpression > CExpressions;

	bool operator<(const CExpression&, const CExpression&);
	bool operator==(const CExpression&, const CExpression&);
	bool operator<(const CExpressions&, const CExpressions&);
	bool operator==(const CExpressions&, const CExpressions&);
#ifdef USE_PRAGMA
#pragma warning( disable : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA
	EC_EXPIMP_TEMPLATE template class ECALC_API std::allocator<CExpressions>;
	EC_EXPIMP_TEMPLATE template class ECALC_API std::vector< CExpressions >;
	EC_EXPIMP_TEMPLATE template class ECALC_API vector2D< CExpressions >;
#ifdef USE_PRAGMA
#pragma warning( default : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA

	//class ECALC_API CMathParamBaseEstimatePtr : public boost::shared_ptr<CMathParamBaseEstimate> 
	//{
	//public:
	//	CMathParamBaseEstimatePtr() {}
	//	CMathParamBaseEstimatePtr(CMathParamBaseEstimate* pEstimate ): 
	//		boost::shared_ptr<CMathParamBaseEstimate>(pEstimate) {}
	//};
	typedef boost::shared_ptr<CMathParamBaseEstimate> CMathParamBaseEstimatePtr;

	template< class T >
	struct ParamEqual {
		MathCalc::CParameter Param;
		ParamEqual( const MathCalc::CParameter& Parameter ): Param(Parameter) {}
		bool operator()( const T& t ) { return t.Parameter == Param; }
	};

	template <> struct ParamEqual<CMathParamBaseEstimatePtr> {
		MathCalc::CParameter Param;
		ParamEqual( const MathCalc::CParameter& Parameter ): Param(Parameter) {}
		bool operator()( const CMathParamBaseEstimatePtr& t ) { return t->Parameter == Param; }
	};

	template< class T >
	class param_vector : public std::vector<T> {
	public:
		typename std::vector<T>::iterator find( const MathCalc::CParameter& Parameter ) {			
			return std::find_if( std::vector<T>::begin(), std::vector<T>::end(), ParamEqual<T>(Parameter) );
		}
	};

	bool operator<(const MathModels::CMathParamBaseEstimatePtr&, const MathModels::CMathParamBaseEstimatePtr&);
	bool operator==(const MathModels::CMathParamBaseEstimatePtr&, const MathModels::CMathParamBaseEstimatePtr&);

#ifdef USE_PRAGMA
#pragma warning( disable : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA
	EC_EXPIMP_TEMPLATE template class ECALC_API std::allocator<CMathParamBaseEstimatePtr>;
	EC_EXPIMP_TEMPLATE template class ECALC_API std::vector< CMathParamBaseEstimatePtr >;
	EC_EXPIMP_TEMPLATE template class ECALC_API param_vector< CMathParamBaseEstimatePtr >;
#ifdef USE_PRAGMA
#pragma warning( default : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA


//#pragma warning( default : 4231 )

} // MathModels

