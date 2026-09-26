#include "stdafx.h"
#include "Model.h"
#include "TSCModel.h"

GroupedTSC::CModel* GroupedTSC::CModel::CreateCModel(const std::string & _sampleFile, 
													 const int * _groupping, 
													 IModelRep * _modelRep)
{
	std::auto_ptr<TSampleManager> 
		sample(Tsc::TableManager::CTableManagerFactory<double>::CreateFromFile(_sampleFile.c_str()));

	sample->GetWidth() != sample->GetHeight() ? true : throw std::exception("Not correct dims");
	std::auto_ptr<TModelManager> modelDef(_modelRep->CreateModelManager(sample->GetWidth(),sample->GetHeight()));

	MathCalc::MathString paramStrRep = _modelRep->FixSimpleEstimates(_modelRep->SetSimpleEstimates(*sample));
	_modelRep->GetComparator().mDim = sample->GetWidth();

	return new CModel(	sample->GetWidth(),
						sample->CreateJoinCells(_groupping),
						modelDef->CreateJoinCells(_groupping),
						paramStrRep, 
						_modelRep);
}


//Create non groupped model from the sample
TSCCALC_DLLENTRY GroupedTSC::CModel*  GroupedTSC::CModel::CreateCModel(const std::string &_sampleFile, 
																	   bool isGroupBrand, 
																	   IModelRep * _modelRep)
{
	TSampleManager* sample = Tsc::TableManager::CTableManagerFactory<double>::CreateFromFile(_sampleFile.c_str());
	_modelRep->GetComparator().mDim = sample->GetWidth();
	return CreateCModel(sample->GetWidth(),
						sample,
						isGroupBrand,
						_modelRep);
}

TSCCALC_DLLENTRY GroupedTSC::CModel*  GroupedTSC::CModel::CreateCModel(	int _dim,
																		TSampleManager* _sampleManager, 
																		bool _isGroupBrand, 
																		IModelRep * _modelRep)
{
	try
	{
		std::auto_ptr<TSampleManager> sample(_sampleManager);
		sample->GetWidth() == sample->GetHeight() ? true : throw std::exception("Not correct dims");

		CMathParameterValues simpleEstimates = _modelRep->SetSimpleEstimates(*sample);

		//create simple groupping
		PLevel::uints uGroupping(sample->GetWidth());
		for (int i=0; i<sample->GetWidth(); ++i)
			uGroupping[i]=i;

		//If there is small brands, then it could be groupped together
		//But there are should be at least 3 brands.
		if(_isGroupBrand)
		{
			groupBrands(_modelRep,simpleEstimates, sample, uGroupping);
		}
		vector<int> groupping(uGroupping.begin(),uGroupping.end());

		//create model string representation
		std::auto_ptr<TModelManager> modelDef(_modelRep->CreateModelManager(sample->GetWidth(),sample->GetHeight()));

		//Calculate simple estimates for simple original model
		MathCalc::MathString paramStrRep = _modelRep->FixSimpleEstimates(simpleEstimates);
		_modelRep->GetComparator().mDim = _dim;
		
		return new CModel(	_dim,
							sample->CreateJoinCrosses(&groupping.front(),true),
							modelDef->CreateJoinCrosses(&groupping.front(),true),
							paramStrRep,
							_modelRep);
	}
	catch(std::exception&/*ex*/)
	{
		throw;
	}
}

GroupedTSC::CModel::CModel(int _dim,
						   TSampleManager *_sample, 
						   TModelManager *_modelDef, 
						   const std::string& _simpleEstimates, 
						   IModelRep * _modelRep):
		mSample(_sample),
		mModelDef(_modelDef),
		mModelRep(_modelRep)
{
	mModel.paramStrRep = _simpleEstimates.c_str();
	mModel.Width = _sample->GetWidth();
	mModel.Height = _sample->GetHeight();
	mModel.m_nDim = _dim;
	InitializeModel();
}

void GroupedTSC::CModel::InitializeModel()
{
	MathCalc::MathContext::ResetMap();
	//1 or 2 dimensional model
	mModel.model = CreateModel(mSample->GetHeight()>1 ? CBaseDiscreteModel::mt2D : CBaseDiscreteModel::mt1D);
	mChi = new PLevel::CPLevel();
}

GroupedTSC::CModel::~CModel(void)
{
	delete mSample;
	delete mModelDef;
	delete mChi;
}

void GroupedTSC::CModel::ProcessInitialCalculation(bool _isInitInfo,
					DataProcessing::CMathOperationIndicator &_indicator /* = CMathOperationIndicator */,
					bool _isBMatrix /*=false*/)
{
	try
	{
		mModel.modelStrRep = mModelRep->SetModelStrRep(*mModelDef);
		////Dumping
		//std::string dumpStr;
		//std::string filterStr=_T("Estimates");
		//dumpStr=_T("Model formulas:\n");
		//Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
		//Dumper::CDump::GetDumper()->Dump(mModel.modelStrRep,mModel.modelStrRep.GetLength(),filterStr.c_str(),(int)filterStr.size());
		//dumpStr="Parameters formulas:\n";
		//Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
		//Dumper::CDump::GetDumper()->Dump(mModel.paramStrRep,mModel.paramStrRep.GetLength(),filterStr.c_str(),(int)filterStr.size());

		//process model
		DataProcessing::ReadExpressions(mModel.modelStrRep,mModel.states,_T("p"));
		DataProcessing::ReadEstimates(mModel.paramStrRep,mModel.SFEstimates);
		CMathParameterInfos paramInfos;
		paramInfos.Construct(mModel.SFEstimates);
		for (size_t i=0; i<paramInfos.size(); ++i)
		{
			paramInfos[i].setOptimizible(mModelRep->isOptimize((int)i));
		}

		mModel.model->Init(mModel.states,
			paramInfos,
			mModelRep->GetComparator(), 
			mModelRep->getLDotCacher(), 
			mModelRep->getInfoCacher(),
			_indicator, 
			_isInitInfo,
			_isBMatrix);

		//CMathParameterInfos paramInfos;
		mModel.sample=mModelRep->SetSample(*mSample);
		mModel.sampleSize=(int)std::accumulate(mModel.sample.begin(),mModel.sample.end(),0.0);

		if(!mModel.model->CalcSimpleEstimates(mModel.sample,mModel.sampleSize,
			mModel.SFEstimates,mModel.simpleEstimates))
			throw std::exception("Can't calculate simple estimates");
		setEstimates();
		setStatesValues();
	}

	catch (std::exception&ex)
	{
		string dumpStr=ex.what();
		string filterStr=_T("Exception");
		Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
		throw;
	}
	catch(CModelInitializationError&)
	{
		string dumpStr=_T("Model initialization error");
		string filterStr=_T("Exception");
		Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
		throw;
	}
	catch (...)
	{
		string dumpStr=_T("Error in calculation");
		string filterStr=_T("Exception");
		Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
		mModel.mPLevelData.plevel=-1;
		mModel.mPLevelData.criterion_value=0;
		mModel.mPLevelData.df=1;
		mModel.mPLevelData.minNPi=0;
		throw;
	}
}

void GroupedTSC::CModel::ProcessEstimatesCalculation(
	DataProcessing::CMathOperationIndicator &_indicator/*=CMathOperationIndicator()*/)
{
	try
	{
		if(!mModel.model->CalcEnhancedEstimates(mModel.sample,mModel.sampleSize,mModel.SFEstimates,
												  mModel.simpleEstimates,0.95)) // 0.95
								throw std::exception("Can't calculate enhanced estimates");
		setEstimates();
	}

	catch (std::exception&ex)
	{
		string dumpStr=ex.what();
		string filterStr=_T("Exception");
		Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
		throw;
	}
	catch(CModelInitializationError&)
	{
		string dumpStr=_T("Model initialization error");
		string filterStr=_T("Exception");
		Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
		throw;
	}
	catch (...)
	{
		string dumpStr=_T("Error in calculation");
		string filterStr=_T("Exception");
		Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
		mModel.mPLevelData.plevel=-1;
		mModel.mPLevelData.criterion_value=0;
		mModel.mPLevelData.df=1;
		mModel.mPLevelData.minNPi=0;
		throw;
	}
}

//tous les mots sont les meme quand ton aime dut parailler plaigne
void GroupedTSC::CModel::ProcessPLevelCalculation(
	DataProcessing::CMathOperationIndicator &_indicator/*=CMathOperationIndicator()*/)
{
	////SimpleEstimates contains enchancedEstimates
	////Dump parameters
	////CString dumpStr = _T("Parameters values:\n");
	////CString filterStr = _T("Estimates");
	////Dumper::CDump::GetDumper()->Dump(dumpStr,dumpStr.GetLength(),filterStr,filterStr.GetLength());

	//mModel.simpleEstimates.SetContextValues();
	////Dump nPij states
	////dumpStr=_T("States values:\n");
	////Dumper::CDump::GetDumper()->Dump(dumpStr,dumpStr.GetLength(),filterStr,filterStr.GetLength());

	//int statesSize = mModel.model->GetStatesCount();
	////CStrings statesNames(statesSize);
	//mModel.statesValues.reserve(statesSize);
	//for(int j=0; j<statesSize; ++j)
	//{
	//	mModel.statesValues.push_back(mModel.model->GetState(j).Eval());
	//	//statesNames[j].Format("np%d=%lf\n", j, mModel.sampleSize*mModel.statesValues.back());
	//}
	////Dumper::CDump::GetDumper()->Dump(&statesNames.front(), mSample->GetWidth(), mSample->GetHeight(), 16,
	////									filterStr, filterStr.GetLength());

	for(size_t i=0; i<mModel.simpleEstimates.size(); ++i)
	{
		if (mModel.simpleEstimates[i].Value<=0||mModel.simpleEstimates[i].Value>=1)
		{
			string dumpStr=_T("Error: estimate not in (0,1)\n");
			string filterStr=_T("Exception");
			Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
			mModel.mPLevelData.df=0;
			mModel.mPLevelData.criterion_value=0;
			mModel.mPLevelData.plevel=-1;
			return;
		}
	}

	setStatesValues();
	mChi->Init(mModel.statesValues, mModel.sampleSize, Tsc::Grouping::NoneGroup<uint,double>());
	mChi->CalcChiValue(mModel.sample, mModel.model->GetStatesCount()-mModel.model->GetNANParametersCount()-1);
	mModel.mPLevelData = mChi->GetPLevelData();
}

GroupedTSC::CModel* GroupedTSC::CModel::CreateSubMatrixModel(int _cacheId, const int *_grouping)
{
	string dumpStr=_T("Groupping matrix\n");
	string filterStr=_T("Groupping");
	Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
	Dumper::CDump::GetDumper()->Dump(_grouping,mSample->GetWidth(),3,filterStr.c_str(),(int)filterStr.size());
	
	IModelRep* copy=mModelRep->Clone();
	copy->GetComparator().mType = _cacheId;

	return new CModel(	mModel.m_nDim,
						mSample->CreateJoinCrosses(_grouping),
						mModelDef->CreateJoinCrosses(_grouping),
						mModel.paramStrRep, 
						copy);
}

GroupedTSC::CModel* GroupedTSC::CModel::CreateSubMatrixModel(Tsc::Grouping::IMergeGrouping<uint,double>& _strategy)
{
	_strategy.setSize(mModel.sampleSize);
	_strategy.setStates(mModel.statesValues);
	_strategy.setLevel(0.0);
	_strategy.setMinSize((int)mModel.estimates.size()+2);
	PLevel::uints tempGroup = _strategy();
	std::vector<int> group(tempGroup.begin(),tempGroup.end());
	return CreateSubMatrixModel(_strategy.getIdentificator(),&group.front());
}

GroupedTSC::CModel* GroupedTSC::CModel::CreateSubModel(int _cacheId, const int *_grouping)const
{
	string dumpStr=_T("Groupping matrix\n");
	string filterStr=_T("Groupping");
	Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
	Dumper::CDump::GetDumper()->Dump(_grouping,mSample->GetWidth(),mSample->GetHeight(),3,filterStr.c_str(),(int)filterStr.size());

	IModelRep* copy=mModelRep->Clone();
	copy->GetComparator().mType = _cacheId;

	return new CModel(	mModel.m_nDim,
						mSample->CreateJoinCells(_grouping),
						mModelDef->CreateJoinCells(_grouping),
						mModel.paramStrRep,
						copy);
}

GroupedTSC::CModel* GroupedTSC::CModel::CreateSubModel(Tsc::Grouping::IMergeGrouping<uint,double>& _strategy)const
{
	_strategy.setSize(mModel.sampleSize);
	_strategy.setStates(mModel.statesValues);
	_strategy.setLevel(0.0);
	_strategy.setMinSize((int)mModel.estimates.size()+2);
	PLevel::uints tempGroup=_strategy();
	std::vector<int> group(tempGroup.begin(),tempGroup.end());

	return CreateSubModel(_strategy.getIdentificator(),&group.front());
}

void GroupedTSC::CModel::groupBrands(IModelRep * _modelRep, 
									 MathModels::CMathParameterValues & _simpleEstimates, 
									 std::auto_ptr<TSampleManager> & _sample,
									 PLevel::uints & _uGroupping)
{
	doubles noneGroupedEstiValues(_simpleEstimates.size()-2);
	for(int i=0; i<(int)_simpleEstimates.size()-2; ++i)
		noneGroupedEstiValues[i] = _simpleEstimates[i].Value;
	noneGroupedEstiValues.push_back(1-std::accumulate(noneGroupedEstiValues.begin(),noneGroupedEstiValues.end(),0.0));
	doubles sampleVal = _modelRep->SetSample(*_sample);
	int sampleSize = (int)std::accumulate(sampleVal.begin(),sampleVal.end(),0.0);
	Tsc::Grouping::AutoGroup<uint,double> diagLimStrategy;
	diagLimStrategy.setSize(sampleSize);
	diagLimStrategy.setStates(noneGroupedEstiValues);
	diagLimStrategy.setLevel(5);
	diagLimStrategy.setMinSize(3);
	_uGroupping = diagLimStrategy();
	vector<int> groupping(_uGroupping.begin(),_uGroupping.end());
	_sample.reset(_sample->CreateJoinCrosses(&groupping.front()));
	_uGroupping.resize(_sample->GetWidth());
	for(int i=0;i<_sample->GetWidth();++i)
		_uGroupping[i]=i;
}

void GroupedTSC::CModel::setStatesValues()
{
	mModel.simpleEstimates.SetContextValues();
	int statesSize = mModel.model->GetStatesCount();
	mModel.statesValues.resize(statesSize);
	for(int j=0; j<statesSize; ++j)
		mModel.statesValues[j]=mModel.model->GetState(j).Eval();
}

void GroupedTSC::CModel::setEstimates()
{
	mModel.simpleEstimates.SetContextValues();
	int paramSize=(int)mModel.simpleEstimates.size();
	mModel.estimates.clear();
	mModel.estimates.reserve(paramSize);
	for(int i=0;i<paramSize;++i)
	{
		mModel.estimates.push_back(CStrDb(mModel.simpleEstimates[i].GetParameterName(),mModel.simpleEstimates[i]));
		std::string filterStr=_T("Estimates");
		Dumper::CDump::GetDumper()->Dump(mModel.simpleEstimates[i].GetParameterName(),filterStr);
		Dumper::CDump::GetDumper()->Dump(mModel.simpleEstimates[i].Value,filterStr.c_str(),(int)filterStr.size());
		Dumper::CDump::GetDumper()->NewLine(filterStr.c_str(),(int)filterStr.size());
	}
}