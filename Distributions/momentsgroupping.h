#pragma once
#include <tchar.h>
#include <math.h>
#include "sampleholder.h"
#include "BoundedSample.h"

namespace Distributions
{
class CMomentsGroupping :public IGroupping
{
	int mMaxMoment;
	vector<double> mMoments;
public:
	CMomentsGroupping(int _maxMoment):mMaxMoment(_maxMoment),mMoments(mMaxMoment){}
	virtual ~CMomentsGroupping(void){}
	virtual /*override*/ CSampleHolder* Group(CSampleHolder* _sample)
	{
		CSampleHolder * sample=new CSampleHolder(mMaxMoment);
		double avg=0;
		for(int i=0;i<_sample->GetLength();++i)
		{
			avg+=_sample->GetAt(i);
		}
		avg/=_sample->GetLength();
		mMoments[0]=avg;
		sample->InsertValue(avg);
		for (int i=0;i<_sample->GetLength();++i)
		{
			double val=_sample->GetAt(i)-avg;
			for(int j=1;j<mMaxMoment;++j)
			{
				mMoments[j]+=(val*=val);
			}
		}
		for (int i=1;i<mMaxMoment;++i)
		{
			sample->InsertValue(mMoments[i]/sample->GetLength());
		}
		return sample;
	}
};

class CDeviationGroupping :public IGroupping
{
	int mMaxMoment;
	vector<double> mMoments;
	double mPreciseValue;
public:
	CDeviationGroupping(double _preciseValue,int _maxMoment):mMaxMoment(_maxMoment),mMoments(mMaxMoment),mPreciseValue(_preciseValue){}
	virtual ~CDeviationGroupping(void){}
	virtual /*override*/ CSampleHolder* Group(CSampleHolder* _sample)
	{
		CSampleHolder * sample=new CSampleHolder(mMaxMoment);
		double avg=0;
		for(int i=0;i<_sample->GetLength();++i)
		{
			avg+=abs(_sample->GetAt(i)-mPreciseValue);
		}
		avg/=_sample->GetLength();
		mMoments[0]=avg;
		sample->InsertValue(avg);
		for (int i=0;i<_sample->GetLength();++i)
		{
			double val=_sample->GetAt(i)-mPreciseValue;
			for(int j=1;j<mMaxMoment;++j)
			{
				mMoments[j]+=(val*=val);
			}
		}
		for (int i=1;i<mMaxMoment;++i)
		{
			sample->InsertValue(mMoments[i]/sample->GetLength());
		}
		return sample;
	}
};

class CBoundedMomentsGroupping :public IGroupping
{
	int mMaxMoment;
public:
	CBoundedMomentsGroupping(int _maxMoment):mMaxMoment(_maxMoment){}
	virtual ~CBoundedMomentsGroupping(void){}
	virtual /*override*/ CSampleHolder* Group(const CSampleHolder* _sample)const
	{
		vector<double> moments(mMaxMoment);
		const CBoundedSample * sourceSample;
		try
		{
			sourceSample=dynamic_cast<const CBoundedSample*>(_sample);
		}
		catch (std::exception /*&ex*/)
		{
			string dumpStr=_T("BoundedHistogram needs CBoundedSample class cast.");
			string filterStr=_T("Exception");
			Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
			throw;
		}
		CBoundedSample * sample=new CBoundedSample(mMaxMoment);
		double avg=0;
		for(int i=0;i<sourceSample->GetValidLength();++i)
		{
			avg+=sourceSample->GetValidAt(i);
		}
		avg/=sourceSample->GetValidLength();
		moments[0]=avg;
		sample->InsertValue(avg);
		for (int i=0;i<sourceSample->GetValidLength();++i)
		{
			double val=sourceSample->GetValidAt(i)-avg;
			for(int j=1;j<mMaxMoment;++j)
			{
				moments[j]+=(val*=val);
			}
		}
		for (int i=1;i<mMaxMoment;++i)
		{
			sample->InsertValue(moments[i]/sourceSample->GetValidLength());
		}
		return sample;
	}
};

class CBoundedDeviationGroupping :public IGroupping
{
	int mMaxMoment;
	double mPreciseValue;
public:
	CBoundedDeviationGroupping(double _preciseValue,int _maxMoment):mMaxMoment(_maxMoment>0?_maxMoment:1),mPreciseValue(_preciseValue){}
	virtual ~CBoundedDeviationGroupping(void){}
	//1: 1/n*sum|x-xPrecise|
	//2: sqrt(1/n*sum|x-xPrecise|^2)
	//3: sqrt3(1/n*sum|x-xPrecise|^3)
	virtual /*override*/ CSampleHolder* Group(const CSampleHolder* _sample)const
	{
		vector<double> moments(mMaxMoment,0.);
		const CBoundedSample * sourceSample;
		try
		{
			sourceSample=dynamic_cast<const CBoundedSample*>(_sample);
		}
		catch (std::exception /*&ex*/)
		{
			string dumpStr=_T("BoundedHistogram needs CBoundedSample class cast.");
			string filterStr=_T("Exception");
			Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
			throw;
		}
		CBoundedSample * sample = new CBoundedSample(mMaxMoment);
		for (int i=0; i<sourceSample->GetValidLength(); ++i)
		{
			double val = abs(sourceSample->GetValidAt(i)-mPreciseValue);
			for(int j=0; j<mMaxMoment; ++j)
			{
				moments[j] += val;
				val *= val;
			}
		}
		for (int i=0; i<mMaxMoment; ++i)
		{
			sample->InsertValue(pow(moments[i]/sourceSample->GetValidLength(), 1./(i+1)));
		}
		return sample;
	}
};
}//namespace Distributions