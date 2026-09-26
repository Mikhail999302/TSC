//#include "MathParameterEstimate.h"
#include "ExportTricks.h"

namespace MathModels {
	//////////////////////////////////////////////////////////////////////
	// Base implementation of estimates collection

	class ECALC_API CMathParamBaseEstimates : public param_vector< CMathParamBaseEstimatePtr > 
	{
	public:
		CMathParamBaseEstimates() {}
		void resize( size_type N ) {
			param_vector<CMathParamBaseEstimatePtr>::resize( N );
			for ( iterator it = begin(); it != end(); ++it )
				*it = CreateNewEstimate();
		}
		void Serialize( CMathSerializer& ar ) 
		{
			MathCalc::SerializeContainerEx<CMathParamBaseEstimatePtr>( ar, 
										*this, 
										CEstimatePtrSerializer(*this) );
		}
		virtual CMathParamBaseEstimatePtr CreateNewEstimate() const = 0;
	private:
		CMathParamBaseEstimates(const CMathParamBaseEstimates&); // not implemented
		CMathParamBaseEstimates& operator=(const CMathParamBaseEstimates&); // not implemented
		class CEstimatePtrSerializer {
			const CMathParamBaseEstimates& mEstimates;
		public:
			CEstimatePtrSerializer( const CMathParamBaseEstimates& Estimates ): mEstimates(Estimates) {}
			void operator() ( CMathSerializer& ar, CMathParamBaseEstimatePtr& pEstimate ) { 
				if ( ar.IsLoading() )
					pEstimate = mEstimates.CreateNewEstimate();
				pEstimate->Serialize( ar );
			}
		};
		friend class CEstimatePtrSerializer;
	};

#ifdef USE_PRAGMA
#pragma warning( disable : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA
	EC_EXPIMP_TEMPLATE template class ECALC_API std::auto_ptr<CMathParamBaseEstimate>;
#ifdef USE_PRAGMA
#pragma warning( default : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA


	//////////////////////////////////////////////////////////////////////
	// Collections of estimates

	// Collections of different estimates should be the instantiations of this template
	template< class Estimate >
	class CParamEstimates : public CMathParamBaseEstimates 
	{
	public:
		virtual CMathParamBaseEstimatePtr CreateNewEstimate() const { return boost::shared_ptr<CMathParamBaseEstimate>(new Estimate()); }
	};

	// Collection of simple estimates
	typedef CParamEstimates<CMathParameterEstimate> CMathParameterEstimates;
#ifdef USE_PRAGMA
#pragma warning( disable : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA
	EC_EXPIMP_TEMPLATE template class ECALC_API CParamEstimates<CMathParameterEstimate>;
#ifdef USE_PRAGMA
#pragma warning( default : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA

} // MathModels
