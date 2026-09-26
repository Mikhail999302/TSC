// DataProcessing.cpp: implementation of data processing functions
//
//////////////////////////////////////////////////////////////////////

#include <stdexcept>

#include "stdafx.h"
#include "DataProcessing_impl.h"
#include "DiscreteModel.h"

using DataProcessing::CDataParameters;

namespace {
	using MathCalc::MathString;
	using DataProcessing::CArrayInfo;
	struct same_ArrayInfo {
		MathString ArrayName;
		explicit same_ArrayInfo( const MathString& ArrayName2Find ): ArrayName(ArrayName2Find) {}
		bool operator() ( const CArrayInfo& Info ) { return ArrayName.compare( Info.ArrayName ) == 0; }
	};
}

//////////////////////////////////////////////////////////////////////
// CArrayInfo

void DataProcessing::CArrayInfo::resize( int nNewSize )
{
	if ( !pExpressions )
		throw CInvalidArrayInfo();
	else if ( IsArray() )
		pExpressions->resize( nNewSize );
	else
		pEstimates->resize( nNewSize );
}

void DataProcessing::CArrayInfo::ProcessDataParameter( const CDataParameter& Parameter, int nIndex ) 
	/*throw( std::exception, CDataProcessingException )*/
{
	if ( !pExpressions )
		throw CInvalidArrayInfo();
	if ( IsArray() )
		(*pExpressions)[nIndex].ParseString( Parameter.Value );
	else
		(*pEstimates)[nIndex]->ParseString( Parameter.Name, Parameter.Value );
}

//////////////////////////////////////////////////////////////////////
// Implementation functions

void DataProcessing::ReadArrays( const CDataParameters& Source, CArrayInfos& ArrayInfos )
	/*throw( std::exception, CDataProcessingException )*/
{
	CArrayInfos::iterator iit = ArrayInfos.begin();
	for (iit = ArrayInfos.begin(); iit != ArrayInfos.end(); ++iit ) 
	{
		Source.FindArraySize( *iit );
		if ( iit->nDimCount < 2 )						// free members or 1D-array
			iit->resize( iit->nSize );
		else											// nDimCount == 2
			iit->resize( iit->nSize * iit->nSize );
	}

	int free_mem_counter = 0;
	for ( CDataParameters::const_iterator pit = Source.begin(); pit != Source.end(); ++pit ) 
	{
		bool bIsArrayElement = pit->IsArrayElement();
		iit = std::find_if( ArrayInfos.begin(), ArrayInfos.end(), 
							same_ArrayInfo( bIsArrayElement ? pit->GetArrayName() : _T("") ) );
		if ( iit != ArrayInfos.end() ) {
			if ( bIsArrayElement ) {
				if ( iit->nDimCount == 1 )
					iit->ProcessDataParameter( *pit, pit->GetArrayIndex1() );
				else
					iit->ProcessDataParameter( *pit, pit->GetArrayIndex1() * iit->nSize + pit->GetArrayIndex2() );
			}
			else
				iit->ProcessDataParameter( *pit, free_mem_counter++ );
		}
	}
}

void DataProcessing::ReadArray( const CDataParameters& Source, CArrayInfo& ArrayInfo )
	/*throw( std::exception, CDataProcessingException )*/
{
	CArrayInfos Infos(1,ArrayInfo);
//	Infos.push_back( ArrayInfo );
	ReadArrays( Source, Infos );
	ArrayInfo = Infos.back();
}

//////////////////////////////////////////////////////////////////////
// Processing functions

void DataProcessing::ReadExpressions( const CDataParameters& Source, CExpressions& Expressions, 
									  LPCTSTR pszArrayName )
	/*throw( std::exception, CDataProcessingException )*/
{
	CArrayInfo Info(pszArrayName, &Expressions);
	ReadArray( Source, Info );
}

//strSource parameter formulas
//Expressions expressions
//pszArrayName name of states
/*inline*/ void DataProcessing::ReadExpressions( const MathString& strSource, CExpressions& Expressions, 
									  LPCTSTR pszArrayName )
	/*throw( std::exception, CDataProcessingException )*/
{
	CDataParameters DataParameters;
	if(!DataParameters.ReadFromData( strSource ))
		throw std::logic_error(std::string("Not correct parameter format. ReadExpressions Failed.").c_str());
	ReadExpressions( DataParameters, Expressions, pszArrayName );
}

void DataProcessing::ReadEstimates( const CDataParameters& Source, CMathParamBaseEstimates& Estimates )
	/*throw( std::exception, CDataProcessingException )*/
{
	CArrayInfo Info(&Estimates);
	ReadArray( Source, Info );
}

/*inline*/ void DataProcessing::ReadEstimates( const MathString& strSource, CMathParamBaseEstimates& Estimates )
	/*throw( std::exception, CDataProcessingException )*/
{
	CDataParameters DataParameters;
	DataParameters.ReadFromData( strSource );
	ReadEstimates( DataParameters, Estimates );
}

MathModels::ModelPtr DataProcessing::InitDiscreteModel( const CDataParameters& Source, 
											CMathParamBaseEstimates& PreliminaryEstimates,
											MathModels::CCacherComparingStruct& _compStruct,
											MathModels::TLDotCacher& ldotCacher,
											MathModels::TInfoCacher& infoCacher,
											const CMathOperationIndicator& OperationIndicator, 
											LPCTSTR pszProbabilitiesArrayName )
	/*throw( std::exception, CDataProcessingException, MathModels::CModelInitializationError )*/
{
	CExpressions States;
	CArrayInfos Infos;
	Infos.push_back( CArrayInfo(pszProbabilitiesArrayName, &States) );
	Infos.push_back( CArrayInfo(&PreliminaryEstimates) );
	MathCalc::MathContext::ResetMap(); // !to enable missing parameters feature
	ReadArrays( Source, Infos );

	ModelPtr pResult;
	if ( Infos.front().nDimCount == 1 )
		pResult = ModelPtr(new MathModels::CDiscreteModel);
	else
		pResult = ModelPtr(new MathModels::CDiscrete2DModel);

	pResult->Init( States, CMathParameterInfos(PreliminaryEstimates), _compStruct, 
				   ldotCacher, infoCacher, OperationIndicator );
	return pResult;
}

void DataProcessing::ReinitDiscreteModel( ModelPtr& pModel, const CDataParameters& Source, 
										  CMathParamBaseEstimates& PreliminaryEstimates,
										  const CMathOperationIndicator& OperationIndicator )
	/*throw( std::exception, CDataProcessingException, MathModels::CModelInitializationError )*/
{
	ReadEstimates( Source, PreliminaryEstimates );
	pModel->Reinit( CMathParameterInfos(PreliminaryEstimates), OperationIndicator );
}

int DataProcessing::ReadSample( CDataIterator& it, doubles& result, LPCTSTR lpszSpaces )
{
	result.clear();
	int nSampleVolume = 0;
	int nSampleElement = 0, nValueStart, nValueEnd;
	double nValue;
	for (; it; ++it ) {
		nValueEnd = 0;
		while( (nValueStart = (int)it->find_first_not_of(lpszSpaces, nValueEnd)) != (int)MathString::npos ) {
			if ( (nValueEnd = (int)it->find_first_of(lpszSpaces, nValueStart)) == (int)MathString::npos )
				nValueEnd = (int)it->length();
			nValue = _ttoi( it->substr( nValueStart, nValueEnd - nValueStart ).c_str() );
			nSampleVolume += int(nValue);
			SetElementAtGrowIndex( result, nSampleElement++, nValue );
		}
	}
	return nSampleVolume;
}

/*inline*/ int DataProcessing::ReadSample( const MathString& strSource, doubles& result, LPCTSTR lpszSpaces )
{
	CSimpleIterator simpleIteration( strSource );
	return ReadSample( simpleIteration, result, lpszSpaces );
}
