#include "MathParameter.h"
#include "Probcalc.h"

using namespace MathModels;

//////////////////////////////////////////////////////////////////////
// CMathParameterInfo

void CMathParameterInfo::Serialize( CMathSerializer& ar )
{
	Parameter.Serialize( ar );
	if (ar.IsStoring())
		ar << isOptimize;
	else
		ar >> isOptimize;
}

void CMathParameterInfo::Construct( const CMathParamBaseEstimate& Estimate )
{
	Parameter = Estimate.Parameter;
	isOptimize = true;
	//IsNumeric = Estimate.IsNumeric();
}

//////////////////////////////////////////////////////////////////////
// CMathParameterValue

void CMathParameterValue::Serialize( CMathSerializer& ar )
{
	CMathParameterInfo::Serialize( ar );
	if (ar.IsStoring())
		ar << Value << Sigma2 << Range.Gamma << Range.Low << Range.High;
	else
		ar >> Value >> Sigma2 >> Range.Gamma >> Range.Low >> Range.High;
}

//inline void CMathParameterValue::Construct( const CMathParamBaseEstimate& Estimate )
//{
//	CMathParameterInfo::Construct( Estimate );
//	Value = Estimate.Eval();
//}

///*inline*/ void CMathParameterValue::SetContextValue() const
//{
//	Parameter.SetValue( Value );
//}

void CMathParameterValue::CalcRange( double SigmaSquare, int n, double Gamma )
{
	Sigma2 = SigmaSquare/n;
	double HalfDelta = sqrt(Sigma2) * xNormal( (1. + Gamma) / 2. );
	Range.Low = Value - HalfDelta;
	Range.High = Value + HalfDelta;
	Range.Gamma = Gamma;
}
