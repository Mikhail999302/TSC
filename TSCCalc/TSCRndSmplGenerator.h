//Creates TableManager with matrix of set parameters.
#pragma once
#include "stdafx.h"
#include "Typedefs.h"
#include <C:\diploma\2012\TSC\Libs/TableManager\Entry.h>
#include "..\TSCGenerator\Generator.h"

namespace GroupedTSC
{

class CTSCSampleGenerator:
	public Tsc::TableManager::IDataLoader<double>
{
	doubles mP;
	int mLength;
	auto_ptr<TSCGenerator::CGenerator> mGen;
	uint mSeed;
	int mSampleSize;
public:
	CTSCSampleGenerator(const double * _p, int _length, double _pch, double _padv, int _sampleSize, uint seed_=0);
	CTSCSampleGenerator(const CTSCSampleGenerator& _rhs);
	CTSCSampleGenerator& operator=(const CTSCSampleGenerator& _rhs);
	virtual ~CTSCSampleGenerator();
	virtual Tsc::TableManager::CTableManager<double>* CreateTableManager();
	virtual /*implement*/ IDataLoader<double>* Clone()
	{
		return new CTSCSampleGenerator(*this);
	}
};
}//namespace GroupedTSC