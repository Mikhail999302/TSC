#pragma once
//#include "../Dumper/dump.h"
#include "DiscreteModel.h"
#include "CacherComparingStruct.h"
namespace MathModels
{
	class CBCacher:public TLengthyInfo
	{
		CCacherComparingStruct mComparator;
		CDiscreteModel* mDiscrete;
		const CMathOperationIndicator& mOperationIndicator;
	public:
		CBCacher(CCacherComparingStruct _comparator, CDiscreteModel* _discreteModel,
			const CMathOperationIndicator& _operationIndicator = CMathOperationIndicator()):
		mComparator(_comparator),mDiscrete(_discreteModel), mOperationIndicator(_operationIndicator){}
		CExpressions2D Calculate()
		{
			int i = 0;
			try 
			{
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
								MathCalc::CExpression Pi_l_derivative(Pi_k_derivatives[l][i]);
								MathCalc::CExpression Pi_k_derivative(Pi_k_derivatives[k][i]);
								mDiscrete->InfoMatrix[k][l].Add(
									Pi_l_derivative.Multiply(Pi_k_derivatives[k][i]).Subtract(
									Pi_k_derivative(NANParams[l]).Multiply(
									mDiscrete->States[i])).Multiply(CExpression(mDiscrete->GetFrequencyParam(i))).Divide(
									mDiscrete->States[i]).Divide(mDiscrete->States[i]));
								mOperationIndicator.Increment();
							}
							mDiscrete->InfoMatrix[k][l].Divide(CExpression(mDiscrete->GetSampleSize()));
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
