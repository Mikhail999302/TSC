#pragma once
//#include "../Dumper/dump.h"
#include "DiscreteModel.h"
#include "CacherComparingStruct.h"
namespace MathModels
{
	class CInfoCacher:public TLengthyInfo
	{
		CCacherComparingStruct mComparator;
		CDiscreteModel* mDiscrete;
		const CMathOperationIndicator& mOperationIndicator;
	public:
		CInfoCacher(CCacherComparingStruct _comparator, CDiscreteModel* _discreteModel,
			const CMathOperationIndicator& _operationIndicator = CMathOperationIndicator()):
		mComparator(_comparator),mDiscrete(_discreteModel), mOperationIndicator(_operationIndicator){}
		CExpressions2D Calculate()
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

				std::vector<std::vector<CExpression> > Pi_k_derivatives(mDiscrete->NANParamCount,
					std::vector<CExpression>(mDiscrete->StatesCount));
				for(int k=0; k < mDiscrete->NANParamCount; ++k)
				{
					for ( i = 0; i < mDiscrete->StatesCount; ++i ) 
					{
						CExpression pi_k_derivative(mDiscrete->States[i](NANParams[k]));
						Pi_k_derivatives[k][i].Swap(pi_k_derivative);
					}
				}
				for (int k = 0; k < mDiscrete->NANParamCount; ++k ) 
				{
					for (int l = 0; l < mDiscrete->NANParamCount; ++l ) 
					{
						mDiscrete->InfoMatrix[k][l] = 0;
						if ( k <= l ) 
						{ // high triangle - full calculations
							for ( i = 0; i < mDiscrete->StatesCount; ++i ) 
							{
								//Pi_k_derivative.Swap((mDiscrete->States[i])(NANParams[k]));
								MathCalc::CExpression Pi_l_derivative(Pi_k_derivatives[l][i]);
								mDiscrete->InfoMatrix[k][l].Add(Pi_l_derivative.Multiply(
										Pi_k_derivatives[k][i]).Divide(mDiscrete->States[i]),true)
									/*.Subtract((Pi_k_derivative)(NANParams[l]), true )*/;
								mOperationIndicator.Increment();
							}
						}
						else
							mDiscrete->InfoMatrix[k][l] = 0;
					}
				}
				return mDiscrete->InfoMatrix;
			}
			catch (CMathException&) 
			{
				std::string dumpStr("Error:Init model while constructing\n");
				std::string filterStr=_T("Exception");
//				Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
				//Dumper::CDump::GetDumper()->Dump(k,filterStr.c_str(),(int)filterStr.size());
				//Dumper::CDump::GetDumper()->Dump(l,filterStr.c_str(),(int)filterStr.size());
				throw;
			}
		}
		CCacherComparingStruct GetComparingStruct(){return mComparator;}
	};
}//namespace MathModels
