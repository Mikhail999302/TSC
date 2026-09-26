//NOTE
//if you set a pointer parameter to const argument you should delete it by yourself
//if pointer parameter passes not as const then it deletes inside.
//NOTE
///////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <memory>

#include <C:\diploma\2012\TSC\Libs\TableManager\Entry.h>
#include <C:\diploma\2012\TSC\Libs\Grouping\Entry.h>
#include <C:\diploma\2012\TSC\Libs\Cache\Entry.h>

#include "..\Dumper\Dump.h"

#include "TypeDefs.h"
#include "ModelInfo.h"
#include "TableManagerFactory.h"
#include "TSCRndSmplGenerator.h"
#include "TSCModelGenerator.h"
#include "TSCTrunkModelGenerator.h"
#include "IModelRep.h"

namespace GroupedTSC
{
#ifdef USE_PRAGMA
#pragma warning( disable : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA
	TSCCALC_DLLENTRYTEMPLATE template class TSCCALC_DLLENTRY std::auto_ptr<IModelRep>;
#ifdef USE_PRAGMA
#pragma warning( default : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA

class TSCCALC_DLLENTRY CModel
{
private:
	TSampleManager *mSample;
	TModelManager *mModelDef;
	PLevel::CPLevel *mChi;

	ModelInfo mModel;
	auto_ptr<IModelRep> mModelRep;

private:
	//The Constructor
	CModel(int _dim,
		   TSampleManager *_sample, 
		   TModelManager *_modelDef, 
		   const std::string& _simpleEstimates, 
		   IModelRep * _modelRep);
private: /*Helper functions*/
	static void groupBrands(IModelRep * _modelRep, 
							MathModels::CMathParameterValues & _simpleEstimates, 
							std::auto_ptr<TSampleManager> & _sample,
							PLevel::uints & _uGroupping);
	void setStatesValues();
	void setEstimates();

public:
	void InitializeModel();
	int GetWidth()const{return mModelDef->GetWidth();}
	int GetHeight()const{return mModelDef->GetHeight();}
	//Initialize the part of data that needs for grouping models, to distinguish the enhanced estimations 
	//for none and other grouping
	void ProcessInitialCalculation(bool _isInitInfo,
		DataProcessing::CMathOperationIndicator &_indicator = CMathOperationIndicator(),
		bool _isBMatrix = false);
	//Process enchansed estimations
	void ProcessEstimatesCalculation(
		DataProcessing::CMathOperationIndicator &_indicator = CMathOperationIndicator());
	//Calculate PLevel
	void ProcessPLevelCalculation(
		DataProcessing::CMathOperationIndicator &_indicator = CMathOperationIndicator());
	PLevel::PLevelData GetPLevel(){return mModel.mPLevelData;}
	CStrDb GetEstimateAt(int _idx){return mModel.estimates[_idx];}
	int GetEstimateCount(){return (int)mModel.estimates.size();}
	~CModel(void);
	CModel* CreateSubMatrixModel(int _cacheId, const int *_grouping);//joined products
	CModel* CreateSubMatrixModel(Tsc::Grouping::IMergeGrouping<uint,double>& _strategy);
	CModel* CreateSubModel(int _cacheId, const int *_grouping)const;//joined elements
	CModel* CreateSubModel(Tsc::Grouping::IMergeGrouping<uint,double>& _strategy)const;

//public://static
	//static CMathParameterValues SetSimpleEstimates(const TSampleManager &_sample, IModelRep * _modelRep);
	//From File, fix groupping
	static CModel* CreateCModel(const std::string &_sampleFile, 
								const int *grouping, 
								IModelRep * _modelRep);
	//From File, merge weak brands isGroupBrand=true
	static CModel* CreateCModel(const std::string &_sampleFile,
								bool isGroupBrand, 
								IModelRep * _modelRep);

	//Create from TSampleManager
	static CModel* CreateCModel(int _dim,
								TSampleManager* _sampleManager, 
								bool _isGroupBrand, 
								IModelRep * _modelRep);
};
}//namespace GroupedTSC