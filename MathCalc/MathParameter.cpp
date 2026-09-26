#include "MathParameter.h"
using namespace MathCalc;

CMathParameter::CMathParameter( const MathString& Name )
{
	Index = MathContext::AddParameter( Name );
}

void CMathParameter::Serialize( CMathSerializer& ar ) 
{
	if ( ar.IsStoring() )
		ar << Index;
	else
		ar >> Index;
}
