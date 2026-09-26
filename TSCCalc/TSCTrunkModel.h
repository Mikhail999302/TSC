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

	class TSCCALC_DLLENTRY CTSCTrunkModel: public IModelRep
	{
		MathModels::CCacherComparingStruct mCompStruct;
		MathModels::TLDotCacher& mLDotCacher;
		MathModels::TInfoCacher& mInfoCacher;
		std::vector<bool> m_isOptimize;
	public /*ctors,dtor*/:
		virtual ~CTSCTrunkModel(){}
	public:
		CTSCTrunkModel( int _dim, int _type,
						MathModels::TLDotCacher& _ldotCacher, 
						MathModels::TInfoCacher& _infoCacher):
					mCompStruct(_dim, _type),
					mLDotCacher(_ldotCacher),
					mInfoCacher(_infoCacher),
					m_isOptimize(_dim,true){}
		//Model Definition
		virtual /*implement*/ MathCalc::MathString SetModelStrRep(const TModelManager&_table);
		//Parameters Definition
		virtual /*implement*/ MathCalc::MathString SetEstimatesFormulasStrRep(int _dim);
		//Sample Definition
		virtual /*implement*/ doubles SetSample(const TSampleManager &_table);
		//Generate Range NOT IMPLEMENTED
		virtual /*implement*/ MathCalc::MathString GenerateRange(void);
		virtual /*implement*/ MathCalc::MathString FixSimpleEstimates(const CMathParameterValues& _simpleEstimates, bool _isMatrix=false);
		//n_i_0+...+n_i_size
		virtual /*implement*/ MathCalc::MathString GetSumByRow(int i,int size);
		virtual /*implement*/ IModelRep* Clone();
		virtual /*implement*/ TModelManager* CreateModelManager(int _width, int _height);
		virtual /*implement*/ CMathParameterValues SetSimpleEstimates(const TSampleManager &_sample);
		virtual /*implement*/ MathModels::CCacherComparingStruct& GetComparator(){return mCompStruct;}
		virtual /*implement*/ MathModels::TLDotCacher& getLDotCacher(){return mLDotCacher;}
		virtual /*implement*/ MathModels::TInfoCacher& getInfoCacher(){return mInfoCacher;}
		virtual /*implement*/ bool isOptimize(int idx){return m_isOptimize[idx];}
		virtual /*implement*/ void setOptimize(int idx, bool val){m_isOptimize[idx] = val;}
	};
}//namespace GroupedTSC