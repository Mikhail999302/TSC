#pragma once
#include <C:\diploma\2012\TSC\Libs\Cache/Entry.h>
#include "CacherComparingStruct.h"
#include "InverseMatrix.h"
#include "MathParameterList.h"

namespace MathModels 
{
	using DataProcessing::CMathOperationIndicator;

	typedef vector2D< CExpressions > CExpressions2D;

	typedef Tsc::Cacher::ILengthyOperation<CExpressions,CCacherComparingStruct> TLengthyLDot;
	typedef Tsc::Cacher::ILengthyOperation<CExpressions2D,CCacherComparingStruct> TLengthyInfo;
	typedef Tsc::Cacher::Cacher<CExpressions, CCacherComparingStruct> TLDotCacher;
	typedef Tsc::Cacher::Cacher<CExpressions2D, CCacherComparingStruct> TInfoCacher;


	struct ECALC_API CChiCriterionData 
	{
		double criterion_value, plevel;
		int df;
	};

	class ECALC_API CBaseDiscreteModel
	{
		friend class CLDotCacher;
		friend class CInfoCacher;
		friend class CBCacher;
		friend class CLDotCacher2D;
		friend class CInfoCacher2D;
		friend class CBCacher2D;
	public:
		enum ModelType { mt1D, mt2D }; // mt1D - vector of states, mt2D - matrix of states
		virtual ~CBaseDiscreteModel() {};

		// Data selectors
		ModelType GetModelType() const { return mType; }
		int GetParametersCount() const { return ParamCount; }
		int GetNANParametersCount() const { return NANParamCount; }
		int GetStatesCount() const { return StatesCount; }

		const CExpression& GetState( int i ) const { return States[i]; }
		const CExpression& GetInfoMatrixItem( int i, int j ) const;
		const CExpression& GetTriangleInfoMatrixItem( int i, int j ) const { return InfoMatrix[i][j]; }
		const CExpression& GetLDotItem( int i ) const { return l_dot[i]; }
		const CMathParameterInfo& GetParamInfo( int i ) const { return ParamInfos[i]; }

		// Initialization
		virtual void Init( const CExpressions& _States, const CMathParameterInfos& _ParamInfos,
			CCacherComparingStruct& _compStruct,
			TLDotCacher& ldotCacher,
			TInfoCacher& infoMatrix,
			const CMathOperationIndicator& OperationIndicator = CMathOperationIndicator(),
			bool _isCalcInfoMatrix = true,
			bool _isBMatrix = false);
		void Reinit( const CMathParameterInfos& NewParamInfos, // can be a subset of ParamInfos
			const CMathOperationIndicator& OperationIndicator = CMathOperationIndicator() )
			/*throw( CModelInitializationError )*/;

		// Estimates calculations
		bool CheckSampleIsValid( const doubles& Sample ) const 
				{ return (int)Sample.size() == StatesCount; }
		bool CalcSimpleEstimates( const doubles& Sample, int n, const CMathParamBaseEstimates& SEFormulas,
			CMathParameterValues& Estimates ) const;
		bool CalcEnhancedEstimates( const doubles& Sample, int n, const CMathParamBaseEstimates& SEFormulas,
			CMathParameterValues& Estimates, double Gamma = 0 ) const;
		bool EnhanceEstimates( int n, CMathParameterValues& Estimates, double Gamma = 0 ) const;
		// Gamma = 0 means skipping of confidence range calculation

		// Additional calculations
		bool CalcIInverseMatrix( const CMathParameterValues& Estimates, doubles2D& result ) const;
		bool CalcCorrelationsMatrix( const doubles2D& CovariationsMatrix, doubles2D& result ) const;
		bool CalcCovariationsMatrix( const CMathParameterValues& Estimates, int n, doubles2D& result ) const;
		bool CalcConfidenceRanges( const doubles2D& IInverseMatrix, int n, double Gamma, 
			CMathParameterValues& Estimates ) const;
		bool CalcChiCriterion( const doubles& Sample, int n, const CMathParameterValues& Estimates, 
			CChiCriterionData& result ) const;
		bool CalcFrequencies( const CMathParameterValues& Estimates, int n, doubles& result ) const;

		// Serialization
		virtual void Serialize( CMathSerializer& ar );

	protected:
		CBaseDiscreteModel( ModelType Type ) : ParamCount(0), StatesCount(0), mType(Type) {};

		int ParamCount;
		int NANParamCount; // Non-numeric parameters count (Not-A-Number)
		int StatesCount;

		CExpressions l_dot; // l derivation
		CExpressions2D InfoMatrix; // info matrix, contains only high triangle
		CExpressions States; // Model definition
		CMathParameterInfos ParamInfos; // numeric / non-numeric

		virtual void SetFrequenciesValues( const doubles& Sample ) const = 0;
	private:
		ModelType mType;
	};

	typedef std::auto_ptr<CBaseDiscreteModel> ModelPtr;
#ifdef USE_PRAGMA
#pragma warning( disable : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA
	EC_EXPIMP_TEMPLATE template class ECALC_API std::auto_ptr<CBaseDiscreteModel>;
#ifdef USE_PRAGMA
#pragma warning( default : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA

	ECALC_API ModelPtr CreateModel( CBaseDiscreteModel::ModelType Type );
}//namespace MathModels
