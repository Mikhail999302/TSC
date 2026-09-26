#include "MathParameterEstimate.h"
#include "MathParameterEstimateList.h"
using namespace MathModels;

//////////////////////////////////////////////////////////////////////
// CMathParameterEstimate

void CMathParameterEstimate::ParseString( const MathString& Name, const MathString& Value ) 
/*throw ( DataProcessing::CInvalidDataParameter )*/
{ 
	if ( !Formula.ParseString( Value ) ) 
		throw DataProcessing::CInvalidDataParameter(); 
	Parameter = CParameter(Name);
}

void CMathParameterEstimate::Serialize( CMathSerializer& ar )
{
	Parameter.Serialize( ar );
	Formula.Serialize( ar );
}

//////////////////////////////////////////////////////////////////////
// CMathParamBaseEstimate

void CMathParamBaseEstimate::ParseString( const MathString& Name, const MathString& Value ) 
/*throw ( DataProcessing::CInvalidDataParameter )*/
{ 
	if ( !Formula.ParseString( Value ) ) 
		throw DataProcessing::CInvalidDataParameter(); 
	Parameter = CParameter(Name);
}

void CMathParamBaseEstimate::Serialize( CMathSerializer& ar )
{
	Parameter.Serialize( ar );
	Formula.Serialize( ar );
}
