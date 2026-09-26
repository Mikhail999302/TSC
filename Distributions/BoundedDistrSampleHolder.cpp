#include "BoundedDistrSampleHolder.h"
#include <Windows.h>

double Distributions::CBoundedDistrSampleHolder::GetEmpiricProbability( double x )
{
	if(!mIsSorted)
	{
		sort(mValues.begin(),mValues.end());
		mIsSorted=true;
	}
	int i,j;
	i=0;
	j=(int)mValues.size()-1;
	while(i<j)
	{
		if(mValues[(i+j)/2]>x)
		{
			j=(i+j)/2;
		}
		else
		{
			i=(i+j)/2+1;
		}
	}
	if(mValues[i]>x)
		return max((i-1)/mValues.size(),0);
	else return i/mValues.size();
}