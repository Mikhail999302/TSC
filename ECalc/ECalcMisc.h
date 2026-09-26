// ECalcMisc.h
//////////////////////////////////////////////////////////////////////
#pragma once

#ifdef USE_ECALC_SOURCE	//use ECalc sources as a part of a project
#	define ECALC_API
#	define EC_EXPIMP_TEMPLATE
#else						//use ECalc as DLL
#	ifdef ECALC_EXPORTS
#		define ECALC_API __declspec(dllexport)
#		define EC_EXPIMP_TEMPLATE
#	else
#		define ECALC_API __declspec(dllimport)
#		define EC_EXPIMP_TEMPLATE extern
#	endif
#endif

//////////////////////////////////////////////////////////////////////
// External reference

#include "../MathCalc/MathCalc.h"

//////////////////////////////////////////////////////////////////////
//->Sergey 
//Make Exceptions none exportable and inherited by exception
#include<exception>
//<-Sergey
namespace DataProcessing {

// Exceptions
	class ECALC_API CDataProcessingException{};//:public std::exception {}; // Base exception for data processing routines
	class ECALC_API CInvalidArrayIndex : public CDataProcessingException {};
	class ECALC_API CInvalidArrayInfo : public CDataProcessingException {};
	class ECALC_API CInvalidDataParameter : public CDataProcessingException {};

//////////////////////////////////////////////////////////////////////
// Lengthy operations indicator - does nothing by default

	class ECALC_API CMathOperationIndicator {
		mutable int i;
	public:
		CMathOperationIndicator() {}
		virtual ~CMathOperationIndicator() {}
		virtual void SetOperationLength( int Length )const{}
		virtual void Show( bool Show = true )const{}
		virtual void Increment()const{}
	};

} // DataProcessing

namespace MathModels {
	class ECALC_API CModelInitializationError {};
} // MathModels
