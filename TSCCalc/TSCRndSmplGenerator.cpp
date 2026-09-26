#include "stdafx.h"
#include "TSCRndSmplGenerator.h"


GroupedTSC::CTSCSampleGenerator::CTSCSampleGenerator(const double *_p,int _length,double _pch,double _padv,int _sampleSize, uint seed_/*=0*/):
		mSeed(seed_),mSampleSize(_sampleSize),mP(_length+2)
{
	mLength=_length+2;
	for(int i=0;i<_length;++i)
	{
		mP[i]=_p[i];
	}
	mP[_length]=_pch;
	mP[_length+1]=_padv;
	mGen.reset(new TSCGenerator::CGenerator(mP,seed_));
}


GroupedTSC::CTSCSampleGenerator::CTSCSampleGenerator(const GroupedTSC::CTSCSampleGenerator &_rhs):
		mSeed(_rhs.mSeed),mSampleSize(_rhs.mSampleSize), mP(_rhs.mP)
{
	mGen.reset(new TSCGenerator::CGenerator(mP,mSeed));
}

GroupedTSC::CTSCSampleGenerator& GroupedTSC::CTSCSampleGenerator::operator=(const CTSCSampleGenerator& _rhs)
{
	if(this!=&_rhs)
	{
		mSeed=_rhs.mSeed;
		mSampleSize=_rhs.mSampleSize;
		if(mLength!=_rhs.mLength)
		{			
			mP=_rhs.mP;
			mLength=_rhs.mLength;
		}
		mGen.reset(new TSCGenerator::CGenerator(mP,mSeed));
	}
	return *this;
}

Tsc::TableManager::CTableManager<double>* GroupedTSC::CTSCSampleGenerator::CreateTableManager()
{
	mGen->Generate(mSampleSize);
	double *values=new double[mGen->GetSample().size()];
	std::copy(mGen->GetSample().begin(),mGen->GetSample().end(),values);
	Tsc::TableManager::CTableManager<double>* rez=new Tsc::TableManager::CTableManager<double>(values,mLength-1,mLength-1);
	delete[]values;
	return rez;
}

GroupedTSC::CTSCSampleGenerator::~CTSCSampleGenerator(void)
{	
}
