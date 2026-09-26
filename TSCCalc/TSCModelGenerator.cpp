#include <C:\boost\boost_1_89_0\boost_1_89_0\boost\algorithm\string/replace.hpp>
#include "stdafx.h"
#include "TSCModelGenerator.h"


///////////Model Generator/////////////////////////////////////////////////////////////////////////////

GroupedTSC::CTSCModelGenerator::CTSCModelGenerator(int _width,int _height):mWidth(_width),mHeight(_height)
{
}

GroupedTSC::CTSCModelGenerator::~CTSCModelGenerator(void)
{
}

//**********************************************
//****   class CTSCModelGenerator_2Pos   *******
//генерирует p[i][j] согласно модели CTSCModel_2Pos - 
//модель с двумя положительными рекламами
//**********************************************
GroupedTSC::TModelManager* GroupedTSC::CTSCModelGenerator_2Pos::CreateTableManager()
{
	MathStrings data;
	data.resize(mWidth*mHeight);

	//Заменить последний элемент на 1-все остальные
	MathString strTemp, Pmax,PmaxEquiv = _T("(1");
	Pmax=boost::str(boost::format(_T("p%1%"))%(mWidth-1));
	//MathCalc::MathStrings::Format(Pmax,Pmax.size(),_T("p%d"), mWidth - 1);
	for (int i = 0; i < mWidth - 1; i++ ) 
	{
		strTemp = boost::str(boost::format(_T("-p%1%"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(), _T("-p%d"), i);
		PmaxEquiv += strTemp;
	}
	PmaxEquiv += _T(")");

	MathString P00 = _T("(padv*px + (1-padv)*(1-pch+pch*p0))");
	MathString P11 = _T("(padv*(1-px) + (1-padv)*(1-pch+pch*p1))");
	MathString Pi0 = _T("(padv*px+(1-padv)*pch*p0)");
	MathString Pi1 = _T("(padv*(1-px)+(1-padv)*pch*p1)");
	MathString Pjj = _T("(1-padv)*(1-pch+pch*pj)");
	MathString Pij = _T("(1-padv)*pch*pj");

	MathString strLine;
	//cout << "Test p[i][j]: \n\n";
	for (int i = 0; i < mHeight; i++ )
		for (int j = 0; j < mWidth; j++ ) 
		{
			strLine = boost::str(boost::format(_T("p%1%*"))%i);
			//MathCalc::MathStrings::Format(strLine,strLine.size(),_T("p%d*"),i);
			if ( i == j ) 
			{
				if ( j == 0 )
					strLine += P00;
				else if (j == 1)
						strLine += P11;
				else strLine += Pjj;
			}
			else 
			{
				if ( j == 0 )
					strLine += Pi0;
				else if (j == 1) 
					strLine += Pi1;
				else strLine += Pij;
			}
			strTemp = boost::str(boost::format(_T("%d"))%j);
			//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("%d"), j);
			boost::replace_all(strLine,_T("j"), strTemp);
			if (i==mHeight-1||j==mWidth-1)
				boost::replace_all(strLine,Pmax, PmaxEquiv);
			data[i*mWidth+j]=strLine;
			//cout<<data[i*mWidth+j]<<endl;
		}


////выше находится автоматическая генерация строкового задания модели
////ниже записан результат, который получается для 4х продуктов
//
//if(mHeight!=4 || mWidth!=4) throw ("Wrong number of products in the model generation");
//MathString p[4][4];
//p[0][0] = _T("p0 * (padv * px + (1-padv)*(1-pch+pch*p0))");
//p[0][1] = _T("p0 * (padv * (1-px) + (1-padv)*pch*p1)");
//p[0][2] = _T("p0 * (1-padv)*pch*p2");
//p[0][3] = _T("p0 * (1-padv) * pch *(1-p0-p1-p2)");
//
//p[1][0] = _T("p1 * (padv * px + (1-padv)*pch*p0)");
//p[1][1] = _T("p1 * (padv * (1-px) + (1-padv)*(1-pch+pch*p1))");
//p[1][2] = _T("p1 * (1-padv)*pch*p2");
//p[1][3] = _T("p1 * (1-padv) * pch *(1-p0-p1-p2)");
//
//p[2][0] = _T("p2 * (padv * px + (1-padv)*pch*p0)");
//p[2][1] = _T("p2 * (padv * (1-px) + (1-padv)*pch*p1)");
//p[2][2] = _T("p2 * (1-padv)*(1-pch+pch*p2)");
//p[2][3] = _T("p2 * (1-padv) * pch *(1-p0-p1-p2)");
//
//p[3][0] = _T("(1-p0-p1-p2) * (padv * px + (1-padv)*pch*p0)");
//p[3][1] = _T("(1-p0-p1-p2) * (padv * (1-px) + (1-padv)*pch*p1)");
//p[3][2] = _T("(1-p0-p1-p2) * (1-padv) * pch *p2");
//p[3][3] = _T("(1-p0-p1-p2) * (1-padv)*(1-pch+pch*(1-p0-p1-p2))");

//cout<<endl<<"Test"<<endl;
//for (int i = 0; i < mHeight; i++ )
//	for (int j = 0; j < mWidth; j++ ) 
//	{
//		data[i*mWidth+j]=p[i][j];
//		cout<<data[i*mWidth+j]<<endl;
//	}

	std::vector<Tsc::Formula::CMathExpString<MathString> > tempdata(data.begin(),data.end());
	TModelManager *rez=new TModelManager(&tempdata.front(),mWidth,mHeight);
	return rez;
}


//**********************************************
//****   class CTSCModelGenerator_1Pos   *******
//генерирует p[i][j] согласно модели CTSCModel_1Pos - 
//модель с одной положительной рекламамой
//**********************************************
GroupedTSC::TModelManager* GroupedTSC::CTSCModelGenerator_1Pos::CreateTableManager()
{
	MathStrings data;
	data.resize(mWidth*mHeight);
	//Заменить последний элемент на 1-все остальные
	MathString strTemp, Pmax,PmaxEquiv = _T("(1");
	Pmax=boost::str(boost::format(_T("p%1%"))%(mWidth-1));
	//MathCalc::MathStrings::Format(Pmax,Pmax.size(),_T("p%d"), mWidth - 1);
	for (int i = 0; i < mWidth - 1; i++ ) 
	{
		strTemp = boost::str(boost::format(_T("-p%1%"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(), _T("-p%d"), i);
		PmaxEquiv += strTemp;
	}
	PmaxEquiv += _T(")");

	MathString P00 = _T("(padv + (1-padv)*(1-pch+pch*p0))");
	MathString Pjj = _T("(1-padv)*(1-pch+pch*pj)");
	MathString P0j = _T("(padv + (1-padv)*pch*pj)");
	MathString Pij = _T("(1-padv)*pch*pj");

	MathString strLine;
	for (int i = 0; i < mHeight; i++ )
		for (int j = 0; j < mWidth; j++ ) 
		{
			strLine = boost::str(boost::format(_T("p%1%*"))%i);
			//MathCalc::MathStrings::Format(strLine,strLine.size(),_T("p%d*"),i);
			if ( i == j ) 
			{
				if ( j == 0 )
					strLine += P00;
				else
					strLine += Pjj;
			}
			else 
			{
				if ( j == 0 )
					strLine += P0j;
				else
					strLine += Pij;
			}
			strTemp = boost::str(boost::format(_T("%d"))%j);
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

//**********************************************
//****   class CTSCModelGenerator_1Pos   *******
//генерирует p[i][j] согласно модели CTSCModel_1Pos - 
//модель с одной положительной рекламамой для новой модели
//**********************************************

GroupedTSC::TModelManager* GroupedTSC::CTSCModelGenerator_1Pos_mine::CreateTableManager()
{
	MathStrings data;
	data.resize(mWidth * mHeight);
	//Заменить последний элемент на 1-все остальные
	MathString strTemp, Pmax, PmaxEquiv = _T("(1");
	Pmax = boost::str(boost::format(_T("p%1%")) % (mWidth - 1));
	//MathCalc::MathStrings::Format(Pmax,Pmax.size(),_T("p%d"), mWidth - 1);
	for (int i = 0; i < mWidth - 1; i++)
	{
		strTemp = boost::str(boost::format(_T("-p%1%")) % i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(), _T("-p%d"), i);
		PmaxEquiv += strTemp;
	}
	PmaxEquiv += _T(")");

	/*MathString P00 = _T("(padv + (1-padv)*(1-pch+pch*p0))");
	MathString Pjj = _T("(1-padv)*(1-pch+pch*pj)");
	MathString P0j = _T("(padv + (1-padv)*pch*pj)");
	MathString Pij = _T("(1-padv)*pch*pj");*/

	MathString P00 = _T("((1-pch)+(pch*padv)+pch*(1-padv)*p0)");
	MathString Pjj = _T("((1-pch)+pch*(1-padv)*pj)");
	MathString P0j = _T("(pch*padv+pch*(1-padv)*p0)");
	MathString Pij = _T("pch*(1-padv)*pj"); 

	MathString strLine;
	for (int i = 0; i < mHeight; i++)
		for (int j = 0; j < mWidth; j++)
		{
			strLine = boost::str(boost::format(_T("p%1%*")) % i);
			//MathCalc::MathStrings::Format(strLine,strLine.size(),_T("p%d*"),i);
			if (i == j)
			{
				if (j == 0)
					strLine += P00;
				else
					strLine += Pjj;
			}
			else
			{
				if (j == 0)
					strLine += P0j;
				else
					strLine += Pij;
			}
			strTemp = boost::str(boost::format(_T("%d")) % j);
			//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("%d"), j);
			boost::replace_all(strLine, _T("j"), strTemp);
			if (i == mHeight - 1 || j == mWidth - 1)
				boost::replace_all(strLine, Pmax, PmaxEquiv);
			data[i * mWidth + j] = strLine;
		}
	std::vector<Tsc::Formula::CMathExpString<MathString> > tempdata(data.begin(), data.end());
	TModelManager* rez = new TModelManager(&tempdata.front(), mWidth, mHeight);
	return rez;
}

//**********************************************
//****   class CTSCModelGenerator_1Neg   *******
//генерирует p[i][j] согласно модели CTSCModel_1Pos - 
//модель с одной положительной рекламамой
//**********************************************
GroupedTSC::TModelManager* GroupedTSC::CTSCModelGenerator_1Neg::CreateTableManager()
{
	MathStrings data;
	data.resize(mWidth*mHeight);
	//Заменить последний элемент на 1-все остальные
	MathString strTemp, Pmax,PmaxEquiv = _T("(1");
	Pmax=boost::str(boost::format(_T("p%1%"))%(mWidth-1));
	//MathCalc::MathStrings::Format(Pmax,Pmax.size(),_T("p%d"), mWidth - 1);
	for (int i = 0; i < mWidth - 1; i++ ) 
	{
		strTemp = boost::str(boost::format(_T("-p%1%"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(), _T("-p%d"), i);
		PmaxEquiv += strTemp;
	}
	PmaxEquiv += _T(")");

	MathString P00 = _T("(1-padv)*(1-pch+pch*p0)");
	MathString Pjj = _T("(padv*pj/(1-p0) + (1-padv)*(1-pch+pch*pj))");
	MathString Pj0 = _T("(1-padv)*pch*pj)");
	MathString Pij = _T("(padv*pj/(1-p0) + (1-padv)*pch*pj)");

	MathString strLine;
	for (int i = 0; i < mHeight; i++ )
		for (int j = 0; j < mWidth; j++ ) 
		{
			strLine = boost::str(boost::format(_T("p%1%*"))%i);
			//MathCalc::MathStrings::Format(strLine,strLine.size(),_T("p%d*"),i);
			if ( i == j ) 
			{
				if ( j == 0 )
					strLine += P00;
				else
					strLine += Pjj;
			}
			else 
			{
				if ( j == 0 )
					strLine += Pj0;
				else
					strLine += Pij;
			}
			strTemp = boost::str(boost::format(_T("%d"))%j);
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



//*************************************************
//****   class CTSCModelGenerator_2NegPos   *******
//модель с двумя рекламами: "-" и "+" *************
//*************************************************
GroupedTSC::TModelManager* GroupedTSC::CTSCModelGenerator_2NegPos::CreateTableManager()
{

	ofstream out("Formulas for p[i][j]_2NegPos.txt");
	MathStrings data;
	data.resize(mWidth*mHeight);

	//Заменить последний элемент на 1-все остальные
	MathString strTemp, Pmax,PmaxEquiv = _T("(1");
	Pmax=boost::str(boost::format(_T("p%1%"))%(mWidth-1));
	//MathCalc::MathStrings::Format(Pmax,Pmax.size(),_T("p%d"), mWidth - 1);
	for (int i = 0; i < mWidth - 1; i++ ) 
	{
		strTemp = boost::str(boost::format(_T("-p%1%"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(), _T("-p%d"), i);
		PmaxEquiv += strTemp;
	}
	PmaxEquiv += _T(")");

	MathString P00 = _T("(1-padv)*(1-pch+pch*p0)");
	MathString P11 = _T("(padv*(1-px + px*p1/(1-p0)) + (1-padv)*(1-pch+pch*p1))");
	MathString Pi0 = _T("(1-padv)*pch*p0");
	MathString Pi1 = _T("(padv*(1-px + px*p1/(1-p0))+(1-padv)*pch*p1)");
	MathString Pjj = _T("(padv*px*pj/(1-p0) + (1-padv)*(1-pch+pch*pj))");
	MathString Pij = _T("(1-padv)*pch*pj");

	MathString strLine;
	cout << "Test p[i][j]: \n\n";
	for (int i = 0; i < mHeight; i++ )
		for (int j = 0; j < mWidth; j++ ) 
		{
			strLine = boost::str(boost::format(_T("p%1%*"))%i);
			//MathCalc::MathStrings::Format(strLine,strLine.size(),_T("p%d*"),i);
			if ( i == j ) 
			{
				if ( j == 0 )
					strLine += P00;
				else if (j == 1)
						strLine += P11;
				else strLine += Pjj;
			}
			else 
			{
				if ( j == 0 )
					strLine += Pi0;
				else if (j == 1) 
					strLine += Pi1;
				else strLine += Pij;
			}
			strTemp = boost::str(boost::format(_T("%d"))%j);
			//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("%d"), j);
			boost::replace_all(strLine,_T("j"), strTemp);
			if (i==mHeight-1||j==mWidth-1)
				boost::replace_all(strLine,Pmax, PmaxEquiv);
			data[i*mWidth+j]=strLine;
			//out<<data[i*mWidth+j]<<endl;
		}


	std::vector<Tsc::Formula::CMathExpString<MathString> > tempdata(data.begin(),data.end());
	TModelManager *rez=new TModelManager(&tempdata.front(),mWidth,mHeight);
	return rez;
}

