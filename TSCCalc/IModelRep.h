#pragma once
#include <C:\diploma\2012\TSC\Libs/TableManager/Entry.h>
#include <C:\diploma\2012\TSC\Libs/StdInterfaces/Entry.h>
#include <C:\diploma\2012\TSC\Libs/Cache/Entry.h>

#include "TypeDefs.h"
#include "ModelInfo.h"
#include "TSCModelGenerator.h"
#include "TSCRndSmplGenerator.h"
#include "model.h"
namespace GroupedTSC
{
class TSCCALC_DLLENTRY IModelRep:virtual public Tsc::StdInterfaces::IClone<IModelRep*>
{
public:
	//Model Definition
	virtual MathCalc::MathString SetModelStrRep(const TModelManager&_table)=0;
	//Parameters Definition
	virtual MathCalc::MathString SetEstimatesFormulasStrRep(int _dim)=0;
	//Sample Definition
	virtual doubles SetSample(const TSampleManager &_table)=0;
	//Generate Range NOT IMPLEMENTED
	virtual MathCalc::MathString GenerateRange(void)=0;
	virtual MathCalc::MathString FixSimpleEstimates(const CMathParameterValues& _simpleEstimates, bool _isMatrix=false)=0;
	//n_i_0+...+n_i_size
	virtual ~IModelRep(){}
	virtual TModelManager* CreateModelManager(int _width, int _height)=0;
	virtual CMathParameterValues SetSimpleEstimates(const TSampleManager &_sample)=0;
	virtual MathModels::CCacherComparingStruct& GetComparator()=0;
	virtual MathModels::TLDotCacher& getLDotCacher()=0;
	virtual MathModels::TInfoCacher& getInfoCacher()=0;
	virtual bool isOptimize(int idx)=0;
	virtual void setOptimize(int idx, bool val)=0;
protected:
	//n_i_0+...+n_i_size
	virtual MathCalc::MathString GetSumByRow(int i,int size)=0;
};
}//namespace GroupedTSC