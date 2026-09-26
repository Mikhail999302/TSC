// DataProcessing_impl.h: Implementation interface of data processing
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "DataProcessing.h"

namespace DataProcessing {

	class ECALC_API CArrayInfo {
	public:
		MathString ArrayName;
		int nSize;
		int nDimCount;
		bool IsArray() const { return !ArrayName.empty();}//.compare(_T("")) != 0; }
		void resize( int nNewSize );
		void ProcessDataParameter( const CDataParameter& Parameter, int nIndex ) 
			/*throw( std::exception, CDataProcessingException )*/;

		CArrayInfo( CMathParamBaseEstimates* pTargetEstimates = NULL ): 
			ArrayName(_T("")), nSize(0), nDimCount(0), pEstimates(pTargetEstimates) {}
		CArrayInfo( MathCalc::LPCTSTR pszArrayName, CExpressions* pTargetExpressions ): 
			ArrayName(pszArrayName), nSize(0), nDimCount(0), pExpressions(pTargetExpressions){}
	private:
		union {
			CExpressions* pExpressions;
			CMathParamBaseEstimates* pEstimates;
		};
	};
	typedef std::vector< CArrayInfo > CArrayInfos;

	// Reads array (1- and 2-dim) into the 1-dim array of expressions (using C-style rep. of multidim. arrays)
	ECALC_API void ReadArray( const CDataParameters& Source, CArrayInfo& ArrayInfo )
		/*throw( std::exception, CDataProcessingException )*/;
	// Version for multiple arrays.
	ECALC_API void ReadArrays( const CDataParameters& Source, CArrayInfos& ArrayInfos )
		/*throw( std::exception, CDataProcessingException )*/;

} // DataProcessing
