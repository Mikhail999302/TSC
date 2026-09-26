#include "stdafx.h"
#include <C:\diploma\2012\TSC\boost/algorithm/string/replace.hpp>
#include "TSCTrunkModel.h"
/*
Definition:
Create Model string representation for current model table manager

args:
_table - Model manager used

return:
CString - Model's string representation for each state

algorithm:
1. Check the dimension of model manager. 
2. Creates p_i(j)=table[i(,j)]

notes:
Independ on model
*/
MathCalc::MathString GroupedTSC::CTSCTrunkModel::SetModelStrRep(const TModelManager&_table)
{
	MathString strModelDefinition,strLine,strTmp;
	if(_table.GetHeight()>1)//table view
	{
		for(int i=0;i<_table.GetWidth();++i)
		{
			for(int j=0;j<_table.GetHeight();++j)
			{
				strLine = boost::str(boost::format(_T("p[%1%][%2%] ="))%j%i);
				//MathCalc::MathStrings::Format(strLine,strLine.size(),_T("p[%d][%d] ="),j,i);
				strLine+=_table[j*_table.GetWidth()+i];
				strLine+=_T(";\n");
				strModelDefinition += strLine;
			}
		}
	}
	else//vector view
	{
		for(int i=0;i<_table.GetWidth();++i)
		{
			strLine = boost::str(boost::format(_T("p[%1%] ="))%i);
			//MathCalc::MathStrings::Format(strLine,strLine.size(),_T("p[%d] ="),i);
			strLine+=(MathString)(_table[i]);
			strLine+=_T(";\n");
			strModelDefinition += strLine;
		}
	}
	return strModelDefinition;
}

/*
Definition:
Define the simple estimation of parameters with a use of sample
Args:
_dim - the namber of brands
return:
CString - collection of strings that represents \theta=\theta(N), where N is vector of sample.
notes:
Depend on model
*/
#pragma message("2DO: Model dependant version")
MathCalc::MathString GroupedTSC::CTSCTrunkModel::SetEstimatesFormulasStrRep(int _dim)
{
	MathString strParamEstimates, strLine;
	int i, nMaxI = _dim - 1;

	// Write simple estimates of pi
	MathString strTemp, PiEstimateTemplate = _T("pi = (");
	for ( i = 0; i < nMaxI; i++ ) 
	{
		strTemp = boost::str(boost::format(_T("n_i_%1%+"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_i_%d+"), i);
		PiEstimateTemplate += strTemp;
	}
	strTemp = boost::str(boost::format(_T("n_i_%1%) / n$;\n"))%i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_i_%d) / n$;\n"), i);
	PiEstimateTemplate += strTemp;

	for ( i = 0; i < nMaxI; i++ ) 
	{
		strLine = PiEstimateTemplate;
		strTemp=boost::str(boost::format(_T("%1%"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("%d"), i);		
		boost::replace_all(strLine,_T("i"), strTemp);
		boost::replace_all(strLine,_T("$"), GenerateRange());
		strParamEstimates += strLine;
	}

	// Write simple estimates of pch and padv
	MathString n_i_0_sum, n_0_i_sum;
	for ( i = 1; i < nMaxI; i++ ) 
	{
		strTemp=boost::str(boost::format(_T("n_%1%_0+"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_0+"), i);
		n_i_0_sum += strTemp;
		strTemp=boost::str(boost::format(_T("n_0_%1%+"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_0_%d+"), i);
		n_0_i_sum += strTemp;
	}
	strTemp=boost::str(boost::format(_T("n_%1%_0"))%i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_0"), i);
	n_i_0_sum += strTemp;
	strTemp=boost::str(boost::format(_T("n_0_%1%"))%i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_0_%d"), i);
	n_0_i_sum += strTemp;

	strParamEstimates+=_T("pch=n*(");
	for(int i=1;i<_dim;++i)
	{
		for(int j=1;j<_dim;++j)
		{
			if(i==j)continue;
			if(i==1&&j==2)
				strTemp = boost::str(boost::format(_T("n_%1%_%2%"))%i%j);
				//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_%d"),i,j);
			else 
				strTemp = boost::str(boost::format(_T("+n_%1%_%2%"))%i%j);
				//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("+n_%d_%d"),i,j);
			strParamEstimates+=strTemp;
		}
	}
	strParamEstimates+=_T(")/(");
	for(int i=1;i<_dim;++i)
	{
		for(int j=1;j<_dim;++j)
		{
			if(i==j)continue;
			if(i==1&&j==2)	strTemp=_T(" (")+GetSumByRow(i,_dim)+_T(")*(")+GetSumByRow(j,_dim)+_T(")");
			else			strTemp=_T("+(")+GetSumByRow(i,_dim)+_T(")*(")+GetSumByRow(j,_dim)+_T(")");
			strParamEstimates+=strTemp;
		}
	}
	strParamEstimates+=_T(")*(n-(n_0_0+") + n_0_i_sum + _T("))");
	strParamEstimates+=_T("/(n-(n_0_0+") + n_i_0_sum + _T("))");
	strParamEstimates+=GenerateRange()+_T(";\n");
	return strParamEstimates;
}

/*
Definition:
Creates vector of doubles from table manager.
Args:
_table - table manager with groupping or not sample
return:
doubles - vector of doubles with sample
notes:
independ on model
*/
doubles GroupedTSC::CTSCTrunkModel::SetSample(const TSampleManager& _table)
{
	int len = _table.GetHeight()*_table.GetWidth();
	doubles rez(len);
	for(int j=0; j<len; ++j)
		rez[j] = _table[j];
	return rez;
}

MathCalc::MathString GroupedTSC::CTSCTrunkModel::GenerateRange(void)
{
	if (false) 
	{
		MathCalc::MathString res=boost::str(boost::format(_T("{%f,%f}"))%0.%1.);
		//MathCalc::MathStrings::Format(res, res.size(),_T("{%f,%f}"), 0.,1.);
		return res;
	}
	else
		return _T("");
}

/*
Definition:
Creates vector of doubles from table manager.
Args:
_table - table manager with groupping or not sample
return:
CString - collection of strings that represents \theta=\theta(N), where N is vector of sample.
notes:
Depend on model
*/
MathCalc::MathString GroupedTSC::CTSCTrunkModel::FixSimpleEstimates(const CMathParameterValues& _simpleEstimates,bool _isMatrix/*=false*/)
{
	//Вставляем в качестве начальных значения простой оценки не группированной модели
	//Each estimate is just a numbler. But as ECalc checks that simple estimates should contain n_ij, I had to add 
	//fake zero to estimates
	MathString strParamEstimates, strLine;
	int i=0, nMaxI = (int)_simpleEstimates.size() - 1;

	MathString strElem;
	if(_isMatrix)strElem = "n_0_0";
	else strElem = "n_0";
	// Write simple estimates of pi
	for ( i = 0; i < nMaxI; i++ ) 
	{
		strLine = boost::str(boost::format(_T("p%1%=(%2%+1)*(%3%/(%4%+1)+1/(%5%+1))-1$;\n"))%i%_simpleEstimates[i].Value%strElem%strElem%strElem);
		//MathCalc::MathStrings::Format(strLine,strLine.size(),"p%d=(%f+1)*(%s/(%s+1)+1/(%s+1))-1$;\n",i,_simpleEstimates[i].Value,strElem,strElem,strElem);
		boost::replace_all(strLine,_T("$"), GenerateRange());
		strParamEstimates += strLine;
	}
	strLine = boost::str(boost::format(_T("pch=(%1%+1)*(%2%/(%3%+1)+1/(%4%+1))-1$;\n"))
		%_simpleEstimates[i].Value%strElem%strElem%strElem);
	//MathCalc::MathStrings::Format(strLine,strLine.size(),"pch=(%f+1)*(%s/(%s+1)+1/(%s+1))-1$;\n",_simpleEstimates[i].Value,strElem,strElem,strElem);
	boost::replace_all(strLine,_T("$"), GenerateRange());
	strParamEstimates += strLine;
	//std::cout<<strParamEstimates;
	return strParamEstimates;
}

/*
Definition:
Helper function - sum sample by row.
Args:
i - index of a row.
size - number in a column
return:
CString - string with sum of sample representation for current row
notes:
Depend on model
*/
MathCalc::MathString GroupedTSC::CTSCTrunkModel::GetSumByRow(int i,int size)
{
	MathString n_i_sum,strTemp;
	for ( int j = 0; j < size; j++ ) 
	{
		if(!j)
			strTemp = boost::str(boost::format(_T("n_%1%_%2%"))%i%j);
			//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_%d"), i,j);
		else
			strTemp = boost::str(boost::format(_T("+n_%1%_%2%"))%i%j);
			//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("+n_%d_%d"), i,j);
		n_i_sum += strTemp;
	}
	return n_i_sum;
}

/*override*/ 
GroupedTSC::IModelRep* GroupedTSC::CTSCTrunkModel::Clone()
{
	return new CTSCTrunkModel(mCompStruct.mDim, mCompStruct.mType, mLDotCacher,mInfoCacher);
}

/*override*/ 
GroupedTSC::TModelManager* GroupedTSC::CTSCTrunkModel::CreateModelManager(int _width, int _height)
{
	std::auto_ptr<CTSCTrunkModelGenerator> gen(new CTSCTrunkModelGenerator(_width,_height));
	return gen->CreateTableManager();
}

/*implement*/ 
/*
definition:
This method calculates simple estimates for 2d model of CTSCModelGenerator type.

args:
_sample - manager of sample, use sample to calculate simple estimates
_modelRep - fake parameter for now to extract procedures for calculation 

return:
values and names of simple estimates.

algorithm:
1. Check that we have deal with matrix model
2. Create new Model generator of model type and generate related model manager
3. Set new Map model for counting parameters
4. Create 2D model, read expressions and fixed simple estimates.
5. Calculate simple estimates
*/
MathModels::CMathParameterValues GroupedTSC::CTSCTrunkModel::SetSimpleEstimates(
	const TSampleManager &_sample/*, IModelRep * _modelRep*/ )
{
	//check that the sample is square
	_sample.GetHeight() == _sample.GetWidth()?true:throw std::exception("Not same dims of quadratic matrix");
	int dim = _sample.GetHeight();

	//Generate square model representation
	std::auto_ptr<CTSCModelGenerator> gen(new CTSCModelGenerator(_sample.GetWidth(),_sample.GetHeight()));
	std::auto_ptr<TModelManager> model(gen->CreateTableManager());

	MathCalc::MathContext::MapIDType mapId = MathCalc::MathContext::AddMap();
	MathCalc::MathContext::SelectMap(mapId);
	//model representation
	ModelInfo initModel;

	//Sample definition
	initModel.sample = /*_modelRep->*/SetSample(_sample);
	initModel.sampleSize = (int)std::accumulate(initModel.sample.begin(),initModel.sample.end(),0.0);

	//model parameters definition
	initModel.paramStrRep = /*_modelRep->*/SetEstimatesFormulasStrRep(dim);
	DataProcessing::ReadEstimates(initModel.paramStrRep,initModel.SFEstimates);
	CMathParameterInfos paramInfos;
	paramInfos.Construct(initModel.SFEstimates);
	for(int i=0; i<(int)paramInfos.size(); ++i)
	{
		paramInfos[i].setOptimizible(isOptimize(i));
	}

	//model states definition
	initModel.modelStrRep = /*_modelRep->*/SetModelStrRep(*model);
	DataProcessing::ReadExpressions(initModel.modelStrRep,initModel.states,_T("p"));

	//Model initialization and parameters calculation
	//MathContext::ResetMap();
	initModel.model = CreateModel(CBaseDiscreteModel::mt2D);
	//2DO:Indicator
	//Note: Cacher is not needed: ldot and InfoMatrix is not being calculated here!
	initModel.model->Init(initModel.states, paramInfos, 
							MathModels::CCacherComparingStruct(0,0),
							MathModels::TLDotCacher(),
							MathModels::TInfoCacher(),
							DataProcessing::CMathOperationIndicator(),false);

	//Generate Simple estimation for square model to use them for grouped after
	if(!initModel.model->CalcSimpleEstimates(initModel.sample,initModel.sampleSize,
		initModel.SFEstimates,initModel.simpleEstimates))
		throw std::exception("Can't calculate simple estimates\n");

	string dumpStr=_T("Simple Estimates values:\n");
	string filterStr=_T("SimpleEsti");
	Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
	int paramSize=(int)initModel.simpleEstimates.size();
	for(int i=0;i<paramSize;++i)
	{
		Dumper::CDump::GetDumper()->Dump(initModel.simpleEstimates[i].GetParameterName().c_str(),
			(int)initModel.simpleEstimates[i].GetParameterName().size(),filterStr.c_str(),filterStr.size());
		Dumper::CDump::GetDumper()->Dump(initModel.simpleEstimates[i].Value,filterStr.c_str(),filterStr.size());
		Dumper::CDump::GetDumper()->NewLine(filterStr.c_str(),filterStr.size());
	}
	MathCalc::MathContext::RemoveMap(mapId);
	return initModel.simpleEstimates;
}