#pragma once
#include "stdafx.h"
#include "TypeDefs.h"
namespace GroupedTSC
{
struct ModelInfo
	{
		doubles sample;
		CMathParameterValues simpleEstimates;
		MathCalc::MathString modelStrRep;
		MathCalc::MathString paramStrRep;
		CExpressions states;
		CMathParameterEstimates SFEstimates;
		MathStrings mModelElems;
		ModelPtr model;
		int sampleSize;
		PLevel::PLevelData mPLevelData;
		doubles statesValues;
		CDict estimates;
		int Width;
		int Height;
		int m_nDim;
	};
}//namespace GroupedTSC