#pragma once
//This class is supposed to calculate values of Information matrix of a model
//and give the least eigen value of it for different values of parameters
//SetModel
//GetInfoMatrix
//SetParameters
//GetInfoMatrix values
//SVD it.
#include <iostream>
#include <vector>
#include <C:\diploma\2012\TSC\Libs/Grouping/Entry.h>
#include <C:\diploma\2012\TSC\Libs/Formula/Entry.h>
#include <C:\diploma\2012\TSC\Libs/TableManager/Entry.h>
#include "TypeDefs.h"
#include "TSCModelGenerator.h"
using namespace std;
//#include "model.h"
#include "TSCModel.h"
namespace GroupedTSC
{

class TSCCALC_DLLENTRY CInfoMatrixEstimation
{
	CExpressions mExps;
	CExpressions2D mInfoMatrix;
	CMathParameterInfos mParamInfos;
	CMathParameterEstimates mEstimates;
public:
	void setParameters(vector<double> _pi, double _pCh, double _pAdv)
	{
		MathString str;
		MathString tmp;
		for(int i=0; i<(int)_pi.size(); ++i)
		{
			tmp = boost::str(boost::format(_T("p%1%=%2%;\n"))%i%_pi[i]);
			//MathCalc::MathStrings::Format(tmp,tmp.size(),_T("p%d=%lf;\n"),i,_pi[i]);
			str+=tmp;
		}
		tmp = boost::str(boost::format(_T("pch=%1%;\n"))%_pCh);
		//MathCalc::MathStrings::Format(tmp,tmp.size(),_T("pch=%lf;\n"),_pCh);
		str+=tmp;
		tmp = boost::str(boost::format(_T("padv=%1%;\n"))%_pAdv);
		//MathCalc::MathStrings::Format(tmp,tmp.size(),_T("padv=%lf;\n"),_pAdv);
		str+=tmp;
		DataProcessing::ReadEstimates(str,mEstimates);
		mParamInfos.Construct(mEstimates);
	}
	void setInfoMatrix(int _dim, Tsc::Grouping::IMergeGrouping<int,double>& _grouping)
	{
		auto_ptr<CTSCModelGenerator> mInitModel(new CTSCModelGenerator(_dim,_dim));
		auto_ptr<TModelManager> modelManager (mInitModel->CreateTableManager());
		vector<int> grouping = _grouping();
		auto_ptr<TModelManager> groupManager (modelManager->CreateJoinCells(&grouping.front()));
		auto_ptr<IModelRep> modelRep (new CTSCModel(_dim, 1,MathModels::TLDotCacher(), MathModels::TInfoCacher()));
		MathCalc::MathString modelRepStr = modelRep->SetModelStrRep(*groupManager);
		DataProcessing::ReadExpressions(modelRepStr,mExps,_T("p"));
		CExpression Pij_k_derivative;
		int paramSize = (int)mParamInfos.size();
		mInfoMatrix.resize2D(paramSize);
		std::vector<CParameter> NANParams(paramSize);
		for (int i = 0; i < paramSize; ++i)
			NANParams[i] = mParamInfos[i].Parameter;
		for (int k = 0; k < paramSize; ++k ) 
		{
			for (int l = 0; l < paramSize; ++l ) 
			{
				if ( k <= l ) 
				{ 
					// high triangle - full calculations
					mInfoMatrix[k][l] = 0;
					for (int i = 0; i < (int)mExps.size(); ++i )
					{
						Pij_k_derivative.Swap((mExps[i])(NANParams[k]));
						mInfoMatrix[k][l].Add(((mExps[i])(NANParams[l])).Multiply(Pij_k_derivative).
							Divide( mExps[i] ), true ).Subtract((Pij_k_derivative)(NANParams[l]), true );
					}
				}
				else 
				{ 
					// low triangle - light calculations
					mInfoMatrix[k][l] = 0;
				}
			}
		}
		//for(int i=0; i < paramSize; ++i)
		//{
		//	for(int j=0; j < paramSize; ++j)
		//	{
		//		cout<<mInfoMatrix[i][j].ToString()<<endl;
		//	}
		//}
	}
	doubles calcInverse()
	{
		CMathParameterValues paramValues;
		paramValues.Construct(mEstimates);
		int paramSize = (int)mParamInfos.size();
		paramValues.SetContextValues();
		doubles2D I( paramSize );
		for ( int i = 0; i < paramSize; ++i )
		{
			for ( int j = 0; j < paramSize; ++j ) 
			{
				I[i][j] = (i<=j) ? mInfoMatrix[i][j].Eval() : I[j][i];
				//cout<<I[i][j]<<" ";
			}
			//cout<<endl;
		}
		doubles2D IInv(paramSize);
		doubles eig(paramSize);
		CalcInverseMatrix(I,IInv,eig);
		sort(eig.begin(),eig.end());
		//for(int i=0; i<(int)eig.size(); ++i)
		//{
		//	cout<< eig[i]<<" ";
		//}
		return eig;
	}
};
}//GroupedTSC