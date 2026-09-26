#pragma once
#include "distributions.h"
#include "SampleHolder.h"

namespace Distributions
{
	class CSampleHolder;

class DLLDIST IGroupping
{
public:
	virtual ~IGroupping(){}
	virtual CSampleHolder* Group(const CSampleHolder*)const=0;
};

class DLLDIST CHistogram: public IGroupping
{
	int mNumberOfGroup;
public:
	CHistogram(int _numberOfGroup=20):mNumberOfGroup(_numberOfGroup){}
	CSampleHolder* Group(const CSampleHolder*_sample)const;
};

class DLLDIST CBoundedHistogram: public IGroupping
{
	int mNumberOfGroup;
	double mLeft;
	double mRight;
	bool mIsBoundsSet;
public:
	CBoundedHistogram(double _left, double _right, int _numberOfGroup = 20):
	  mNumberOfGroup(_numberOfGroup), mLeft(_left), mRight(_right), mIsBoundsSet(true){}
	  CBoundedHistogram(int _numberOfGroup=20):
	  mNumberOfGroup(_numberOfGroup),mIsBoundsSet(false){}
	  //return CBoundedSample. takes CBoundedSample.
	  CSampleHolder* Group(const CSampleHolder*_sample)const;
};
}//namespace Distributions