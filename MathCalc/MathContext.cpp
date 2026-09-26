// MathContext.cpp: implementation of the MathContext
//
//////////////////////////////////////////////////////////////////////

#include "MathCalcMisc.h"
#include "MathContext.h"

//////////////////////////////////////////////////////////////////////
// Module variables

namespace MathCalc 
{
namespace MathContext 
{
namespace 
{

	typedef std::list<CMathMap>::iterator MapsIterator;
	//MapsIterator FindMap( MapIDType MapID )//;
	//{
	//	return std::find_if( m_MathMaps.begin(), m_MathMaps.end(), is_samemap(MapID) );
	//}

	std::list<CMathMap> m_MathMaps;
	CMathMap* m_pCurrentMap = NULL;
	CMathMap m_DefaultMap;

	struct is_samemap : std::unary_function<CMathMap*, bool> {
		CMathMap* _pSearchedMap;
		explicit is_samemap( CMathMap* pSearchedMap ) : _pSearchedMap(pSearchedMap) {}
		bool operator()( const CMathMap& _X ) const	{ return &_X == _pSearchedMap; }
	};
	MapsIterator FindMap( MapIDType MapID )//;
	{
		return std::find_if( m_MathMaps.begin(), m_MathMaps.end(), is_samemap(MapID) );
	}
}
}
}

using namespace MathCalc::MathContext;
using MathCalc::MathString;

//////////////////////////////////////////////////////////////////////
// CMathMap
//////////////////////////////////////////////////////////////////////

MathCalc::UINT CMathMap::AddParameter( const MathString& ParameterName )
{
	std::map<MathString, UINT>::const_iterator it = m_IndicesMap.find( ParameterName );
	if ( it == m_IndicesMap.end() ) {
		m_IndicesMap.insert(std::pair<MathString, int>(ParameterName,m_nLastIndex));//[ParameterName] = m_nLastIndex++;
		m_NamesArray.push_back( ParameterName );
		return m_nLastIndex++;
	}
	else
		return it->second;
}

bool CMathMap::GetParameterIndex( const MathString& ParameterName, MathCalc::UINT& ParameterIndex )
{
	std::map<MathString, UINT>::const_iterator it = m_IndicesMap.find( ParameterName );
	if ( it != m_IndicesMap.end() ) {
		ParameterIndex = it->second;
		return true;
	}
	else
		return false;
}

MathCalc::MathString& CMathMap::GetParameterName( MathCalc::UINT ParameterIndex ) 
	/*throw( MathCalc::CMathException_InvalidParameterIndex )*/
{
	if ( m_nLastIndex <= ParameterIndex || ParameterIndex < 0 )
		throw CMathException_InvalidParameterIndex();
	return m_NamesArray[ParameterIndex];
}

double CMathMap::GetParameterValue( const MathString& ParameterName )
{
	return m_ValuesMap[ParameterName];
}

double CMathMap::GetParameterValue( MathCalc::UINT ParameterIndex )
{
	return m_ValuesMap[GetParameterName(ParameterIndex)];
}

void CMathMap::SetParameterValue( const MathString& ParameterName, double Value )
{
	m_ValuesMap[ParameterName] = Value;
}

void CMathMap::SetParameterValue( MathCalc::UINT ParameterIndex, double Value )
{
	m_ValuesMap[GetParameterName(ParameterIndex)] = Value;
}

void CMathMap::Reset()
{
	m_IndicesMap.clear();
	m_NamesArray.clear();
	m_ValuesMap.clear();
	m_nLastIndex = 0;
}

void CMathMap::ClearValues()
{
	m_ValuesMap.clear();
}

void CMathMap::Serialize( MathCalc::CMathSerializer& ar )
{
	if ( ar.IsStoring() ) {
		ar << m_nLastIndex;

		ar.WriteCount( (int)m_NamesArray.size() );
		{for ( std::vector<MathString>::const_iterator it = m_NamesArray.begin(); it != m_NamesArray.end(); ++it )
			ar.WriteString( *it );}

		ar.WriteCount( (int)m_IndicesMap.size() );
		{for ( std::map<MathString, UINT>::const_iterator it = m_IndicesMap.begin(); it != m_IndicesMap.end(); ++it ) {
			ar.WriteString( it->first );
			ar << it->second;
		}}
	}
	else {
		ar >> m_nLastIndex;

		DWORD nNewCount = ar.ReadCount();
		m_NamesArray.resize( nNewCount );
		for ( std::vector<MathString>::iterator it = m_NamesArray.begin(); it != m_NamesArray.end(); ++it )
			ar.ReadString( *it );

		nNewCount = ar.ReadCount();
		while ( nNewCount-- ) {
			MathString str;
			ar.ReadString( str );
			UINT Index;
			ar >> Index;
			m_IndicesMap[str] = Index;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// MathContext
//////////////////////////////////////////////////////////////////////

MathCalc::UINT MathCalc::MathContext::GetParametersCount()
{
	if ( m_pCurrentMap )
		return m_pCurrentMap->GetParametersCount();
	else
		return m_DefaultMap.GetParametersCount();
}

MathCalc::UINT MathCalc::MathContext::AddParameter( const MathString& ParameterName ) 
{ 
	if ( m_pCurrentMap )
		return m_pCurrentMap->AddParameter(ParameterName);
	else
		return m_DefaultMap.AddParameter(ParameterName);
}

bool MathCalc::MathContext::GetParameterIndex( const MathString& ParameterName, UINT& ParameterIndex ) 
{ 
	if ( m_pCurrentMap )
		return m_pCurrentMap->GetParameterIndex(ParameterName, ParameterIndex); 
	else
		return m_DefaultMap.GetParameterIndex(ParameterName, ParameterIndex); 
}

MathCalc::MathString& MathCalc::MathContext::GetParameterName( UINT ParameterIndex )
	/*throw( MathCalc::CMathException_InvalidParameterIndex )*/ 
{ 
	if ( m_pCurrentMap )
		return m_pCurrentMap->GetParameterName(ParameterIndex); 
	else
		return m_DefaultMap.GetParameterName(ParameterIndex); 
}

double MathCalc::MathContext::GetParameterValue( const MathString& ParameterName ) 
{ 
	if ( m_pCurrentMap )
		return m_pCurrentMap->GetParameterValue(ParameterName); 
	else
		return m_DefaultMap.GetParameterValue(ParameterName); 
}

double MathCalc::MathContext::GetParameterValue( MathCalc::UINT ParameterIndex ) 
{ 
	if ( m_pCurrentMap )
		return m_pCurrentMap->GetParameterValue(ParameterIndex); 
	else
		return m_DefaultMap.GetParameterValue(ParameterIndex); 
}

void MathCalc::MathContext::SetParameterValue( const MathString& ParameterName, double Value ) 
{ 
	if ( m_pCurrentMap )
		m_pCurrentMap->SetParameterValue(ParameterName, Value); 
	else
		m_DefaultMap.SetParameterValue(ParameterName, Value);
}

void MathCalc::MathContext::SetParameterValue( UINT ParameterIndex, double Value ) 
{ 
	if ( m_pCurrentMap )
		m_pCurrentMap->SetParameterValue(ParameterIndex, Value); 
	else
		m_DefaultMap.SetParameterValue(ParameterIndex, Value); 
}

void MathCalc::MathContext::ResetMap() 
{ 
	if ( m_pCurrentMap )
		m_pCurrentMap->Reset(); 
	else
		m_DefaultMap.Reset();  
}

void MathCalc::MathContext::ClearValues() 
{
	if ( m_pCurrentMap )
		m_pCurrentMap->ClearValues();
	else
		m_DefaultMap.ClearValues(); 
}

//MapsIterator MathCalc::MathContext::FindMap( MapIDType MapID )
//{
//	return std::find_if( m_MathMaps.begin(), m_MathMaps.end(), is_samemap(MapID) );
//}

MapIDType MathCalc::MathContext::AddMap()
{
	m_MathMaps.push_back( CMathMap() );
	m_pCurrentMap = &m_MathMaps.back();
	return m_pCurrentMap;
}

void MathCalc::MathContext::SelectMap( MapIDType MapID ) 
{ // 0 means default map selection
	if ( MapID ) {
		MapsIterator itMap = FindMap(MapID);
		if ( itMap != m_MathMaps.end() )
			m_pCurrentMap = &*itMap; // get real pointer to Map 
	}
	else
		m_pCurrentMap = NULL;
}

void MathCalc::MathContext::RemoveMap( MapIDType MapID )
{
	MapsIterator itMap = FindMap( MapID );
	if ( itMap != m_MathMaps.end() ) {
		if ( MapID == m_pCurrentMap )
			SelectMap( MapIDType(0) );
		m_MathMaps.erase( itMap ); 
	}
}


void MathCalc::MathContext::Serialize( CMathSerializer& ar, MapIDType MapID )
{
	if ( MapID ) {
		MapsIterator itMap = FindMap(MapID);
		if ( itMap != m_MathMaps.end() )
			itMap->Serialize( ar );
	}
	else
		m_DefaultMap.Serialize( ar );
}
