//#include "../Dumper/dump.h"
#include "BaseDiscreteModel.h"
#include "Probcalc.h"
#include "../Dumper/dump.h"
using namespace MathModels;

//////////////////////////////////////////////////////////////////////
// CBaseDiscreteModel
//////////////////////////////////////////////////////////////////////

namespace
{
inline bool IsNumericParameter(const CMathParameterInfo& Info)
{
	return !Info.isOptimizible();
}
inline int CalcNumericParametersCount(const CMathParameterInfos& Infos)
{
	return (int)std::count_if(Infos.begin(), Infos.end(), IsNumericParameter);
}
inline int CalcNANParametersCount(const CMathParameterInfos& Infos)
{
	return (int)Infos.size() - CalcNumericParametersCount(Infos);
}
}

inline const CExpression& CBaseDiscreteModel::GetInfoMatrixItem(int i, int j) const
{
	if (i <= j) // high triangle
		return InfoMatrix[i][j];
	else
		return InfoMatrix[j][i];
}

void CBaseDiscreteModel::Serialize(CMathSerializer& ar)
{
	using namespace MathCalc;
	SerializeContainer<CExpression>(ar, l_dot);
	SerializeContainerEx<CExpressions>(ar, InfoMatrix, CContainerSerializer<CExpression,
			CExpressions>() );
	SerializeContainer<CExpression>(ar, States);
	SerializeContainer<CMathParameterInfo>(ar, ParamInfos);
	if (ar.IsLoading() )
	{
		ParamCount = (int)ParamInfos.size();
		NANParamCount = CalcNANParametersCount(ParamInfos);
		StatesCount = (int)States.size();
	}
}

void CBaseDiscreteModel::Init(const CExpressions& _States, const CMathParameterInfos& _ParamInfos,
		CCacherComparingStruct& _compStruct, TLDotCacher& ldotCacher, TInfoCacher& infoMatrix,
		const CMathOperationIndicator& /*=OperationIndicator()*/, bool _isCalcInfoMatrix/*=true*/,
		bool _isBMatrix/*=false*/)
{
	ParamInfos = _ParamInfos;
	ParamCount = (int)ParamInfos.size();
	NANParamCount = CalcNANParametersCount(ParamInfos);
	States = _States;
	StatesCount = (int)States.size();
	l_dot.resize(NANParamCount);
	InfoMatrix.resize2D(NANParamCount);
}

void CBaseDiscreteModel::Reinit(const CMathParameterInfos& NewParamInfos,
		const CMathOperationIndicator& OperationIndicator)
/*throw( CModelInitializationError )*/
{
	bool bNeedReinitializeModel = false;
	CMathParameterInfos::iterator iit;
	for (CMathParameterInfos::const_iterator newiit = NewParamInfos.begin(); newiit
			!= NewParamInfos.end(); ++newiit)
	{
		if ( (iit = ParamInfos.find(newiit->Parameter)) != ParamInfos.end() )
		{
			if (newiit->isOptimizible() != iit->isOptimizible())
			{
				iit->setOptimizible(newiit->isOptimizible());
				bNeedReinitializeModel = true;
			}
		}
	}
	if (bNeedReinitializeModel)
	{
		//Fake Cacher parameters.
		CCacherComparingStruct comparingStruct(0, 0);
		TLDotCacher ldotCacher;
		TInfoCacher infoCacher;
		Init(States, ParamInfos, comparingStruct, ldotCacher, infoCacher, OperationIndicator);
	}
}

//////////////////////////////////////////////////////////////////////
// Estimates calculations

bool CBaseDiscreteModel::CalcSimpleEstimates(const doubles& Sample, int n,
		const CMathParamBaseEstimates& SEFormulas, CMathParameterValues& Estimates) const
{
	if ( !CheckSampleIsValid(Sample) )
		return false;

	try
	{
		CParameter(_T("n")).SetValue( n );
		SetFrequenciesValues( Sample );
		Estimates.Construct( SEFormulas );
		return true;
	}
	catch (MathCalc::CMathException&)
	{
		std::string filterStr(_T("Exception"));
		std::string dumpStr("Error:CalcSimpeEstimates\n");
		//		Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
		return false;
	}
}

bool CBaseDiscreteModel::EnhanceEstimates(int n, CMathParameterValues& Estimates, double Gamma) const
{
	//ofstream f_out("results.txt");
	try
	{
		doubles2D CovariationMatrix;
		if ( !CalcIInverseMatrix( Estimates, CovariationMatrix ) )
		return false;

		int i, j;
		// calculating l derivative value
		doubles l_dot_Estimate( NANParamCount );
		for ( i = 0; i < NANParamCount; ++i )
		{
			l_dot_Estimate[i] = l_dot[i].Eval() / n;// l_dot_Estimate[i] = l_dot[i].Eval() / n;
			//Dumper::CDump::GetDumper()->GetDumper()->Dump(l_dot[i].ToString());
			//cout << "\n****************\n"<< i << "  l_dot[i] " << endl << l_dot[i].ToString() << endl;
			//cout << " l_dot[i] " << l_dot_Estimate[i] << endl;
		}

		// calculating Addon = I_Inverse * l_dot_Estimate
		doubles Addon( NANParamCount );
		for ( i = 0; i < NANParamCount; ++i )
		{
			Addon[i] = 0;
			for ( j = 0; j < NANParamCount; ++j )
			Addon[i] += CovariationMatrix[i][j] * l_dot_Estimate[j];
		}

		// final calculation of P_SimpleEstimate + I_Inverse * l_dot
		// and confidence ranges
		i = 0;
		CMathParameterValues::iterator eit;
		for ( CMathParameterInfos::const_iterator iit = ParamInfos.begin(); iit != ParamInfos.end(); ++iit )
		if ( (eit = Estimates.find(iit->Parameter)) != Estimates.end() && eit->isOptimizible() )
		{
			eit->Value += Addon[i];
			if ( Gamma )
			eit->CalcRange( CovariationMatrix[i][i], n, Gamma );
			++i;
		}
		return true;
	}
	catch (MathCalc::CMathException&)
	{
		std::string dumpStr("Error:EnhanceEstimates\n");
		std::string filterStr(_T("Exception"));
		//		Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
		return false;
	}
}

bool CBaseDiscreteModel::CalcEnhancedEstimates(const doubles& Sample, int n,
		const CMathParamBaseEstimates& SEFormulas, CMathParameterValues& Estimates, double Gamma) const
{
	if ( !CalcSimpleEstimates(Sample, n, SEFormulas, Estimates) )
		return false;
	return EnhanceEstimates(n, Estimates, Gamma);
}

bool CBaseDiscreteModel::CalcIInverseMatrix(const CMathParameterValues& Estimates, doubles2D& result) const
{
	//for catch
	int i=0;
	int j=0;
	try
	{
		Estimates.SetContextValues();

		doubles2D I( NANParamCount );
		//-->Sergey
		//std::cout<<"Covariation Matrix:\n";
		//--<Sergey
		for (i = 0; i < NANParamCount; ++i )
		{
			for (j = 0; j < NANParamCount; ++j )
			{
				if ( i <= j ) // high triangle
				I[i][j] = InfoMatrix[i][j].Eval(); // try change sign
				else
				I[i][j] = I[j][i];
				//-->Sergey
				//if(i=0)
					//Dumper::CDump::GetDumper()->Dump(InfoMatrix[i][j].ToString());
				//--<Sergey
			}
			//-->Sergey
			//std::cout<<std::endl;
			//--<Sergey
		}
		doubles eig;
		return CalcInverseMatrix( I, result,eig );
	}
	catch (MathCalc::CMathException&)
	{
		std::string dumpStr("Error:CalcIInverseMatrix\n");
		dumpStr += InfoMatrix[i][j].ToString();
		std::string filterStr(_T("Exception"));
		//		Dumper::CDump::GetDumper()->Dump(dumpStr, filterStr);		
		return false;
	}
}

//////////////////////////////////////////////////////////////////////
// Additional calculations

bool CBaseDiscreteModel::CalcCovariationsMatrix(const CMathParameterValues& Estimates, int n,
		doubles2D& result) const
{
	try
	{
		Estimates.SetContextValues();

		doubles2D In( NANParamCount );
		for ( int i = 0; i < NANParamCount; ++i )
		for ( int j = 0; j < NANParamCount; ++j )
		{
			if ( i <= j ) // high triangle
			In[i][j] = InfoMatrix[i][j].Eval() * n;
			else
			In[i][j] = In[j][i];
		}
		doubles eig;
		return CalcInverseMatrix( In, result,eig );
	}
	catch (MathCalc::CMathException&)
	{
		std::string dumpStr("Error:CalcCovariationsMatrix\n");
		std::string filterStr(_T("Exception"));
		//		Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
		return false;
	}
}

bool CBaseDiscreteModel::CalcCorrelationsMatrix(const doubles2D& CovariationsMatrix,
		doubles2D& result) const
{
	if ( (int)CovariationsMatrix.size() != NANParamCount)
		return false;
	doubles Sigmas(NANParamCount);
	int i=0;
	for (i = 0; i < NANParamCount; ++i)
		if ( (Sigmas[i] = sqrt(CovariationsMatrix[i][i])) == 0)
			return false;
	result.resize2D(NANParamCount);
	for (i = 0; i < NANParamCount; ++i)
		for (int j = 0; j < NANParamCount; ++j)
			result[i][j] = CovariationsMatrix[i][j] / (Sigmas[i]*Sigmas[j]);
	return true;
}

bool CBaseDiscreteModel::CalcConfidenceRanges(const doubles2D& IInverseMatrix, int n, double Gamma,
		CMathParameterValues& Estimates) const
{
	if ( (int)IInverseMatrix.size() != NANParamCount)
		return false;

	int i = 0;
	CMathParameterValues::iterator eit;
	for (CMathParameterInfos::const_iterator iit = ParamInfos.begin(); iit != ParamInfos.end(); ++iit)
		if ( (eit = Estimates.find(iit->Parameter)) != Estimates.end() && eit->isOptimizible())
		{
			eit->CalcRange(IInverseMatrix[i][i], n, Gamma);
			++i;
		}
	return true;
}

bool CBaseDiscreteModel::CalcChiCriterion(const doubles& Sample, int n,
		const CMathParameterValues& Estimates, CChiCriterionData& result) const
{
	if ( !CheckSampleIsValid(Sample) )
		return false;

	try
	{
		Estimates.SetContextValues();

		result.criterion_value = 0;
		double nominator, n_EnhancedEstimate;
		for ( int i = 0; i < StatesCount; ++i )
		{
			n_EnhancedEstimate = n * States[i].Eval();
			if ( n_EnhancedEstimate != 0 )
			{
				nominator = Sample[i] - n_EnhancedEstimate;
				result.criterion_value += (nominator * nominator) / n_EnhancedEstimate;
			}
			else
			return false;
		}
		result.df = StatesCount - NANParamCount - 1;
		result.plevel = 1. - pChi( result.criterion_value, result.df );
		return true;
	}
	catch (MathCalc::CMathException&)
	{
		return false;
	}
}

bool CBaseDiscreteModel::CalcFrequencies(const CMathParameterValues& Estimates, int n,
		doubles& result) const
{
	try
	{
		Estimates.SetContextValues();

		result.resize( StatesCount );
		for ( int i = 0; i < StatesCount; ++i )
		result[i] = n * States[i].Eval();
		return true;
	}
	catch (MathCalc::CMathException&)
	{
		std::string dumpStr("Error:CalcFrequencies\n");
		std::string filterStr(_T("Exception"));
		//		Dumper::CDump::GetDumper()->Dump(dumpStr, filterStr);
		return false;
	}
}
