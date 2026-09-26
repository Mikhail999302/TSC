// DiscreteModel.h: interface for the CDiscreteModel class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

//-->Sergey
//#include <Cache/Entry.h>
//#include "stdafx.h"
//#include "..\Dumper\Dump.h"
//--<Sergey
//#include "ECalcMisc.h"
//#include "InverseMatrix.h"
//#include "ECalcTemplates.h"
//#include "MathParameterEstimate.h"
//#include "MathParameter.h"

//#include "ExportTricks2.h"
//#include "MathParameterList.h"
#include <boost/format.hpp>
#include "BaseDiscreteModel.h"

namespace MathModels 
{

//////////////////////////////////////////////////////////////////////
// Base model definition

	using DataProcessing::CMathOperationIndicator;

//////////////////////////////////////////////////////////////////////
// Discrete models definition

	class ECALC_API CDiscreteModel : public CBaseDiscreteModel
	{
		friend class CLDotCacher;
		friend class CInfoCacher;
		friend class CBCacher;
	public:
		CDiscreteModel(): CBaseDiscreteModel(mt1D) {}
		virtual void Init( const CExpressions& _States, const CMathParameterInfos& _ParamInfos,
						   CCacherComparingStruct& _compStruct,
						   TLDotCacher& ldotCacher,
						   TInfoCacher& infoMatrix,
						   const CMathOperationIndicator& OperationIndicator = CMathOperationIndicator(),
						   bool _isCalcInfoMatrix = true,
						   bool _isBMatrix = false)
			 /*throw( CModelInitializationError )*/;

	protected:
		virtual void SetFrequenciesValues( const doubles& Sample ) const;
		CParameter GetSampleSize()const	{
			return CParameter(MathString(_T("n")));
		}
		CParameter GetFrequencyParam( int i ) const { 
			MathString res = boost::str(boost::format(_T("n_%1%"))% i); 
			//MathCalc::MathStrings::Format(res, 10, _T("n_%d"), i); 
			return CParameter(res); 
		}
	private:
		void CalcLDotInfoMatrix(CCacherComparingStruct& _compStruct,
								TLDotCacher& ldotCacher,
								TInfoCacher& infoMatrix,
								const CMathOperationIndicator& OperationIndicator = CMathOperationIndicator(),
								bool _isBMatrix=false);
	};

	class ECALC_API CDiscrete2DModel : public CBaseDiscreteModel
	{
		friend class CLDotCacher2D;
		friend class CInfoCacher2D;
		friend class CBCacher2D;
	public:
		CDiscrete2DModel(): CBaseDiscreteModel(mt2D), ProductsCount(0) {}
		int GetProductsCount() const { return ProductsCount; }

		virtual void Serialize( CMathSerializer& ar );
		virtual void Init( const CExpressions& _States, const CMathParameterInfos& _ParamInfos,
						   CCacherComparingStruct& _compStruct,
						   TLDotCacher& ldotCacher,
						   TInfoCacher& infoMatrix,
						   const CMathOperationIndicator& OperationIndicator = CMathOperationIndicator(),
						   bool _isCalcInfoMatrix = true,
						   bool _isBMatrix = false)
			 /*throw( CModelInitializationError )*/;

	protected:
		int ProductsCount;
		CParameter GetSampleSize()const	{
			return CParameter(MathString(_T("n")));
		}
		virtual void SetFrequenciesValues( const doubles& Sample ) const;
		CParameter GetFrequencyParam( int i, int j ) const { 
			MathString res = boost::str(boost::format(_T("n_%1%_%2%"))%i%j); 
			//MathCalc::MathStrings::Format(res, 18, _T("n_%d_%d"), i, j); 
			return CParameter(res); 
		}
	private:
		void CalcLDotInfoMatrix(CCacherComparingStruct& _compStruct,
								TLDotCacher& ldotCacher,
								TInfoCacher& infoMatrix,
								const CMathOperationIndicator& OperationIndicator = CMathOperationIndicator(),
								bool _usBMatrix=false);

	};

} // MathModels
