//All static methods from class model has been moved here
#pragma once
#include <vector>
#include <C:\diploma\2012\TSC\Libs/Formula/Entry.h>
#include <C:\diploma\2012\TSC\Libs/TableManager/Entry.h>
#include "TypeDefs.h"
#include "ModelInfo.h"
#include "TSCModelGenerator.h"
#include "TSCRndSmplGenerator.h"
#include "model.h"
#include "IModelRep.h"
namespace GroupedTSC
{
#ifdef USE_PRAGMA
#pragma warning( disable : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA
	TSCCALC_DLLENTRYTEMPLATE template class TSCCALC_DLLENTRY std::allocator<bool>;
	TSCCALC_DLLENTRYTEMPLATE template class TSCCALC_DLLENTRY std::vector<bool>;
#ifdef USE_PRAGMA
#pragma warning( default : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA


//базовый класс - "модель двухэтапного опроса"
class TSCCALC_DLLENTRY CTSCModel: public IModelRep
{
protected:
	MathModels::CCacherComparingStruct mCompStruct;
	MathModels::TLDotCacher& mLDotCacher;
	MathModels::TInfoCacher& mInfoCacher;
	std::vector<bool> m_isOptimize;
private:
	void resizeOptimize(int idx);
public /*ctors,dtor*/:
	virtual ~CTSCModel(){}
public:
	 CTSCModel( int _dim, int _type,
			MathModels::TLDotCacher& _ldotCacher, 
			MathModels::TInfoCacher& _infoCacher):
		mCompStruct(_dim, _type),
		mLDotCacher(_ldotCacher),
		mInfoCacher(_infoCacher), 
		m_isOptimize(_dim+1, true){}
	//Model Definition
	virtual /*implement*/ MathCalc::MathString SetModelStrRep(const TModelManager&_table);

	//Parameters Definition
	virtual /*implement*/ MathCalc::MathString SetEstimatesFormulasStrRep(int _dim){return 0;};
	
	//Sample Definition
	virtual /*implement*/ doubles SetSample(const TSampleManager &_table);
	//Generate Range NOT IMPLEMENTED
	virtual /*implement*/ MathCalc::MathString GenerateRange(void);

	virtual /*implement*/ MathCalc::MathString FixSimpleEstimates(const CMathParameterValues& _simpleEstimates, bool _isMatrix=false){return 0;};
	//n_i_0+...+n_i_size
	virtual /*implement*/ MathCalc::MathString GetSumByRow(int i,int size);
	virtual /*implement*/ IModelRep* Clone();

	virtual /*implement*/ TModelManager* CreateModelManager(int _width, int _height){return 0;};

	virtual /*implement*/ CMathParameterValues SetSimpleEstimates(const TSampleManager &_sample){return CMathParameterValues();};

	virtual /*implement*/ MathModels::CCacherComparingStruct& GetComparator(){return mCompStruct;}
	virtual /*implement*/ MathModels::TLDotCacher& getLDotCacher(){return mLDotCacher;}
	virtual /*implement*/ MathModels::TInfoCacher& getInfoCacher(){return mInfoCacher;}
	virtual /*implement*/ bool isOptimize(int idx);
	virtual /*implement*/ void setOptimize(int idx, bool val);

};

//*************************************
//****   class CTSCModel_2Pos   *******
//модель с двумя положительными рекламами
//*************************************

class TSCCALC_DLLENTRY CTSCModel_2Pos: public  CTSCModel
{
public:
	CTSCModel_2Pos( int _dim, int _type,
			MathModels::TLDotCacher& _ldotCacher, 
			MathModels::TInfoCacher& _infoCacher):
		CTSCModel(_dim, _type, _ldotCacher, _infoCacher){};

	virtual ~CTSCModel_2Pos(){};
public:

	virtual /*implement*/ CMathParameterValues SetSimpleEstimates(const TSampleManager &_sample);
	//Parameters Definition
	virtual /*implement*/ MathCalc::MathString SetEstimatesFormulasStrRep(int _dim);
	virtual /*implement*/ MathCalc::MathString FixSimpleEstimates(const CMathParameterValues& _simpleEstimates, bool _isMatrix=false);
	virtual /*implement*/ TModelManager* CreateModelManager(int _width, int _height);
};


//*************************************
//****   class CTSCModel_1Pos   *******
//модель с одной положительной рекламой
//*************************************

class TSCCALC_DLLENTRY CTSCModel_1Pos: public  CTSCModel
{
public:
	CTSCModel_1Pos( int _dim, int _type,
			MathModels::TLDotCacher& _ldotCacher, 
			MathModels::TInfoCacher& _infoCacher):
		CTSCModel(_dim, _type, _ldotCacher, _infoCacher){};

	virtual ~CTSCModel_1Pos(){};
public:

	virtual /*implement*/ CMathParameterValues SetSimpleEstimates(const TSampleManager &_sample);
	//Parameters Definition
	virtual /*implement*/ MathCalc::MathString SetEstimatesFormulasStrRep(int _dim);
	virtual /*implement*/ MathCalc::MathString FixSimpleEstimates(const CMathParameterValues& _simpleEstimates, bool _isMatrix=false);
	virtual /*implement*/ TModelManager* CreateModelManager(int _width, int _height);
};


//*************************************
//****   class CTSCModel_1Pos   *******
//модель с одной положительной рекламой
//*************************************

class TSCCALC_DLLENTRY CTSCModel_1Pos_mine : public  CTSCModel_1Pos
{
public:
	CTSCModel_1Pos_mine(int _dim, int _type,
		MathModels::TLDotCacher& _ldotCacher,
		MathModels::TInfoCacher& _infoCacher) :
		CTSCModel_1Pos(_dim, _type, _ldotCacher, _infoCacher) {};

	virtual ~CTSCModel_1Pos_mine() {};
public:

	virtual /*implement*/ CMathParameterValues SetSimpleEstimates(const TSampleManager& _sample);
	//Parameters Definition
	virtual /*implement*/ MathCalc::MathString SetEstimatesFormulasStrRep(int _dim);
	virtual /*implement*/ MathCalc::MathString FixSimpleEstimates(const CMathParameterValues& _simpleEstimates, bool _isMatrix = false);
	virtual /*implement*/ TModelManager* CreateModelManager(int _width, int _height);
};


//*************************************
//****   class CTSCModel_1Neg   *******
//модель с одной отрицательной рекламамой
//*************************************

class TSCCALC_DLLENTRY CTSCModel_1Neg: public  CTSCModel
{
public:
	CTSCModel_1Neg( int _dim, int _type,
			MathModels::TLDotCacher& _ldotCacher, 
			MathModels::TInfoCacher& _infoCacher):
		CTSCModel(_dim, _type, _ldotCacher, _infoCacher){};

	virtual ~CTSCModel_1Neg(){};
public:

	virtual /*implement*/ CMathParameterValues SetSimpleEstimates(const TSampleManager &_sample);
	//Parameters Definition
	virtual /*implement*/ MathCalc::MathString SetEstimatesFormulasStrRep(int _dim);
	virtual /*implement*/ MathCalc::MathString FixSimpleEstimates(const CMathParameterValues& _simpleEstimates, bool _isMatrix=false);
	virtual /*implement*/ TModelManager* CreateModelManager(int _width, int _height);
};



//****************************************
//****   class CTSCModel_2NegPos   *******
//модель с двумя рекламами "-" и "+" *****
//****************************************

class TSCCALC_DLLENTRY CTSCModel_2NegPos: public  CTSCModel
{
public:
	CTSCModel_2NegPos( int _dim, int _type,
			MathModels::TLDotCacher& _ldotCacher, 
			MathModels::TInfoCacher& _infoCacher):
		CTSCModel(_dim, _type, _ldotCacher, _infoCacher){};

	virtual ~CTSCModel_2NegPos(){};
public:

	virtual /*implement*/ CMathParameterValues SetSimpleEstimates(const TSampleManager &_sample);
	//Parameters Definition
	virtual /*implement*/ MathCalc::MathString SetEstimatesFormulasStrRep(int _dim);
	virtual /*implement*/ MathCalc::MathString FixSimpleEstimates(const CMathParameterValues& _simpleEstimates, bool _isMatrix=false);
	virtual /*implement*/ TModelManager* CreateModelManager(int _width, int _height);
};



}//namespace GroupedTSC