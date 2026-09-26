#pragma once
#include "boundedsample.h"

namespace Distributions
{
	//Empiric distribution
	class CBoundedDistrSampleHolder :public Distributions::CBoundedSample
	{
		bool mIsSorted;
	public:
		CBoundedDistrSampleHolder(void):mIsSorted(false){}
		virtual ~CBoundedDistrSampleHolder(void);
		virtual double GetEmpiricProbability(double x);
		virtual /*overridden*/ int InsertValue(double _value)
		{
			CBoundedSample::InsertValue(_value);
			mIsSorted=false;
		}
	};
}//namespace Distributions