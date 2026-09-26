// DataParameters.cpp: implementation of data parameters
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataParameters.h"
#include "DataProcessing_impl.h"

using namespace DataProcessing;

/////////////////////////////////////////////////////////////////////////////
// CDataParameter

/*inline*/ bool CDataParameter::HasSameName( const MathString& Name2Check ) const
{
	return GetArrayName().compare( Name2Check ) == 0;
}

/*inline*/ bool CDataParameter::IsArrayElement() const
{
	return Name.find(TCHAR('[')) != MathString::npos;
}

/*inline*/ MathString CDataParameter::GetArrayName() const
{
	int IndexPos = (int)Name.find(TCHAR('['));
	return IndexPos != (int)MathString::npos ? Name.substr(0, IndexPos) : Name;
}

int CDataParameter::GetArrayIndex1() const /*throw( std::exception, CInvalidArrayIndex )*/
{
	int IndexPos = (int)Name.find(TCHAR('['));
	if ( IndexPos != (int)MathString::npos )
		return _ttoi( Name.substr(IndexPos + 1).c_str() );
	else
		throw CInvalidArrayIndex();
}

int CDataParameter::GetArrayIndex2() const /*throw( std::exception, CInvalidArrayIndex )*/
{
	int IndexPos = (int)Name.find(_T("]["));
	if ( IndexPos != (int)MathString::npos )
		return _ttoi( Name.substr(IndexPos + 2).c_str() );
	else
		throw CInvalidArrayIndex();
}

/////////////////////////////////////////////////////////////////////////////
// CDataParameters

CDataParameter* CDataParameters::FindParameter( const MathString& Name )
{
	for ( iterator it = begin(); it != end(); ++it )
		if ( it->HasSameName( Name ) )
			return &*it;
	return NULL;
}

void CDataParameters::FindArraySize( CArrayInfo& ArrayInfo ) const
{
	ArrayInfo.nSize = -1;
	ArrayInfo.nDimCount = 0;
	for ( const_iterator it = begin(); it != end(); ++it )
		if ( ArrayInfo.ArrayName != _T("") ) {
			if ( it->IsArrayElement() && it->HasSameName( ArrayInfo.ArrayName ) ) {
				int nIndex = it->GetArrayIndex1();
				if ( nIndex > ArrayInfo.nSize )
					ArrayInfo.nSize = nIndex;
				if ( ArrayInfo.nDimCount < 1 )
					ArrayInfo.nDimCount = it->Name.find(_T("][")) != MathString::npos ? 2 : 1;
			}
		}
		else if ( !it->IsArrayElement() )
			ArrayInfo.nSize++;
	ArrayInfo.nSize++;
}

// Reads parameters from string(s) using format "<parameter name> = <parameter value>;"
bool CDataParameters::ReadFromData( CDataIterator& it, int nForceCount )
{
	clear();
	MathString ParameterLine;
	int start_search_pos, nParamsProcessed = 0;
	for (; (nForceCount <= 0 || (nForceCount > 0 && nParamsProcessed < nForceCount)) && it; ++it ) 
	{
		start_search_pos = (int)ParameterLine.size();
		ParameterLine += *it;
		if ( ParameterLine.find(TCHAR(';'), start_search_pos) != MathString::npos ) 
		{
			MathCalc::MathStrings::TrimAllSpaces( ParameterLine );
			int expr_start_pos = 0, expr_end_pos, eq_pos;
			while ( (expr_end_pos = (int)ParameterLine.find(TCHAR(';'), expr_start_pos)) != (int)MathString::npos ) 
			{
				eq_pos = (int) ParameterLine.find(TCHAR('='), expr_start_pos);
				if ( eq_pos > expr_end_pos || eq_pos == expr_start_pos ) // '=' not found for expression
					return false;										 // or there is no name for parameter

				CDataParameter DataParameter;
				DataParameter.Name = ParameterLine.substr( expr_start_pos, eq_pos - expr_start_pos )/*.c_str()*/;
				DataParameter.Value = ParameterLine.substr( eq_pos + 1, expr_end_pos - eq_pos - 1 )/*.c_str()*/;
				push_back( DataParameter );
				nParamsProcessed++;
				expr_start_pos = expr_end_pos + 1;
			}
			ParameterLine = ParameterLine.substr( expr_start_pos );
		}
	}
	if ( nForceCount > 0 && nParamsProcessed < nForceCount )
		return false;
	else
		return true;
}

/*inline*/ bool CDataParameters::ReadFromData( const MathString& strSource, int nForceCount )
{
	CSimpleIterator simpleIter(strSource);
	return ReadFromData(simpleIter, nForceCount);
}
