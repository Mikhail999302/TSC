#include <C:\boost\boost_1_89_0\boost_1_89_0\boost\algorithm\string/replace.hpp>
#include "stdafx.h"
#include "TSCModel.h"
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
MathCalc::MathString GroupedTSC::CTSCModel::SetModelStrRep(const TModelManager&_table)
{
	MathCalc::MathString strModelDefinition,strLine,strTmp;
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
			strLine+=(MathCalc::MathString)(_table[i]);
			strLine+=_T(";\n");
			strModelDefinition += strLine;
		}
	}
	return strModelDefinition;
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
doubles GroupedTSC::CTSCModel::SetSample(const TSampleManager& _table)
{
	int len = _table.GetHeight()*_table.GetWidth();
	doubles rez(len);
	for(int j=0; j<len; ++j)
		rez[j] = _table[j];
	return rez;
}

MathCalc::MathString GroupedTSC::CTSCModel::GenerateRange(void)
{
	if (false) 
	{
		MathString res = boost::str(boost::format(_T("{%1%,%2%}"))%0.%1.);
		//MathCalc::MathStrings::Format(res, res.size(),_T("{%f,%f}"), 0.,1.);
		return res;
	}
	else
		return _T("");
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
MathCalc::MathString GroupedTSC::CTSCModel::GetSumByRow(int i,int size)
{
	MathString n_i_sum,strTemp;
	for ( int j = 0; j < size; j++ ) 
	{
		if(!j)
			strTemp = boost::str(boost::format(_T("n_%1%_%2%"))%i%j);
			//MathCalc::MathStrings::Format(strTemp,strTemp.size(), _T("n_%d_%d"), i,j);
		else 
			strTemp = boost::str(boost::format(_T("+n_%1%_%2%"))%i%j);
			//MathCalc::MathStrings::Format(strTemp,strTemp.size(), _T("+n_%d_%d"), i,j);
		n_i_sum += strTemp;
	}
	return n_i_sum;
}

/*override*/ 
GroupedTSC::IModelRep* GroupedTSC::CTSCModel::Clone()
{
	return new CTSCModel(mCompStruct.mDim, mCompStruct.mType, mLDotCacher,mInfoCacher);
}


bool GroupedTSC::CTSCModel::isOptimize(int idx)
{
	resizeOptimize(idx);
	return m_isOptimize[idx];
}

void GroupedTSC::CTSCModel::setOptimize(int idx, bool val)
{
	resizeOptimize(idx);
	m_isOptimize[idx] = val;
}

void GroupedTSC::CTSCModel::resizeOptimize(int idx)
{
	if (idx>=(int)m_isOptimize.size())
		m_isOptimize.resize(idx+1,true);
}

//*************************************
//****   class CTSCModel_2Pos   *******
//*************************************

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
MathModels::CMathParameterValues GroupedTSC::CTSCModel_2Pos::SetSimpleEstimates(const TSampleManager &_sample)
{
	//check that the sample is square
	_sample.GetHeight() == _sample.GetWidth()?true:throw std::exception("Not same dims of quadratic matrix");
	int dim = _sample.GetHeight();

	//Generate square model representation
	std::auto_ptr<CTSCModelGenerator_2Pos> gen(new CTSCModelGenerator_2Pos(_sample.GetWidth(),_sample.GetHeight()));
	std::auto_ptr<TModelManager> model(gen->CreateTableManager());//задаютс€ p[i][j]

	MathCalc::MathContext::MapIDType mapId = MathCalc::MathContext::AddMap();
	MathCalc::MathContext::SelectMap(mapId);
	//model representation
	ModelInfo initModel;

	//Sample definition
	initModel.sample = SetSample(_sample);
	initModel.sampleSize = (int)std::accumulate(initModel.sample.begin(),initModel.sample.end(),0.0);

	//model parameters definition
	initModel.paramStrRep = SetEstimatesFormulasStrRep(dim);//задаютс€ формулы дл€ начальных оценок параметров
	DataProcessing::ReadEstimates(initModel.paramStrRep,initModel.SFEstimates);
	CMathParameterInfos paramInfos;
	paramInfos.Construct(initModel.SFEstimates);
	for (size_t i=0; i<paramInfos.size(); ++i)
	{
		paramInfos[i].setOptimizible(isOptimize((int)i));
	}

	//model states definition
	initModel.modelStrRep = SetModelStrRep(*model);
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

/*
Definition:
Define the simple estimation of parameters with a use of sample
Args:
_dim - the number of brands
return:
CString - collection of strings that represents \theta=\theta(N), where N is vector of sample.
notes:
Depend on model
*/
#pragma message("2DO: Model dependant version");
MathCalc::MathString GroupedTSC::CTSCModel_2Pos::SetEstimatesFormulasStrRep(int _dim)
{
	MathString strParamEstimates, strLine;
	int i, nMaxI = _dim - 1;

	// Write simple estimates of pi
	MathString strTemp, PiEstimateTemplate = _T("pi = (");
	for ( i = 0; i < nMaxI; i++ ) 
	{
		strTemp = boost::str(boost::format(_T("n_i_%1%+"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(), _T("n_i_%d+"), i);
		PiEstimateTemplate += strTemp;
	}
	strTemp = boost::str(boost::format(_T("n_i_%1%) / n$;\n"))%i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(), _T("n_i_%d) / n$;\n"), i);
	PiEstimateTemplate += strTemp;

	for ( i = 0; i < nMaxI; i++ ) 
	{
		strLine = PiEstimateTemplate;
		strTemp = boost::str(boost::format(_T("%1%"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("%d"), i);
		boost::replace_all(strLine,_T("i"), strTemp);
		boost::replace_all(strLine,_T("$"), GenerateRange());
		strParamEstimates += strLine;
	}

	// Write simple estimates of pch and padv and px

	//тут ввод€тс€ доп. переменные, которые часто фигурируют в оценках,
	MathString n_i_0_sum, n_0_i_sum, n_i_1_sum, n_1_i_sum;
	for ( i = 0; i < nMaxI; i++ ) 
	{
		strTemp = boost::str(boost::format(_T("n_%1%_0+"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_0+"), i);
		n_i_0_sum += strTemp;
		strTemp = boost::str(boost::format(_T("n_0_%1%+"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_0_%d+"), i);
		n_0_i_sum += strTemp;

		strTemp = boost::str(boost::format(_T("n_%1%_1+"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_1+"), i);
		n_i_1_sum += strTemp;
		strTemp = boost::str(boost::format(_T("n_1_%1%+"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_1_%d+"), i);
		n_1_i_sum += strTemp;

	}
	strTemp = boost::str(boost::format(_T("n_%1%_0"))%i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_0"), i);
	n_i_0_sum += strTemp;
	strTemp = boost::str(boost::format(_T("n_0_%1%"))%i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_0_%d"), i);
	n_0_i_sum += strTemp;

	strTemp = boost::str(boost::format(_T("n_%1%_1"))%i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_1"), i);
	n_i_1_sum += strTemp;
	strTemp = boost::str(boost::format(_T("n_1_%1%"))%i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_1_%d"), i);
	n_1_i_sum += strTemp;


	//write estimate for pch
	strParamEstimates+=_T("pch=n*(");
	for(int i=2;i<_dim;++i)
	{
		for(int j=2;j<_dim;++j)
		{
			if(i==j)continue;
			if(i==2&&j==3)
				strTemp = boost::str(boost::format(_T("n_%1%_%2%"))%i%j);
				//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_%d"),i,j);

			else 
				strTemp = boost::str(boost::format(_T("+n_%1%_%2%"))%i%j);
				//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("+n_%d_%d"),i,j);
			strParamEstimates+=strTemp;
		}
	}
	strParamEstimates+=_T(")/(");
	for(int i=2;i<_dim;++i)
	{
		for(int j=2;j<_dim;++j)
		{
			if(i==j)continue;
			if(i==2&&j==3)	strTemp=_T(" (")+GetSumByRow(i,_dim)+_T(")*(")+GetSumByRow(j,_dim)+_T(")");
			else			strTemp=_T("+(")+GetSumByRow(i,_dim)+_T(")*(")+GetSumByRow(j,_dim)+_T(")");
			strParamEstimates+=strTemp;
		}
	}
	strParamEstimates+=_T(")*(n-(") + n_0_i_sum +_T("+") + n_1_i_sum + _T("))");
	strParamEstimates+=_T("/(n-(") + n_i_0_sum +_T("+") + n_i_1_sum + _T("))");
	strParamEstimates+=GenerateRange()+_T(";\n");

	//write estimate for padv
	MathCalc::MathString tempPadv = _T("(") + n_i_0_sum +_T("+") + n_i_1_sum + _T("-(") + n_0_i_sum +_T("+") + n_1_i_sum +  
		_T(")) / (n - (") + n_0_i_sum +_T("+") + n_1_i_sum + _T("))");

	strParamEstimates += _T("padv = ") + tempPadv + GenerateRange()+  _T(";\n");

	//write estimate for px
	strParamEstimates += _T("px = (") + n_i_0_sum + _T("- (1-") + tempPadv +_T(") * (") + 
		n_0_i_sum + _T(")) / n/(") + tempPadv + _T(")")+ GenerateRange()+  _T(";\n");


	//std::ofstream out("FORMULAS FOR SIMPLE ESTIMATES.txt");
	//cout << "\n\n\n\********   HERE ARE FORMULAS FOR SIMPLE ESTIMATES:\n\n" << strParamEstimates << "\n********\n\n";

////¬ыше находитс€ автоматическа€ генераци€ формул. «десь они просто заданы дл€ случа€ 4 продуктов. 
//if(_dim!=4) throw("Wrong number of products in the test example");
//strParamEstimates+=_T("p0 = (n_0_0+n_0_1+n_0_2+n_0_3) / n;\n");
//strParamEstimates+=_T("p1 = (n_1_0+n_1_1+n_1_2+n_1_3) / n;\n");
//strParamEstimates+=_T("p2 = (n_2_0+n_2_1+n_2_2+n_2_3) / n;\n");
//strParamEstimates+=_T("pch = n*(n_2_3+n_3_2) / (2 * (n_2_0+n_2_1+n_2_2+n_2_3)*(n_3_0+n_3_1+n_3_2+n_3_3)) * (n-(n_0_0+n_0_1+n_0_2+n_0_3+n_1_0+n_1_1+n_1_2+n_1_3)) / (n-(n_0_0+n_1_0+n_2_0+n_3_0+n_0_1+n_1_1+n_2_1+n_3_1));\n");
//strParamEstimates+=_T("px = (n_0_0+n_1_0+n_2_0+n_3_0 - (1-((n_2_0+n_3_0+n_2_1+n_3_1-(n_0_2+n_0_3+n_1_2+n_1_3)) / (n - (n_0_0+n_0_1+n_0_2+n_0_3+n_1_0+n_1_1+n_1_2+n_1_3))))*(n_0_0+n_0_1+n_0_2+n_0_3)) / n / ((n_2_0+n_3_0+n_2_1+n_3_1-(n_0_2+n_0_3+n_1_2+n_1_3)) / (n - (n_0_0+n_0_1+n_0_2+n_0_3+n_1_0+n_1_1+n_1_2+n_1_3)));\n");
//strParamEstimates+=_T("padv = (n_2_0+n_3_0+n_2_1+n_3_1-(n_0_2+n_0_3+n_1_2+n_1_3)) / (n - (n_0_0+n_0_1+n_0_2+n_0_3+n_1_0+n_1_1+n_1_2+n_1_3));\n");
//

    //cout<<strParamEstimates;
	return strParamEstimates;
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
MathCalc::MathString GroupedTSC::CTSCModel_2Pos::FixSimpleEstimates(const MathModels::CMathParameterValues& _simpleEstimates,bool _isMatrix/*=false*/)
{
	//¬ставл€ем в качестве начальных значени€ простой оценки не группированной модели
	//Each estimate is just a numbler. But as ECalc checks that simple estimates should contain n_ij, I had to add 
	//fake zero to estimates
	MathString strParamEstimates, strLine;
	int i=0, nMaxI = (int)_simpleEstimates.size()- 3;

	MathString strElem;
	if(_isMatrix)strElem="n_0_0";
	//else strElem="n_0";
	// Write simple estimates of pi
	for ( i = 0; i < nMaxI; i++ ) 
	{
		//strLine.Format("p%d=(%f+1)*(%s/(%s+1)+1/(%s+1))-1$;\n",i,_simpleEstimates[i].Value,strElem,strElem,strElem);
		if(_simpleEstimates[i].Value>0)
			strLine = boost::str(boost::format(_T("p%1%=%2%$;\n"))%i%_simpleEstimates[i].Value);
		//MathCalc::MathStrings::Format(strLine,strLine.size(),"p%d=%f$;\n",i,_simpleEstimates[i].Value);
		else 
			strLine = boost::str(boost::format(_T("p%1%=0.0001$;\n"))%i);
		//MathCalc::MathStrings::Format(strLine,strLine.size(),"p%d=0.0001$;\n",i);
		boost::replace_all(strLine,_T("$"), GenerateRange());
		strParamEstimates += strLine;
	}

	//Remade model without pch.
	if(_simpleEstimates[i].Value>0)
		//strLine.Format("pch=(%f+1)*(%s/(%s+1)+1/(%s+1))-1$;\n",_simpleEstimates[i].Value,strElem,strElem,strElem);
		strLine = boost::str(boost::format(_T("pch=%1%$;\n"))%_simpleEstimates[i].Value);
	//MathCalc::MathStrings::Format(strLine,strLine.size(),"pch=%f$;\n",_simpleEstimates[i].Value);
	else 
		strLine = boost::str(boost::format(_T("pch=0.0001$;\n")));
	//MathCalc::MathStrings::Format(strLine,strLine.size(),"pch=0.0001$;\n");
	//else
	//strLine.Format("pch=(%f+1+0.000001)*(%s/(%s+1)+1/(%s+1))-1$;\n",_simpleEstimates[i].Value,strElem,strElem,strElem);
	boost::replace_all(strLine,_T("$"), GenerateRange());
	strParamEstimates += strLine;
	++i;

	//strLine.Format("padv=(%f+1)*(%s/(%s+1)+1/(%s+1))-1$;\n",_simpleEstimates[i].Value,strElem,strElem,strElem);
	strLine = boost::str(boost::format(_T("padv=%1%$;\n"))%_simpleEstimates[i].Value);
	boost::replace_all(strLine,_T("$"), GenerateRange());
	strParamEstimates += strLine;
	++i;

	strLine = boost::str(boost::format(_T("px=%1%$;\n"))%_simpleEstimates[i].Value);
	//MathCalc::MathStrings::Format(strLine,strLine.size(),"padv=%f$;\n",_simpleEstimates[i].Value);
	boost::replace_all(strLine,_T("$"), GenerateRange());
	strParamEstimates += strLine;
	//std::cout<<__FUNCTION__ << " \n  here are simple estimates\n" <<std::endl;
	//std::cout<<strParamEstimates;
	return strParamEstimates;
}

/*override*/ 
GroupedTSC::TModelManager* GroupedTSC::CTSCModel_2Pos::CreateModelManager(int _width, int _height)
{
	std::auto_ptr<CTSCModelGenerator_2Pos> gen(new CTSCModelGenerator_2Pos(_width,_height));
	return gen->CreateTableManager();
}


//*************************************
//****   class CTSCModel_1Pos   *******
//модель с одной положительной рекламамой
//*************************************


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
MathModels::CMathParameterValues GroupedTSC::CTSCModel_1Pos::SetSimpleEstimates(const TSampleManager &_sample)
{
	//check that the sample is square
	_sample.GetHeight() == _sample.GetWidth()?true:throw std::exception("Not same dims of quadratic matrix");
	int dim = _sample.GetHeight();

	//Generate square model representation
	std::auto_ptr<CTSCModelGenerator_1Pos> gen(new CTSCModelGenerator_1Pos(_sample.GetWidth(),_sample.GetHeight()));
	std::auto_ptr<TModelManager> model(gen->CreateTableManager());

	MathCalc::MathContext::MapIDType mapId = MathCalc::MathContext::AddMap();
	MathCalc::MathContext::SelectMap(mapId);
	//model representation
	ModelInfo initModel;

	//Sample definition
	initModel.sample = SetSample(_sample);
	initModel.sampleSize = (int)std::accumulate(initModel.sample.begin(),initModel.sample.end(),0.0);

	//model parameters definition
	initModel.paramStrRep = SetEstimatesFormulasStrRep(dim);
	DataProcessing::ReadEstimates(initModel.paramStrRep,initModel.SFEstimates);
	CMathParameterInfos paramInfos;
	paramInfos.Construct(initModel.SFEstimates);
	for (size_t i=0; i<paramInfos.size(); ++i)
	{
		paramInfos[i].setOptimizible(isOptimize((int)i));
	}

	//model states definition
	initModel.modelStrRep = SetModelStrRep(*model);
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
MathCalc::MathString GroupedTSC::CTSCModel_1Pos::SetEstimatesFormulasStrRep(int _dim)
{
	MathString strParamEstimates, strLine;
	int i, nMaxI = _dim - 1;

	// Write simple estimates of pi
	MathString strTemp, PiEstimateTemplate = _T("pi = (");
	for ( i = 0; i < nMaxI; i++ ) 
	{
		strTemp = boost::str(boost::format(_T("n_i_%1%+"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(), _T("n_i_%d+"), i);
		PiEstimateTemplate += strTemp;
	}
	strTemp = boost::str(boost::format(_T("n_i_%1%) / n$;\n"))%i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(), _T("n_i_%d) / n$;\n"), i);
	PiEstimateTemplate += strTemp;

	for ( i = 0; i < nMaxI; i++ ) 
	{
		strLine = PiEstimateTemplate;
		strTemp = boost::str(boost::format(_T("%1%"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("%d"), i);
		boost::replace_all(strLine,_T("i"), strTemp);
		boost::replace_all(strLine,_T("$"), GenerateRange());
		strParamEstimates += strLine;
	}

	// Write simple estimates of pch and padv
	MathString n_i_0_sum, n_0_i_sum;
	for ( i = 1; i < nMaxI; i++ ) 
	{
		strTemp = boost::str(boost::format(_T("n_%1%_0+"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_0+"), i);
		n_i_0_sum += strTemp;
		strTemp = boost::str(boost::format(_T("n_0_%1%+"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_0_%d+"), i);
		n_0_i_sum += strTemp;
	}
	strTemp = boost::str(boost::format(_T("n_%1%_0"))%i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_0"), i);
	n_i_0_sum += strTemp;
	strTemp = boost::str(boost::format(_T("n_0_%1%"))%i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_0_%d"), i);
	n_0_i_sum += strTemp;
	if (_dim == 2) {
		strParamEstimates += _T("pch = n * n_0_1 / ((n_0_0 + n_0_1) * (n - n_0_0 - n_1_0));\n");
	}
	else {
		strParamEstimates += _T("pch=n*(");
		for (int i = 1; i < _dim; ++i)
		{
			for (int j = 1; j < _dim; ++j)

			{
				if (i == j)continue;
				if (i == 1 && j == 2)
					strTemp = boost::str(boost::format(_T("n_%1%_%2%")) % i % j);
				//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_%d"),i,j);

				else
					strTemp = boost::str(boost::format(_T("+n_%1%_%2%")) % i % j);
				//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("+n_%d_%d"),i,j);
				strParamEstimates += strTemp;
			}
		}
		strParamEstimates += _T(")/(");
		for (int i = 1; i < _dim; ++i)
		{
			for (int j = 1; j < _dim; ++j)
			{
				if (i == j)continue;
				if (i == 1 && j == 2)	strTemp = _T(" (") + GetSumByRow(i, _dim) + _T(")*(") + GetSumByRow(j, _dim) + _T(")");
				else			strTemp = _T("+(") + GetSumByRow(i, _dim) + _T(")*(") + GetSumByRow(j, _dim) + _T(")");
				strParamEstimates += strTemp;
			}
		}
		strParamEstimates += _T(")*(n-(n_0_0+") + n_0_i_sum + _T("))");

		strParamEstimates += _T("/(n-(n_0_0+") + n_i_0_sum + _T("))");
		strParamEstimates += GenerateRange() + _T(";\n");
	}
	strParamEstimates += _T("padv = (") + n_i_0_sum + _T("-(") + n_0_i_sum + 
		_T(")) / (n - (n_0_0+") + n_0_i_sum + _T("))") +GenerateRange()+  _T(";\n");
	return strParamEstimates;
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
MathCalc::MathString GroupedTSC::CTSCModel_1Pos::FixSimpleEstimates(const CMathParameterValues& _simpleEstimates,bool _isMatrix/*=false*/)
{
	//¬ставл€ем в качестве начальных значени€ простой оценки не группированной модели
	//Each estimate is just a numbler. But as ECalc checks that simple estimates should contain n_ij, I had to add 
	//fake zero to estimates
	MathString strParamEstimates, strLine;
	int i=0, nMaxI = (int)_simpleEstimates.size()- 2;

	MathString strElem;
	if(_isMatrix)strElem="n_0_0";
	//else strElem="n_0";
	// Write simple estimates of pi
	for ( i = 0; i < nMaxI; i++ ) 
	{
		//strLine.Format("p%d=(%f+1)*(%s/(%s+1)+1/(%s+1))-1$;\n",i,_simpleEstimates[i].Value,strElem,strElem,strElem);
		if(_simpleEstimates[i].Value>0)
			strLine = boost::str(boost::format(_T("p%1%=%2%$;\n"))%i%_simpleEstimates[i].Value);
			//MathCalc::MathStrings::Format(strLine,strLine.size(),"p%d=%f$;\n",i,_simpleEstimates[i].Value);
		else 
			strLine = boost::str(boost::format(_T("p%1%=0.0001$;\n"))%i);
			//MathCalc::MathStrings::Format(strLine,strLine.size(),"p%d=0.0001$;\n",i);
		boost::replace_all(strLine,_T("$"), GenerateRange());
		strParamEstimates += strLine;
	}

	//Remade model without pch.
	if(_simpleEstimates[i].Value>0)
		//strLine.Format("pch=(%f+1)*(%s/(%s+1)+1/(%s+1))-1$;\n",_simpleEstimates[i].Value,strElem,strElem,strElem);
		strLine = boost::str(boost::format(_T("pch=%1%$;\n"))%_simpleEstimates[i].Value);
		//MathCalc::MathStrings::Format(strLine,strLine.size(),"pch=%f$;\n",_simpleEstimates[i].Value);
	else 
		strLine = boost::str(boost::format(_T("pch=0.0001$;\n")));
		//MathCalc::MathStrings::Format(strLine,strLine.size(),"pch=0.0001$;\n");
	//else
		//strLine.Format("pch=(%f+1+0.000001)*(%s/(%s+1)+1/(%s+1))-1$;\n",_simpleEstimates[i].Value,strElem,strElem,strElem);
	boost::replace_all(strLine,_T("$"), GenerateRange());
	strParamEstimates += strLine;
	++i;
	//strLine.Format("padv=(%f+1)*(%s/(%s+1)+1/(%s+1))-1$;\n",_simpleEstimates[i].Value,strElem,strElem,strElem);
	strLine = boost::str(boost::format(_T("padv=%1%$;\n"))%_simpleEstimates[i].Value);
	//MathCalc::MathStrings::Format(strLine,strLine.size(),"padv=%f$;\n",_simpleEstimates[i].Value);

	boost::replace_all(strLine,_T("$"), GenerateRange());
	strParamEstimates += strLine;
	//std::cout<<strParamEstimates;
	return strParamEstimates;
}

/*override*/ 
GroupedTSC::TModelManager* GroupedTSC::CTSCModel_1Pos::CreateModelManager(int _width, int _height)
{
	std::auto_ptr<CTSCModelGenerator_1Pos> gen(new CTSCModelGenerator_1Pos(_width,_height));
	return gen->CreateTableManager();
}

//*************************************
//****   class CTSCModel_1Pos   *******
//модель с одной положительной рекламамой
// дл€ новой модели
//*************************************

MathModels::CMathParameterValues GroupedTSC::CTSCModel_1Pos_mine::SetSimpleEstimates(const TSampleManager& _sample)
{
	//check that the sample is square
	_sample.GetHeight() == _sample.GetWidth() ? true : throw std::exception("Not same dims of quadratic matrix");
	int dim = _sample.GetHeight();

	//Generate square model representation
	std::auto_ptr<CTSCModelGenerator_1Pos_mine> gen(new CTSCModelGenerator_1Pos_mine(_sample.GetWidth(), _sample.GetHeight()));
	std::auto_ptr<TModelManager> model(gen->CreateTableManager());

	MathCalc::MathContext::MapIDType mapId = MathCalc::MathContext::AddMap();
	MathCalc::MathContext::SelectMap(mapId);
	//model representation
	ModelInfo initModel;

	//Sample definition
	initModel.sample = SetSample(_sample);
	initModel.sampleSize = (int)std::accumulate(initModel.sample.begin(), initModel.sample.end(), 0.0);

	//model parameters definition
	initModel.paramStrRep = SetEstimatesFormulasStrRep(dim);
	DataProcessing::ReadEstimates(initModel.paramStrRep, initModel.SFEstimates);
	CMathParameterInfos paramInfos;
	paramInfos.Construct(initModel.SFEstimates);
	for (size_t i = 0; i < paramInfos.size(); ++i)
	{
		paramInfos[i].setOptimizible(isOptimize((int)i));
	}

	//model states definition
	initModel.modelStrRep = SetModelStrRep(*model);
	DataProcessing::ReadExpressions(initModel.modelStrRep, initModel.states, _T("p"));

	//Model initialization and parameters calculation
	//MathContext::ResetMap();
	initModel.model = CreateModel(CBaseDiscreteModel::mt2D);
	//2DO:Indicator
	//Note: Cacher is not needed: ldot and InfoMatrix is not being calculated here!
	initModel.model->Init(initModel.states, paramInfos,
		MathModels::CCacherComparingStruct(0, 0),
		MathModels::TLDotCacher(),
		MathModels::TInfoCacher(),
		DataProcessing::CMathOperationIndicator(), false);

	//Generate Simple estimation for square model to use them for grouped after
	if (!initModel.model->CalcSimpleEstimates(initModel.sample, initModel.sampleSize,
		initModel.SFEstimates, initModel.simpleEstimates))
		throw std::exception("Can't calculate simple estimates\n");

	string dumpStr = _T("Simple Estimates values:\n");
	string filterStr = _T("SimpleEsti");
	Dumper::CDump::GetDumper()->Dump(dumpStr, filterStr);
	int paramSize = (int)initModel.simpleEstimates.size();
	for (int i = 0; i < paramSize; ++i)
	{
		Dumper::CDump::GetDumper()->Dump(initModel.simpleEstimates[i].GetParameterName().c_str(),
			(int)initModel.simpleEstimates[i].GetParameterName().size(), filterStr.c_str(), filterStr.size());
		Dumper::CDump::GetDumper()->Dump(initModel.simpleEstimates[i].Value, filterStr.c_str(), filterStr.size());
		Dumper::CDump::GetDumper()->NewLine(filterStr.c_str(), filterStr.size());
	}
	MathCalc::MathContext::RemoveMap(mapId);
	return initModel.simpleEstimates;
}

MathCalc::MathString GroupedTSC::CTSCModel_1Pos_mine::SetEstimatesFormulasStrRep(int _dim)
{
	MathString strParamEstimates, strLine;
	int i, nMaxI = _dim - 1;

	// Write simple estimates of pi
	MathString strTemp, PiEstimateTemplate = _T("pi = (");
	for (i = 0; i < nMaxI; i++)
	{
		strTemp = boost::str(boost::format(_T("n_i_%1%+")) % i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(), _T("n_i_%d+"), i);
		PiEstimateTemplate += strTemp;
	}
	strTemp = boost::str(boost::format(_T("n_i_%1%) / n$;\n")) % i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(), _T("n_i_%d) / n$;\n"), i);
	PiEstimateTemplate += strTemp;

	for (i = 0; i < nMaxI; i++)
	{
		strLine = PiEstimateTemplate;
		strTemp = boost::str(boost::format(_T("%1%")) % i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("%d"), i);
		boost::replace_all(strLine, _T("i"), strTemp);
		boost::replace_all(strLine, _T("$"), GenerateRange());
		strParamEstimates += strLine;
	}

	// Write simple estimates of pch and padv
	MathString n_i_0_sum, n_0_i_sum;
	for (i = 1; i < nMaxI; i++)
	{
		strTemp = boost::str(boost::format(_T("n_%1%_0+")) % i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_0+"), i);
		n_i_0_sum += strTemp;
		strTemp = boost::str(boost::format(_T("n_0_%1%+")) % i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_0_%d+"), i);
		n_0_i_sum += strTemp;
	}
	strTemp = boost::str(boost::format(_T("n_%1%_0")) % i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_0"), i);
	n_i_0_sum += strTemp;
	strTemp = boost::str(boost::format(_T("n_0_%1%")) % i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_0_%d"), i);
	n_0_i_sum += strTemp;

	strParamEstimates += _T("pch=n*(");
	//for (int i = 1; i < _dim; ++i)
	//{
	//	for (int j = 1; j < _dim; ++j)
	//	{
	//		if (i == j)continue;
	//		if (i == 1 && j == 2)
	//			strTemp = boost::str(boost::format(_T("n_%1%_%2%")) % i % j);
	//		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_%d"),i,j);

	//		else
	//			strTemp = boost::str(boost::format(_T("+n_%1%_%2%")) % i % j);
	//		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("+n_%d_%d"),i,j);
	//		strParamEstimates += strTemp;
	//	}
	//}
	strParamEstimates += n_i_0_sum;
	strParamEstimates += _T(")/((n-(");
	strParamEstimates += GetSumByRow(0, _dim);
	strParamEstimates += _T("))*(");
	strParamEstimates += GetSumByRow(0, _dim);
	strParamEstimates += _T("))-((");
	strParamEstimates += n_i_0_sum + _T(")-(") + n_0_i_sum + _T("))/(") + GetSumByRow(0, _dim) + _T(");\n");
	/*for (int i = 1; i < _dim; ++i)
	{
		for (int j = 1; j < _dim; ++j)
		{
			if (i == j)continue;
			if (i == 1 && j == 2)	strTemp = _T(" (") + GetSumByRow(i, _dim) + _T(")*(") + GetSumByRow(j, _dim) + _T(")");
			else			strTemp = _T("+(") + GetSumByRow(i, _dim) + _T(")*(") + GetSumByRow(j, _dim) + _T(")");
			strParamEstimates += strTemp;
		}
	}
	strParamEstimates += _T(")*(n-(n_0_0+") + n_0_i_sum + _T("))");

	strParamEstimates += _T("/(n-(n_0_0+") + n_i_0_sum + _T("))");
	strParamEstimates += GenerateRange() + _T(";\n");

	strParamEstimates += _T("padv = (") + n_i_0_sum + _T("-(") + n_0_i_sum +
		_T(")) / (n - (n_0_0+") + n_0_i_sum + _T("))") + GenerateRange() + _T(";\n");*/
	MathString sS0 = _T("(") + GetSumByRow(0, _dim) + _T(")");
	MathString sA = _T("(") + n_i_0_sum + _T(")");
	MathString sB = _T("(") + n_0_i_sum + _T(")");
	MathString sDiff = _T("(") + sA + _T("-") + sB + _T(")");
	MathString sNum = _T("(") + sS0 + _T("*") + sDiff + _T(")");
	MathString sDen = _T("(") + sNum + _T("+n*") + sB + _T(")");
	strParamEstimates += _T("padv = ") + sNum + _T("/") + sDen + _T(";\n");
	//strParamEstimates += _T("padv = 1;\n");
	return strParamEstimates;
}

MathCalc::MathString GroupedTSC::CTSCModel_1Pos_mine::FixSimpleEstimates(const CMathParameterValues& _simpleEstimates, bool _isMatrix/*=false*/)
{
	//¬ставл€ем в качестве начальных значени€ простой оценки не группированной модели
	//Each estimate is just a numbler. But as ECalc checks that simple estimates should contain n_ij, I had to add 
	//fake zero to estimates
	MathString strParamEstimates, strLine;
	int i = 0, nMaxI = (int)_simpleEstimates.size() - 2;

	MathString strElem;
	if (_isMatrix)strElem = "n_0_0";
	//else strElem="n_0";
	// Write simple estimates of pi
	for (i = 0; i < nMaxI; i++)
	{
		//strLine.Format("p%d=(%f+1)*(%s/(%s+1)+1/(%s+1))-1$;\n",i,_simpleEstimates[i].Value,strElem,strElem,strElem);
		if (_simpleEstimates[i].Value > 0)
			strLine = boost::str(boost::format(_T("p%1%=%2%$;\n")) % i % _simpleEstimates[i].Value);
		//MathCalc::MathStrings::Format(strLine,strLine.size(),"p%d=%f$;\n",i,_simpleEstimates[i].Value);
		else
			strLine = boost::str(boost::format(_T("p%1%=0.0001$;\n")) % i);
		//MathCalc::MathStrings::Format(strLine,strLine.size(),"p%d=0.0001$;\n",i);
		boost::replace_all(strLine, _T("$"), GenerateRange());
		strParamEstimates += strLine;
	}

	//Remade model without pch.
	if (_simpleEstimates[i].Value > 0)
		//strLine.Format("pch=(%f+1)*(%s/(%s+1)+1/(%s+1))-1$;\n",_simpleEstimates[i].Value,strElem,strElem,strElem);
		strLine = boost::str(boost::format(_T("pch=%1%$;\n")) % _simpleEstimates[i].Value);
	//MathCalc::MathStrings::Format(strLine,strLine.size(),"pch=%f$;\n",_simpleEstimates[i].Value);
	else
		strLine = boost::str(boost::format(_T("pch=0.0001$;\n")));
	//MathCalc::MathStrings::Format(strLine,strLine.size(),"pch=0.0001$;\n");
//else
	//strLine.Format("pch=(%f+1+0.000001)*(%s/(%s+1)+1/(%s+1))-1$;\n",_simpleEstimates[i].Value,strElem,strElem,strElem);
	boost::replace_all(strLine, _T("$"), GenerateRange());
	strParamEstimates += strLine;
	++i;
	//strLine.Format("padv=(%f+1)*(%s/(%s+1)+1/(%s+1))-1$;\n",_simpleEstimates[i].Value,strElem,strElem,strElem);
	strLine = boost::str(boost::format(_T("padv=%1%$;\n")) % _simpleEstimates[i].Value);
	//MathCalc::MathStrings::Format(strLine,strLine.size(),"padv=%f$;\n",_simpleEstimates[i].Value);

	boost::replace_all(strLine, _T("$"), GenerateRange());
	strParamEstimates += strLine;
	//std::cout<<strParamEstimates;
	return strParamEstimates;
}

GroupedTSC::TModelManager* GroupedTSC::CTSCModel_1Pos_mine::CreateModelManager(int _width, int _height)
{
	std::auto_ptr<CTSCModelGenerator_1Pos_mine> gen(new CTSCModelGenerator_1Pos_mine(_width, _height));
	return gen->CreateTableManager();
}



//*************************************
//****   class CTSCModel_1Neg   *******
//модель с одной отрицательной рекламой
//*************************************


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
MathModels::CMathParameterValues GroupedTSC::CTSCModel_1Neg::SetSimpleEstimates(const TSampleManager &_sample)
{
	//check that the sample is square
	_sample.GetHeight() == _sample.GetWidth()?true:throw std::exception("Not same dims of quadratic matrix");
	int dim = _sample.GetHeight();

	//Generate square model representation
	std::auto_ptr<CTSCModelGenerator_1Neg> gen(new CTSCModelGenerator_1Neg(_sample.GetWidth(),_sample.GetHeight()));
	std::auto_ptr<TModelManager> model(gen->CreateTableManager());

	MathCalc::MathContext::MapIDType mapId = MathCalc::MathContext::AddMap();
	MathCalc::MathContext::SelectMap(mapId);
	//model representation
	ModelInfo initModel;

	//Sample definition
	initModel.sample = SetSample(_sample);
	initModel.sampleSize = (int)std::accumulate(initModel.sample.begin(),initModel.sample.end(),0.0);

	//model parameters definition
	initModel.paramStrRep = SetEstimatesFormulasStrRep(dim);
	DataProcessing::ReadEstimates(initModel.paramStrRep,initModel.SFEstimates);
	CMathParameterInfos paramInfos;
	paramInfos.Construct(initModel.SFEstimates);
	for (size_t i=0; i<paramInfos.size(); ++i)
	{
		paramInfos[i].setOptimizible(isOptimize((int)i));
	}

	//model states definition
	initModel.modelStrRep = SetModelStrRep(*model);
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
MathCalc::MathString GroupedTSC::CTSCModel_1Neg::SetEstimatesFormulasStrRep(int _dim)
{
	MathString strParamEstimates, strLine;
	int i, nMaxI = _dim - 1;

	// Write simple estimates of pi
	MathString strTemp, PiEstimateTemplate = _T("pi = (");
	for ( i = 0; i < nMaxI; i++ ) 
	{
		strTemp = boost::str(boost::format(_T("n_i_%1%+"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(), _T("n_i_%d+"), i);
		PiEstimateTemplate += strTemp;
	}
	strTemp = boost::str(boost::format(_T("n_i_%1%) / n$;\n"))%i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(), _T("n_i_%d) / n$;\n"), i);
	PiEstimateTemplate += strTemp;

	for ( i = 0; i < nMaxI; i++ ) 
	{
		strLine = PiEstimateTemplate;
		strTemp = boost::str(boost::format(_T("%1%"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("%d"), i);
		boost::replace_all(strLine,_T("i"), strTemp);
		boost::replace_all(strLine,_T("$"), GenerateRange());
		strParamEstimates += strLine;
	}

	// Write simple estimates of pch and padv
	MathString n_i_0_sum, n_0_i_sum;
	n_i_0_sum += _T("(");
	n_0_i_sum += _T("(");
	for ( i = 0; i < nMaxI; i++ ) 
	{
		strTemp = boost::str(boost::format(_T("n_%1%_0+"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_0+"), i);
		n_i_0_sum += strTemp;
		strTemp = boost::str(boost::format(_T("n_0_%1%+"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_0_%d+"), i);
		n_0_i_sum += strTemp;
	}
	strTemp = boost::str(boost::format(_T("n_%1%_0)"))%i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_0"), i);
	n_i_0_sum += strTemp;
	strTemp = boost::str(boost::format(_T("n_0_%1%)"))%i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_0_%d"), i);
	n_0_i_sum += strTemp;


	MathString tempPadv;
	tempPadv += _T("(") + n_0_i_sum + _T("-") + n_i_0_sum + _T(") / ") + n_0_i_sum;

	strParamEstimates+=_T("pch=n*( (");
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
	strParamEstimates+=_T(")* (n-") + n_0_i_sum +_T(") /(");
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
	strParamEstimates+=_T(")+(")+n_i_0_sum+_T("-")+n_0_i_sum+_T(") / ")+n_0_i_sum+_T(")");
	strParamEstimates+=_T("*") + n_0_i_sum+_T(" / ") + n_i_0_sum + _T(" / (n-")+n_0_i_sum+_T(")");
	strParamEstimates+=GenerateRange()+_T(";\n");

	strParamEstimates += _T("padv = (") + n_0_i_sum + _T("-") + n_i_0_sum + 
		_T(") / ") + n_0_i_sum  +GenerateRange()+  _T(";\n");

	//std::ofstream out("FORMULAS FOR SIMPLE ESTIMATES.txt");
	//cout << "\n\n\n\********   HERE ARE FORMULAS FOR SIMPLE ESTIMATES:\n\n" << strParamEstimates << "\n********\n\n";

	return strParamEstimates;
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
MathCalc::MathString GroupedTSC::CTSCModel_1Neg::FixSimpleEstimates(const CMathParameterValues& _simpleEstimates,bool _isMatrix/*=false*/)
{
	//¬ставл€ем в качестве начальных значени€ простой оценки не группированной модели
	//Each estimate is just a numbler. But as ECalc checks that simple estimates should contain n_ij, I had to add 
	//fake zero to estimates
	MathString strParamEstimates, strLine;
	int i=0, nMaxI = (int)_simpleEstimates.size()- 2;

	MathString strElem;
	if(_isMatrix)strElem="n_0_0";
	//else strElem="n_0";
	// Write simple estimates of pi
	for ( i = 0; i < nMaxI; i++ ) 
	{
		//strLine.Format("p%d=(%f+1)*(%s/(%s+1)+1/(%s+1))-1$;\n",i,_simpleEstimates[i].Value,strElem,strElem,strElem);
		if(_simpleEstimates[i].Value>0)
			strLine = boost::str(boost::format(_T("p%1%=%2%$;\n"))%i%_simpleEstimates[i].Value);
			//MathCalc::MathStrings::Format(strLine,strLine.size(),"p%d=%f$;\n",i,_simpleEstimates[i].Value);
		else 
			strLine = boost::str(boost::format(_T("p%1%=0.0001$;\n"))%i);
			//MathCalc::MathStrings::Format(strLine,strLine.size(),"p%d=0.0001$;\n",i);
		boost::replace_all(strLine,_T("$"), GenerateRange());
		strParamEstimates += strLine;
	}

	//Remade model without pch.
	if(_simpleEstimates[i].Value>0)
		//strLine.Format("pch=(%f+1)*(%s/(%s+1)+1/(%s+1))-1$;\n",_simpleEstimates[i].Value,strElem,strElem,strElem);
		strLine = boost::str(boost::format(_T("pch=%1%$;\n"))%_simpleEstimates[i].Value);
		//MathCalc::MathStrings::Format(strLine,strLine.size(),"pch=%f$;\n",_simpleEstimates[i].Value);
	else 
		strLine = boost::str(boost::format(_T("pch=0.0001$;\n")));
		//MathCalc::MathStrings::Format(strLine,strLine.size(),"pch=0.0001$;\n");
	//else
		//strLine.Format("pch=(%f+1+0.000001)*(%s/(%s+1)+1/(%s+1))-1$;\n",_simpleEstimates[i].Value,strElem,strElem,strElem);
	boost::replace_all(strLine,_T("$"), GenerateRange());
	strParamEstimates += strLine;
	++i;
	//strLine.Format("padv=(%f+1)*(%s/(%s+1)+1/(%s+1))-1$;\n",_simpleEstimates[i].Value,strElem,strElem,strElem);
	strLine = boost::str(boost::format(_T("padv=%1%$;\n"))%_simpleEstimates[i].Value);
	//MathCalc::MathStrings::Format(strLine,strLine.size(),"padv=%f$;\n",_simpleEstimates[i].Value);

	boost::replace_all(strLine,_T("$"), GenerateRange());
	strParamEstimates += strLine;
	//std::cout<<strParamEstimates;
	return strParamEstimates;
}

/*override*/ 
GroupedTSC::TModelManager* GroupedTSC::CTSCModel_1Neg::CreateModelManager(int _width, int _height)
{
	std::auto_ptr<CTSCModelGenerator_1Neg> gen(new CTSCModelGenerator_1Neg(_width,_height));
	return gen->CreateTableManager();
}



//****************************************
//****   class CTSCModel_2NegPos   *******
//модель с двум€ рекламами "-" и "+" *****
//****************************************
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
MathModels::CMathParameterValues GroupedTSC::CTSCModel_2NegPos::SetSimpleEstimates(const TSampleManager &_sample)
{
	//check that the sample is square
	_sample.GetHeight() == _sample.GetWidth()?true:throw std::exception("Not same dims of quadratic matrix");
	int dim = _sample.GetHeight();

	//Generate square model representation
	std::auto_ptr<CTSCModelGenerator_2NegPos> gen(new CTSCModelGenerator_2NegPos(_sample.GetWidth(),_sample.GetHeight()));
	std::auto_ptr<TModelManager> model(gen->CreateTableManager());//задаютс€ p[i][j]

	MathCalc::MathContext::MapIDType mapId = MathCalc::MathContext::AddMap();
	MathCalc::MathContext::SelectMap(mapId);
	//model representation
	ModelInfo initModel;

	//Sample definition
	initModel.sample = SetSample(_sample);
	initModel.sampleSize = (int)std::accumulate(initModel.sample.begin(),initModel.sample.end(),0.0);

	//model parameters definition
	initModel.paramStrRep = SetEstimatesFormulasStrRep(dim);//задаютс€ формулы дл€ начальных оценок параметров
	DataProcessing::ReadEstimates(initModel.paramStrRep,initModel.SFEstimates);
	CMathParameterInfos paramInfos;
	paramInfos.Construct(initModel.SFEstimates);
	for (size_t i=0; i<paramInfos.size(); ++i)
	{
		paramInfos[i].setOptimizible(isOptimize((int)i));
	}

	//model states definition
	initModel.modelStrRep = SetModelStrRep(*model);
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

/*
Definition:
Define the simple estimation of parameters with a use of sample
Args:
_dim - the number of brands
return:
CString - collection of strings that represents \theta=\theta(N), where N is vector of sample.
notes:
Depend on model
*/
#pragma message("2DO: Model dependant version");
MathCalc::MathString GroupedTSC::CTSCModel_2NegPos::SetEstimatesFormulasStrRep(int _dim)
{
	MathString strParamEstimates, strLine;
	int i, nMaxI = _dim - 1;

	// Write simple estimates of pi
	MathString strTemp, PiEstimateTemplate = _T("pi = (");
	for ( i = 0; i < nMaxI; i++ ) 
	{
		strTemp = boost::str(boost::format(_T("n_i_%1%+"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(), _T("n_i_%d+"), i);
		PiEstimateTemplate += strTemp;
	}
	strTemp = boost::str(boost::format(_T("n_i_%1%) / n$;\n"))%i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(), _T("n_i_%d) / n$;\n"), i);
	PiEstimateTemplate += strTemp;

	for ( i = 0; i < nMaxI; i++ ) 
	{
		strLine = PiEstimateTemplate;
		strTemp = boost::str(boost::format(_T("%1%"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("%d"), i);
		boost::replace_all(strLine,_T("i"), strTemp);
		boost::replace_all(strLine,_T("$"), GenerateRange());
		strParamEstimates += strLine;
	}

	// Write simple estimates of pch and padv and px
	//тут ввод€тс€ доп. переменные, которые часто фигурируют в оценках,
	MathString n_i_0_sum, n_0_i_sum, n_i_1_sum, n_1_i_sum;
	n_i_0_sum += _T("(");
	n_0_i_sum += _T("(");
	n_i_1_sum += _T("(");
	n_1_i_sum += _T("(");
	for ( i = 0; i < nMaxI; i++ ) 
	{
		strTemp = boost::str(boost::format(_T("n_%1%_0+"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_0+"), i);
		n_i_0_sum += strTemp;
		strTemp = boost::str(boost::format(_T("n_0_%1%+"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_0_%d+"), i);
		n_0_i_sum += strTemp;

		strTemp = boost::str(boost::format(_T("n_%1%_1+"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_1+"), i);
		n_i_1_sum += strTemp;
		strTemp = boost::str(boost::format(_T("n_1_%1%+"))%i);
		//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_1_%d+"), i);
		n_1_i_sum += strTemp;

	}
	strTemp = boost::str(boost::format(_T("n_%1%_0 )"))%i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_0"), i);
	n_i_0_sum += strTemp;
	strTemp = boost::str(boost::format(_T("n_0_%1% )"))%i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_0_%d"), i);
	n_0_i_sum += strTemp;

	strTemp = boost::str(boost::format(_T("n_%1%_1 )"))%i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_1"), i);
	n_i_1_sum += strTemp;
	strTemp = boost::str(boost::format(_T("n_1_%1% )"))%i);
	//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_1_%d"), i);
	n_1_i_sum += strTemp;


	//help to write estimate for pch
	MathCalc::MathString tempPadv = _T("(") + n_0_i_sum + _T("-") + n_i_0_sum + _T(") / ") + n_0_i_sum;
	MathCalc::MathString tempPx = _T(" (1-(") + n_i_1_sum +_T("-")+ n_1_i_sum + _T("-n_0_1 + n_1_0)*")+ n_0_i_sum+
									+ _T("/(")+n_0_i_sum +_T("-") + n_i_0_sum + _T(") / (n-")+ n_0_i_sum +_T("-")+ n_1_i_sum +_T(") )");

	//write estimate for pch
	strParamEstimates+=_T("pch=(n*(");
	for(int i=2;i<_dim;++i)
	{
		for(int j=2;j<_dim;++j)
		{
			if(i==j)continue;
			if(i==2&&j==3)
				strTemp = boost::str(boost::format(_T("n_%1%_%2%"))%i%j);
				//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("n_%d_%d"),i,j);

			else 
				strTemp = boost::str(boost::format(_T("+n_%1%_%2%"))%i%j);
				//MathCalc::MathStrings::Format(strTemp,strTemp.size(),_T("+n_%d_%d"),i,j);
			strParamEstimates+=strTemp;
		}
	}
	strParamEstimates+=_T(")/(");
	for(int i=2;i<_dim;++i)
	{
		for(int j=2;j<_dim;++j)
		{
			if(i==j)continue;
			if(i==2&&j==3)	strTemp=_T(" (")+GetSumByRow(i,_dim)+_T(")*(")+GetSumByRow(j,_dim)+_T(")");
			else			strTemp=_T("+(")+GetSumByRow(i,_dim)+_T(")*(")+GetSumByRow(j,_dim)+_T(")");
			strParamEstimates+=strTemp;
		}
	}
	strParamEstimates+=_T(")-") + tempPadv + _T("*0.3")/*+tempPx */ + _T(") / (1-(") + tempPadv +_T(") )/ (1-")+n_0_i_sum +_T("/n)");
	strParamEstimates+=GenerateRange()+_T(";\n");

	//write estimate for padv
	
	strParamEstimates += _T("padv = ") + tempPadv + GenerateRange()+  _T(";\n");

	//write estimate for px
	strParamEstimates += _T("px = ") + tempPx + GenerateRange()+  _T(";\n");


	std::ofstream out("FORMULAS FOR SIMPLE ESTIMATES_2NegPos.txt");
	out << "\n\n\n\********   HERE ARE FORMULAS FOR SIMPLE ESTIMATES:\n\n" << strParamEstimates << "\n********\n\n";


	return strParamEstimates;
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
MathCalc::MathString GroupedTSC::CTSCModel_2NegPos::FixSimpleEstimates(const MathModels::CMathParameterValues& _simpleEstimates,bool _isMatrix/*=false*/)
{
	//¬ставл€ем в качестве начальных значени€ простой оценки не группированной модели
	//Each estimate is just a numbler. But as ECalc checks that simple estimates should contain n_ij, I had to add 
	//fake zero to estimates
	MathString strParamEstimates, strLine;
	int i=0, nMaxI = (int)_simpleEstimates.size()- 3;

	MathString strElem;
	if(_isMatrix)strElem="n_0_0";
	//else strElem="n_0";
	// Write simple estimates of pi
	for ( i = 0; i < nMaxI; i++ ) 
	{
		//strLine.Format("p%d=(%f+1)*(%s/(%s+1)+1/(%s+1))-1$;\n",i,_simpleEstimates[i].Value,strElem,strElem,strElem);
		if(_simpleEstimates[i].Value>0)
			strLine = boost::str(boost::format(_T("p%1%=%2%$;\n"))%i%_simpleEstimates[i].Value);
		//MathCalc::MathStrings::Format(strLine,strLine.size(),"p%d=%f$;\n",i,_simpleEstimates[i].Value);
		else 
			strLine = boost::str(boost::format(_T("p%1%=0.0001$;\n"))%i);
		//MathCalc::MathStrings::Format(strLine,strLine.size(),"p%d=0.0001$;\n",i);
		boost::replace_all(strLine,_T("$"), GenerateRange());
		strParamEstimates += strLine;
	}

	//Remade model without pch.
	if(_simpleEstimates[i].Value>0)
		//strLine.Format("pch=(%f+1)*(%s/(%s+1)+1/(%s+1))-1$;\n",_simpleEstimates[i].Value,strElem,strElem,strElem);
		strLine = boost::str(boost::format(_T("pch=%1%$;\n"))%_simpleEstimates[i].Value);
	//MathCalc::MathStrings::Format(strLine,strLine.size(),"pch=%f$;\n",_simpleEstimates[i].Value);
	else 
		strLine = boost::str(boost::format(_T("pch=0.0001$;\n")));
	//MathCalc::MathStrings::Format(strLine,strLine.size(),"pch=0.0001$;\n");
	//else
	//strLine.Format("pch=(%f+1+0.000001)*(%s/(%s+1)+1/(%s+1))-1$;\n",_simpleEstimates[i].Value,strElem,strElem,strElem);
	boost::replace_all(strLine,_T("$"), GenerateRange());
	strParamEstimates += strLine;
	++i;

	//strLine.Format("padv=(%f+1)*(%s/(%s+1)+1/(%s+1))-1$;\n",_simpleEstimates[i].Value,strElem,strElem,strElem);
	strLine = boost::str(boost::format(_T("padv=%1%$;\n"))%_simpleEstimates[i].Value);
	boost::replace_all(strLine,_T("$"), GenerateRange());
	strParamEstimates += strLine;
	++i;

	strLine = boost::str(boost::format(_T("px=%1%$;\n"))%_simpleEstimates[i].Value);
	//MathCalc::MathStrings::Format(strLine,strLine.size(),"padv=%f$;\n",_simpleEstimates[i].Value);
	boost::replace_all(strLine,_T("$"), GenerateRange());
	strParamEstimates += strLine;
	//std::cout<<__FUNCTION__ << " \n  here are simple estimates\n" <<std::endl;
	//std::cout<<strParamEstimates;
	return strParamEstimates;
}

/*override*/ 
GroupedTSC::TModelManager* GroupedTSC::CTSCModel_2NegPos::CreateModelManager(int _width, int _height)
{
	std::auto_ptr<CTSCModelGenerator_2NegPos> gen(new CTSCModelGenerator_2NegPos(_width,_height));
	return gen->CreateTableManager();
}

