#include "stdafx.h"
#include <boost/format.hpp>
#include <boost/algorithm/string/replace.hpp>
#include "TSCTrunkModelGenerator.h"


///////////Model Generator/////////////////////////////////////////////////////////////////////////////

GroupedTSC::CTSCTrunkModelGenerator::CTSCTrunkModelGenerator(int _width,int _height):mWidth(_width),mHeight(_height)
{
}

GroupedTSC::CTSCTrunkModelGenerator::~CTSCTrunkModelGenerator(void)
{
}

GroupedTSC::TModelManager* GroupedTSC::CTSCTrunkModelGenerator::CreateTableManager()
{
	MathStrings data;
	data.resize(mWidth*mHeight);
	//Заменить последний элемент на 1-все остальные
	MathString strTemp, PmaxEquiv = _T("(1");
	MathString Pmax = boost::str(boost::format(_T("p%1%"))%(mWidth - 1));
	//MathCalc::MathStrings::Format(Pmax,Pmax.size(),_T("p%d"), mWidth - 1);
	for (int i = 0; i < mWidth - 1; i++ ) 
	{
		strTemp = boost::str(boost::format(_T("-p%1%"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("-p%d"), i);
		PmaxEquiv += strTemp;
	}
	PmaxEquiv += _T(")");

	MathString Pjj = _T("(1-pch+pch*pj)");
	MathString Pij = _T("pch*pj");

	for (int i = 0; i < mHeight; i++ )
		for (int j = 0; j < mWidth; j++ ) 
		{
			MathString strLine = boost::str(boost::format(_T("p%1%*"))%i);
			//MathCalc::MathStrings::Format(strLine,strLine.size(),_T("p%d*"),i);
			strLine += (i == j)?Pjj:Pij;
			strTemp=boost::str(boost::format(_T("%1%"))%j);
			//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("%d"), j);
			boost::replace_all(strLine,_T("j"), strTemp);
			if (i==mHeight-1||j==mWidth-1)
				boost::replace_all(strLine,Pmax, PmaxEquiv);
			data[i*mWidth+j]=strLine;
		}
	std::vector<Tsc::Formula::CMathExpString<MathString> > tempdata(data.begin(),data.end());
	TModelManager *rez=new TModelManager(&tempdata.front(),mWidth,mHeight);
	return rez;
}
