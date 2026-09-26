// MathParameterEstimate.h: Parameters estimates classes
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "ECalcMisc.h"
//#include "ECalcTemplates.h"

namespace MathModels 
{

	using MathCalc::MathString;
	using MathCalc::CMathSerializer;
	using MathCalc::CParameter;
	using MathCalc::CExpression;

//////////////////////////////////////////////////////////////////////
// CMathParamBaseEstimate - generic interface for parameters estimates

	class ECALC_API CMathParamBaseEstimate 
	{
	protected:
		CExpression Formula;
	public:
		CParameter Parameter;

		CMathParamBaseEstimate( const CParameter& Param = CParameter() ): Parameter(Param) {}
		virtual ~CMathParamBaseEstimate() {}
		MathString GetParameterName() const { return Parameter.ToString(); }
	// Functions for override
		//virtual bool IsNumeric() const = 0;
		//virtual double Eval() const = 0;
		virtual bool IsNumeric() const { return Formula.IsNumber(); }
		virtual double Eval() const { return Formula.Eval(); }
		virtual void ParseString( const MathString& Name, const MathString& Value ) 
			/*throw ( DataProcessing::CInvalidDataParameter )*/;// = 0;
		virtual void Serialize( CMathSerializer& ar );// = 0;
	//private:
	//	CMathParamBaseEstimate& operator=(const CMathParamBaseEstimate&); // not implemented
	//	CMathParamBaseEstimate(const CMathParamBaseEstimate&); // not implemented
	};
	//typedef boost::shared_ptr<CMathParamBaseEstimate> CMathParamBaseEstimatePtr;

//////////////////////////////////////////////////////////////////////
// CMathParameterEstimate - simple parameter estimate

	class ECALC_API CMathParameterEstimate : public CMathParamBaseEstimate 
	{
	public:
		CMathParameterEstimate( const CParameter& Param = CParameter(), const CExpression& Expr = 0 ): 
			CMathParamBaseEstimate(Param), Formula(Expr) {}

		CExpression& GetFormula() { return Formula; }
	// CMathParamBaseEstimate interface implementation
		virtual bool IsNumeric() const { return Formula.IsNumber(); }
		virtual double Eval() const { return Formula.Eval(); }
		virtual void ParseString( const MathString& Name, const MathString& Value ) 
			/*throw ( DataProcessing::CInvalidDataParameter )*/;
		virtual void Serialize( CMathSerializer& ar );
	protected:
		CExpression Formula;
	};
	
} // MathModels

//#include "ExportTricks.h"
//
//namespace MathModels {
////////////////////////////////////////////////////////////////////////
//// Base implementation of estimates collection
//
//	class ECALC_API CMathParamBaseEstimates : public param_vector< CMathParamBaseEstimatePtr > {
//	public:
//		CMathParamBaseEstimates() {}
//		void resize( size_type N ) {
//			param_vector<CMathParamBaseEstimatePtr>::resize( N );
//			for ( iterator it = begin(); it != end(); ++it )
//				*it = CreateNewEstimate();
//		}
//		void Serialize( CMathSerializer& ar ) {
//			MathCalc::SerializeContainerEx<CMathParamBaseEstimatePtr>( ar, *this, CEstimatePtrSerializer(*this) );
//		}
//		virtual CMathParamBaseEstimatePtr CreateNewEstimate() const = 0;
//	private:
//		CMathParamBaseEstimates(const CMathParamBaseEstimates&); // not implemented
//		CMathParamBaseEstimates& operator=(const CMathParamBaseEstimates&); // not implemented
//		class CEstimatePtrSerializer {
//			const CMathParamBaseEstimates& mEstimates;
//		public:
//			CEstimatePtrSerializer( const CMathParamBaseEstimates& Estimates ): mEstimates(Estimates) {}
//			void operator() ( CMathSerializer& ar, CMathParamBaseEstimatePtr& pEstimate ) { 
//				if ( ar.IsLoading() )
//					pEstimate = mEstimates.CreateNewEstimate();
//				pEstimate->Serialize( ar );
//			}
//		};
//		friend class CEstimatePtrSerializer;
//	};
//
//	EC_EXPIMP_TEMPLATE template class ECALC_API std::auto_ptr<CMathParamBaseEstimate>;
//
////////////////////////////////////////////////////////////////////////
//// Collections of estimates
//
//	// Collections of different estimates should be the instantiations of this template
//	template< class Estimate >
//	class CParamEstimates : public CMathParamBaseEstimates 
//	{
//	public:
//		virtual CMathParamBaseEstimatePtr CreateNewEstimate() const { return boost::shared_ptr<CMathParamBaseEstimate>(new Estimate()); }
//	};
//
//	// Collection of simple estimates
//	typedef CParamEstimates<CMathParameterEstimate> CMathParameterEstimates;
//#pragma warning( disable : 4231 )
//	EC_EXPIMP_TEMPLATE template class ECALC_API CParamEstimates<CMathParameterEstimate>;
//#pragma warning( default : 4231 )
//} // MathModels
