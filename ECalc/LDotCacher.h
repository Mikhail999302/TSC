#pragma once 
#include "DiscreteModel.h"
namespace MathModels
{
using namespace MathCalc;
class CLDotCacher : public TLengthyLDot
{
	CCacherComparingStruct mComparator;
	CDiscreteModel* mDiscrete;
	const CMathOperationIndicator& mOperationIndicator;
public:
	CLDotCacher(CCacherComparingStruct _comparator, CDiscreteModel* _discreteModel,
			const CMathOperationIndicator& _operationIndicator = CMathOperationIndicator()) :
		mComparator(_comparator), mDiscrete(_discreteModel),
				mOperationIndicator(_operationIndicator)
	{
	}
	CExpressions Calculate()
	{
		int i = 0;
		try
		{
			// InfoMatrix and l_dot calculation
			std::vector<CParameter> NANParams(mDiscrete->NANParamCount);
			for ( CMathParameterInfos::iterator iit = mDiscrete->ParamInfos.begin();
					iit != mDiscrete->ParamInfos.end();
					++iit )
			if ( iit->isOptimizible() )
				NANParams[i++] = iit->Parameter;

			for (int k = 0; k < mDiscrete->NANParamCount; ++k )
			{
				mDiscrete->l_dot[k] = 0;
				for ( i = 0; i < mDiscrete->StatesCount; ++i )
				{
					CExpression pi_k_derivation((mDiscrete->States[i])(NANParams[k]));
					mDiscrete->l_dot[k].Add( CExpression(mDiscrete->GetFrequencyParam(i)).Multiply(pi_k_derivation, 
							true ).Divide(mDiscrete->States[i]), true );
					mOperationIndicator.Increment();
				}
			}
			return mDiscrete->l_dot;
		}
		catch (CMathException&)
		{
			std::string dumpStr("Error:Init model while constructing\n");
			std::string filterStr=_T("Exception");
//			Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
			//Dumper::CDump::GetDumper()->Dump(k,filterStr.c_str(),(int)filterStr.size());
			//Dumper::CDump::GetDumper()->Dump(l,filterStr.c_str(),(int)filterStr.size());
			throw;
		}
	}
	CCacherComparingStruct GetComparingStruct()
	{
		return mComparator;
	}
};
}//namespace MathModels
