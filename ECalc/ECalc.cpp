// ECalc.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "DiscreteModel.h"
#include "DataParameters.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

//////////////////////////////////////////////////////////////////////
// Dummy operators definition

inline bool MathCalc::operator<(const CExpression&, const CExpression&) { return true; }
inline bool MathCalc::operator==(const CExpression&, const CExpression&) { return false; }

inline bool MathModels::operator<(const CMathParamBaseEstimatePtr&, const CMathParamBaseEstimatePtr&) { return true; }
inline bool MathModels::operator==(const CMathParamBaseEstimatePtr&, const CMathParamBaseEstimatePtr&) { return false; }

inline bool MathModels::operator<(const CExpression&, const CExpression&) { return true; }
inline bool MathModels::operator==(const CExpression&, const CExpression&) { return false; }

inline bool MathModels::operator<(const CExpressions&, const CExpressions&) { return true; }
inline bool MathModels::operator==(const CExpressions&, const CExpressions&) { return false; }

inline bool MathModels::operator<(const CMathParameterInfo&, const CMathParameterInfo&) { return true; }
inline bool MathModels::operator==(const CMathParameterInfo&, const CMathParameterInfo&) { return false; }

inline bool MathModels::operator<(const CMathParameterValue&, const CMathParameterValue&) { return true; }
inline bool MathModels::operator==(const CMathParameterValue&, const CMathParameterValue&) { return false; }

inline bool DataProcessing::operator<(const CDataParameter&, const CDataParameter&) { return true; }
inline bool DataProcessing::operator==(const CDataParameter&, const CDataParameter&) { return false; }
