#ifndef MATHCONTEXT_H
#define MATHCONTEXT_H

// MathContext.h: implementation details
//
//////////////////////////////////////////////////////////////////////

#include "MathExpressions.h"

namespace MathCalc { 
namespace MathContext 
{

	class CMathMap 
	{
		UINT m_nLastIndex;
		std::vector<MathString> m_NamesArray;
		std::map<MathString, UINT> m_IndicesMap;
		std::map<MathString, double> m_ValuesMap;

	public:
		CMathMap(): m_nLastIndex(0) { IncreaseObjectsCounter(); };
		~CMathMap() { DecreaseObjectsCounter(); }

		UINT GetParametersCount() const { return m_nLastIndex; }

		UINT AddParameter( const MathString& ParameterName );
		bool GetParameterIndex( const MathString& ParameterName, UINT& ParameterIndex );
		MathString& GetParameterName( UINT ParameterIndex ) 
			/*throw( MathCalc::CMathException_InvalidParameterIndex )*/;
		double GetParameterValue( const MathString& ParameterName );
		double GetParameterValue( UINT ParameterIndex );
		void SetParameterValue( const MathString& ParameterName, double Value );
		void SetParameterValue( UINT ParameterIndex, double Value );

		void Reset();
		void ClearValues();

		void Serialize( CMathSerializer& ar );
	};

}}
#endif //MATHCONTEXT_H
