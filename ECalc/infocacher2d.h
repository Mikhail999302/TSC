#pragma once 
#include "DiscreteModel.h"
namespace MathModels
{
	class CInfoCacher2D:public TLengthyInfo
	{
		CCacherComparingStruct mComparator;
		CDiscrete2DModel* mDiscrete;
		const CMathOperationIndicator& mOperationIndicator;
	public:
		CInfoCacher2D(CCacherComparingStruct _comparator, CDiscrete2DModel* _discreteModel,
			const CMathOperationIndicator& _operationIndicator = CMathOperationIndicator()):
		mComparator(_comparator),mDiscrete(_discreteModel), mOperationIndicator(_operationIndicator){}
		CExpressions2D Calculate()
		{
			try 
			{
				// InfoMatrix and l_dot calculation
				int i = 0, j/*, k, l*/;
				std::vector<CParameter> NANParams( mDiscrete->NANParamCount );
				for ( CMathParameterInfos::const_iterator iit = mDiscrete->ParamInfos.begin(); iit != mDiscrete->ParamInfos.end(); ++iit )
					if ( iit->isOptimizible() )
						NANParams[i++] = iit->Parameter;
				//CExpression Pij_k_derivative;
				std::vector<std::vector<CExpression> > Pij_k_derivatives(mDiscrete->NANParamCount,
									std::vector<CExpression>(mDiscrete->ProductsCount*mDiscrete->ProductsCount));
				for(int k=0; k < mDiscrete->NANParamCount; ++k)
				{
					for ( i = 0; i < mDiscrete->ProductsCount; ++i )
						for ( j = 0; j < mDiscrete->ProductsCount; ++j ) 
						{
							CExpression pij_k_derivative((mDiscrete->States[i*mDiscrete->ProductsCount + j])(NANParams[k]));
							Pij_k_derivatives[k][i*mDiscrete->ProductsCount + j].Swap(pij_k_derivative);
						}
				}

				for (int k = 0; k < mDiscrete->NANParamCount; ++k ) {
					for (int l = 0; l < mDiscrete->NANParamCount; ++l ) {
						if ( k <= l ) { // high triangle - full calculations
							mDiscrete->InfoMatrix[k][l] = 0;
							for ( i = 0; i < mDiscrete->ProductsCount; ++i )
								for ( j = 0; j < mDiscrete->ProductsCount; ++j ) {
									//Pij_k_derivative.Swap((mDiscrete->States[i*mDiscrete->ProductsCount + j])(NANParams[k]));
									MathCalc::CExpression Pij_l_derivative(Pij_k_derivatives[l][i*mDiscrete->ProductsCount + j]);
									mDiscrete->InfoMatrix[k][l].Add( 
										Pij_l_derivative.Multiply(
										Pij_k_derivatives[k][i*mDiscrete->ProductsCount + j]).Divide( 
										mDiscrete->States[i*mDiscrete->ProductsCount + j]),true)
										/*.Subtract((Pij_k_derivative)(NANParams[l]), true )*/;
									mOperationIndicator.Increment();
								}
						}
						else 
						{ // low triangle - light calculations
							mDiscrete->InfoMatrix[k][l] = 0;
						}
					}
				}
				return mDiscrete->InfoMatrix;
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
