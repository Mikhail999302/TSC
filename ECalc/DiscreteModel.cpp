// DiscreteModel.cpp: implementation of the CDiscreteModel class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DiscreteModel.h"
#include "LDotCacher.h"
#include "InfoCacher.h"
#include "BCacher.h"
#include "BCacher2D.h"
#include "LDotCacher2D.h"
#include "InfoCacher2D.h"
#include "Probcalc.h"

using MathCalc::CMathException;
using namespace MathModels;

//////////////////////////////////////////////////////////////////////
// Model creation

ModelPtr MathModels::CreateModel( CBaseDiscreteModel::ModelType Type )
{
	if ( Type == CBaseDiscreteModel::mt1D )
		return ModelPtr(new CDiscreteModel);
	else
		return ModelPtr(new CDiscrete2DModel);
}

//////////////////////////////////////////////////////////////////////
// CDiscreteModel
//////////////////////////////////////////////////////////////////////

void CDiscreteModel::Init( const CExpressions& _States, const CMathParameterInfos& _ParamInfos,
						   CCacherComparingStruct& _compStruct,
						   TLDotCacher& ldotCacher,
						   TInfoCacher& infoCacher,
						   const CMathOperationIndicator& OperationIndicator /*= CMathOperationIndicator()*/,
						   bool _isCalcInfoMatrix/*=true*/,
						   bool _isBMatrix /*=false*/) /*throw( CModelInitializationError )*/
{
	CBaseDiscreteModel::Init( _States, _ParamInfos, _compStruct, ldotCacher, infoCacher, OperationIndicator,_isBMatrix);

	OperationIndicator.SetOperationLength( (NANParamCount+1)*NANParamCount*StatesCount/2 + NANParamCount*StatesCount );
	OperationIndicator.Show();

	if(_isCalcInfoMatrix)
	{
		CalcLDotInfoMatrix(_compStruct, ldotCacher, infoCacher, OperationIndicator,_isBMatrix);
	}
}

void CDiscreteModel::SetFrequenciesValues( const doubles& Sample ) const
{
	double sum=0;
	for ( int i = 0; i < StatesCount; ++i )
	{
		sum+=Sample[i];
		GetFrequencyParam(i).SetValue( Sample[i] );
	}
	GetSampleSize().SetValue(sum);
}

void CDiscreteModel::CalcLDotInfoMatrix(CCacherComparingStruct& _compStruct,
										TLDotCacher& ldotCacher,
										TInfoCacher& infoCacher,
										const CMathOperationIndicator& OperationIndicator /*= CMathOperationIndicator()*/,
										bool _isBMatrix /*=false*/ )
{
	//Add n_i_j parameters to the information
	for (int i = 0; i < StatesCount; ++i )
		GetFrequencyParam(i);
	CLDotCacher ldotCalculator(_compStruct, this, OperationIndicator);
	l_dot = ldotCacher.GetCalculatedValue(ldotCalculator);

	auto_ptr<TLengthyInfo> infoCalculator;
	if (_isBMatrix)
	{
		infoCalculator.reset(new CBCacher(_compStruct,this, OperationIndicator));
	}
	else
	{
		infoCalculator.reset(new CInfoCacher(_compStruct,this, OperationIndicator));
	}
	InfoMatrix = infoCacher.GetCalculatedValue(*infoCalculator);	
}

//////////////////////////////////////////////////////////////////////
// CDiscrete2DModel
//////////////////////////////////////////////////////////////////////

void CDiscrete2DModel::Serialize( CMathSerializer& ar )
{
	CBaseDiscreteModel::Serialize( ar );
	if ( ar.IsLoading() )
		ProductsCount = int(sqrt((double)StatesCount));
}

void CDiscrete2DModel::Init( const CExpressions& _States, const CMathParameterInfos& _ParamInfos,
							 CCacherComparingStruct& _compStruct,
							 TLDotCacher& ldotCacher,
							 TInfoCacher& infoCacher,
							 const CMathOperationIndicator& OperationIndicator,
							 bool _isCalcInfoMatrix/*=true*/,
							 bool _isBMatrix /*=false*/) /*throw( CModelInitializationError )*/
{
	CBaseDiscreteModel::Init( _States, _ParamInfos, _compStruct, ldotCacher, infoCacher, OperationIndicator,_isBMatrix);
	ProductsCount = int(sqrt((double)StatesCount));

	OperationIndicator.SetOperationLength( (NANParamCount+1)*NANParamCount*StatesCount/2 + NANParamCount*StatesCount );
	OperationIndicator.Show();

	if(_isCalcInfoMatrix)
	{
		CalcLDotInfoMatrix(_compStruct, ldotCacher, infoCacher, OperationIndicator);
	}
}

void CDiscrete2DModel::SetFrequenciesValues( const doubles& Sample ) const
{
	for ( int i = 0; i < ProductsCount; ++i )
		for ( int j = 0; j < ProductsCount; ++j )
			GetFrequencyParam(i, j).SetValue( Sample[i*ProductsCount + j] );
}

void CDiscrete2DModel::CalcLDotInfoMatrix(CCacherComparingStruct& _compStruct,
										  TLDotCacher& ldotCacher,
										  TInfoCacher& infoCacher,
										  const CMathOperationIndicator& OperationIndicator /*= CMathOperationIndicator()*/,
										  bool _usBMatrix /*=false*/)
{
	//Add n_i_j parameters to the information
	GetSampleSize();
	for (int i = 0; i < ProductsCount; ++i )
		for (int j = 0; j < ProductsCount; ++j ) 
		{
			GetFrequencyParam(i,j);
		}
	CLDotCacher2D ldotCalculator(_compStruct, this, OperationIndicator);
	l_dot = ldotCacher.GetCalculatedValue(ldotCalculator);

	auto_ptr<TLengthyInfo> infoCalculator;
	if (_usBMatrix)
	{
		infoCalculator.reset(new CBCacher2D(_compStruct,this, OperationIndicator));
	}
	else
	{
		infoCalculator.reset(new CInfoCacher2D(_compStruct,this, OperationIndicator));
	}
	InfoMatrix = infoCacher.GetCalculatedValue(*infoCalculator);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
//MathParameterValues
bool operator<(const CMathParameterValues&, const CMathParameterValues&){return true;}
bool operator==(const CMathParameterValues&, const CMathParameterValues&){return false;}
