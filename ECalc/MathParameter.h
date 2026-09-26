#pragma once

//#include "ECalcMisc.h"
//#include "ECalcTemplates.h"
#include "MathParameterEstimate.h"

namespace MathModels 
{
	//////////////////////////////////////////////////////////////////////
	// Parameters infos and values

using MathCalc::MathString;
using MathCalc::CMathSerializer;
using MathCalc::CParameter;

class ECALC_API CMathParameterInfo 
{
	bool isOptimize;
public:
	CParameter Parameter;
	//bool IsNumeric;

	CMathParameterInfo( const CParameter& Param = CParameter(), bool isOptimize_ = true ): 
		Parameter(Param), isOptimize(isOptimize_) {}
	bool isOptimizible()const{return isOptimize;}
	void setOptimizible(bool isOptimize_){isOptimize=isOptimize_;}
	virtual ~CMathParameterInfo() {}
	MathString GetParameterName() const { return Parameter.ToString(); }
	virtual void Construct( const CMathParamBaseEstimate& Estimate );
	virtual void Serialize( CMathSerializer& ar );
};

class ECALC_API CMathParameterValue : public CMathParameterInfo 
{
public:
	double Value;
	double Sigma2;
	struct ECALC_API ConfidenceRange 
	{
		double Gamma;
		double Low, High;
		ConfidenceRange(): Gamma(0), Low(0), High(0) {}
	} Range;
	CMathParameterValue( const CParameter& Param = CParameter(), double Val = 0 ):
	CMathParameterInfo(Param), Value(Val), Sigma2(0) {}
	inline virtual void Construct( const CMathParamBaseEstimate& Estimate )
	{
		CMathParameterInfo::Construct( Estimate );
		Value = Estimate.Eval();
	}
	virtual void Serialize( CMathSerializer& ar );
	// Set current value into MathContext
	void SetContextValue() const
	{
		Parameter.SetValue( Value );
	}
	void CalcRange( double SigmaSquare, int n, double Gamma );
};
} // MathModels
