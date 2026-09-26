// DataProcessing.h: Interface for data processing
//
//////////////////////////////////////////////////////////////////////

#pragma once

//#include "ECalcMisc.h"
//#include "MathParameterList.h"
#include "DataParameters.h"
#include "CacherComparingStruct.h"
#include "BaseDiscreteModel.h"

namespace DataProcessing {

	using MathCalc::MathString;
	using MathCalc::CExpression;
	using MathModels::CExpressions;
	using MathModels::CMathParameterInfos;
	using MathModels::CMathParamBaseEstimates;

	// Reads expressions from data
	// Empty array name means free expressions (ex. "param1 = <expression>;")
	// Array name (ex. "arr") is used in the following expr.: "arr[<index>] = <expression>;"
	// or "arr[<index1>][<index2>] = <expression>;", but dims couldn't mix
	ECALC_API void ReadExpressions( const CDataParameters& Source, CExpressions& Expressions, 
									MathCalc::LPCTSTR pszArrayName = _T("") )
		/*throw( std::exception, CDataProcessingException )*/;
	ECALC_API void ReadExpressions( const MathString& strSource, CExpressions& Expressions, 
									MathCalc::LPCTSTR pszArrayName = _T("") )
		/*throw( std::exception, CDataProcessingException )*/;

	// Reads estimates from data
	// Intended for use with CMathParamBaseEstimates-derived classes for extra processing
	ECALC_API void ReadEstimates( const CDataParameters& Source, CMathParamBaseEstimates& Estimates )
		/*throw( std::exception, CDataProcessingException )*/;
	ECALC_API void ReadEstimates( const MathString& strSource, CMathParamBaseEstimates& Estimates )
		/*throw( std::exception, CDataProcessingException )*/;
	
	// Reads sample from data or string
	ECALC_API int ReadSample( CDataIterator& it, doubles& result, MathCalc::LPCTSTR lpszSpaces = _T(" \t") );
	ECALC_API int ReadSample( const MathString& strSource, doubles& result, MathCalc::LPCTSTR lpszSpaces = _T(" \t") );

	// Reads and initializes discrete model
	using MathModels::ModelPtr;
	ECALC_API ModelPtr InitDiscreteModel( const CDataParameters& Source,
										  CMathParamBaseEstimates& PreliminaryEstimates,
										  MathModels::CCacherComparingStruct& _compStruct,
										  MathModels::TLDotCacher& ldotCacher,
										  MathModels::TInfoCacher& infoMatrix,
										  const CMathOperationIndicator& OperationIndicator = CMathOperationIndicator(), 
										  MathCalc::LPCTSTR pszProbabilitiesArrayName = _T("p") )
		/*throw( std::exception, CDataProcessingException, MathModels::CModelInitializationError )*/;

	// Reads new estimates and reinits model if necessary
	// Source can contain only a subset of estimates
	ECALC_API void ReinitDiscreteModel(	ModelPtr& pModel, const CDataParameters& Source, 
										CMathParamBaseEstimates& PreliminaryEstimates,
										const CMathOperationIndicator& OperationIndicator = CMathOperationIndicator() )
		/*throw( std::exception, CDataProcessingException, MathModels::CModelInitializationError )*/;
} // DataProcessing
