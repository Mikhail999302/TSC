#pragma once 
#include "DiscreteModel.h"
namespace MathModels
{
	using namespace MathCalc;
	class CLDotCacher2D:public TLengthyLDot
	{
		CCacherComparingStruct mComparator;
		CDiscrete2DModel* mDiscrete;
		const CMathOperationIndicator& mOperationIndicator;
	public:
		CLDotCacher2D(CCacherComparingStruct _comparator, CDiscrete2DModel* _discreteModel,
			const CMathOperationIndicator& _operationIndicator = CMathOperationIndicator()):
			mComparator(_comparator), mDiscrete(_discreteModel), mOperationIndicator(_operationIndicator){}
		CExpressions Calculate()
		{
			try 
			{
				// InfoMatrix and l_dot calculation
				int i = 0, j, k/*, l*/;
				std::vector<CParameter> NANParams( mDiscrete->NANParamCount );
				for ( CMathParameterInfos::const_iterator iit = mDiscrete->ParamInfos.begin(); iit != mDiscrete->ParamInfos.end(); ++iit )
					if ( iit->isOptimizible() )
						NANParams[i++] = iit->Parameter;
				//CExpression Pij_k_derivative;
				for ( k = 0; k < mDiscrete->NANParamCount; ++k ) 
				{
					mDiscrete->l_dot[k] = 0;
					for ( i = 0; i < mDiscrete->ProductsCount; ++i )
						for ( j = 0; j < mDiscrete->ProductsCount; ++j ) 
						{
							CExpression pij_k_derivative((mDiscrete->States[i*mDiscrete->ProductsCount + j])(NANParams[k]));
							mDiscrete->l_dot[k].Add( CExpression(mDiscrete->GetFrequencyParam(i, j)).Multiply(pij_k_derivative, 
									true).Divide(mDiscrete->States[i*mDiscrete->ProductsCount + j]), true );
							mOperationIndicator.Increment();
						}
						
				}
				//for (int i=0; i<(int)mDiscrete->l_dot.size(); ++i)
				//{
				//	string str  = mDiscrete->l_dot[i].ToString();
				//	Dumper::CDump::GetDumper()->Dump(str.c_str(),(int)str.size());
				//}
				//Dumper::CDump::GetDumper()->NewLine();
				return mDiscrete->l_dot;
			}
			catch (std::exception&) 
			{
				std::string dumpStr("Error:Init 2D\n");
				std::string filterStr=_T("Exception");
//				Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
				throw CModelInitializationError();
			}
		}
		CCacherComparingStruct GetComparingStruct(){return mComparator;}
	};
}//namespace MathModels
